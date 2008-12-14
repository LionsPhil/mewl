#include <stdlib.h>
#include <SDL.h>
#include "game.hpp"
#include "platform.hpp"

int main(int argc, char** argv) {
	bool run;
	ControlManager controlman;
	SDL_Event event;

	trace("Startup");
	if(SDL_Init(0) < 0)
		{ warn("Unable to initialise SDL: %s", SDL_GetError()); die(); }

	// Poke UI code to init VIDEO and AUDIO around here

	if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) // Nonfatal
		{ warn("Joystick initialisation failed: %s", SDL_GetError()); }

	controlman.populate();

	trace("Running");
	run = true;
	while(run) {
		/* Process events */
		while(SDL_PollEvent(&event)) { switch(event.type) {
			/* case SDL_ACTIVEEVENT:
				if(event.active.state == SDL_APPACTIVE)
					{ freeze(!event.active.gain); }
				break; */
			case SDL_QUIT:
				run = false; break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_JOYAXISMOTION:
			case SDL_JOYBALLMOTION:
			case SDL_JOYHATMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				controlman.feedEvent(event);
		}}

		/* Poke game logic to tick */
		/* Poke UI to render gamestate */
	}

	trace("Clean exit");
	SDL_Quit();
	return EXIT_SUCCESS;
}

