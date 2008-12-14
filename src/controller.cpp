#include "controller.hpp"

/** Dummy controller for computer players. */
class DummyController : public Controller {
public:
	DummyController() {}
	bool hasPosition() { return false; }
	std::pair<double, double> getPosition()
		{ return std::pair<double, double>(0, 0); }
	Direction getDirection() { return DIR_CENTRE; }
	bool hadButtonPress() { return false; }
};

Controller& ControlManager::getDummy()
	{ static DummyController dc; return dc; }

