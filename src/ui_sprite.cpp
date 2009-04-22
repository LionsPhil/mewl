#include <string>
#ifdef FPS_COUNTER
# include <deque>
# include <stdio.h>
#endif
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
		for(hash_map<const char*, Mix_Chunk*>::iterator i =
			resources.samples.begin();
			i != resources.samples.end(); i++) {
				Mix_FreeChunk(i->second); }
		resources.samples.clear();
		for(hash_map<const char*, SDL_Surface*>::iterator i =
			resources.textures.begin();
			i != resources.textures.end(); i++) {
				SDL_FreeSurface(i->second); }
		resources.textures.clear();
		// Shutdown TTF and Mixer
		if(TTF_WasInit()) { TTF_Quit(); }
		if(Mix_QuerySpec(&dum1, &dum2, &dum3)) { Mix_CloseAudio(); }
	}

private:
#ifdef FPS_COUNTER
	std::deque<int> fps_history;
#endif
	const char* getDataDir() { return "data/"; }
	const char* findFontFile() { return "data/mainfont.ttf"; }
	const char* findThemeMusicFile() { return "data/theme.mp3"; }

	bool loadSample(const char* name) {
		std::string file(getDataDir());
		file += name;
		Mix_Chunk* sample = Mix_LoadWAV(file.c_str());
		if(sample) {
			resources.samples[name] = sample;
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
			resources.textures[name] = texture;
			return true;
		} else {
			warn("Unable to load texture: %s", IMG_GetError());
			return false;
		}
	}

	bool setupVideo() {
		if(SDL_SetVideoMode(640, 480, 0,
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
		// Load audio samples TODO
		// Load music (failure is nonfatal)
		if(!(resources.music_theme = Mix_LoadMUS(findThemeMusicFile())))
			{ warn("Unable to load music: %s", Mix_GetError()); }
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
			renderer->init(setup, game, ticks, resources);
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

