#include <algorithm>
#include <functional>
#include "controller.hpp"
#include "platform.hpp"

/** Dummy controller for computer players. */
class DummyController : public Controller {
public:
	DummyController() {}
	bool hasPosition() { return false; }
	std::pair<double, double> getPosition()
		{ return std::pair<double, double>(0, 0); }
	Direction getDirection() { return DIR_CENTRE; }
	bool hadButtonPress() { return false; }
	void feedEvent(SDL_Event& event) {}
};

class KeyboardController: public Controller {
	SDLKey k_n, k_ne, k_e, k_se, k_s, k_sw, k_w, k_nw, k_fire;
	bool   d_n, d_ne, d_e, d_se, d_s, d_sw, d_w, d_nw;
	bool fired;
public:
	KeyboardController(SDLKey n, SDLKey ne, SDLKey e, SDLKey se, SDLKey s,
		SDLKey sw, SDLKey w, SDLKey nw, SDLKey fire)
		: k_n(n), k_ne(ne), k_e(e), k_se(se), k_s(s), k_sw(sw), k_w(w),
		k_nw(nw), k_fire(fire), d_n(false), d_ne(false), d_e(false),
		d_se(false), d_s(false), d_sw(false), d_w(false), d_nw(false),
		fired(false) {}

	bool hasPosition() { return false; }
	std::pair<double, double> getPosition()
		{ return std::pair<double, double>(0, 0); }

	Direction getDirection() {
		return DIR_CENTRE;
	}

	bool hadButtonPress() { bool f = fired; fired = false; return f; }

	void feedEvent(SDL_Event& event) {
	}

	bool operator==(KeyboardController& other) {
trace("Compare %p with %p", this, &other);
return
		k_n == other.k_n && k_ne == other.k_ne && k_e == other.k_e &&
		k_se == other.k_se && k_s == other.k_s && k_sw == other.k_sw &&
		k_w == other.k_w && k_nw == other.k_nw && k_fire == other.k_fire
	;}
};

void ControlManager::addController(std::vector<Controller*> set,
	Controller* controller) {
	// I am aware of find and find_if, but they are awkward with pointers
	// when I want to perform object comparison instead. And the vector
	// stores pointers because I'm using polymorphism. Want closures, grr.
	bool found = false;
	for(std::vector<Controller*>::const_iterator i = set.begin();
		i < set.end(); ++i) {
		if(**i == *controller) { found = true; break; }
	}

	if(found) { trace("\tduplicate ignored"); delete controller; } else {
		set.push_back(controller);
		controllers.push_back(controller);
	}
}

static void _deleteCtrlr(Controller* stlfoo) { delete stlfoo; } // sigh
ControlManager::~ControlManager() {
	for_each(controllers.begin(), controllers.end(), _deleteCtrlr);
}

void ControlManager::populate() {
	/* Be liberal in detection. There is no penalty for false positives,
	 * as long as they don't leave to false button presses as well. */
	/* Keyboard: three possible, WASD, HJKL, arrows, and numpad. */
	const SDLKey nk = (SDLKey)(SDLK_FIRST - 1); // No key
	trace("Adding keyboard controllers");
	addController(controllers_key, new KeyboardController(
		SDLK_w, nk, SDLK_d, nk, SDLK_s, nk, SDLK_a, nk, SDLK_LCTRL));
	addController(controllers_key, new KeyboardController(
		SDLK_k, nk, SDLK_l, nk, SDLK_j, nk, SDLK_h, nk, SDLK_SPACE));
	addController(controllers_key, new KeyboardController(
		SDLK_UP, nk, SDLK_RIGHT, nk,
		SDLK_DOWN, nk, SDLK_LEFT, nk, SDLK_RCTRL));
	addController(controllers_key, new KeyboardController(
		SDLK_KP8, SDLK_KP9, SDLK_KP6, SDLK_KP3,
		SDLK_KP2, SDLK_KP1, SDLK_KP4, SDLK_KP7, SDLK_KP_ENTER));
	/* Mouse: one positional controller. */
	/* SDL-detected joysticks. */
}

void ControlManager::feedEvent(SDL_Event& event) {
}

Controller& ControlManager::getDummy()
	{ static DummyController dc; return dc; }

const std::vector<Controller*>& ControlManager::getControllers()
	{ return controllers; }

