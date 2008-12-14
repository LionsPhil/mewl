#include "gamesetup.hpp"

PlayerSetup::PlayerSetup() : species(Species::COMPUTER), computer(true),
	controller(ControlManager::getDummy()) {}

