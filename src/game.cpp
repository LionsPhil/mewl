#include <assert.h>
#include "game.hpp"
#include "util.hpp"

Tile::Tile() : m_mountains(0), m_crystal(0), m_river(false), m_owned(false) {}
uint8_t& Tile::mountains() { return m_mountains; }
uint8_t& Tile::crystal() { return m_crystal; }
const bool Tile::river() { return m_river; }
const bool Tile::owned() { return m_owned; }
const Player& Tile::owner() { assert(m_owned); return *m_owner; }
const Resource::Type& Tile::equipment() { assert(m_owned); return m_equipment; }
void Tile::setUnowned() { m_owned = false; }
void Tile::setOwnership(Player& owner, Resource::Type equipment) {
	m_owned = true;
	m_owner = &owner;
	m_equipment = equipment;
}

Terrain::Terrain() : width(9), height(5), cityx(4), cityy(2) {
	tiles.resize(width * height);
}
uint8_t Terrain::getSizeX() { return width; }
uint8_t Terrain::getSizeY() { return height; }
uint8_t Terrain::getCityX() { return cityx; }
uint8_t Terrain::getCityY() { return cityy; }
Tile& Terrain::tile(uint8_t x, uint8_t y) {
	assert(x < width); assert(y < height);
	return tiles[x + (y * width)];
}

/* Oh, how I'd love to have closures for this sort of thing. */
static void depositCrystalSafely(Terrain& t, int x, int y, int level) {
	if(x < 0 || y < 0) { return; }
	if(x >= t.getSizeX()) { return; }
	if(y >= t.getSizeY()) { return; }
	if(t.tile(x, y).crystal() < level) // Keep higher values
		{ t.tile(x, y).crystal() = level; }
}

Game::Game(const GameSetup& setup) : difficulty(setup.difficulty), month(0) {
	// Set up the players
	for(int i = 0; i < PLAYERS; i++) {
		players[i].setup = setup.playersetup[i];
		players[i].money = 1000 +
			Species::getStartingBonus(setup.playersetup[i].species);
		if(setup.playersetup[i].computer &&
			Difficulty::hasComputerBonus(difficulty))
			{ players[i].money += 200; }
		Difficulty::initialPlayerStock(difficulty, players[i].stock);
	}
	/* Generate terrain. Yay hardcoding! Genericising this is not on my
	 * list of fun things to do at the moment, and without scaling or
	 * panning (both unnecessary for classic), the UI would only suffer. */
	const uint8_t w = terrain.getSizeX();
	const uint8_t h = terrain.getSizeY();
	const uint8_t river = terrain.getCityX();
	for(uint8_t y = 0; y < h; y++) {
		for(uint8_t x = 0; x < w; x++) {
			Tile& tile = terrain.tile(x, y);
			tile.mountains() = 0;
			tile.crystal() = 0;
			/* No mutator for river, as things outside of this
			 * shouldn't be able to change it. */
			tile.m_river = (x == river);
			tile.setUnowned();
		}
		/* Generate mountains. This isn't the same algorithm as the
		 * original game, but generates the same distribution. */
		uint8_t mountleft  = random_uniform(0, river - 1);
		uint8_t mountright = random_uniform(river + 1, w - 1);
		uint8_t mountains  = random_uniform(1, 3); // left side
		terrain.tile(mountleft,  y).mountains() = mountains;
		terrain.tile(mountright, y).mountains() = 4 - mountains;
	}
	if(Difficulty::hasCrystal(difficulty)) {
		/* Generate crystal deposits. True to the original game, even
		 * though you can't mine there, they might land on the river or
		 * town itself. */
		for(int i = 1; i <= 4; i++) {
			uint8_t x = random_uniform(0, w - 1);
			uint8_t y = random_uniform(0, h - 1);
			terrain.tile(x, y).crystal() = 3;
			depositCrystalSafely(terrain, x-1, y  , 2);
			depositCrystalSafely(terrain, x  , y-1, 2);
			depositCrystalSafely(terrain, x+1, y  , 2);
			depositCrystalSafely(terrain, x  , y+1, 2);
			depositCrystalSafely(terrain, x-2, y  , 1);
			depositCrystalSafely(terrain, x-1, y-1, 1);
			depositCrystalSafely(terrain, x  , y-2, 1);
			depositCrystalSafely(terrain, x+1, y-1, 1);
			depositCrystalSafely(terrain, x+2, y  , 1);
			depositCrystalSafely(terrain, x+1, y+1, 1);
			depositCrystalSafely(terrain, x  , y+2, 1);
			depositCrystalSafely(terrain, x-1, y+1, 1);
		}
	}
	// Initialise store
	Difficulty::initialStoreStock(difficulty, store);
	Difficulty::initialStorePrices(difficulty, prices);
}

