#ifndef GAMESETUP_HPP_
#define GAMESETUP_HPP_

#include <stdint.h>
#include "species.hpp"
#include "controller.hpp"
#include "difficulty.hpp"

const int PLAYERS = 4; // UI is very likely to assume this

class PlayerSetup {
	// Colour comes from player index.
	// Might be interesting to see if we can use Miis instead of Species
	// for the Wii version. Would also allow for names.
public: // This is just a struct with an ego problem
	PlayerSetup();
	Species::Type species;
	bool computer;
	Controller& controller; // meaningless if computer player
};

class GameSetup {
public: // Another fancy-pants struct
	PlayerSetup playersetup[PLAYERS];
	Difficulty::Type difficulty;
};

#endif

