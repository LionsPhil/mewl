#include <assert.h>
#include "game.hpp"
#include "util.hpp"

/* The logic can reset a stage's state by using placement new:
 * http://www.parashift.com/c++-faq-lite/dtors.html#faq-11.10
 * http://stackoverflow.com/questions/222557/cs-placement-new
 * http://www.research.att.com/~bs/bs_faq2.html
 * Don't forget to call the dtor first (even though it's CURRENTLY a no-op).
 * These are zealously thorough initialisers, doing a little more work than
 * is strictly necessary, but the cost is immeasurable, and null-bugs are
 * much easier to deal with than random-uninitialised-bugs. */
ProductionEvent::ProductionEvent() : type(NONE), x(0), y(0),
	landslide(DIR_CENTRE) {}
GameStageState::GameStageState() {} // it's all up to the inner contructors
GameStageState::Title::Title() {for(int i=0;i<PLAYERS;i++)playerready[i]=false;}
GameStageState::Colour::Colour() : player(0) {}
GameStageState::Species::Species() : player(0) {}
GameStageState::Scoreboard::Scoreboard() : message(ScoreboardMessage::NONE) {
	for(int i = 0; i < PLAYERS; i++) {
		landvalue[ i] = 0;
		goodsvalue[i] = 0;
	}
}
GameStageState::LandGrab::LandGrab() : x(0), y(0) {}
GameStageState::LandAuction::LandAuction() : x(0), y(0) {}
GameStageState::PreAuction::PreAuction() : resource(Resource::NONE), store(0) {
	for(int i = 0; i < PLAYERS; i++) {
		stock[     i] = 0;
		production[i] = 0;
		spoilage[  i] = 0;
		surplus[   i] = 0;
	}
}
GameStageState::AuctionDeclare::AuctionDeclare() : time(0), timemax(0)
	{ for(int i = 0; i < PLAYERS; i++) { buyer[i] = false; } }
GameStageState::Auction::Auction() : storebuy(0), storesell(0) {
	for(int i = 0; i < PLAYERS; i++) {
		bid[   i] = 0;
		traded[i] = 0;
	}
}
GameStageState::PreDevelop::PreDevelop() : player(0), eventhappens(false),
	eventtype(PlayerEvent::CARE_PACKAGE) {}
GameStageState::DevelopHuman::DevelopHuman() : dir(DIR_N), town(true),
	mule(false), muletype(Resource::NONE), time(0), timemax(0)
	{ /* TODO more */ }
GameStageState::Wampus::Wampus() : player(0), prize(0) {}
GameStageState::DevelopComp::DevelopComp() : player(0), x(0), y(0) {}
GameStageState::PostDevelop::PostDevelop() : player(0), winnings(0) {}
GameStageState::PreProduct::PreProduct() {}
GameStageState::Product::Product() {
	for(int y = 0; y < TERRAIN_HEIGHT; y++) {
		for(int x = 0; x < TERRAIN_WIDTH; x++)
			{ production[x][y] = 0; }
	}
}
GameStageState::PostProduct::PostProduct() {}

Tile::Tile() : m_mountains(0), m_crystal(0), m_river(false), m_owned(false) {}
uint8_t& Tile::mountains() { return m_mountains; }
uint8_t& Tile::crystal() { return m_crystal; }
bool Tile::river() const { return m_river; }
bool Tile::owned() const { return m_owned; }
int Tile::owner() const { assert(m_owned); return m_owner; }
const Resource::Type& Tile::equipment() const {
	assert(m_owned);
	return m_equipment;
}
void Tile::setUnowned() { m_owned = false; }
void Tile::setOwnership(int owner, Resource::Type equipment) {
	m_owned = true;
	m_owner = owner;
	m_equipment = equipment;
}

Terrain::Terrain() : width(TERRAIN_WIDTH), height(TERRAIN_HEIGHT),
	cityx(4), cityy(2) {

	tiles.resize(width * height);
}
uint8_t Terrain::getSizeX() const { return width; }
uint8_t Terrain::getSizeY() const { return height; }
uint8_t Terrain::getCityX() const { return cityx; }
uint8_t Terrain::getCityY() const { return cityy; }
Tile& Terrain::tile(uint8_t x, uint8_t y) {
	assert(x < width); assert(y < height);
	return tiles[x + (y * width)];
}
const Tile& Terrain::tile(uint8_t x, uint8_t y) const {
	// This should call the non-const version, rather than a tight loop
	return const_cast<Terrain*>(this)->tile(x, y);
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

uint8_t countTilesOfType(const Game& game, int player, const Stock& types) {
	uint8_t count = 0;
	for(uint8_t y = 0; y < game.terrain.getSizeY(); y++) {
		for(uint8_t x = 0; x < game.terrain.getSizeX(); x++) {
			const Tile& tile = game.terrain.tile(x, y);
			if(tile.owned() &&
				((player < 0) || (tile.owner() == player))) {
				
				switch(tile.equipment()) {
					case Resource::NONE:
						if(types.workers){count++;}
						break;
					case Resource::FOOD:
						if(types.food){count++;} break;
					case Resource::ENERGY:
						if(types.energy){count++;}break;
					case Resource::ORE:
						if(types.ore){count++;} break;
					case Resource::CRYSTAL:
						if(types.crystal){count++;}
						break;
				}
			}
		}
	}
	return count;
}
