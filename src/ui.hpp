#ifndef UI_HPP_
#define UI_HPP_
#include "factory.hpp"

class UserInterface {
public:
	virtual ~UserInterface() {}
	/// Initialise audio and graphics as needed. Return success.
	virtual bool init(bool fullscreen) = 0;

	/* Factory allows implementation detail to be completely opaque. */
	FACTORY_REGISTER_IF(UserInterface)
};

#endif

