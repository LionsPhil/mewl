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
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "ui.hpp"
#include "platform.hpp"

class UserInterfaceSprite : public UserInterface {
	bool fullscreen;
	TTF_Font* fonttitle;
	TTF_Font* fontlarge;
	TTF_Font* fontsmall;
	Mix_Music* music;
	hash_map<const char*, Mix_Chunk*> samples;
	hash_map<const char*, SDL_Surface*> textures;

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
		//s = TTF_RenderUTF8_Solid(font, text, colour);
		s = TTF_RenderUTF8_Blended(font, text, colour);
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
			SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0))) {
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
		// Done
		return true;
	}

	void toggleFullscreen() { fullscreen = !fullscreen; setupVideo(); }
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterface,UserInterfaceSprite)

