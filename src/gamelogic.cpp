#include "gamelogic.hpp"

GameLogicJumps::GameLogicJumps(Game** ptrgame, const UserInterface& ui)
	: ptrgame(ptrgame), ui(ui) {}

void GameLogicJumps::startTheGameAlready(const GameSetup& setup) {
	*ptrgame = new Game(setup);
}

void GameLogicJumps::destroyTheGameAlready() {
	delete *ptrgame;
}

class GameLogicTitle : public GameLogic {
	virtual GameStage::Type getStage() { return GameStage::TITLE; }
	virtual GameLogic* simulate(GameSetup& setup, Game* game,
		GameStageState& state) {
		
		return 0; // TODO
	}
public:
	GameLogicTitle(GameLogicJumps* jumps) : GameLogic(jumps) {}
};

GameLogic::GameLogic(GameLogicJumps* jumps) : jumps(jumps) {}
GameLogic::~GameLogic() {}
GameLogic* GameLogic::getTitleState(GameLogicJumps* jumps)
	{ return new GameLogicTitle(jumps); }

