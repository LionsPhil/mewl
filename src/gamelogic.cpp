#include <new> // Fear the placement new! See game.cpp.

#include "gamelogic.hpp"

// This be some serious voodoo, mon.
#define STAGESTATE_RESET(FIELD, KLASS) \
	state.FIELD.~KLASS(); \
	new(&state.FIELD) GameStageState::KLASS();

GameLogicJumps::GameLogicJumps(Game** ptrgame, const UserInterface& ui)
	: ptrgame(ptrgame), ui(ui) {}

void GameLogicJumps::startTheGameAlready(const GameSetup& setup) {
	*ptrgame = new Game(setup);
}

void GameLogicJumps::destroyTheGameAlready() {
	delete *ptrgame;
}

class GameLogicColour : public GameLogic {
	int time;
	virtual GameStage::Type getStage() { return GameStage::COLOUR; }
	virtual GameLogic* simulate(GameSetup& setup, Game* game) {
		if(++time > 150) { // incomplete for DEBUG; needs to skip claim
			state.colour.player++;
			if(state.colour.player > PLAYERS)
				{ state.colour.player = 0; }
			time = 0;
		}
		return 0; // TODO claims, and player shuffle when all done
	}
public:
	GameLogicColour(GameLogicJumps* jumps, GameStageState& state) :
		GameLogic(jumps, state), time(0)
		{ STAGESTATE_RESET(colour, Colour) }
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

