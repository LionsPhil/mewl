#ifndef GAMELOGIC_HPP_
#define GAMELOGIC_HPP_
#include "game.hpp"

class UserInterface;
/** A class for allowing the game logic to trigger some events affecting the
 *  rest of the code in a controlled fashion. Set up by main. */
class GameLogicJumps {
private:
	Game** ptrgame;
	const UserInterface& ui;
public:
	GameLogicJumps(Game** ptrgame, const UserInterface& ui);
	/** Create a game, held at the pointer, using given setup. */
	void startTheGameAlready(const GameSetup& setup);
	/** Deconstruct the Game and zero the pointer. */
	void destroyTheGameAlready();
	/*  Pass through to UserInterface:: ... */
	//bool isPlayerTouchingMountain();
};

/** Gamelogic drives the MEWL program, effectively, including during the setup
 * when there isn't actually a Game object yet. We don't use the factory for
 * these as they are not really an extensible set: OK to have a big list of
 * declarations. */
class GameLogic {
protected:
	GameLogicJumps* jumps;
public:
	GameLogic(GameLogicJumps* jumps);
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
	static GameLogic* getTitleState(GameLogicJumps* jumps);
};

#endif

