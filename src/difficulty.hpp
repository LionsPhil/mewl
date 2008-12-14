#ifndef DIFFICULTY_HPP_
#define DIFFICULTY_HPP_
#include <stdint.h>

namespace Difficulty {
	typedef enum {
		BEGINNER,
		STANDARD,
		TOURNAMENT
	} Type;

	uint8_t getGameDuration(Type self);
	uint32_t getMinimumColonyWorth(Type self); // ...or you all lose
	double getTimeMultiplier(Type self); // <1 == shorter auctions and moves
	bool hasRandomEvents(Type self);
	bool hasVariableProduction(Type self);
	bool hasLandAuctions(Type self);
	bool hasCriticalSelling(Type self); // Can sell beyond critical level
	bool hasSellPriceLimit(Type self); // Cannot sell at 'scroll up' prices
	bool hasFussyPlacement(Type self); // Mules run off if you miss the hut
	bool hasInfiniteWorkers(Type self); // ...and at a fixed price of $100
	bool hasCrystal(Type self);
	bool hasCollusion(Type self);
	bool hasFewStartResources(Type self); // Colony sufficient for one turn
	bool hasComputerBonus(Type self); // Computers start with +$200
}

#endif

