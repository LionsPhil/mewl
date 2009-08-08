#include <assert.h>
#include "ui_sprite_pointer.hpp"
#include "platform.hpp"

UserInterfaceSpritePointer::UserInterfaceSpritePointer(
	UserInterfaceSpriteResources& resources, SDL_Color player,
	SDL_Surface* centre, SDL_Surface* north, SDL_Surface* northeast) :
	UserInterfaceSpriteSprite(resources, centre) {
	
	// Validate that images are same size, or this will go VERY wrong
	assert(centre && north && northeast);
	if(!(
		(centre->w == north->w) && (centre->w == northeast->w) &&
		(centre->w == centre->h) && (centre->w == north->h) &&
		(centre->w == northeast->h))) { // (...a==b==c in maths terms)
		warn("Pointer graphics are not equal size and square; built "
			"incorrectly from SVG? (%dx%d, %dx%d, %dx%d)",
			centre->w, centre->h, north->w, north->h,
			northeast->w, northeast->h);
		die();
	}
	if(!((   centre->format->BitsPerPixel == 32) &&
	     (    north->format->BitsPerPixel == 32) &&
	     (northeast->format->BitsPerPixel == 32))) {
		warn("Pointer graphics are not 32bpp; built incorrectly from "
			"SVG?");
		die();
	}
	
	// Calculate the offset
	offset.x = -(centre->w / 2);
	offset.y = -(centre->h / 2);
	
	// Recolour the unrotated directions
	pixmaps[DIR_CENTRE] = copyRecoloured(centre,    player);
	pixmaps[DIR_N     ] = copyRecoloured(north,     player);
	pixmaps[DIR_NE    ] = copyRecoloured(northeast, player);
	
	// Rotate the other directions from our recoloured ones
	// (interleaving the directions thrashes cache locality less)
	pixmaps[DIR_E ] = copyRotated(pixmaps[DIR_N ]);
	pixmaps[DIR_S ] = copyRotated(pixmaps[DIR_E ]);
	pixmaps[DIR_W ] = copyRotated(pixmaps[DIR_S ]);
	pixmaps[DIR_SE] = copyRotated(pixmaps[DIR_NE]);
	pixmaps[DIR_SW] = copyRotated(pixmaps[DIR_SE]);
	pixmaps[DIR_NW] = copyRotated(pixmaps[DIR_SW]);

	/* We initialise the base class to the _raw_ centre graphic (sprites
	 * don't own provided textures, so that's safe---it just wants to make
	 * a background store the right size). So now we need to set a sane
	 * state that's the right colour. */
	direction(DIR_CENTRE);
}

UserInterfaceSpritePointer::~UserInterfaceSpritePointer() {
	for(int dir = 0; dir < 9; dir++) { SDL_FreeSurface(pixmaps[dir]); }
}

void UserInterfaceSpritePointer::direction(Direction dir) {
	pixmap = pixmaps[dir];
}

void UserInterfaceSpritePointer::move(Sint16 x, Sint16 y) {
	pos.x = x + offset.x;
	pos.y = y + offset.y;
}

Uint32 UserInterfaceSpritePointer::recolour(SDL_PixelFormat *fmt, Uint32 pixel,
	SDL_Color player) {

	Uint8 mixplayer, mixborder, mixunused, mixalpha;
	Uint16 r, g, b, a; // Want saturating arithmetic, so oversize types

	SDL_GetRGBA(pixel, fmt, &mixplayer, &mixborder, &mixunused, &mixalpha);

	r = ((player.r * mixplayer) / 255) + mixborder;
	g = ((player.g * mixplayer) / 255) + mixborder;
	b = ((player.b * mixplayer) / 255) + mixborder;
	r = r > 255 ? 255 : r;
	g = g > 255 ? 255 : g;
	b = b > 255 ? 255 : b;
	a = mixalpha;

	return SDL_MapRGBA(fmt, r, g, b, a);
}

SDL_Surface* UserInterfaceSpritePointer::copyRecoloured(SDL_Surface* source,
	SDL_Color player) {

	SDL_Surface* dest;
	SDL_PixelFormat* format = pixmap->format;
	Uint32* pixsource;
	Uint32* pixdest;
	int pixcount;

	dest = SDL_CreateRGBSurface(SDL_HWSURFACE, source->w, source->h, 32,
		format->Rmask, format->Gmask, format->Bmask, format->Amask);
	pixcount = source->w * source->h;

	SDL_LockSurface(source);
	SDL_LockSurface(dest);
	pixsource = (Uint32*) source->pixels;
	pixdest   = (Uint32*) dest->pixels;
	for(int i = 0; i < pixcount; i++)
		{ pixdest[i] = recolour(format, pixsource[i], player); }
	SDL_UnlockSurface(dest);
	SDL_UnlockSurface(source);
	return dest;
}

SDL_Surface* UserInterfaceSpritePointer::copyRotated(SDL_Surface* source) {
	SDL_Surface* dest;
	SDL_PixelFormat* format = pixmap->format;
	Uint32* pixsource;
	Uint32* pixdest;
	int divpitch;

	dest = SDL_CreateRGBSurface(SDL_HWSURFACE, source->w, source->h, 32,
		format->Rmask, format->Gmask, format->Bmask, format->Amask);
	divpitch   = source->pitch / 4; // counter the Uint32 [] 4x byte offset

	SDL_LockSurface(source);
	SDL_LockSurface(dest);
	pixsource = (Uint32*) source->pixels;
	pixdest   = (Uint32*) dest->pixels;
	for(int y = 0; y < source->h; y++) {
		for(int x = 0; x < source->w; x++) {
			int x2 = source->w - (y + 1);
			int y2 = x;
			pixdest[x2 + (y2 * divpitch)] =
				pixsource[x + (y * divpitch)];
		}
	}
	SDL_UnlockSurface(dest);
	SDL_UnlockSurface(source);
	return dest;
}

