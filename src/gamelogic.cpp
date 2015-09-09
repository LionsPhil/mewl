#include <new> // Fear the placement new! See game.cpp.
#include <assert.h>

#include "gamelogic.hpp"
#include "platform.hpp"

// This be some serious voodoo, mon.
#define STAGESTATE_RESET(FIELD, KLASS) \
	state.FIELD.~KLASS(); \
	new(&state.FIELD) GameStageState::KLASS();

/** Clear up any stray button presses by throwing away the controller flag.
 *  Useful as a synchronisation point before players have to press buttons. */
void clear_stray_presses(GameSetup& setup) {
	for(int p = 0; p < PLAYERS; ++p) {
		if(!setup.playersetup[p].computer) {
			setup.playersetup[p].controller->hadButtonPress();
		}
	}
}

GameLogicJumps::GameLogicJumps(Game** ptrgame, const UserInterface& ui)
	: ptrgame(ptrgame), ui(ui) {}

void GameLogicJumps::startTheGameAlready(const GameSetup& setup) {
	*ptrgame = new Game(setup);
}

void GameLogicJumps::destroyTheGameAlready() {
	delete *ptrgame;
}

class GameLogicSpecies : public GameLogic {
	virtual GameStage::Type getStage() { return GameStage::SPECIES; }
	virtual GameLogic* simulate(GameSetup& setup, Game* game) {
		// While computer player and < PLAYERS, set as COMP and player++
		while((state.species.player < PLAYERS) &&
			setup.playersetup[state.species.player].computer) {

			setup.playersetup[state.species.player].species
				= Species::COMPUTER;
			state.species.player++;
			state.species.defined = false;
		}

		// Have we finished here?
		if(state.species.player >= PLAYERS) {
			abort();
			/*warn("return (next logic) required"); // TODO
			die();*/
		}

		// If direction pressed, set PlayerSetup species, defined = true
		Direction d = setup.playersetup[state.species.player]
			.controller->getDirection();
		if(d != DIR_CENTRE) {
			Species::Type s = Species::FIRST;
			while(d != DIR_N) { --d; ++s; }
			setup.playersetup[state.species.player].species = s;
			if(!state.species.defined) {
				/* If this is their first direction, eat a button press, in
				 * case they pressed *before* pushing a direction. */
				setup.playersetup[state.species.player].controller
					->hadButtonPress();
			}
			state.species.defined = true;
		}

		// If button pressed && defined, defined = false, player++
		if(state.species.defined &&
			setup.playersetup[state.species.player].controller
				->hadButtonPress()) {

			state.species.player++;
			state.species.defined = false;
			clear_stray_presses(setup);
		}

		return 0;
	}
public:
	GameLogicSpecies(GameLogicJumps* jumps, GameStageState& state) :
		GameLogic(jumps, state) {
		
		STAGESTATE_RESET(species, Species);
	}
};

class GameLogicColour : public GameLogic {
	int time; // Ticks spent on the current colour
	bool claimed[PLAYERS]; // Colour [i] has been claimed
	virtual GameStage::Type getStage() { return GameStage::COLOUR; }
	virtual GameLogic* simulate(GameSetup& setup, Game* game) {
		bool gone = false; // The current colour has been claimed
		bool done = true; // Everybody who wants one has a colour

		for(int p = 0; p < PLAYERS; ++p) {
			// See if anyone is claiming this colour
			if(!gone && !setup.playersetup[p].computer && setup
				.playersetup[p].controller->hadButtonPress()) {
				
				if(state.colour.claim[p] == -1) {
					state.colour.claim[p] =
						state.colour.offer;
					claimed[state.colour.offer] = true;
					gone = true;
				} // else you've already got one
			}
			// Does this player still need a colour?
			if(!setup.playersetup[p].computer &&
				state.colour.claim[p] == -1) { done = false; }
		}
		// Cycle to next colour
		if(gone || (++time > 150)) {
			do { ++state.colour.offer; }
				while(claimed[state.colour.offer]);
			if(state.colour.offer >= PLAYERS)
				{ state.colour.offer = 0; }
			while(claimed[state.colour.offer])
				{ ++state.colour.offer; }
			assert(state.colour.offer < PLAYERS);
			time = 0;
		}
		if(!done) { return 0; }
		// We're done!
		// Hokay, first allocate colours to computer players
		for(int p = 0; p < PLAYERS; ++p) {
			if(state.colour.claim[p] == -1) {
				for(int c = 0; c < PLAYERS; ++c) {
					if(!claimed[c]) {
						state.colour.claim[p] = c;
						claimed[c] = true;
						break;
					}
				}
			}
		}
		// Shuffle players into their new colour slots
		{
			PlayerSetup original[PLAYERS];
			for(int p = 0; p < PLAYERS; ++p)
				{ original[p] = setup.playersetup[p]; }
			for(int p = 0; p < PLAYERS; ++p) {
				setup.playersetup[state.colour.claim[p]]
					= original[p];
			}
		}
		// Clear up any mess players may have made on the way out
		clear_stray_presses(setup);
		return new GameLogicSpecies(jumps, state);
	}
public:
	GameLogicColour(GameLogicJumps* jumps, GameStageState& state) :
		GameLogic(jumps, state), time(0) {
		
		STAGESTATE_RESET(colour, Colour);
		for(int p = 0; p < PLAYERS; ++p){ claimed[p] = false; }
	}
};

class GameLogicTitle : public GameLogic {
	// While later logics can use the references from Player, we need to
	// see the total set to detect controls activating/deactivating.
	ControlManager& controlman;
	// 'Votes' for moving the difficulty. This is actually a kind-of
	// workaround for not having a latching mechanism on left/right in the
	// control abstraction. One player * 50 ticks (one half-sec) = change.
	Sint16 diffvotes;
	
	virtual GameStage::Type getStage() { return GameStage::TITLE; }
	// TODO When a new controller tries to activate but there are no free
	// player slots, either drop oldest or somehow poke UI to report it.
	virtual GameLogic* simulate(GameSetup& setup, Game* game) {
		const std::vector<Controller*>& controllers =
			controlman.getControllers(); // may repopulate ad-hoc

		// Add/remove players
		for(std::vector<Controller*>::const_iterator controller =
			controllers.begin();
			controller != controllers.end(); ++controller) {

			if((*controller)->hadButtonPress()) {
				// Is this a player bowing out?
				for(int player = 0; player < PLAYERS; ++player){
					if(!setup.playersetup[player].computer
					&&  setup.playersetup[player].controller
						== *controller) {
						// Yes, yes it is.
						setup.playersetup[player]
							.computerPlayer();
						goto next_controller;
					}
				}
				// It's a player joining; displace a computer
				for(int player = 0; player < PLAYERS; ++player){
					if(setup.playersetup[player].computer) {
						setup.playersetup[player]
							.humanPlayer(
								*controller);
						goto next_controller;
					}
				}
			}
next_controller: ;
		}

		// Now that we have all the players
		bool voting = false;
		bool allready = true;
		bool allcpu = true;
		for(int player = 0; player < PLAYERS; ++player) {
			PlayerSetup* ps = &setup.playersetup[player];

			// Vote for difficulty
			if(!ps->computer) {
				switch(ps->controller->getDirection()) {
					case DIR_W:
						--diffvotes; voting = true;
						break;
					case DIR_E:
						++diffvotes; voting = true;
					default: ;
				}
			}

			// Set ready flags
			state.title.playerready[player] = ps->computer ||
				ps->controller->getDirection() == DIR_N;
			if(!state.title.playerready[player])
				{ allready = false; }
			if(!ps->computer) { allcpu = false; }
		}

		// Adjust difficulty
		if(!voting) { diffvotes = 0; } // reset on release
		if(diffvotes <= -50) {
			if(setup.difficulty > Difficulty::FIRST)
				{ --setup.difficulty; }
			diffvotes = 0;
		} else if(diffvotes >= 50) {
			if(setup.difficulty < Difficulty::LAST)
				{ ++setup.difficulty; }
			diffvotes = 0;
		} 

		return allready && !allcpu
			? new GameLogicColour(jumps, state) : 0;
	}
public:
	GameLogicTitle(GameLogicJumps* jumps, GameStageState& state,
		ControlManager& controlman) :
		GameLogic(jumps, state), controlman(controlman), diffvotes(0)
		{ STAGESTATE_RESET(title, Title) }
};

GameLogic::GameLogic(GameLogicJumps* jumps, GameStageState& state) :
	jumps(jumps), state(state) {}

GameLogic::~GameLogic() {}
GameLogic* GameLogic::getTitleState(GameLogicJumps* jumps,
	GameStageState& state, ControlManager& controlman) {

	return new GameLogicTitle(jumps, state, controlman);
}

