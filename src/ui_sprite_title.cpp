#include <vector>
#include <SDL_ttf.h>
#include "platform.hpp"
#include "ui_sprite.hpp"
#include "util.hpp"

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
	unsigned int music_ticks;
	int music_beats;
	int message_idx;

public:
	void init(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) {

		message_idx = 0;

		SDL_Surface* screen = SDL_GetVideoSurface();
		// Blank the screen
		SDL_FillRect(screen, 0,
			SDL_MapRGB(screen->format,
				background.r, background.g, background.b));
		// Generate the inner title text
		title_text = TTF_RenderUTF8_Solid(resources.font_title,
			" M.E.W.L.", background);
		if(!title_text)
			{warn("TTF error (title): %s", TTF_GetError()); die();}
		title_pos.y = 32;
		title_pos.x = (screen->w - title_text->w) / 2;
		title_pos.w = title_text->w; title_pos.h = title_text->h;
		// Draw the outline
		SDL_Rect border_pos;
		SDL_Surface* title_soft;
		SDL_Color black = {0, 0, 0, 0};
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
		SDL_UpdateRect(screen, 0, 0, 0, 0);
		SDL_FreeSurface(title_soft);
		// Draw the inner text
		SDL_BlitSurface(title_text, NULL, screen, &title_pos);
		// Find the pixels to colourise later
		title_pixels.reserve(title_text->w * title_text->h);
		if(title_text->format->BytesPerPixel == 1) {
			SDL_LockSurface(title_text);
			for(int p = 0;
				p < title_text->w*title_text->h; p++) {
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
		for(b = 0; b == r || b == g; b++);
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

		// TODO intro text with color 55,0,0
	}

	~UserInterfaceSpriteTitle() {
		if(title_text) { SDL_FreeSurface(title_text); }
	}

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) {
		
		bool beat = false;
		SDL_Surface* screen = SDL_GetVideoSurface();
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
				newpalette[i] = oldpalette[((i + t) % 254) + 2];
			}
			SDL_SetColors(title_text, newpalette, 2, 254);
		}
		// Blit, because we don't write directly to screen (it's 32-bit)
		SDL_BlitSurface(title_text, NULL, screen, &title_pos);
		// Redraw the title area
		SDL_UpdateRect(screen,
			title_pos.x, title_pos.y, title_pos.w, title_pos.h);

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
		}

		// TODO Draw characters running about etc.

		// Draw cycling message bar
		if(beat && ((music_beats % 4) == 0)) {
			const char* message = messages[message_idx];
			if(!message) {
				message_idx = 0;
				message = messages[0];
			}
			message_idx++;			

			SDL_Rect bar = {0, 384, 640, 0};
			SDL_Surface* text = resources.renderText(
				resources.font_small, message, textcolour);
			bar.h = text->h;
			SDL_FillRect(screen, &bar, SDL_MapRGB(screen->format,
				background.r, background.g, background.b));
			bar.w = text->w;
			bar.x = (screen->w - bar.w) / 2;
			SDL_BlitSurface(text, NULL, screen, &bar);
			SDL_FreeSurface(text);
			SDL_UpdateRect(screen, 0, 384, 640, bar.h);
		}

		// TODO Draw the GameSetup

		return true; // TODO Put a music fadeout on the colour stage
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterfaceSpriteRenderer,UserInterfaceSpriteTitle)

