#ifndef GAME_HPP_
#define GAME_HPP_

#include <vector>
#include "controller.hpp"
#include "resources.hpp"
#include "gamesetup.hpp"

/** Current stage of the entire game programme. */
namespace GameStage { typedef enum { TITLE, COLOUR, SPECIES, SCOREBOARD,
	LANDGRAB, LANDAUCTION, PREAUCTION, AUCTIONDECLARE, AUCTION, PREDEVELOP,
	DEVELOPHUMAN, WAMPUS, DEVELOPCOMP, POSTDEVELOP, PREPRODUCT, PRODUCT,
	POSTPRODUCT } Type; };

/** Production-time random events. Some of these need a location; some just a
 *  row, others nothing at all. The quake needs a mountain, but since the
 *  details of such are left to the UI, we just give it a tile and direction.
 *  The 'none' event is used because of the pre/post separation. */
/*namespace ProductionEvent { typedef enum { NONE, PESTS, PIRATES, RAIN, QUAKE,
	SUN, METEORITE, RADIATION, FIRE } Type; };*/
struct ProductionEvent { ProductionEvent();
	enum { NONE, PESTS, PIRATES, RAIN, QUAKE, SUN, METEORITE, RADIATION,
	FIRE } type;
	uint8_t x; uint8_t y;
	Direction landslide; // may be centre if no mountain moves
};

/** Stage-specific data pertinent to both logic and UI.
 * It's not quite MVC---this is only the shared part of the model. */
class GameStageState {
public:	
	/* An advantage of duplicating even very common fields like "player" is
	 * that we don't have to worry about the UI and the logic being one
	 * stage transition apart: the logic shouldn't trample state the UI is
	 * still using. (The disadvantage is all this verbosity :/ ) */
	struct title          { title();
		bool playerready[PLAYERS];
	};
	struct colour         { colour();
		int player; // of colour up for grabs
	};
	struct species        { species();
		int player;
	};
	struct scoreboard     { scoreboard();
		uint32_t landvalue[PLAYERS];
		uint32_t goodsvalue[PLAYERS];
		// TODO type/parameters of message
	};
	struct landgrab       { landgrab();
		uint8_t x; uint8_t y;
	};
	struct landauction    { landauction();
		uint8_t x; uint8_t y;
	};
	struct preauction { preauction();
		Resource::Type resource; // 'none' means 'land'; rest varied
		uint32_t stock[     PLAYERS];
		uint32_t production[PLAYERS];
		uint32_t spoilage[  PLAYERS];
		 int32_t surplus[   PLAYERS]; // negative = deficit
		uint32_t store;
	};
	struct auctiondeclare { auctiondeclare();
		// Preauction still valid, plus:
		bool buyer[PLAYERS]; // else seller
		double time;
		double timemax;
	};
	struct auction        { auction();
		// Preauction and auctiondeclare still valid (inc. time), plus:
		uint32_t bid[   PLAYERS];
		uint32_t traded[PLAYERS];
		uint32_t storebuy;
		uint32_t storesell;
	};
	struct predevelop     { predevelop();
		int player;
		// TODO type/parameters of random event
	};
	struct develophuman   { develophuman();
		int player;
		// TODO position...more specific than per-tile: float? fixed?
		Direction dir;
		bool town; // else colony view
		bool mule; // in tow?
		Resource::Type muletype;
		double time; // seconds remaining
		double timemax; // 'normal' maximum time for player for scale
		// TODO wampus mountain, visibility
	};
	struct wampus         { wampus();
		int player;
		uint32_t prize;
	};
	struct developcomp    { developcomp();
		int player;
		uint8_t x; // cursor, not player-character, position
		uint8_t y;
	};
	struct postdevelop    { postdevelop();
		int player;
		uint32_t winnings; // if zero, ran out of time
	};
	struct preproduct     { preproduct();
		ProductionEvent event;
	};
	struct product        { product();
		// TODO production in each square
	};
	struct postproduct    { postproduct();
		ProductionEvent event;
	};

	GameStageState();
};

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

