#ifndef GAMELOGIC_HPP_
#define GAMELOGIC_HPP_

#include "controller.hpp"
#include "game.hpp"

/** \file
 * \brief Game mechanic mutations of the game state */

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
	// We need this permanently, rather than in simulate(), because stage
	// logic constructors need to reinitialise parts of it.
	GameStageState& state;
	GameLogic(GameLogicJumps* jumps, GameStageState& state);
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
	static GameLogic* getTitleState(GameLogicJumps* jumps,
		GameStageState& state, ControlManager& controlman);
};

#endif

