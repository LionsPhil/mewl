#ifndef GAMELOGIC_HPP_
#define GAMELOGIC_HPP_
#include "game.hpp"

/*struct GameLogicJumps {
	void startTheGameAlready();
	void destroyTheGameAlready();
	bool isPlayerTouchingMountain();
};*/

/** Gamelogic drives the MEWL program, effectively, including during the setup
 * when there isn't actually a Game object yet. */

class GameLogic {
public:
	virtual ~GameLogic();
	/** Get the stage this object does the processing for, so that the
	 * renderer can be told to transition. */
	virtual GameStage::Type getStage() = 0;
	/** Simulate the game for one tick, and return a logic for the next
	 * tick; if NULL, continue using the current logic. Replaced logics
	 * should be deleted. Simulation may be skipped while the UI
	 * transitions. */
	virtual GameLogic* simulate(GameSetup& setup, Game* game) = 0;

	/** Get the initial game logic, for the title screen. */
	static GameLogic* getTitleState();
};

#endif

