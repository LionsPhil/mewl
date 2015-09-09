#include <string>
#include <algorithm>
#include <functional>
#ifdef FPS_COUNTER
# include <deque>
# include <stdio.h>
#endif
#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "factory.hpp"
#include "platform.hpp"
#include "ui.hpp"
#include "ui_sprite.hpp"
#include "ui_sprite_pointer.hpp"

/* Yay for API changes on patchlevel versions! */
#if SDL_IMAGE_MAJOR_VERSION >= 1
# if (SDL_IMAGE_MINOR_VERSION >  2) || \
     ((SDL_IMAGE_MINOR_VERSION == 2) && (SDL_IMAGE_PATCHLEVEL >= 8))
#  define IMG_NEEDS_INIT
# endif
#endif
#if SDL_IMAGE_MAJOR_VERSION == 1 && SDL_IMAGE_MINOR_VERSION == 2 && \
	SDL_IMAGE_PATCHLEVEL == 9
# error "sdl-image 1.2.9 is known-broken and will fail with no error message"
#endif

class UserInterfaceSprite : public UserInterface {
	bool fullscreen;
	UserInterfaceSpriteResources resources;
	UserInterfaceSpriteRenderer* renderer;
	GameStage::Type laststage;

	~UserInterfaceSprite() {
		int dum1; Uint16 dum2; int dum3;
		std::vector<char*> keys;
		// Free resources (can has C++0x type inference plz?)
		TTF_CloseFont(resources.font_title);
		TTF_CloseFont(resources.font_large);
		TTF_CloseFont(resources.font_small);
		if(resources.music_theme) {
			Mix_FreeMusic(resources.music_theme);
			resources.music_theme = NULL;
		}
		for(int p = 0; p < PLAYERS; p++)
			{ delete resources.playerpointers[p]; }
		/* See removeSuffix for why we're freeing the keys too. We have
		 * to collect them up for later free(), else we'll pull them out
		 * from under the hash iterator's operator++ (yay valgrind). */
		for(UserInterfaceSpriteResources::samples_type::iterator i =
			resources.samples.begin();
			i != resources.samples.end(); i++) {
				keys.push_back(const_cast<char*>(i->first));
				Mix_FreeChunk(i->second); }
		resources.samples.clear();
		for(UserInterfaceSpriteResources::textures_type::iterator i =
			resources.textures.begin();
			i != resources.textures.end(); i++) {
				keys.push_back(const_cast<char*>(i->first));
				SDL_FreeSurface(i->second); }
		resources.textures.clear();
		for_each(keys.begin(), keys.end(), free_functor());
		// Zap the renderer
		if(renderer) { delete renderer; renderer = 0; }
		// Shutdown Image, TTF and Mixer
#ifdef IMG_NEEDS_INIT
		IMG_Quit();
#endif
		if(TTF_WasInit()) { TTF_Quit(); }
		if(Mix_QuerySpec(&dum1, &dum2, &dum3)) { Mix_CloseAudio(); }
		// Restore the mouse cursor
		SDL_ShowCursor(SDL_ENABLE);
	}

private:
#ifdef FPS_COUNTER
	std::deque<int> fps_history;
#endif
	const char* getDataDir() { return "data/"; }
	const char* findFontFile() { return "data/mainfont.ttf"; }
	const char* findThemeMusicFile() { return "data/theme.mp3"; }

	/* Why do we strdup here?
	 * The hash can't copy the string storage (it only sees a pointer type),
	 * and while you can convince std::strings to work, we'd then have to
	 * keep constructing them ad-hoc for lookups, which is just silly
	 * overhead. So we give the hash a C-string of its own (it's that or
	 * manual string-constant keys). */
	char* removeSuffix(const char* name) {
		char* keycpy;
		std::string key(name);
		size_t dot = key.find_last_of('.');
		keycpy = strdup(key.substr(0, dot).c_str());
		if(!keycpy) { warn("Out of memory"); die(); }
		return keycpy;
	}

	bool loadSample(const char* name) {
		std::string file(getDataDir());
		file += name;
		Mix_Chunk* sample = Mix_LoadWAV(file.c_str());
		if(sample) {
			resources.samples[removeSuffix(name)] = sample;
			return true;
		} else {
			warn("Unable to load sample: %s", Mix_GetError());
			return false;
		}
	}

	bool loadTexture(const char* name) {
		std::string file(getDataDir());
		file += name;
		SDL_Surface* texture = IMG_Load(file.c_str());
		if(texture) {
			resources.textures[removeSuffix(name)] = texture;
			return true;
		} else {
			warn("Unable to load texture %s: %s", file.c_str(),
				IMG_GetError());
			return false;
		}
	}

	bool setupVideo() {
		if(SDL_SetVideoMode(640, 480, 0,
#ifdef __APPLE__ /* Without this, blitting in toggleFullscreen fails */
			SDL_HWSURFACE |
#endif
#ifdef DOUBLE_BUFFER /* Performance hit on Felix/Win98; 95 FPS -> 40. */
			SDL_HWSURFACE | SDL_DOUBLEBUF |
#endif
			SDL_HWPALETTE | (fullscreen ? SDL_FULLSCREEN : 0))) {
			trace("Got %s video at %dbpp",
				(fullscreen ? "fullscreen" : "windowed"),
				SDL_GetVideoSurface()->format->BitsPerPixel);
			return true;
		} else {
			warn("Unable to set video mode: %s", SDL_GetError());
			return false;
		}
	}

public:
	bool init(bool fullscreen) {
		// Initialise the SDL subsystems
		if(SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
			warn("Unable to initialise SDL: %s", SDL_GetError());
			return false;
		}
		// Set up the window (icon must be done before video mode)
		SDL_WM_SetCaption("M.E.W.L.", "M.E.W.L.");
		// SDL_WM_SetIcon(...32x32 surface..., NULL); // TODO
		// Set the video mode
		this->fullscreen = fullscreen;
		if(!setupVideo()) { return false; }
		// Lose the mouse cursor
		SDL_ShowCursor(SDL_DISABLE);
		// Initialise mixer
		if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
			warn("Unable to initialise Mixer: %s", Mix_GetError());
			return false;
		}
		// Initialise TTF
		if(TTF_Init() < 0) {
			warn("Unable to initialise TTF: %s", TTF_GetError());
			return false;
		}
		// Initialise Image
#ifdef IMG_NEEDS_INIT
		if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
			warn("Unable to initialise Image: %s", IMG_GetError());
			return false;
		}
#endif
		// Load font
		const char* fontfile = findFontFile();
		resources.font_title = TTF_OpenFont(fontfile, 64);
		resources.font_large = TTF_OpenFont(fontfile, 24);
		resources.font_small = TTF_OpenFont(fontfile, 16);
		if(!resources.font_title
		|| !resources.font_large
		|| !resources.font_small) {
			warn("Unable to load font: %s", TTF_GetError());
			return false;
		}
		// Load sprite textures
		if(!loadTexture("pointer-centre.png")
		|| !loadTexture("pointer-north.png")
		|| !loadTexture("pointer-northeast.png")
			) { return false; }
		// Load audio samples TODO
		// Load music (failure is nonfatal)
		if(!(resources.music_theme = Mix_LoadMUS(findThemeMusicFile())))
			{ warn("Unable to load music: %s", Mix_GetError()); }
		resources.music_theme_bpm = 120; // Correct for Mule-Funk-Shun
		// Generate player pointers
		for(int p = 0; p < PLAYERS; p++) {
			resources.playerpointers[p] = new
				UserInterfaceSpritePointer(resources,
				UserInterfaceSpriteConstants::col_player[p],
				resources.textures["pointer-centre"],
				resources.textures["pointer-north"],
				resources.textures["pointer-northeast"]);
		}
		// Other initialisation
		renderer = NULL;
		laststage = GameStage::SCOREBOARD; // carefully crafted lie
		// Done
		return true;
	}

	void toggleFullscreen() {
		// Preserve the framebuffer, else we may lose e.g. background
		SDL_Surface* screen = SDL_GetVideoSurface();
		SDL_Surface* liferaft = SDL_CreateRGBSurface(SDL_HWSURFACE,
			screen->w, screen->h, screen->format->BitsPerPixel,
			screen->format->Rmask, screen->format->Gmask,
			screen->format->Bmask, screen->format->Amask);
		SDL_BlitSurface(screen, NULL, liferaft, NULL);
		// Make the change
		fullscreen = !fullscreen;
		setupVideo();
		// Restore the framebuffer
		SDL_BlitSurface(liferaft, NULL, screen, NULL);
		SDL_FreeSurface(liferaft);
		SDL_Flip(screen); // or SDL_UpdateRect(screen, 0, 0, 0, 0);
	}

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		GameStageState& state, uint32_t ticks) {

		bool allowtransition = renderer ?
			renderer->render(stage, setup, game, state, ticks,
				resources)
			: true;
		
		if(allowtransition && (stage != laststage)) {
			std::string rc("UserInterfaceSprite");
			if(renderer) { delete renderer; renderer = NULL; }
			switch(stage) {
				case GameStage::TITLE: rc += "Title"; break;
				case GameStage::COLOUR: rc += "Colour"; break;
				case GameStage::SPECIES: rc += "Species"; break;
				case GameStage::SCOREBOARD:
					rc += "Scoreboard"; break;
				case GameStage::LANDGRAB:
					rc += "LandGrab"; break;
				case GameStage::LANDAUCTION:
					rc += "LandAuction"; break;
				case GameStage::PREAUCTION:
					rc += "PreAuction"; break;
				case GameStage::AUCTIONDECLARE:
					rc += "AuctionDeclare"; break;
				case GameStage::AUCTION:
					rc += "Auction"; break;
				case GameStage::PREDEVELOP:
					rc += "PreDevelop"; break;
				case GameStage::DEVELOPHUMAN:
					rc += "DevelopHuman"; break;
				case GameStage::WAMPUS:
					rc += "Wampus"; break;
				case GameStage::DEVELOPCOMP:
					rc += "DevelopComp"; break;
				case GameStage::POSTDEVELOP:
					rc += "PostDevelop"; break;
				case GameStage::PREPRODUCT:
					rc += "PreProduct"; break;
				case GameStage::PRODUCT:
					rc += "Product"; break;
				case GameStage::POSTPRODUCT:
					rc += "PostProduct"; break;
			}
			renderer = FACTORY_FOR(UserInterfaceSpriteRenderer)
				.create(rc.c_str());
			if(!renderer) {
				warn("Miscompiled: class '%s' missing from UI.",
					rc.c_str());
				die();
			}
			renderer->init(stage, setup, game, ticks, resources);
			laststage = stage;
		}

#ifdef FPS_COUNTER
		{ // FPS counter for diagnostics
			char str[8];
			SDL_Color white = {255, 255, 255, 0};
			SDL_Surface* screen = SDL_GetVideoSurface();
			fps_history.push_back(100 / ticks);
			if(fps_history.size() > 32) { fps_history.pop_front(); }
			int sumfps = 0;
			for(std::deque<int>::iterator i = fps_history.begin();
				i < fps_history.end(); i++) { sumfps += *i; }
			snprintf(str,7, "%3d",(int)(sumfps/fps_history.size()));
			str[7] = '\0';
			SDL_Surface* sur = resources.renderText(
				resources.font_small, str, white);
			SDL_Rect rect = {0, 0,
				static_cast<Uint16>(sur->w), static_cast<Uint16>(sur->h)};
			SDL_FillRect(screen, &rect,
				SDL_MapRGB(screen->format, 0, 0, 0));
			SDL_BlitSurface(sur, NULL, screen, NULL);
			resources.updateRect(0, 0, sur->w, sur->h);
			SDL_FreeSurface(sur);
		}
#endif
#ifdef DOUBLE_BUFFER
		SDL_Flip(SDL_GetVideoSurface());
#else
		/* Theoretically, the documentation suggests attempting to avoid
		 * overdraw here. Realistically, there's not much we can do
		 * about that without subdivision, and apparently SDL ships out
		 * all the rectangles in one go to the graphics driver, so we'll
		 * let that do it if it feels so inclined. (We _could_ remove
		 * trivially subsumed rectangles, or replace them all with a
		 * tight bounding rectangle, but neither fits our usage pattern
		 * well.) */
		SDL_UpdateRects(SDL_GetVideoSurface(),
			resources.dirtyrects.size(),
			&resources.dirtyrects[0]); /* docs imply this is safe */
		resources.dirtyrects.clear();
#endif

		return allowtransition;
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterface,UserInterfaceSprite)

void UserInterfaceSpriteResources::updateRect(
	Sint16 x, Sint16 y, Uint16 w, Uint16 h) {
#ifndef DOUBLE_BUFFER /* Don't track what we don't use (or clear!) */
	SDL_Surface* screen = SDL_GetVideoSurface();
	Uint16 sw = screen->w - 1;
	Uint16 sh = screen->h - 1;
	SDL_Rect rect;
	 // Offscreen sprites may clip away to nothing
	if(x < 0) { w += x; x = 0; if(w <= 0) { return; }}
	rect.x = x > sw ? sw : x; sw -= rect.x;
	if(sw == 0) { return; }
	if(y < 0) { h += y; y = 0; if(h <= 0) { return; }}
	rect.y = y > sh ? sh : y; sh -= rect.y;
	if(sh == 0) { return; }
	rect.w = w > sw ? sw : w;
	rect.h = h > sh ? sh : h;
	dirtyrects.push_back(rect);
#endif
}

SDL_Surface* UserInterfaceSpriteResources::renderText(TTF_Font* font,
	const char* text, SDL_Color colour) {

	SDL_Surface* s;
	s = TTF_RenderUTF8_Blended(font, text, colour);
	if(!s) { warn("TTF error: %s", TTF_GetError()); }
	return s;
}

bool UserInterfaceSpriteResources::displayTextLine(TTF_Font* font,
	const char* text, SDL_Color foreground, SDL_Color background, Sint16 y) {

	SDL_Surface* screen = SDL_GetVideoSurface();
	SDL_Rect bar = {0, y, 640, 0};
	SDL_Surface* textpix = renderText(font, text, foreground);
	if(!textpix) { return false; }
	bar.h = textpix->h;
	SDL_FillRect(screen, &bar, SDL_MapRGB(screen->format,
		background.r, background.g, background.b));
	bar.w = textpix->w;
	bar.x = (screen->w - bar.w) / 2;
	SDL_BlitSurface(textpix, NULL, screen, &bar);
	SDL_FreeSurface(textpix);
	updateRect(0, y, 640, bar.h);
	return true;
}

void UserInterfaceSpriteResources::displaySprites(
	const std::vector<UserInterfaceSpriteSprite*>& sprites) {
	
	SDL_Surface* screen = SDL_GetVideoSurface();
	for_each(sprites.begin(), sprites.end(), std::bind2nd(
		std::mem_fun(&UserInterfaceSpriteSprite::save), screen));
	for_each(sprites.begin(), sprites.end(), std::bind2nd(
		std::mem_fun(&UserInterfaceSpriteSprite::draw), screen));
}

void UserInterfaceSpriteResources::eraseSprites(
	const std::vector<UserInterfaceSpriteSprite*>& sprites) {
	
	SDL_Surface* screen = SDL_GetVideoSurface();
	for_each(sprites.begin(), sprites.end(),
		std::bind2nd(std::mem_fun(&UserInterfaceSpriteSprite::restore),
			screen));
}

UserInterfaceSpriteSprite::UserInterfaceSpriteSprite(
	UserInterfaceSpriteResources& resources, const SDL_Surface* pixmap)
	: resources(resources), saved(false), visible(true), pixmap(pixmap) {
	
	assert(pixmap);
	SDL_PixelFormat* format = pixmap->format;
	background = SDL_CreateRGBSurface(SDL_HWSURFACE, pixmap->w, pixmap->h,
		format->BitsPerPixel,
		format->Rmask, format->Gmask, format->Bmask, format->Amask);
	// We do NOT want to blend restored background, so turn alpha OFF!
	SDL_SetAlpha(background, SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
	pos.x = 0; pos.w = pixmap->w;
	pos.y = 0; pos.h = pixmap->h;
}

UserInterfaceSpriteSprite::~UserInterfaceSpriteSprite() {
	//SDL_FreeSurface(pixmap);     pixmap = 0;
	SDL_FreeSurface(background); background = 0;
}

void UserInterfaceSpriteSprite::move(Sint16 x, Sint16 y)
	{ pos.x = x; pos.y = y; }

void UserInterfaceSpriteSprite::showhide(bool visible) {
	saved = false; // Skip restore from old data after becoming visible
	this->visible = visible;
}
	
void UserInterfaceSpriteSprite::save(SDL_Surface* screen) {
	if(!visible) { return; }
	SDL_BlitSurface(screen, &pos, background, 0);
	saved = true;
}

void UserInterfaceSpriteSprite::draw(SDL_Surface* screen) {
	if(!visible) { return; }
	SDL_Rect clip = pos; // SDL_BlitSurface will trample
	SDL_BlitSurface(const_cast<SDL_Surface*>(pixmap), 0, screen, &clip);
	resources.updateRect(pos.x, pos.y, pos.w, pos.h);
}

void UserInterfaceSpriteSprite::restore(SDL_Surface* screen) {	
	if(!saved || !visible) { return; }// Avoid restoring garbage first frame
	SDL_Rect clip = pos;
	SDL_BlitSurface(background, 0, screen, &clip);
	resources.updateRect(pos.x, pos.y, pos.w, pos.h);
	/* This doesn't cause flicker, as the update is deferred until the same
	 * time as the draw() update thanks to updateRect()'s coalescing. */
}

const char* Difficulty::getName(Difficulty::Type self) {
	switch(self) {
		case Difficulty::BEGINNER:   return "Beginner";
		case Difficulty::STANDARD:   return "Standard";
		case Difficulty::TOURNAMENT: return "Tournament";
	}
	return 0; // shush, g++
}
