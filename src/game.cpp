#include <assert.h>
#include "game.hpp"

Player& Player::getDummyPlayer() { static Player p; return p; }

Tile::Tile() : m_owner(&Player::getDummyPlayer()) {}
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
std::pair<uint8_t, uint8_t> Terrain::getSize()
	{ return std::pair<uint8_t, uint8_t>(width, height); }
std::pair<uint8_t, uint8_t> Terrain::getCity()
	{ return std::pair<uint8_t, uint8_t>(cityx, cityy); }
Tile& Terrain::tile(uint8_t x, uint8_t y) {
	assert(x < width); assert(y < height);
	return tiles[x + (y * width)];
}

