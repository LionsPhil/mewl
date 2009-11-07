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
	Controller* controller; // meaningless if computer player
	// Use these to set the above to keep comp => controller==NULL invariant
	void humanPlayer(Controller* controller);
	void computerPlayer();
};
inline bool operator==(const PlayerSetup& one, const PlayerSetup& two) {
	return one.species == two.species
		&& one.computer == two.computer
		&& one.controller == two.controller;
}

class GameSetup {
public: // Another fancy-pants struct
	GameSetup();
	PlayerSetup playersetup[PLAYERS];
	Difficulty::Type difficulty;
};

#endif

