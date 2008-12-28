#ifndef RESOURCES_HPP_
#define RESOURCES_HPP_
#include <stdint.h>

namespace Resource { typedef enum { None, Food, Energy, Ore, Crystal } Type; }

// Initialised by difficulty level; this gets used for both price AND quantity
struct Stock {
	uint32_t food;
	uint32_t energy;
	uint32_t ore;     // (Smithore)
	uint32_t crystal; // (Crystite)
	uint32_t workers; // (Mules) (Players never have workers 'in stock')
};

#endif

