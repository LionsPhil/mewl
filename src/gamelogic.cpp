#include "gamelogic.hpp"

class GameLogicTitle : public GameLogic {
	virtual GameStage::Type getStage() { return GameStage::TITLE; }
	virtual GameLogic* simulate(GameSetup& setup, Game* game) {
		return 0; // TODO
	}
};

GameLogic::~GameLogic() {}
GameLogic* GameLogic::getTitleState() { return new GameLogicTitle(); }

