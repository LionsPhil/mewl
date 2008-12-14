#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

#include <vector>
#include <utility> // (pair)
#include <SDL.h>

typedef enum {
	DIR_CENTRE, DIR_N, DIR_NE, DIR_E, DIR_SE, DIR_S, DIR_SW, DIR_W, DIR_NW
} Direction;

/** A Controller is any of various possible devices which can be used to
 * control the game: mouse, keyboard, joystick, Wiimote... */
class Controller {
public:
	/* Is the controller capable of reporting a screen position right now?
	 * This may change repeatedly during the use of a controller, due to
	 * grabless mice and Wiimotes pointed away from the screen. */
	virtual bool hasPosition() = 0;
	/* What is the screen position pointed to (for a top-left origin,
	 * positive Y downwards, range 0-1)? If not hasPosition(), should report
	 * the last position held. If no position ever held, not defined. */
	virtual std::pair<double, double> getPosition() = 0;
	/* Get a direction value. For devices with no directional stick,
	 * interpret the position as a direction based on the nearest screen
	 * edge. Remember to leave an area in the centre. */
	virtual Direction getDirection() = 0;
	/* Return true if the main button (whichever one[s] you wish to make
	 * sensitive) has been pressed since this function was last called.
	 * Button presses should latch a flag which this function clears so as
	 * to avoid double or missed presses from polling. */
	virtual bool hadButtonPress() = 0;

	/** Process SDL event. For the ControlManager. */
	virtual void feedEvent(SDL_Event& event) = 0;
	/** Should also overload operator== so that ControlManager can detect
	 * duplicates. Two controllers for the same input are equal. */
	virtual bool operator==(const Controller& other) { return false; }
};

/** The ControlManager owns all Controller instances, and is responsible for
 * probing hardware and creating them in the first place. */
class ControlManager {
	/* Specific sets allow us to avoid spamming everything with all events*/
	std::vector<Controller*> controllers_key;
	std::vector<Controller*> controllers_mouse;
	std::vector<Controller*> controllers_joy;
	/** All of the controllers; this one owns the memory */
	std::vector<Controller*> controllers;
	/** Add a controller, with duplicate testing. Takes ownership. */
	void addController(std::vector<Controller*>& set,
		Controller* controller);
public:
	~ControlManager();
	/** Populate the list of controllers. To avoid invalidating controllers
	 * which may be used in PlayerSetups, this will only ever add to the
	 * vector, and will avoid creating duplicates. Designed to allow for
	 * late-connected devices. */
	void populate();
	/** Provide an SDL event so that controller states can be updated. */
	void feedEvent(SDL_Event& event);
	/** Get the dummy controller for computer players. There is only ever
	 * one (there need be no more, and it's useful for construction). */
	static Controller& getDummy();
	/** Get const access to the set of controllers. */
	const std::vector<Controller*>& getControllers();
};

#endif

