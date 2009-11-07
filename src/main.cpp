#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include "game.hpp"
#include "gamelogic.hpp"
#include "platform.hpp"
#include "ui.hpp"

static const Uint32 tickduration = 10; /* ms -> 100Hz */

static int realmain(bool fullscreen) {
	bool run;
	ControlManager controlman;
	GameSetup gamesetup;
	GameStageState gamestate;
	Game* game;
	GameLogicJumps* gamejumps;
	GameLogic* gamelogic;
	bool transitionok;
	UserInterface* userintf;
	SDL_Event event;
	Uint32 tickerror, ticklast;

	trace("M.E.W.L. version " VERSION " starting");
	platform_init();
	if(SDL_Init(0) < 0)
		{ warn("Unable to initialise SDL: %s", SDL_GetError()); die(); }

	// Build the user interface
	userintf = FACTORY_FOR(UserInterface).create("UserInterface" USERINTF);
	if(!userintf) {
		warn("Miscompiled: class '%s' not found by UI factory.",
			"UserInterface" USERINTF);
		SDL_Quit(); die();
	}
	// And initialise it
	if(!userintf->init(fullscreen)) {
		warn("Unable to initialise user interface.");
		delete userintf; SDL_Quit(); die();
	}

	if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) {
		SDL_JoystickEventState(SDL_ENABLE); // May erase all events(!)
	} else { // Nonfatal
		warn("Joystick initialisation failed: %s", SDL_GetError());
	}

	controlman.populate();

	game = 0;
	gamejumps = new GameLogicJumps(&game, *userintf);
	gamelogic = GameLogic::getTitleState(gamejumps, gamestate, controlman);
	transitionok = true;

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
				switch(event.key.keysym.sym) {
					/* Easy to mistrigger; ^Q is even worse
					 * due to controls. Once we have load/
					 * save, we can autosave on quit and
					 * make this much less of a problem. */
					case SDLK_ESCAPE: run = false; break;
					// Doesn't reach WM to generate SDL_QUIT
					case SDLK_F4:
						if(event.key.keysym.mod &
							KMOD_ALT) {run = false;}
						break;
					case SDLK_F11:
						userintf->toggleFullscreen();
						break;
					case SDLK_RETURN:
						if(event.key.keysym.mod &
							KMOD_ALT) { userintf->
							toggleFullscreen(); }
						break;
					default: break;
				}
				// ...and chain down to control manager
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
			uint32_t ticks = 0;
			do {
				tickerror -= tickduration;

				/* Poke game logic to tick */
				if(transitionok) {
					GameLogic* nextlogic;
					nextlogic = gamelogic->simulate(
						gamesetup, game);
					if(nextlogic) {
						delete gamelogic;
						gamelogic = nextlogic;
						// Block any more simulation,
						// so that we don't jump two
						// states before the UI gets
						// to react.
						transitionok = false;
					}
				}
				ticks++;
			} while(tickerror >= tickduration);

			/* Poke UI to render game state */
			transitionok = userintf->render(gamelogic->getStage(),
				gamesetup, game, gamestate, ticks);
		} else {
			/* Have a nap until we actually have at least one tick
			 * to run */
			SDL_Delay(tickduration);
		}
	}

	trace("Clean exit");
	delete userintf;
	delete gamelogic;
	delete gamejumps;
	if(game) { delete game; }
	SDL_Quit();
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	bool fullscreen = false;
	// Do all the horrible command-line processing malarky
	for(int a = 1; a < argc; a++) {
		const char* arg = argv[a];
		if(0) {
		} else if(!strcmp(arg, "-h") || !strcmp(arg, "--help")
		       || !strcmp(arg, "/h") || !strcmp(arg, "/?")) {
			puts("Usage: mewl [-f]\n");
			puts("  -h --help       : this text");
			puts("  -v --version    : show version information");
			puts("  -f --fullscreen : run fullscreen");
			return 0;
		} else if(!strcmp(arg, "-v") || !strcmp(arg, "--version")) {
			puts("M.E.W.L. version " VERSION);
			puts("Licensed under the GNU GPL.");
			return 0;
		} else if(!strcmp(arg, "-f") || !strcmp(arg, "--fullscreen")) {
			fullscreen = true;
		}
	}

	// Now do the 'real' main routine
	return realmain(fullscreen);
}

