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

class UserInterfaceSprite : public UserInterface {
	bool fullscreen;
	UserInterfaceSpriteResources resources;
	UserInterfaceSpriteRenderer* renderer;
	GameStage::Type laststage;

	~UserInterfaceSprite() {
		int dum1; Uint16 dum2; int dum3;
		// Free resources (can has C++0x type inference plz?)
		TTF_CloseFont(resources.font_title);
		TTF_CloseFont(resources.font_large);
		TTF_CloseFont(resources.font_small);
		if(resources.music_theme) {
			Mix_FreeMusic(resources.music_theme);
			resources.music_theme = NULL;
		}
		for(UserInterfaceSpriteResources::samples_type::iterator i =
			resources.samples.begin();
			i != resources.samples.end(); i++) {
				Mix_FreeChunk(i->second); }
		resources.samples.clear();
		for(UserInterfaceSpriteResources::textures_type::iterator i =
			resources.textures.begin();
			i != resources.textures.end(); i++) {
				SDL_FreeSurface(i->second); }
		resources.textures.clear();
		// Shutdown TTF and Mixer
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

	std::string removeSuffix(const char* name) {
		std::string key(name);
		size_t dot = key.find_last_of('.');
		return key.substr(0, dot);
	}

	bool loadSample(const char* name) {
		std::string file(getDataDir());
		file += name;
		Mix_Chunk* sample = Mix_LoadWAV(file.c_str());
		if(sample) {
			std::string key = removeSuffix(name);
			resources.samples[key.c_str()] = sample;
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
			// Unnecessary: looks like SDL-image sets alpha if 32-bit source.
			//SDL_SetAlpha(texture, SDL_SRCALPHA|SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
			std::string key = removeSuffix(name);
			resources.textures[key.c_str()] = texture;
			return true;
		} else {
			warn("Unable to load texture: %s", IMG_GetError());
			return false;
		}
	}

	bool setupVideo() {
		if(SDL_SetVideoMode(640, 480, 0,
#ifdef __APPLE__ /* Without this, blitting in toggleFullscreen fails */
			SDL_HWSURFACE |
#endif
#ifdef DOUBLE_BUFFER /* Performance hit on Felix/Win98; 95 FPS -> 30. */
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
		// Load sprite textures TODO
		loadTexture("pointer1.png");
		// Load audio samples TODO
		// Load music (failure is nonfatal)
		if(!(resources.music_theme = Mix_LoadMUS(findThemeMusicFile())))
			{ warn("Unable to load music: %s", Mix_GetError()); }
		resources.music_theme_bpm = 120; // Correct for Mule-Funk-Shun
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
		SDL_UpdateRect(screen, 0, 0, 0, 0);
	}

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks) {

		bool allowtransition = renderer ?
			renderer->render(stage, setup, game, ticks, resources)
			: true;
		
		if(allowtransition && (stage != laststage)) {
			std::string rc("UserInterfaceSprite");
			if(renderer) { delete renderer; renderer = NULL; }
			switch(stage) {
				case GameStage::TITLE: rc += "Title"; break;
				// TODO all others
				case GameStage::COLOUR: rc += ""; break;
				case GameStage::SPECIES: rc += ""; break;
				case GameStage::SCOREBOARD: rc += ""; break;
				case GameStage::LANDGRAB: rc += ""; break;
				case GameStage::LANDAUCTION: rc += ""; break;
				case GameStage::PREAUCTION: rc += ""; break;
				case GameStage::AUCTIONDECLARE: rc += ""; break;
				case GameStage::AUCTION: rc += ""; break;
				case GameStage::PREDEVELOP: rc += ""; break;
				case GameStage::DEVELOPHUMAN: rc += ""; break;
				case GameStage::WAMPUS: rc += ""; break;
				case GameStage::DEVELOPCOMP: rc += ""; break;
				case GameStage::POSTDEVELOP: rc += ""; break;
				case GameStage::PREPRODUCT: rc += ""; break;
				case GameStage::PRODUCT: rc += ""; break;
				case GameStage::POSTPRODUCT: rc += ""; break;
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
			snprintf(str, 7, "%3d", sumfps / fps_history.size());
			str[7] = '\0';
			SDL_Surface* sur = resources.renderText(
				resources.font_small, str, white);
			SDL_Rect rect = {0, 0, sur->w, sur->h};
			SDL_FillRect(screen, &rect,
				SDL_MapRGB(screen->format, 0, 0, 0));
			SDL_BlitSurface(sur, NULL, screen, NULL);
			SDL_UpdateRect(screen, 0, 0, sur->w, sur->h);
			SDL_FreeSurface(sur);
		}
#endif
#ifdef DOUBLE_BUFFER
		SDL_Flip(SDL_GetVideoSurface());
#endif

		return allowtransition;
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterface,UserInterfaceSprite)

SDL_Surface* UserInterfaceSpriteResources::renderText(TTF_Font* font,
	const char* text, SDL_Color colour) {

	SDL_Surface* s;
	s = TTF_RenderUTF8_Blended(font, text, colour);
	if(!s) { warn("TTF error: %s", TTF_GetError()); }
	return s;
}

bool UserInterfaceSpriteResources::displayTextLine(TTF_Font* font,
	const char* text, SDL_Color foreground, SDL_Color background, int y) {

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
	SDL_UpdateRect(screen, 0, y, 640, bar.h);
	return true;
}

void UserInterfaceSpriteResources::displaySprites(
	const std::vector<UserInterfaceSpriteSprite*>& sprites) {
	
	SDL_Surface* screen = SDL_GetVideoSurface();
	for_each(sprites.begin(), sprites.end(),
		std::bind2nd(std::mem_fun(&UserInterfaceSpriteSprite::save), screen));
	for_each(sprites.begin(), sprites.end(),
		std::bind2nd(std::mem_fun(&UserInterfaceSpriteSprite::draw), screen));
}

void UserInterfaceSpriteResources::eraseSprites(
	const std::vector<UserInterfaceSpriteSprite*>& sprites) {
	
	SDL_Surface* screen = SDL_GetVideoSurface();
	for_each(sprites.begin(), sprites.end(),
		std::bind2nd(std::mem_fun(&UserInterfaceSpriteSprite::restore),
			screen));
}

UserInterfaceSpriteSprite::UserInterfaceSpriteSprite(SDL_Surface* pixmap)
	: pixmap(pixmap), saved(false) {
	
	assert(pixmap);
	SDL_PixelFormat* format = pixmap->format;
	background = SDL_CreateRGBSurface(SDL_HWSURFACE, pixmap->w, pixmap->h,
		format->BitsPerPixel,
		format->Rmask, format->Gmask, format->Bmask, format->Amask);
	// We do NOT want to blend restored background!
	SDL_SetAlpha(background, SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
	pos.x = 0; pos.w = pixmap->w; erasepos.w = pos.w;
	pos.y = 0; pos.h = pixmap->h; erasepos.h = pos.h;
}

UserInterfaceSpriteSprite::~UserInterfaceSpriteSprite() {
	SDL_FreeSurface(pixmap);     pixmap = 0;
	SDL_FreeSurface(background); background = 0;
}

void UserInterfaceSpriteSprite::move(Sint16 x, Sint16 y)
	{ pos.x = x; pos.y = y; }
	
void UserInterfaceSpriteSprite::save(SDL_Surface* screen) {
	SDL_BlitSurface(screen, &pos, background, 0);
	saved = true;
}

void UserInterfaceSpriteSprite::draw(SDL_Surface* screen) {
	// FIXME UpdateRect does NOT get clipped, so we should be doing it
	// ourselves here!
	SDL_BlitSurface(pixmap, 0, screen, &pos);
	SDL_UpdateRect(screen, pos.x, pos.y, pos.w, pos.h);
	// Also update where the old cursor image was.
	if((pos.x != erasepos.x) || (pos.y != erasepos.y)) {
		SDL_UpdateRect(screen, erasepos.x, erasepos.y, erasepos.w, erasepos.h);
	}
}

void UserInterfaceSpriteSprite::restore(SDL_Surface* screen) {
	if(!saved) { return; } // Avoid restoring garbage first frame
	SDL_BlitSurface(background, 0, screen, &pos);
	// DON'T UpdateRect, else we will give a full render()'s worth of flicker.
	// Instead, remember where we're going to update later.
	erasepos.x = pos.x;
	erasepos.y = pos.y;
}
