#ifndef SPECIES_HPP_
#define SPECIES_HPP_
#include <stdint.h>

namespace Species {
	// That there are eight of these is a fairly strong assumption in the
	// UI, due to the 'select species by pushing in its direction' interface
	typedef enum {
		COMPUTER, // Mechtron (still playable by humans)
		REGULAR1, // Gollumer
		REGULAR2, // Packer
		REGULAR3, // Spheroid
		ADVANCED, // Humanoid
		REGULAR4, // Leggite
		BEGINNER, // Flapper
		REGULAR5  // Bonzoid
	} Type;

/* Because there is only one game logic implementation (vs. many UIs), we'll
 * define these here and avoid reaching JAVA levels of source file dilution. */
	/** Amount to add to starting money. May be negative. */
	int32_t getStartingBonus(Type self);
	/** Multiplier for move time. > 1 means more time to move. */
	double getTimeModifier(Type self);
}

//namespace Species { void TheoreticalPerUIData(Type self); } // XXX

#endif

