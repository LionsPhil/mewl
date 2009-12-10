#ifndef SPECIES_HPP_
#define SPECIES_HPP_
#include <assert.h>
#include <stdint.h>

namespace Species {
	// That there are eight of these is a fairly strong assumption in the
	// UI, due to the 'select species by pushing in its direction' interface
	typedef enum {
		REGULAR1, // Gollumer
		REGULAR2, // Packer
		REGULAR3, // Spheroid
		ADVANCED, // Humanoid
		REGULAR4, // Leggite
		BEGINNER, // Flapper
		REGULAR5, // Bonzoid
		COMPUTER  // Mechtron (still playable by humans)
	} Type;
	/** Which species maps to the top direction on the select screen?
	  * Others work from here clockwise, as with the Direction type.
	  * Must also be the first enumeration value, for operators. */
	const Type FIRST = REGULAR1;
	const Type LAST = COMPUTER;

/* Because there is only one game logic implementation (vs. many UIs), we'll
 * define these here and avoid reaching JAVA levels of source file dilution. */
	/** Amount to add to starting money. May be negative. */
	int32_t getStartingBonus(Type self);
	/** Multiplier for move time. > 1 means more time to move. */
	double getTimeModifier(Type self);
}

inline void operator++(Species::Type& d) {
	assert(d != Species::LAST);
	d = static_cast<Species::Type>(d + 1);
}
inline void operator--(Species::Type& d) {
	assert(d != Species::FIRST);
	d = static_cast<Species::Type>(d - 1);
}

#endif

