#ifndef UI_SPRITE_HPP_
#define UI_SPRITE_HPP_
/* NOTE: hash_map is not strictly standard (yet); it is a common extension.
 * Apparently Visual Studio of late has moved it out of std:: into stdext::,
 * so if this code won't compile in Microsoftland, that may be your problem.
 * (Add another special case, as GNU G++ has forced us to, hurrah.)
 * Of course, GNU being GNU, they changed it again some point during v4, and
 * made the replacement also marked down as TR1 unstable fun. */
#ifdef __GNUC__
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#    undef __DEPRECATED
/*#    include <unordered_map>
typedef tr1::unordered_map hash_map;*/
#  endif
#  include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#  include <hash_map>
using std::hash_map;
#endif
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "factory.hpp"
#include "game.hpp"
#include "gamesetup.hpp"

/* Note that this is NOT a header for ui_sprite per se, as that is a subclass
 * which is instantiated via a factory---it isn't known to main, etc. This
 * header is instead for the other components of ui_sprite to know about the
 * common resources it provides. */

/* These low control characters are where the Atari arrows are mapped in the
 * font. (This is where they're documented; FontForge was needed to move them
 * into place, as they were /originally/ at very high codepoints.) */
#define ARROW_UP    "\x1c"
#define ARROW_DOWN  "\x1d"
#define ARROW_LEFT  "\x1e"
#define ARROW_RIGHT "\x1f"

namespace UserInterfaceSpriteConstants {
	const SDL_Color col_player[] = {
		{ 26,  68, 155, 0},
		{140,  33,  35, 0},
		{ 39, 105,   0, 0},
		{106,  26, 159, 0}};
	const SDL_Color col_text_black = { 32,  31,  32, 0};
	const SDL_Color col_text_gray  = {130, 129, 130, 0};
	const SDL_Color col_text_white = {197, 197, 197, 0};
	const SDL_Color col_text_gold  = {161,  85,  18, 0};
	const SDL_Color col_text_red   = {109,   0,   3, 0};
	const SDL_Color col_text_green = { 39, 105,   0, 0};
	const SDL_Color col_text_blue  = { 13,  21, 144, 0};
};

struct UserInterfaceSpriteResources {
	TTF_Font* font_title;
	TTF_Font* font_large;
	TTF_Font* font_small;
	Mix_Music* music_theme;
	Uint16 music_theme_bpm;
	hash_map<const char*, Mix_Chunk*> samples;
	hash_map<const char*, SDL_Surface*> textures;

	/** Render some text in a sprite to a new surface. */
	SDL_Surface* renderText(TTF_Font* font, const char* text,
		SDL_Color colour);
	/** Render a full-width line of text to the screen. */
	bool displayTextLine(TTF_Font* font, const char* text,
		SDL_Color foreground, SDL_Color background, int y);
	/* TODO utility method to render a pointer for mouse/wiimotes when
	 * their controller is enabled + in use (title, develop, auctions...) */
};

class UserInterfaceSpriteRenderer {
public:
	virtual inline ~UserInterfaceSpriteRenderer() {}
	/// Init after construction (factory pattern); can do initial render
	virtual inline void init(GameStage::Type stage, GameSetup& setup, Game*
		game, uint32_t ticks, UserInterfaceSpriteResources& resources){}
	/// Normal rendering (passthrough of UI-level render())
	virtual bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) = 0;

	/* It's time for that delicious factory pattern again, folks. */
	FACTORY_REGISTER_IF(UserInterfaceSpriteRenderer)
};

#endif

