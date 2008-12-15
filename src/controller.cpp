#include <algorithm>
#include <typeinfo>
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
	bool operator==(const Controller& other) { return false; }
};

class KeyboardController: public Controller {
	SDLKey key[8];
	SDLKey k_fire;
	bool down[8];
	bool fired;
public:
	KeyboardController(SDLKey n, SDLKey ne, SDLKey e, SDLKey se, SDLKey s,
		SDLKey sw, SDLKey w, SDLKey nw, SDLKey fire)
		: k_fire(fire), fired(false) {
		key[DIR_N]= n; key[DIR_NE]= ne; key[DIR_E]= e; key[DIR_SE]= se;
		key[DIR_S]= s; key[DIR_SW]= sw; key[DIR_W]= w; key[DIR_NW]= nw;
		for(int i = 0; i < 8; ++i) { down[i] = false; }
	}

	bool hasPosition() { return false; }
	std::pair<double, double> getPosition()
		{ return std::pair<double, double>(0, 0); }

	Direction getDirection() {
		/* We give diagonals priority to assist 'rolling' through
		 * adjacent directions (and for simplicity). If you wibble
		 * between opposite directions, you will find one dominant.
		 * The only fix I can see for that would be to keep a MRU of
		 * keypresses, which would appear to be massive overkill.
		 * Merely having individual flags (vs. setting a direction in
		 * keydown/keyup) already lets us allow 'tapped' diagonals. */
		if(down[DIR_NE] ||(down[DIR_N] && down[DIR_E])) {return DIR_NE;}
		if(down[DIR_SE] ||(down[DIR_S] && down[DIR_E])) {return DIR_SE;}
		if(down[DIR_SW] ||(down[DIR_S] && down[DIR_W])) {return DIR_SW;}
		if(down[DIR_NW] ||(down[DIR_N] && down[DIR_W])) {return DIR_NW;}
		if(down[DIR_N]) { return DIR_N; }
		if(down[DIR_E]) { return DIR_E; }
		if(down[DIR_S]) { return DIR_S; }
		if(down[DIR_W]) { return DIR_W; }
		return DIR_CENTRE;
	}

	bool hadButtonPress() { bool f = fired; fired = false; return f; }

	void feedEvent(SDL_Event& event) {
		if(event.key.keysym.sym == k_fire) {
			if(event.type == SDL_KEYDOWN) { fired = true; }
			return;
		}
		for(int i = 0; i < 8; ++i) {
			if(event.key.keysym.sym == key[i])
				{ down[i] = (event.type == SDL_KEYDOWN); }
		}
	}

	bool operator==(const Controller& other) {
		try {
			const KeyboardController& o =
				dynamic_cast<const KeyboardController&>(other);
			if(k_fire != o.k_fire) { return false; }
			for(int i = 0; i < 8; ++i)
				{ if(key[i] != o.key[i]) { return false; } }
			return true;
		} catch(std::bad_cast e) { return false; }
	}
};

class MouseController : public Controller {
public:
	MouseController() {}
	bool hasPosition() { return true; }
	std::pair<double, double> getPosition()
		{ return std::pair<double, double>(0, 0); } // TODO
	Direction getDirection() { return DIR_CENTRE; } // TODO
	bool hadButtonPress() { return false; } // TODO
	void feedEvent(SDL_Event& event) {} // TODO
	bool operator==(const Controller& other) {
		try {
			MouseController& o = const_cast<MouseController&>
				(dynamic_cast<const MouseController&>(other));
			o.hasPosition(); // Shut up, g++
			return true; // No distinguishing state
		} catch(std::bad_cast e) { return false; }
	}
};

class JoystickController : public Controller {
	int jindex;
	SDL_Joystick* joy;
public:
	JoystickController(int index) : jindex(index) {
		const char* name = SDL_JoystickName(index);
		if(!name) { name = "unknown"; }
		if(!SDL_JoystickOpened(index)) {
			if((joy = SDL_JoystickOpen(index))) {
				trace("Got joystick %d (%s) with %d axes and "
					"%d buttons", index + 1, name,
					SDL_JoystickNumAxes(joy),
					SDL_JoystickNumButtons(joy));
			} else {
				warn("Can't open joystick %d (%s): %s",
					index + 1, name, SDL_GetError());
			}
		}
	}

	~JoystickController() {
		if(SDL_JoystickOpened(jindex)) {
			if(joy) { SDL_JoystickClose(joy); } else {
				const char* name = SDL_JoystickName(jindex);
				if(!name) { name = "unknown"; }
				warn("Joystick %d (%s) open but handle lost!",
					jindex + 1, name);
			}
		}
	}

	bool hasPosition() { return false; }
	std::pair<double, double> getPosition()
		{ return std::pair<double, double>(0, 0); }
	Direction getDirection() { return DIR_CENTRE; } // TODO
	bool hadButtonPress() { return false; } // TODO
	void feedEvent(SDL_Event& event) {} // TODO
	bool operator==(const Controller& other) {
		try {
			const JoystickController& o =
				dynamic_cast<const JoystickController&>(other);
			return jindex == o.jindex;
		} catch(std::bad_cast e) { return false; }
	}
};

void ControlManager::addController(std::vector<Controller*>& set,
	Controller* controller) {
	// I am aware of find and find_if, but they are awkward with pointers
	// when I want to perform object comparison instead. And the vector
	// stores pointers because I'm using polymorphism. Want closures, grr.
	bool found = false;
	for(std::vector<Controller*>::const_iterator i = set.begin();
		!found && (i < set.end()); ++i) {
		if(**i == *controller) { found = true; }
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
	trace("Adding mouse controller");
	addController(controllers_mouse, new MouseController());
	/* SDL-detected joysticks. */
	trace("Adding joystick controllers");
	for(int i = 0; i < SDL_NumJoysticks(); ++i)
		{ addController(controllers_joy, new JoystickController(i)); }
}

void ControlManager::feedEvent(SDL_Event& event) {
}

Controller& ControlManager::getDummy()
	{ static DummyController dc; return dc; }

const std::vector<Controller*>& ControlManager::getControllers()
	{ return controllers; }

/*
		k_n == other.k_n && k_ne == other.k_ne && k_e == other.k_e &&
		k_se == other.k_se && k_s == other.k_s && k_sw == other.k_sw &&
		k_w == other.k_w && k_nw == other.k_nw && k_fire == other.k_fire
virtual, overloaded operator== without RTTI overhead for everything...hahahahaha
*/

