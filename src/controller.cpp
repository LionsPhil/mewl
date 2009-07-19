#include <algorithm>
#include <typeinfo>
#include "controller.hpp"
#include "platform.hpp"
#include "util.hpp"

Controller::Controller() : fired(false) {}
Controller::~Controller() {}
bool Controller::hadButtonPress() { bool f = fired; fired = false; return f; }

/// Turn -1, 0, 1 axes into a direction
static Direction make_direction(int x, int y) {
	switch(y) {
		case -1: switch(x) {
	case -1: return DIR_NW; case 0: return DIR_N; case 1: return DIR_NE; }
		case  0: switch(x) {
	case -1: return DIR_W; case 0: return DIR_CENTRE; case 1: return DIR_E;}
		case  1: switch(x) {
	case -1: return DIR_SW; case 0: return DIR_S; case 1: return DIR_SE; }
	}
	trace("Impossible direction (%d, %d)", x, y); return DIR_CENTRE;
}

class KeyboardController: public Controller {
	const char* desc;
	SDLKey key[8];
	SDLKey k_fire;
	bool down[8];
public:
	KeyboardController(const char* desc, SDLKey n, SDLKey ne, SDLKey e,
		SDLKey se, SDLKey s, SDLKey sw, SDLKey w, SDLKey nw,SDLKey fire)
		: desc(desc), k_fire(fire) {
		key[DIR_N]= n; key[DIR_NE]= ne; key[DIR_E]= e; key[DIR_SE]= se;
		key[DIR_S]= s; key[DIR_SW]= sw; key[DIR_W]= w; key[DIR_NW]= nw;
		for(int i = 0; i < 8; ++i) { down[i] = false; }
	}

	const char* getDescription() { return desc; }
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
	double calibx, caliby;
	/** Update calibrated position, and return true if in window. */
	bool updatePosition() {
		int mx, my;
		double cx, cy;
		const SDL_VideoInfo* info;
		// Attempt one to identify out-of-window state
		if(!(SDL_GetAppState() & SDL_APPMOUSEFOCUS)) { return false; }
		// Get a calibrated mouse position
		info = SDL_GetVideoInfo();
		SDL_GetMouseState(&mx, &my); // 0,0 aligns with window 0,0
		cx = ((double) mx) / info->current_w;
		cy = ((double) my) / info->current_h;
		// Attempt two to identify out-of-window state
		if(cx < 0 || cx >= 1 || cy < 0 || cy >= 1 ) { return false; }
		// Ok, we think we're inside the window
		calibx = cx; caliby = cx;
		return true;
	}
public:
	MouseController() : calibx(0.5), caliby(0.5) {}

	const char* getDescription() { return "(default)"; }
	bool hasPosition() { return updatePosition(); }

	std::pair<double, double> getPosition() {
		updatePosition();
		return std::pair<double, double>(calibx, caliby);
	}

	Direction getDirection() {
		int x = -1;
		int y = -1;
		updatePosition();
		if(calibx > 0.33) { x = 0; } if(calibx > 0.67) { x = 1; }
		if(caliby > 0.33) { y = 0; } if(caliby > 0.67) { y = 1; }
		return make_direction(x, y);
	}

	void feedEvent(SDL_Event& event) {
		// We don't care about button releases or motion
		if(event.type == SDL_MOUSEBUTTONDOWN &&
			event.button.button == SDL_BUTTON_LEFT)
			{ fired = true; }
	}

	bool operator==(const Controller& other) {
		try {
			MouseController& o = const_cast<MouseController&>
				(dynamic_cast<const MouseController&>(other));
			o.hasPosition(); // Shut up, g++
			return true; // No distinguishing mouse ID
		} catch(std::bad_cast e) { return false; }
	}
};

class JoystickController : public Controller {
	int jindex;
	SDL_Joystick* joy;
	Sint16 jx, jy;
public:
	JoystickController(int index) : jindex(index), jx(0), jy(0) {
		if(!SDL_JoystickOpened(index)) {
			if((joy = SDL_JoystickOpen(index))) {
				trace("Got joystick %d (%s) with %d axes and "
					"%d buttons", index+1, getDescription(),
					SDL_JoystickNumAxes(joy),
					SDL_JoystickNumButtons(joy));
			} else {
				warn("Can't open joystick %d (%s): %s", index+1,
					getDescription(), SDL_GetError());
			}
		}
	}

	~JoystickController() {
		if(SDL_JoystickOpened(jindex)) {
			if(joy) { SDL_JoystickClose(joy); } else {
				warn("Joystick %d (%s) open but handle lost!",
					jindex + 1, getDescription());
			}
		}
	}

	const char* getDescription() {
		const char* name = SDL_JoystickName(jindex);
		if(!name) { name = "(unknown)"; }
		return name;
	}

	bool hasPosition() { return false; }
	std::pair<double, double> getPosition()
		{ return std::pair<double, double>(0, 0); }

	Direction getDirection() {
		// We only need direction, so we can have a massive deadzone
		int x = -1;
		int y = -1;
		if(jx > -16384) { x = 0; } if(jx > 16384) { x = 1; }
		if(jy > -16384) { y = 0; } if(jy > 16384) { y = 1; }
		return make_direction(x, y);
	}

	void feedEvent(SDL_Event& event) {
		uint8_t which = (event.type == SDL_JOYAXISMOTION) ?
			event.jaxis.which : event.jbutton.which;
		if(which != jindex) { return; } // Not this stick
		switch(event.type) {
			case SDL_JOYAXISMOTION:
				switch(event.jaxis.axis) {
					case 0: jx = event.jaxis.value; break;
					case 1: jy = event.jaxis.value; break;
				} // Any other axis is ignored
				break;
			case SDL_JOYBUTTONDOWN:
				/* We are sensitive to the first two buttons,
				 * as on most physical devices I've seen this
				 * should offer some ergonomic flexibility,
				 * without turning random shoulderpads etc. into
				 * potential misfires. */
				if(event.jbutton.button < 2) { fired = true; }
		} // We don't care about button releases
	}

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

	bool found;
	trace("\t%s", controller->getDescription());

	// I am aware of find and find_if, but they are awkward with pointers
	// when I want to perform object comparison instead. And the vector
	// stores pointers because I'm using polymorphism. Want closures, grr.
	found = false;
	for(std::vector<Controller*>::const_iterator i = set.begin();
		!found && (i < set.end()); ++i) {
		if(**i == *controller) { found = true; }
	}

	if(found) { trace("\t\tduplicate ignored"); delete controller; } else {
		set.push_back(controller);
		controllers.push_back(controller);
	}
}

ControlManager::~ControlManager() {
	for_each(controllers.begin(), controllers.end(), delete_functor());
}

void ControlManager::populate() {
	/* Be liberal in detection. There is no penalty for false positives,
	 * as long as they don't lead to false button presses as well. */
	/* Keyboard: three possible, WASD, HJKL, arrows, and numpad. */
	const SDLKey nk = (SDLKey)(SDLK_FIRST - 1); // No key
	trace("Adding keyboard controllers");
	addController(controllers_key, new KeyboardController("WASD+L Ctrl",
		SDLK_w, nk, SDLK_d, nk, SDLK_s, nk, SDLK_a, nk, SDLK_LCTRL));
	addController(controllers_key, new KeyboardController("HJKL+Space",
		SDLK_k, nk, SDLK_l, nk, SDLK_j, nk, SDLK_h, nk, SDLK_SPACE));
	addController(controllers_key, new KeyboardController("Arrows+R Ctrl",
		SDLK_UP, nk, SDLK_RIGHT, nk,
		SDLK_DOWN, nk, SDLK_LEFT, nk, SDLK_RCTRL));
	addController(controllers_key, new KeyboardController("Numpad",
		SDLK_KP8, SDLK_KP9, SDLK_KP6, SDLK_KP3,
		SDLK_KP2, SDLK_KP1, SDLK_KP4, SDLK_KP7, SDLK_KP_ENTER));
	/* Mouse: one positional controller. */
	trace("Adding mouse controller");
	addController(controllers_mouse, new MouseController());
	/* SDL-detected joysticks. */
	trace("Adding joystick controllers");
	for(int i = 0; i < SDL_NumJoysticks(); ++i)
		{ addController(controllers_joy, new JoystickController(i)); }
	/* Wiimotes, etc... */
}

void ControlManager::feedEvent(SDL_Event& event) {
	// I wish STL for_each was useful.
	std::vector<Controller*>* set;
	switch(event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			set = &controllers_key; break;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			set = &controllers_mouse; break;
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			set = &controllers_joy; break;
		default:
			trace("ControlManager being fed unsupported events");
			return;
	}
	for(std::vector<Controller*>::iterator i = set->begin(); i < set->end();
		++i) { (*i)->feedEvent(event); }
}

const std::vector<Controller*>& ControlManager::getControllers()
	{ return controllers; }
