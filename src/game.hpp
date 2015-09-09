#ifndef GAME_HPP_
#define GAME_HPP_

#include <vector>
#include "gamesetup.hpp"

namespace Resource { typedef enum { Food, Energy, Ore, Crystal } Type; }

class Stocks {
	uint32_t food;
	uint32_t energy;
	uint32_t ore;     // (Smithore)
	uint32_t crystal; // (Crystite)
};

class Player {
	PlayerSetup setup;
	uint32_t money;
	Stocks stocks;
	// Land ownership is handled as a property of the terrain
public:
	static Player& getDummyPlayer(); // For Tile constructor
};

class Tile {
	Tile();
	uint8_t mountains;
	uint8_t crystal;
	bool river;
	bool owned; // Owned; otherwise, owner and equipment are undefined.
	Player& owner;
	// What has it been outfitted to produce? Only valid if owned.
	Resource::Type equipment;
};

class Terrain {
	uint8_t width; // 9
	uint8_t height; // 5
	uint8_t cityx; // 4
	uint8_t cityy; // 2
	// The river runs vertically through the city. Anything else may not
	// be drawn correctly by the UI.
	std::vector<Tile> tiles;
};

/** The state of one game in progress. */
class Game {
	Player players[PLAYERS];
	Terrain terrain;
public:
	Game(GameSetup& setup);
};

#endif

