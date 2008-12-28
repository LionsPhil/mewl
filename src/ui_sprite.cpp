#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "ui.hpp"
#include "platform.hpp"

class UserInterfaceSprite : public UserInterface {
	bool fullscreen;
	TTF_Font* fonttitle;
	TTF_Font* fontlarge;
	TTF_Font* fontsmall;

	~UserInterfaceSprite() {
		int dum1; Uint16 dum2; int dum3;
		if(TTF_WasInit()) { TTF_Quit(); }
		if(Mix_QuerySpec(&dum1, &dum2, &dum3)) { Mix_CloseAudio(); }
	}

private:
	const char* findFontFile() { return "data/mainfont.ttf"; }

	SDL_Surface* renderText(TTF_Font* font, const char* text,
		SDL_Color colour) {

		SDL_Surface* s;
		//s = TTF_RenderUTF8_Solid(font, text, colour);
		s = TTF_RenderUTF8_Blended(font, text, colour);
		if(!s) { warn("TTF error: %s", TTF_GetError()); }
		return s;
	}

	bool setupVideo() {
		if(SDL_SetVideoMode(640, 480, 0,
			SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0))) {
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
		fonttitle = TTF_OpenFont(fontfile, 64);
		fontlarge = TTF_OpenFont(fontfile, 24);
		fontsmall = TTF_OpenFont(fontfile, 16);
		if(!fonttitle || !fontlarge || !fontsmall) {
			warn("Unable to load font: %s", TTF_GetError());
			return false;
		}
		// Load sprites (failure is nonfatal) TODO
		// Load audio (failure is nonfatal) TODO
		// Done
		return true;
	}

	void toggleFullscreen() { fullscreen = !fullscreen; setupVideo(); }
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterface,UserInterfaceSprite)

