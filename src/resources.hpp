#ifndef RESOURCES_HPP_
#define RESOURCES_HPP_
#include <stdint.h>

/** \file
 * \brief Resource types and stocks */

namespace Resource { typedef enum { NONE, FOOD, ENERGY, ORE, CRYSTAL } Type; }

/** The constructor zeros out the fields for miscellaneous use, e.g. showing
 * differences in auctions. Initialised correctly for player/store by difficulty
 * level; this gets used for both price AND quantity. */
struct Stock {
	Stock();
	int32_t food;
	int32_t energy;
	int32_t ore;     ///< (Smithore)
	int32_t crystal; ///< (Crystite)
	int32_t workers; ///< (Mules) (Players never have workers 'in stock')
};

#endif

