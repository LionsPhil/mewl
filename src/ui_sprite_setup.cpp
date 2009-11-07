#include "ui_sprite.hpp"
#include "ui_sprite_pointer.hpp"

// This covers both the Colour and Species stages, to coalesce tiny files

class UserInterfaceSpriteColour : public UserInterfaceSpriteRenderer {
private:
	int last_player;

public:
	void init(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) {

		last_player = -1;

		using namespace UserInterfaceSpriteConstants;
		SDL_Surface* screen = SDL_GetVideoSurface();
		// Blank the screen
		SDL_FillRect(screen, 0,
			SDL_MapRGB(screen->format, 0, 0, 0));
		// Show the static text
		const SDL_Color black = {0, 0, 0, 0};
		resources.displayTextLine(resources.font_large, "Colour Choice",
				col_text_gold, black, 64);
		resources.displayTextLine(resources.font_small,
				"Press your button to select",
				col_text_gold, black, 384);
		// Repaint everything to clear the screen
		SDL_Flip(screen);
	}

	~UserInterfaceSpriteColour() { }

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		GameStageState& state, uint32_t ticks,
		UserInterfaceSpriteResources& resources) {

		// Nice and easy
		if(state.colour.player != last_player) {
			last_player = state.colour.player;

			using namespace UserInterfaceSpriteConstants;
			SDL_Rect box = { 320 - 32, 192, 64, 64 };
			SDL_Surface* screen = SDL_GetVideoSurface();
			SDL_FillRect(screen, &box,
				SDL_MapRGB(screen->format,
					col_player[last_player].r,
					col_player[last_player].g,
					col_player[last_player].b));
			resources.updateRect(box.x, box.y, box.w, box.h);
		}
		// TODO draw claims...controller name in colour?

		return true;
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterfaceSpriteRenderer,UserInterfaceSpriteColour)

