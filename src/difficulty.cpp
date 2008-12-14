#include "difficulty.hpp"

uint8_t Difficulty::getGameDuration(Difficulty::Type self)
	{ return (self == Difficulty::BEGINNER) ? 6 : 12; }

uint32_t Difficulty::getMinimumColonyWorth(Difficulty::Type self)
	{ return (self == Difficulty::BEGINNER) ? 0 : 60000; }

double Difficulty::getTimeMultiplier(Difficulty::Type self) {
	switch(self) {
		case Difficulty::BEGINNER:   return 1.2; // TWEAK
		case Difficulty::STANDARD:   return 1.0; // TWEAK
		case Difficulty::TOURNAMENT: return 0.9; // TWEAK
	}
	return 0.0; // Shush, G++; should trigger enum case warning if possible
}

bool Difficulty::hasRandomEvents(Difficulty::Type self)
	{ return !(self == Difficulty::BEGINNER); }

bool Difficulty::hasVariableProduction(Difficulty::Type self)
	{ return !(self == Difficulty::BEGINNER); }

bool Difficulty::hasLandAuctions(Difficulty::Type self)
	{ return !(self == Difficulty::BEGINNER); }

bool Difficulty::hasCriticalSelling(Difficulty::Type self)
	{ return !(self == Difficulty::BEGINNER); }

bool Difficulty::hasSellPriceLimit(Difficulty::Type self)
	{ return (self == Difficulty::BEGINNER); }

bool Difficulty::hasFussyPlacement(Difficulty::Type self)
	{ return !(self == Difficulty::BEGINNER); }

bool Difficulty::hasInfiniteWorkers(Difficulty::Type self)
	{ return (self == Difficulty::BEGINNER); }

bool Difficulty::hasCrystal(Difficulty::Type self)
	{ return (self == Difficulty::TOURNAMENT); }

bool Difficulty::hasCollusion(Difficulty::Type self)
	{ return (self == Difficulty::TOURNAMENT); }

bool Difficulty::hasFewStartResources(Difficulty::Type self)
	{ return (self == Difficulty::TOURNAMENT); }

bool Difficulty::hasComputerBonus(Difficulty::Type self)
	{ return (self == Difficulty::TOURNAMENT); }

