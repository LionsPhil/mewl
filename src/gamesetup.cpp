#include "gamesetup.hpp"

PlayerSetup::PlayerSetup() : species(Species::COMPUTER), computer(true),
	controller(NULL) {}

void PlayerSetup::humanPlayer(Controller* controller)
	{ computer = false; this->controller = controller; }

void PlayerSetup::computerPlayer() { computer = true; controller = NULL; }

GameSetup::GameSetup() : difficulty(Difficulty::BEGINNER) {}
