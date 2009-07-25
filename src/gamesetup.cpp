#include "gamesetup.hpp"

PlayerSetup::PlayerSetup() : species(Species::COMPUTER), computer(true),
	controller(NULL) {}

GameSetup::GameSetup() : difficulty(Difficulty::BEGINNER) {}
