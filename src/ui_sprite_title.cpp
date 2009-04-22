#include <vector>
#include "platform.hpp"
#include "ui_sprite.hpp"
#include "util.hpp"

class UserInterfaceSpriteTitle : public UserInterfaceSpriteRenderer {
private:
	SDL_Surface* title_text;
	std::vector<unsigned int> title_pixels;
	int title_wmult;
	int title_hmult;
	bool title_cycledir;

public:
	UserInterfaceSpriteTitle() : title_text(0) {
		// TODO move the if(!title_text) bit up here
	}

	~UserInterfaceSpriteTitle() {
		if(title_text) { SDL_FreeSurface(title_text); }
	}

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) {
		
		SDL_Surface* screen = SDL_GetVideoSurface();
		SDL_Rect titlepos = {0, 32, 0, 0};
		bool drawborder = false;
		if(!title_text) { // welcome to the title
			SDL_Color black = {0, 0, 0, 0};
			SDL_FillRect(screen, 0,
				SDL_MapRGB(screen->format, 178, 125, 20));
			title_text = resources.renderText(resources.font_title,
				" M.E.W.L.", black);
			drawborder = true;
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
			// Select a pattern
			/*title_wmult = random_uniform(1, 3);
			title_hmult = random_uniform(0, 5);*/
			/*trace("Title pattern selected: %d, %d",
				title_wmult, title_hmult);*/
			switch(random_uniform(0, 2)) {
				case 1: title_wmult = 1; title_hmult = 2; break;
				case 2: title_wmult = 3; title_hmult = 1; break;
				default: title_wmult = 0; title_hmult = 0;
			}
			title_cycledir = random_uniform(0, 1);
			// TODO intro text with color 55,0,0
		}
		titlepos.x = (screen->w - title_text->w) / 2;
		titlepos.w = title_text->w; titlepos.h = title_text->h;
		if(drawborder) {
			SDL_Color palette[255];
			SDL_Rect borderpos;
			borderpos.w = titlepos.w; borderpos.h = titlepos.h;
			for(Sint16 x = titlepos.x-3; x <= titlepos.x+3; x++) {
				borderpos.x = x;
				for(Sint16 y = titlepos.y-3; y <= titlepos.y+3;
					y++) {
					borderpos.y = y;
					SDL_BlitSurface(title_text, NULL,
						screen, &borderpos);
				}
			}
			SDL_UpdateRect(screen, 0, 0, 0, 0);
			// Now that we've done that, we can redo the palette
			// (leave 0 as transparent, and 1 is now background)
			palette[0].r = 178;
			palette[0].g = 125;
			palette[0].b = 20;
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
				palette[i].r = c[r];
				palette[i].g = c[g];
				palette[i].b = c[b];
			}
			SDL_SetColors(title_text, palette, 1, 255);
		}
		SDL_BlitSurface(title_text, NULL, screen, &titlepos);
		// Colourise some random pixels
		SDL_LockSurface(title_text);
		int k = title_hmult ?
			((title_text->w * title_wmult) /
			 (title_text->h * title_hmult))
			: 0; // sparkle instead
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
		// Redraw the title area
		SDL_UpdateRect(screen,
			titlepos.x, titlepos.y, titlepos.w, titlepos.h);

		return true; // TODO not always
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterfaceSpriteRenderer,UserInterfaceSpriteTitle)

