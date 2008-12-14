#include "species.hpp"

int32_t Species::getStartingBonus(Species::Type self) {
	switch(self) {
		case Species::BEGINNER:
			return +600;
		case Species::ADVANCED:
			return -400;
		default:
			return 0;
	} // The COMPUTER 200 bonus is handled as a difficulty setting
}

double Species::getTimeModifier(Species::Type self) {
	switch(self) {
		case Species::BEGINNER:
			return 1.2; // TWEAK
		case Species::ADVANCED:
			return 0.9; // TWEAK
		default:
			return 1.0;
	}
}

//void Species::TheoreticalPerUIData(Species::Type self) { } // XXX

