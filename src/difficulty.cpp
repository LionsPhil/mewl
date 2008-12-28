#include "difficulty.hpp"
#include "util.hpp"

uint8_t Difficulty::getGameDuration(Difficulty::Type self)
	{ return (self == Difficulty::BEGINNER) ? 6 : 12; }

/* In original MULE, a full-food standard move is 101 PTU == 707 BTU ==
 * 707/15 seconds (PAL @ 60Hz) == 47.13... seconds. In beginner, 101 PTU is
 * 909 BTU, which comes out as 60.6 seconds. */
double Difficulty::getMoveTime(Difficulty::Type self)
	{ return (self == Difficulty::BEGINNER) ? 60 : 47; }

/* Original MULE controls various auction values via Auction Time Units. */
static inline double ATU(Difficulty::Type self) {
	switch(self) {
		case Difficulty::BEGINNER:   return 50.0/15.0;
		case Difficulty::STANDARD:   return 30.0/15.0;
		case Difficulty::TOURNAMENT: return 25.0/15.0;
	}
	return 0.0; // Shush, G++; should trigger enum case warning if possible
}

double Difficulty::getDeclareTime(Difficulty::Type self)
	{ return 30 * ATU(self); }

double Difficulty::getAuctionTime(Difficulty::Type self)
	{ return 70 * ATU(self); }

/* The original values are 8, 4, and 3, giving fractions of seconds
 * approximately equal to 1/2, 1/4, and 1/5. We are /slightly/ more forgiving,
 * as all the twitchy people are now kept occupied by CounterStrike. */
double Difficulty::getLandGrabTime(Difficulty::Type self) {
	switch(self) {
		case Difficulty::BEGINNER:   return 1.0/2.0;
		case Difficulty::STANDARD:   return 1.0/3.0;
		case Difficulty::TOURNAMENT: return 1.0/4.0;
	}
	return 0.0;
}

/* This is the same ratio as the move time, as the basis is the same: you get
 * more Wumpus time in Beginner, because PTUs are longer. */
double Difficulty::getWumpusTimeMultiplier(Difficulty::Type self)
	{ return (self == Difficulty::BEGINNER) ? 9.0/7.0 : 1.0; }

void Difficulty::initialPlayerStock(Type self, Stock& initialise) {
	initialise.food   = (self == Difficulty::BEGINNER) ? 8 : 4;
	initialise.energy = (self == Difficulty::BEGINNER) ? 4 : 2;
	initialise.ore = initialise.crystal = initialise.workers = 0;
}

void Difficulty::initialStoreStock(Type self, Stock& initialise) {
	initialise.food   = (self == Difficulty::BEGINNER) ? 16 : 8;
	initialise.energy = (self == Difficulty::BEGINNER) ? 16 : 8;
	initialise.ore    = (self == Difficulty::BEGINNER) ?  0 : 8;
	initialise.crystal = 0;
	initialise.workers = 14; // (Overridden as infinite for BEGINNER)
}

void Difficulty::initialStorePrices(Type self, Stock& initialise) {
	initialise.food    = 30;
	initialise.energy  = 25;
	initialise.ore     = 50;
	initialise.crystal = 100;
	initialise.workers = 100;
} // Doesn't actually vary with difficulty level

bool Difficulty::hasColonyRating(Difficulty::Type self)
	{ return !(self == Difficulty::BEGINNER); }

bool Difficulty::hasRandomEvents(Difficulty::Type self)
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

bool Difficulty::hasComputerBonus(Difficulty::Type self)
	{ return (self == Difficulty::TOURNAMENT); }

// This is 'max' insofar that land auctions are limited by unclaimed land.
uint8_t Difficulty::calcMaxLandAuctions(Difficulty::Type self) {
	if(self == Difficulty::BEGINNER) { return 0; }
	return random_normal(-1, 4, (self == Difficulty::STANDARD)?0.5:1.0) + 1;
}

int8_t Difficulty::calcProductionVariation(Difficulty::Type self) {
	switch(self) {
		case Difficulty::BEGINNER:   return 0;
		case Difficulty::STANDARD:   return random_normal(-2, 2, 0.5);
		case Difficulty::TOURNAMENT: return random_normal(-3, 3, 1.0);
	}
	return 0;
}

