#include "game.hpp"

Player& Player::getDummyPlayer() { static Player p; return p; }

Tile::Tile() : owner(Player::getDummyPlayer()) {}

