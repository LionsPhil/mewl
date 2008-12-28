#include "species.hpp"

int32_t Species::getStartingBonus(Species::Type self) {
	switch(self) {
		case Species::BEGINNER:
			return +600;
		case Species::ADVANCED:
			return -400;
		default:
			return 0;
	} // _Computers_ get a 200 bonus in Tourny; _not_ all Mechtrons.
}

/* In original MULE, the species adjusts the PTU/BTU conversion. Normal is 1 PTU
 * to 7 BTU; Flappers get 9, Humans get 5. 9/7 is about 1.29; 5/7 is ~0.71. */
double Species::getTimeModifier(Species::Type self) {
	switch(self) {
		case Species::BEGINNER:
			return 9.0/7.0;
		case Species::ADVANCED:
			return 5.0/7.0;
		default:
			return 1.0;
	}
}

//void Species::TheoreticalPerUIData(Species::Type self) { } // XXX

