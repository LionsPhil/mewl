#ifndef GAME_HPP_
#define GAME_HPP_

#include <vector>
#include "resources.hpp"
#include "gamesetup.hpp"

class Player {
public:
	PlayerSetup setup;
	uint32_t money;
	Stock stock;
	// Land ownership is handled as a property of the terrain
	static Player& getDummyPlayer(); // For Tile constructor
};

class Tile {
	uint8_t m_mountains; // 0--3
	uint8_t m_crystal; // 0--4
	bool m_river;
	bool m_owned; // Owned; otherwise, owner and equipment are undefined.
	Player* m_owner; // Needs to be ptr, as can be reseated
	// What has it been outfitted to produce? Only valid if owned.
	Resource::Type m_equipment;
public:
	Tile();
	uint8_t& mountains(); // Can be mutated by planetquakes
	uint8_t& crystal(); // Can be mutated by meteor strikes
	const bool river();
	const bool owned();
	const Player& owner();
	const Resource::Type& equipment();
	void setUnowned();
	void setOwnership(Player& owner, Resource::Type equipment);
};

class Terrain {
	uint8_t width; // 9
	uint8_t height; // 5
	uint8_t cityx; // 4
	uint8_t cityy; // 2
	// The river runs vertically through the city. Anything else may not
	// be drawn correctly by the UI.
	std::vector<Tile> tiles;
public:
	Terrain();
	std::pair<uint8_t, uint8_t> getSize();
	std::pair<uint8_t, uint8_t> getCity();
	Tile& tile(uint8_t x, uint8_t y);
};

/** The state of one game in progress. This covers things like inventory; it
 *  does not cover the logic, even the terrain initialisation. */
class Game {
public: // Another fancypants struct
	Player players[PLAYERS];
	Difficulty::Type difficulty;
	Terrain terrain;
	uint8_t month;
	Stock store;
	Stock prices; // (store)
};

#endif

