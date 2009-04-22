/* NOTE: hash_map is not strictly standard (yet); it is a common extension.
 * Apparently Visual Studio of late has moved it out of std:: into stdext::,
 * so if this code won't compile in Microsoftland, that may be your problem.
 * (Add another special case, as GNU G++ has forced us to, hurrah.) */
#ifdef __GNUC__
#  include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#  include <hash_map>
using std::hash_map;
#endif
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "ui.hpp"
#include "platform.hpp"
#include "util.hpp"

class UserInterfaceSprite : public UserInterface {
	bool fullscreen;
	TTF_Font* fonttitle;
	TTF_Font* fontlarge;
	TTF_Font* fontsmall;
	Mix_Music* music;
	hash_map<const char*, Mix_Chunk*> samples;
	hash_map<const char*, SDL_Surface*> textures;

	SDL_Surface* title_text;
	std::vector<unsigned int> title_pixels;
	int title_wmult;
	int title_hmult;
	bool title_cycledir;

	~UserInterfaceSprite() {
		int dum1; Uint16 dum2; int dum3;
		// Free samples and textures (can has C++0x type inference plz?)
		if(music) { Mix_FreeMusic(music); music = NULL; }
		for(hash_map<const char*, Mix_Chunk*>::iterator i =
			samples.begin(); i != samples.end(); i++) {
				Mix_FreeChunk(i->second); }
		samples.clear();
		for(hash_map<const char*, SDL_Surface*>::iterator i =
			textures.begin(); i != textures.end(); i++) {
				SDL_FreeSurface(i->second); }
		textures.clear();
		// Free other resources
		if(title_text) { SDL_FreeSurface(title_text); }
		// Shutdown TTF and Mixer
		if(TTF_WasInit()) { TTF_Quit(); }
		if(Mix_QuerySpec(&dum1, &dum2, &dum3)) { Mix_CloseAudio(); }
	}

private:
	const char* getDataDir() { return "data/"; }
	const char* findFontFile() { return "data/mainfont.ttf"; }
	const char* findMusicFile() { return "data/theme.mp3"; }

	SDL_Surface* renderText(TTF_Font* font, const char* text,
		SDL_Color colour) {

		SDL_Surface* s;
		s = TTF_RenderUTF8_Solid(font, text, colour);
		//s = TTF_RenderUTF8_Blended(font, text, colour);
		if(!s) { warn("TTF error: %s", TTF_GetError()); }
		return s;
	}

	bool loadSample(const char* name) {
		std::string file(getDataDir());
		file += name;
		Mix_Chunk* sample = Mix_LoadWAV(file.c_str());
		if(sample) {
			samples[name] = sample;
			return true;
		} else {
			warn("Unable to load sample: %s", Mix_GetError());
			return false;
		}
	}

	bool loadTexture(const char* name) {
		std::string file(getDataDir());
		file += name;
		SDL_Surface* texture = IMG_Load(file.c_str());
		if(texture) {
			textures[name] = texture;
			return true;
		} else {
			warn("Unable to load texture: %s", IMG_GetError());
			return false;
		}
	}

	bool setupVideo() {
		if(SDL_SetVideoMode(640, 480, 0,
			SDL_HWPALETTE | (fullscreen ? SDL_FULLSCREEN : 0))) {
			trace("Got %s video at %dbpp",
				(fullscreen ? "fullscreen" : "windowed"),
				SDL_GetVideoSurface()->format->BitsPerPixel);
			return true;
		} else {
			warn("Unable to set video mode: %s", SDL_GetError());
			return false;
		}
	}

public:
	bool init(bool fullscreen) {
		// Initialise the SDL subsystems
		if(SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
			warn("Unable to initialise SDL: %s", SDL_GetError());
			return false;
		}
		// Set up the window (icon must be done before video mode)
		SDL_WM_SetCaption("M.E.W.L.", "M.E.W.L.");
		// SDL_WM_SetIcon(...32x32 surface..., NULL); // TODO
		// Set the video mode
		this->fullscreen = fullscreen;
		if(!setupVideo()) { return false; }
		// Initialise mixer
		if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
			warn("Unable to initialise Mixer: %s", Mix_GetError());
			return false;
		}
		// Initialise TTF
		if(TTF_Init() < 0) {
			warn("Unable to initialise TTF: %s", TTF_GetError());
			return false;
		}
		// Load font
		const char* fontfile = findFontFile();
		fonttitle = TTF_OpenFont(fontfile, 64);
		fontlarge = TTF_OpenFont(fontfile, 24);
		fontsmall = TTF_OpenFont(fontfile, 16);
		if(!fonttitle || !fontlarge || !fontsmall) {
			warn("Unable to load font: %s", TTF_GetError());
			return false;
		}
		// Load sprite textures (failure is nonfatal) TODO
		// Load audio samples (failure is nonfatal) TODO
		// Load music (failure is nonfatal)
		if(!(music = Mix_LoadMUS(findMusicFile())))
			{ warn("Unable to load music: %s", Mix_GetError()); }
		// Init other state
		title_text = 0;
		// Done
		return true;
	}

	void toggleFullscreen() { fullscreen = !fullscreen; setupVideo(); }

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks) {
		
		// TODO temp--move to conditional on TITLE stage
		SDL_Surface* screen = SDL_GetVideoSurface();
		SDL_Rect titlepos = {0, 32, 0, 0};
		bool drawborder = false;
		if(!title_text) { // welcome to the title
			SDL_Color black = {0, 0, 0, 0};
			SDL_FillRect(screen, 0,
				SDL_MapRGB(screen->format, 178, 125, 20));
			title_text = renderText(fonttitle, " M.E.W.L.", black);
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
		// TODO zap title_border and clear title_pixels once title over
		
		return true; // TODO not always
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterface,UserInterfaceSprite)

