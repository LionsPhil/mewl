#ifndef GAMELOGIC_HPP_
#define GAMELOGIC_HPP_
#include "game.hpp"

class GameLogic {
	virtual ~GameLogic();
public:
	GameLogic* getTitleState();
	/** Get the stage this object does the processing for, so that the
	 * renderer can be told to transition. */
	virtual GameStage::Type getStage() = 0;
	/* Simulate the game for one tick, and return a logic for the next tick;
	 * if NULL, continue using the current tick. Replaced ticks should be
	 * deleted. Simulation may be skipped while the UI transitions. */
	virtual GameLogic* simulate(Game& game) = 0;
};

#endif

