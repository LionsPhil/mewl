#include "ui_sprite.hpp"
#include "ui_sprite_pointer.hpp"

// This covers both the Colour and Species stages, to coalesce tiny files

class UserInterfaceSpriteColour : public UserInterfaceSpriteRenderer {
private:
//	bool first_frame;

public:
	void init(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) {

		SDL_Surface* screen = SDL_GetVideoSurface();
		// Blank the screen
		SDL_FillRect(screen, 0,
			SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_Flip(screen);
		// TODO
	}

	~UserInterfaceSpriteColour() { }

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		GameStageState& state, uint32_t ticks,
		UserInterfaceSpriteResources& resources) {

		return true; // TODO
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterfaceSpriteRenderer,UserInterfaceSpriteColour)

