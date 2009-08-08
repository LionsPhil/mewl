#ifndef UI_SPRITE_POINTER_
#define UI_SPRITE_POINTER_
#include "controller.hpp" // for Direction
#include "ui_sprite.hpp"

/** Dynamic sprite which renders as an appropriate pointer graphic.
 *
 * The pointer-*.png files (generated from SVG) are not raw graphics; they need
 * to be recoloured, which is what this class is for. The channel mapping is:
 *  Red   -> Player colour
 *  Green -> Border colour (white)
 *  Blue  -> Unused
 *  Alpha -> Alpha
 * We also need to generate the other directions by rotating the images.
 * The pointer hotspot is assumed to be in the dead centre, and rendering will
 * be offset to allow for this. */
class UserInterfaceSpritePointer : public UserInterfaceSpriteSprite {
private:
	SDL_Surface* pixmaps[9]; ///< Array indicies are Direction enum values
	SDL_Rect offset; ///< Hotspot adjustment to make to move()

	/// Remaps the channels into a blended colour as per class docs
	Uint32 recolour(SDL_PixelFormat *fmt, Uint32 pixel, SDL_Color player);
	/// Copies to a new surface, run through recolour
	SDL_Surface* copyRecoloured(SDL_Surface* source, SDL_Color player);
	/// Copies to a new surface, rotated 90 degrees clockwise
	SDL_Surface* copyRotated(SDL_Surface* source);

public:
	UserInterfaceSpritePointer(UserInterfaceSpriteResources& resources,
		SDL_Color player, SDL_Surface* centre, SDL_Surface* north,
		SDL_Surface* northeast);
	virtual ~UserInterfaceSpritePointer();

	/** Update the direction of the player's controller so that the correct
	 *  sprite will be used. */
	void direction(Direction dir);
	/** Position the sprite's *hotspot*. Again, not while drawn. */
	virtual void move(Sint16 x, Sint16 y);
};

#endif

