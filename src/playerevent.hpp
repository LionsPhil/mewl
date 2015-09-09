#ifndef PLAYEREVENT_HPP_
#define PLAYEREVENT_HPP_

#include "resources.hpp"

/** \file
 * \brief Game events that can happen to players */

class Game;

namespace PlayerEvent {
	typedef enum {
		CARE_PACKAGE,
		WANDERING_TRAVELLER,
		MULE_WINNINGS_1, // best built
		MULE_WINNINGS_2, // tap-dancing
		AGRICULTURE_GRANT,
		WINNINGS_1, // wart worm
		WINNINGS_2, // museum computer
		WINNINGS_3, // swamp eel
		WINNINGS_4, // charity
		WINNINGS_5, // investments
		WINNINGS_6, // inheritence
		WINNINGS_7, // moose rat
		EXTRA_LAND,
		/* Important! Sort this enum so that good events come first,
		 * and keep FIRST_BAD pointing to the others. */
		FOOD_STOLEN,
		MULE_COST,
		MULE_MINING_COST,
		MULE_SOLAR_COST,
		LOSSES_1, // inlaws
		LOSSES_2, // cat-bugs
		LOSSES_3, // races
		LOSSES_4, // hospital
		LOST_LAND
	} Type;
	const Type FIRST_BAD = FOOD_STOLEN;
	const int MINIMUM = CARE_PACKAGE; // These are used to
	const int MAXIMUM = LOST_LAND;    // randomly select one
	
	/* As with Species, we'll put the logic-side declarations here.
	 * There's some Extra Magic involved in event selection:
	 *  - The same event cannot be selected twice. (The flags live in the
	 *    logic's state.)
	 *  - A valid selection (AFTER flagging the above) may get changed to
	 *    a food package if the player is low on food. */
	/// Is the event good for the player (else it is bad)?
	bool good(PlayerEvent::Type self);
	/// Are any *specific* preconditions for the event met for this player?
	bool precondition(PlayerEvent::Type self, int player, const Game& game);
	
	/** Thankfully, all the random effects can be immediate, and we don't
	 * have to keep the differences around for the UI (else we'd be here
	 * making twenty-odd subclasses instead of twenty-odd enum values). So
	 * we have a method to calculate the financial/stock changes (which the
	 * UI can also invoke for display), and one to apply other changes.
	 * The UI is not guaranteed to report changes it's not expecting!
	 * e.g. only events with strings about food will show food changes.
	 * Some events have an 'each' parameter; this is only for the UI, and
	 * only records magnitude: it is always positive. */
	Stock changes(PlayerEvent::Type self, int player, const Game& game,
		int32_t* money, uint16_t* each);
	void applyOther(PlayerEvent::Type self, int player, Game& game);
};

#endif

