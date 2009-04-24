#ifndef GAME_HPP_
#define GAME_HPP_

#include <vector>
#include "resources.hpp"
#include "gamesetup.hpp"

/** Current stage of the entire game programme. */
namespace GameStage { typedef enum { TITLE, COLOUR, SPECIES, SCOREBOARD,
	LANDGRAB, LANDAUCTION, PREAUCTION, AUCTIONDECLARE, AUCTION, PREDEVELOP,
	DEVELOPHUMAN, WAMPUS, DEVELOPCOMP, POSTDEVELOP, PREPRODUCT, PRODUCT,
	POSTPRODUCT } Type; };

class Player {
public:
	PlayerSetup setup;
	uint32_t money;
	Stock stock;
	// Land ownership is handled as a property of the terrain
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
	/*const*/ bool river();
	/*const*/ bool owned();
	const Player& owner();
	const Resource::Type& equipment();
	void setUnowned();
	void setOwnership(Player& owner, Resource::Type equipment);
	friend class Game; // Let the terrain generator set the river
};

class Terrain {
	uint8_t width;
	uint8_t height;
	uint8_t cityx;
	uint8_t cityy;
	/* The river runs vertically through the city. Anything else may not
	 * be drawn correctly by the UI. */
	std::vector<Tile> tiles;
public:
	Terrain();
	/** Don't mistake this and getCity as an indicator that changing the
	 *  terrain size is easy and properly abstracted. The UI is likely to
	 *  make assumptions such that it'll fit on the screen without scaling
	 *  or panning, and the terrain generator won't adjust densities.
	 *  (Logically, pairs would be a nice way to model this; realistically,
	 *  they're a hell of a lot of 'fluff' to just get some damn values. */
	uint8_t getSizeX(); uint8_t getSizeY();
	uint8_t getCityX(); uint8_t getCityY();
	Tile& tile(uint8_t x, uint8_t y);
};

/** The state of one game in progress. This covers things like inventory; it
 *  does not cover the logic, although it does initialise itself sensibly. */
class Game {
public: // Another fancypants struct
	Player players[PLAYERS];
	Difficulty::Type difficulty;
	Terrain terrain;
	uint8_t month;
	Stock store;
	Stock prices; // (store)

	Game(const GameSetup& setup);
};

#endif

