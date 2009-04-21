#ifndef DIFFICULTY_HPP_
#define DIFFICULTY_HPP_
#include <stdint.h>
#include "resources.hpp"

namespace Difficulty {
	typedef enum {
		BEGINNER,
		STANDARD,
		TOURNAMENT
	} Type;

	uint8_t getGameDuration(Type self);
	double getMoveTime(Type self); // seconds (full food, normal species)
	double getDeclareTime(Type self); // seconds (set buyer/seller)
	double getAuctionTime(Type self); // seconds (both land and trade)
	double getLandGrabTime(Type self); // seconds (time per tile)
	double getWampusTimeMultiplier(Type self); // >1 == longer
	void initialPlayerStock(Type self, Stock& initialise);
	void initialStoreStock(Type self, Stock& initialise);
	void initialStorePrices(Type self, Stock& initialise);
	bool hasColonyRating(Type self); // Can all lose overall?
	bool hasRandomEvents(Type self);
	bool hasLandAuctions(Type self);
	bool hasCriticalSelling(Type self); // Can sell beyond critical level
	bool hasSellPriceLimit(Type self); // Cannot sell at 'scroll up' prices
	bool hasFussyPlacement(Type self); // Mules run off if you miss the hut
	bool hasInfiniteWorkers(Type self); // ...and at a fixed price of $100
	bool hasCrystal(Type self);
	bool hasCollusion(Type self);
	bool hasComputerBonus(Type self); // Computers start with +$200
	uint8_t calcMaxLandAuctions(Type self); // Generate random
	int8_t calcProductionVariation(Type self); // Generate random
	
	/* The Wampus is interesting. In the original MULE, the difficulty
	 * affects the brightness of his cave light, and also (via PTU/BTU
	 * conversion) how long he takes to appear (as a function of how long
	 * the player takes to begin their turn). We don't replicate this
	 * mechanic, but we do need to apply the species time modifier to its
	 * motions, and another which allows for PTU calculations. */
}

#endif

