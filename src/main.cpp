#include <stdlib.h>
#include <SDL.h>
#include "game.hpp"
#include "platform.hpp"

static const Uint32 tickduration = 10; /* ms -> 100Hz */

int main(int argc, char** argv) {
	bool run;
	ControlManager controlman;
	SDL_Event event;
	Uint32 tickerror, ticklast;

	trace("Startup");
	if(SDL_Init(0) < 0)
		{ warn("Unable to initialise SDL: %s", SDL_GetError()); die(); }

	// Poke UI code to init VIDEO and AUDIO around here // TODO

	if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) {
		SDL_JoystickEventState(SDL_ENABLE); // May erase all events(!)
	} else { // Nonfatal
		warn("Joystick initialisation failed: %s", SDL_GetError());
	}

	controlman.populate();

	trace("Running");
	tickerror = 0;
	ticklast = SDL_GetTicks();
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

		/* Process the passage of time */
		if(1) { const Uint32 now = SDL_GetTicks();
		tickerror += (now - ticklast);
		ticklast = now; }
		if(tickerror >= tickduration) {
			do {
				tickerror -= tickduration;

				/* Poke game logic to tick */ // TODO
			} while(tickerror >= tickduration);

			/* Poke UI to render gamestate */ // TODO
		} else {
			/* Have a nap until we actually have at least one tick
			 * to run */
			SDL_Delay(tickduration);
		}
	}

	trace("Clean exit");
	SDL_Quit();
	return EXIT_SUCCESS;
}

