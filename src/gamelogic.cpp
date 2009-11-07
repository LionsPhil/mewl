#include <new> // Fear the placement new! See game.cpp.

#include "gamelogic.hpp"
#include "platform.hpp" // DEBUG

GameLogicJumps::GameLogicJumps(Game** ptrgame, const UserInterface& ui)
	: ptrgame(ptrgame), ui(ui) {}

void GameLogicJumps::startTheGameAlready(const GameSetup& setup) {
	*ptrgame = new Game(setup);
}

void GameLogicJumps::destroyTheGameAlready() {
	delete *ptrgame;
}

class GameLogicTitle : public GameLogic {
	// While later logics can use the references from Player, we need to
	// see the total set to detect controls activating/deactivating.
	ControlManager& controlman;
	// TODO When a new controller tries to activate but there are no free
	// player slots, either drop oldest or somehow poke UI to report it.
	virtual GameStage::Type getStage() { return GameStage::TITLE; }
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
							.computer = true;
						setup.playersetup[player]
							.controller = NULL;
						goto next_controller;
					}
				}
				// It's a player joining; displace a computer
				for(int player = 0; player < PLAYERS; ++player){
					if(setup.playersetup[player].computer) {
						setup.playersetup[player]
							.computer = false;
						setup.playersetup[player]
							.controller=*controller;
						player = PLAYERS; // break
					}
				}
			}
next_controller: ;
		}

		// Set ready flags
		for(int player = 0; player < PLAYERS; ++player) {
			state.title.playerready[player] =
				setup.playersetup[player].computer ||
				setup.playersetup[player].controller
					->getDirection() == DIR_N;
		}
//if(!setup.playersetup[0].computer) { trace("%d", setup.playersetup[0].controller->getDirection()); } // DEBUG
//trace("%s", state.title.playerready[0] ? "RDY" : "not"); // DEBUG

		return 0; // TODO next state if all ready
	}
public:
	GameLogicTitle(GameLogicJumps* jumps, GameStageState& state,
		ControlManager& controlman) :
		GameLogic(jumps, state), controlman(controlman) {
		// This be some serious voodoo, mon.
		state.title.~Title();
		new(&state.title) GameStageState::Title();
	}
};

GameLogic::GameLogic(GameLogicJumps* jumps, GameStageState& state) :
	jumps(jumps), state(state) {}
GameLogic::~GameLogic() {}
GameLogic* GameLogic::getTitleState(GameLogicJumps* jumps,
	GameStageState& state, ControlManager& controlman) {

	return new GameLogicTitle(jumps, state, controlman);
}

