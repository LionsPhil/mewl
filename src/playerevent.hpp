#ifndef PLAYEREVENT_HPP_
#define PLAYEREVENT_HPP_

class Game;

namespace PlayerEvent {
	typedef enum {
		CARE_PACKAGE = 0,
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
	const int events = LOST_LAND + 1; // used to randomly select one
	
	/* As with Species, we'll put the logic-side declarations here.
	 * There's some Extra Magic involved in event selection:
	 *  - The same event cannot be selected twice. (The flags live in the
	 *    logic's state.)
	 *  - A valid selection (AFTER flagging the above) may get changed to
	 *    a food package if the player is low on food. */
	/// Is the event good for the player (else it is bad)?
	bool good(PlayerEvent::Type type);
	/// Are any *specific* preconditions for the event met for this player?
	bool precondition(PlayerEvent::Type type, int player, Game* game);
	
	/* Thankfully, all the random effects can be immediate, and we don't
	 * have to keep the differences around for the UI (else we'd be here
	 * making twenty-odd subclasses instead of twenty-odd enum values). So
	 * we have a method to calculate the financial/stock changes (which the
	 * UI can also invoke for display), and one to apply other changes.
	 * The UI is not guaranteed to report changes it's not expecting!
	 * e.g. only events with strings about food will show food changes. */
	Stock changes(PlayerEvent::Type, int player, Game* game,
		int32_t* money);
	void applyOther(PlayerEvent::Type type, int player, Game* game);
};

// TODO impl. will want a static 'm' calculator; see "Player event effect" tbl

#endif

