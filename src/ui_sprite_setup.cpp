#include "ui_sprite.hpp"
#include "ui_sprite_pointer.hpp"

// This covers both the Colour and Species stages, to coalesce tiny files

static const char* species_name(Species::Type s) {
	switch(s) {
		case Species::REGULAR1: return "Gollumer";
		case Species::REGULAR2: return "Packer";
		case Species::REGULAR3: return "Spheroid";
		case Species::ADVANCED: return "Humanoid";
		case Species::REGULAR4: return "Leggite";
		case Species::BEGINNER: return "Flapper";
		case Species::REGULAR5: return "Bonzoid";
		case Species::COMPUTER: return "Mechtron";
	}
	abort();
}

static const char* species_descriptions[][3] = {{
"gollumer blah",
"",
"",
}, {
"packer blah",
"",
"",
}, {
"spheroid blah",
"",
"",
}, {
"humanoid blah",
"",
"",
}, {
"leggite blah",
"",
"",
}, {
"flapper blah",
"",
"",
}, {
"bonzoid blah",
"",
"",
}, {
"mechtron blah",
"",
"",
}};

static const char** species_desc(Species::Type s) {
	switch(s) { // Yeah yeah, pedantic type safety; may remove later.
		case Species::REGULAR1: return species_descriptions[0];
		case Species::REGULAR2: return species_descriptions[1];
		case Species::REGULAR3: return species_descriptions[2];
		case Species::ADVANCED: return species_descriptions[3];
		case Species::REGULAR4: return species_descriptions[4];
		case Species::BEGINNER: return species_descriptions[5];
		case Species::REGULAR5: return species_descriptions[6];
		case Species::COMPUTER: return species_descriptions[7];
	}
	abort();
}

class UserInterfaceSpriteColour : public UserInterfaceSpriteRenderer {
private:
	GameStageState::Colour last_state;

public:
	void init(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) {

		// Fudge this to force first-frame repaint
		last_state.offer = -1;

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
		if(state.colour.offer != last_state.offer) {
			using namespace UserInterfaceSpriteConstants;
			SDL_Rect box = { 320 - 32, 192, 64, 64 };
			SDL_Surface* screen = SDL_GetVideoSurface();
			SDL_FillRect(screen, &box,
				SDL_MapRGB(screen->format,
					col_player[state.colour.offer].r,
					col_player[state.colour.offer].g,
					col_player[state.colour.offer].b));
			resources.updateRect(box.x, box.y, box.w, box.h);
		}
		
		// Draw claims (controller name in colour)
		for(int p = 0; p < PLAYERS; ++p) {
			if((last_state.offer == -1) ||
				(state.colour.claim[p] != last_state.claim[p])){
			
				const SDL_Color black = {0, 0, 0, 0};
				PlayerSetup* ps = &setup.playersetup[p];
				const char* description = ps->computer
					? "Computer player"
					: ps->controller->getDescription();
				int claim = state.colour.claim[p];
			
				using namespace UserInterfaceSpriteConstants;	
				resources.displayTextLine(resources.font_small,
					description,
					claim == -1
						? (ps->computer
							? col_text_black
							: col_text_gray)
						: col_player[claim],
					black, 280 + (p * 24));
			}
		}
		
		// Update last state (can get away with shallow copy)
		last_state = state.colour;

		return true;
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterfaceSpriteRenderer,UserInterfaceSpriteColour)

class UserInterfaceSpriteSpecies : public UserInterfaceSpriteRenderer {
private:
	GameStageState::Species last_state;
	Species::Type last_species;

public:
	void init(GameStage::Type stage, GameSetup& setup, Game* game,
		uint32_t ticks, UserInterfaceSpriteResources& resources) {

		last_state.player = -1; // first frame fudge
		last_species = Species::COMPUTER;

		using namespace UserInterfaceSpriteConstants;
		SDL_Surface* screen = SDL_GetVideoSurface();
		// Blank the screen
		SDL_FillRect(screen, 0,
			SDL_MapRGB(screen->format, 0, 0, 0));
		// Show the static text TODO placeholder
		const SDL_Color black = {0, 0, 0, 0};
		resources.displayTextLine(resources.font_large,
				"Species Choice", col_text_gold, black, 64);
		resources.displayTextLine(resources.font_small,
				"Push direction and press button",
				col_text_gold, black, 384);
		// Repaint everything to clear the screen
		SDL_Flip(screen);
	}

	~UserInterfaceSpriteSpecies() { }

	bool render(GameStage::Type stage, GameSetup& setup, Game* game,
		GameStageState& state, uint32_t ticks,
		UserInterfaceSpriteResources& resources) {

		// TODO If state mismatch, reset animation and draw species

		// TODO If species mismatch (and defined), show text

		// TODO Animate any defined species
		
		// Update last state
		last_state = state.species;
		last_species = setup.playersetup[state.species.player].species;

		// XXX DEBUG
		using namespace UserInterfaceSpriteConstants;
		const SDL_Color black = {0, 0, 0, 0};
		resources.displayTextLine(resources.font_small,
				state.species.defined
					? species_name(last_species)
					: "choose",
				col_text_gold, black, 400);
		species_desc(last_species); // useless call to make used

		return true;
	}
};
/* Register with the factory */
FACTORY_REGISTER_IMPL(UserInterfaceSpriteRenderer,UserInterfaceSpriteSpecies)

