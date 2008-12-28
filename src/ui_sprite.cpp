#include <SDL.h>
#include "ui.hpp"
#include "platform.hpp"

class UserInterfaceSprite : public UserInterface {
	~UserInterfaceSprite() {}

	bool init(bool fullscreen) {
		// Initialise the SDL subsystems
		if(SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
			warn("Unable to initialise SDL: %s", SDL_GetError());
			return false;
		}
		// Set the video mode
		if(SDL_SetVideoMode(640, 480, 0,
			SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0))) {
			trace("Got video at %dbpp",
				SDL_GetVideoSurface()->format->BitsPerPixel);
		} else {
			warn("Unable to set video mode: %s", SDL_GetError());
			return false;
		}
		return true;
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterface,UserInterfaceSprite)

