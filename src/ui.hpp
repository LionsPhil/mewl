#ifndef UI_HPP_
#define UI_HPP_
#include "factory.hpp"
#include "game.hpp"

/* UserInterface is a slight misnomer; it really means 'multimedia display'.
 * The controls are a sufficiently intrinsic part of MEWL that they go to the
 * GameLogic, not via the UI. You don't add, say, PDA stylus input by writing
 * a new PDA UI; you add a new PDA stylus Controller. This applies all the way
 * down to setting up the game, which is arguably overkill. */

class UserInterface {
public:
	virtual ~UserInterface() {}
	/// Initialise audio and graphics as needed. Return success.
	virtual bool init(bool fullscreen) = 0;
	/// Toggle fullscreen, if that makes sense for this interface.
	virtual void toggleFullscreen() {}

	/** Render the game, given that N ticks have passed since last render.
	 * If the stage has changed, but the previous stage still has UI work
	 * to perform (e.g. an animation), return false and this will be
	 * recalled without additional simulation work until it returns true.
	 * If the stage is the same, also return true. Note that Game may be
	 * NULL for some game stages. */
	virtual bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks) = 0;

	/* Factory allows implementation detail to be completely opaque. */
	FACTORY_REGISTER_IF(UserInterface)
};

#endif

