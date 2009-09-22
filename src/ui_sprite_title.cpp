#include <vector>
#include <stdlib.h>
#include <SDL_ttf.h>
#include "platform.hpp"
#include "ui_sprite.hpp"
#include "ui_sprite_pointer.hpp"
#include "util.hpp"

/* This isn't an exact mirror of the M.U.L.E. titles (e.g. the text border
 * starts visible, rather than dissolving in; we dissolve the filling instead);
 * it's just supposed to evoke it. */

/* Workaround _Solid failing without an error message. (This happens with
 * SDL_ttf under OS X from Fink, so we do it automatically for OS X. */
#ifdef __APPLE__
# define WORKAROUND_SOLID
#endif

static const SDL_Color background = {178, 125, 20, 0};
static const SDL_Color textcolour = { 55,   0,  0, 0};

static const char* messages[] = { /* "max length " <- ends there */
	"M.E.W.L. version " VERSION,
	"Code: Philip Boulain; Music: DelMurice",
	"Art: Philip Boulain; Font: Mark Simonson",
	" ",

	"Keyboard 1: W, D, S, A, Left control",
	"Keyboard 2: K, L, J, H, Spacebar",
	"Keyboard 3: Arrow keys, Right control",
	"Keyboard 4: Numeric 8, 6, 2, 4, Enter",

	"Mouse and joysticks also supported",
	"Fire to toggle use of controller",
	"Left and right to select difficulty",
	"All players press up to begin",

	" ",
	"Licensed under the GNU GPL",
	"Based on a game by Dani Bunten",
	" ",
0 };

class UserInterfaceSpriteTitle : public UserInterfaceSpriteRenderer {
private:
	SDL_Surface* title_text;
	std::vector<unsigned int> title_pixels;
	SDL_Rect title_pos;
	int title_wmult;
	int title_hmult;
	bool title_cycledir;
	std::vector<UserInterfaceSpriteSprite*> sprites;
	Difficulty::Type last_difficulty;
	PlayerSetup last_playersetup[PLAYERS];
	bool last_playerready[PLAYERS];
	bool first_frame;
	unsigned int music_ticks;
	int music_beats;
	int message_idx;

public:
	void init(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) {

		SDL_Color black = {0, 0, 0, 0};
		last_difficulty = Difficulty::BEGINNER;
		for(int player = 0; player < PLAYERS; player++)
			{ last_playerready[player] = false; }
		first_frame = true;
		message_idx = 0;

		SDL_Surface* screen = SDL_GetVideoSurface();
		// Blank the screen
		SDL_FillRect(screen, 0,
			SDL_MapRGB(screen->format,
				background.r, background.g, background.b));
		// Generate the inner title text
#ifdef WORKAROUND_SOLID
		title_text = TTF_RenderUTF8_Shaded(resources.font_title,
			" M.E.W.L.", background, black);
		SDL_SetColorKey(title_text, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			SDL_MapRGB(title_text->format, 0, 0, 0));
		SDL_SetColors(title_text, const_cast<SDL_Color*>(&background),
			1, 1);
#else
		title_text = TTF_RenderUTF8_Solid(resources.font_title,
			" M.E.W.L.", background);
#endif
		if(!title_text)
			{warn("TTF error (title): %s", TTF_GetError()); die();}
		title_pos.y = 32;
		title_pos.x = (screen->w - title_text->w) / 2;
		title_pos.w = title_text->w; title_pos.h = title_text->h;
		// Draw the outline
		SDL_Rect border_pos;
		SDL_Surface* title_soft;
		title_soft = resources.renderText(resources.font_title,
			" M.E.W.L.", black);
		border_pos.w = title_pos.w; border_pos.h = title_pos.h;
		for(Sint16 x = title_pos.x-3; x <= title_pos.x+3; x++) {
			border_pos.x = x;
			for(Sint16 y = title_pos.y-3; y <= title_pos.y+3; y++) {
				border_pos.y = y;
				SDL_BlitSurface(title_soft, NULL, screen,
					&border_pos);
			}
		}
		SDL_Flip(screen); //SDL_UpdateRect(screen, 0, 0, 0, 0);
		SDL_FreeSurface(title_soft);
		// Draw the inner text
		SDL_BlitSurface(title_text, NULL, screen, &title_pos);
		// Find the pixels to colourise later
		title_pixels.reserve(title_text->w * title_text->h);
		if(title_text->format->BytesPerPixel == 1) {
			SDL_LockSurface(title_text);
			for(int p = 0;
				p < title_text->w*title_text->h; p++) {
#ifdef WORKAROUND_SOLID
				if(((char*) title_text->pixels)[p])
					{ ((char*) title_text->pixels)[p] = 1; }
#endif
				if(((char*) title_text->pixels)[p])
					{ title_pixels.push_back(p); }
			}
			SDL_UnlockSurface(title_text);
		} else { warn("Text didn't render as 1 byte/pixel!"); }
		// Generate a palette
		// (leave 0 as transparent, and 1 as background)
		SDL_Color palette[254];
		int c[3];
		int r = random_uniform(0, 2);
		int g = random_uniform(0, 1);
		if(r == g) { g++; }
		int b;
		for(b = 0; b == r || b == g; b++) {}
		bool leadbright = random_uniform(0, 2); // 2/3 likely
		for(int i = 1; i < 255; i++) {
			int j = leadbright ? i : 255 - i;
			c[0] = j + 128; c[0] = c[0] > 255 ? 255 : c[0];
			c[1] = j;
			c[2] = j - 128; c[2] = c[2] < 0 ? 0 : c[2];
			palette[i-1].r = c[r];
			palette[i-1].g = c[g];
			palette[i-1].b = c[b];
		}
		SDL_SetColors(title_text, palette, 2, 254);
		// Select a pattern from some good presets
		switch(random_uniform(0, 2)) {
			case 1: title_wmult = 1; title_hmult = 2; break;
			case 2: title_wmult = 3; title_hmult = 1; break;
			default: title_wmult = 0; title_hmult = 0;
		}
		title_cycledir = random_uniform(0, 1);
		
		// DEBUG activate all pointer sprites
		for(int i = 0; i < PLAYERS; i++) {
			sprites.push_back(resources.playerpointers[i]);
		}

		// DEBUG set some directions
		resources.playerpointers[1]->direction(DIR_E);
		resources.playerpointers[2]->direction(DIR_SW);
		resources.playerpointers[3]->direction(DIR_S);
	}

	~UserInterfaceSpriteTitle() {
		if(title_text) { SDL_FreeSurface(title_text); }
		
		// Do NOT delete sprites' contents; union of subset of others
		sprites.clear();
	}

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		GameStageState& state, uint32_t ticks,
		UserInterfaceSpriteResources& resources) {
		
		bool beat = false;
		SDL_Surface* screen = SDL_GetVideoSurface();
		
		resources.eraseSprites(sprites);
		
		// Colourise some random pixels
		int k = title_hmult ?
			((title_text->w * title_wmult) /
			 (title_text->h * title_hmult))
			: 0; // sparkle instead
		SDL_LockSurface(title_text);
		for(uint32_t t = 0; !title_pixels.empty() && t < ticks*2; t++) {
			int idx = random_uniform(0, title_pixels.size() - 1);
			int p = title_pixels[idx];
			int c = k ? p * k : random_uniform(0, 253);
			title_pixels.erase(title_pixels.begin() + idx);
			((char*) title_text->pixels)[p] = (c % 254) + 2;
		}
		SDL_UnlockSurface(title_text);
		// Cycle the palette
		if(title_text->format->palette &&
			(title_text->format->palette->ncolors == 256)){

			SDL_Color newpalette[254];
			SDL_Color* oldpalette =
				title_text->format->palette->colors;
			int t = title_cycledir ? ticks : 254 - ticks;
			for(int i = 0; i < 254; i++) {
				newpalette[i] = oldpalette[abs((i+t) % 254) +2];
			}
			SDL_SetColors(title_text, newpalette, 2, 254);
		}
		// Blit, because we don't write directly to screen (it's 32-bit)
		SDL_BlitSurface(title_text, NULL, screen, &title_pos);
		// Redraw the title area
		resources.updateRect(title_pos.x, title_pos.y,
			title_pos.w, title_pos.h);

		// Let there be music
		if(Mix_PlayingMusic()) {
			int newbeats;
			long unsigned int bpmticks =
				resources.music_theme_bpm * music_ticks;
			music_ticks += ticks;
			newbeats = bpmticks / 6000;
			if(newbeats > music_beats) {
				beat = true;
				music_beats = newbeats;
			}
		} else if(resources.music_theme) {
			Mix_PlayMusic(resources.music_theme, 1);
			music_ticks = 0;
			music_beats = -1;
			message_idx = 0; // It's the right length to resync
		}

		// TODO Draw characters running about etc.
		// DEBUG dance the mouse pointers for sprite debug
		resources.playerpointers[0]->move(
			(music_ticks % 670)-31,
			((music_beats % 2) * 12) + 128);
		resources.playerpointers[1]->move(
			(((int)(0.9*music_ticks)) % 670)-31 + 4,
			((music_beats % 3) * 12) + 128 + 16);
		resources.playerpointers[2]->move(
			(((int)(0.8*music_ticks)) % 670)-31 + 8,
			((music_beats % 5) * 12) + 128 + 32);
		resources.playerpointers[3]->move(
			320, //(((int)(0.7*music_ticks)) % 670)-31 + 12,
			((music_beats % 4) * 12) + 128 + 48);

		// Draw cycling message bar
		if(beat && ((music_beats % 4) == 0)) {
			const char* message = messages[message_idx];
			if(!message) {
				message_idx = 0;
				message = messages[0];
			}
			message_idx++;			

			resources.displayTextLine(resources.font_small,
				message, textcolour, background, 384);
		}

		// Draw the GameSetup
		if(first_frame || last_difficulty != setup.difficulty) {
			std::string difftext(ARROW_LEFT " ");
			difftext += Difficulty::getName(setup.difficulty);
			difftext += " " ARROW_RIGHT;
			resources.displayTextLine(resources.font_small,
				difftext.c_str(), textcolour, background, 408);
		}
		// TODO
		{
/*			resources.displayTextLine(resources.font_small,
				"mouse key1 key2 joy1",
				textcolour, background, 432); */

			resources.displayTextLine(resources.font_small,
				"      key1          ",
				UserInterfaceSpriteConstants::col_text_white,
				background, 432);
			// TODO put each in a 1/4 of the screen, white if ready
			// logic needs to keep active set to four, dropping out
			// oldest. put "CPU" in place in gold if <4.
		}
		// Remember the last GameSetup state
		last_difficulty = setup.difficulty;
		for(int player = 0; player < PLAYERS; player++) {
			last_playersetup[player] = setup.playersetup[player];
			last_playerready[player] =
				state.title.playerready[player];
		}
		
		resources.displaySprites(sprites);
		first_frame = false;

		return true; // TODO Put a music fadeout on the colour stage
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterfaceSpriteRenderer,UserInterfaceSpriteTitle)

