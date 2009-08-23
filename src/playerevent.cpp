#include "playerevent.hpp"
#include "game.hpp"

// Calculate the round-dependent multiplier on some event magnitudes.
static int multiplier(const Game& game) { return 25 * ((game.month / 4) + 1); }

bool PlayerEvent::good(PlayerEvent::Type self) { return self < FIRST_BAD; }

bool PlayerEvent::precondition(PlayerEvent::Type self, int player,
	const Game& game) {

	Stock select;
	switch(self) {
		case MULE_WINNINGS_1:
		case MULE_WINNINGS_2:
		case MULE_COST: // At least one exploitation (non-None land)
			select.food = 1; select.energy  = 1;
			select.ore  = 1; select.crystal = 1;
			return countTilesOfType(game, player, select);
		case AGRICULTURE_GRANT: // At least one food exploitation
			select.food = 1;
			return countTilesOfType(game, player, select);
		case EXTRA_LAND: // At least one unowned land
			{int freeland =
				(game.terrain.getSizeX() *
				 game.terrain.getSizeY()) - 1; // not city!
			select.food = 1; select.energy  = 1;
			select.ore  = 1; select.crystal = 1; select.workers = 1;
			freeland -= countTilesOfType(game, player, select);
			return freeland;}
		case MULE_MINING_COST: // At least one ore or crystal tile
			select.ore  = 1; select.crystal = 1;
			return countTilesOfType(game, player, select);
		case MULE_SOLAR_COST: // At least one energy exploitation
			select.energy = 1;
			return countTilesOfType(game, player, select);
		case LOST_LAND: // At least one plot of land owned
			select.food = 1; select.energy  = 1;
			select.ore  = 1; select.crystal = 1; select.workers = 1;
			return countTilesOfType(game, player, select);
		default: return true; // Nothing stopping it
	}
}
	
Stock PlayerEvent::changes(PlayerEvent::Type self, int player, const Game& game,
	int32_t* money, uint16_t* each) {

	Stock select;
	Stock change;
	*money = 0;
	*each = 0;
	switch(self) {
		case CARE_PACKAGE:
			change.food = 3; change.energy = 2; break;
		case WANDERING_TRAVELLER:
			change.ore = 2; break;
		case MULE_WINNINGS_1:
			*money = 2 * multiplier(game); break;
		case MULE_WINNINGS_2:
			*money = 4 * multiplier(game); break;
		case AGRICULTURE_GRANT:
			*each  = 2 * multiplier(game);
			select.food = 1;
			*money = *each * countTilesOfType(game, player, select);
			break;
		case WINNINGS_1:
			*money = 4 * multiplier(game); break;
		case WINNINGS_2:
			*money = 8 * multiplier(game); break;
		case WINNINGS_3:
			*money = 2 * multiplier(game); break;
		case WINNINGS_4:
			*money = 3 * multiplier(game); break;
		case WINNINGS_5:
			*money = 6 * multiplier(game); break;
		case WINNINGS_6:
			*money = 4 * multiplier(game); break;
		case WINNINGS_7:
			*money = 2 * multiplier(game); break;
		case EXTRA_LAND: break; // Only has side-effects
		case FOOD_STOLEN:
			// TWEAK Does this round up?
			change.food = -(game.players[player].stock.food / 2);
			break;
		case MULE_COST:
			*money = -3 * multiplier(game); break;
		case MULE_MINING_COST:
			*each  =  2 * multiplier(game);
			select.ore = 1; select.crystal = 1;
			*money = -1 * *each *
				countTilesOfType(game, player, select);
			break;
		case MULE_SOLAR_COST:
			*each = multiplier(game);
			select.energy = 1;
			*money = -1 * *each *
				countTilesOfType(game, player, select);
			break;
		case LOSSES_1:
			*money = -6 * multiplier(game); break;
		case LOSSES_2:
			*money = -4 * multiplier(game); break;
		case LOSSES_3:
			*money = -4 * multiplier(game); break;
		case LOSSES_4:
			*money = -4 * multiplier(game); break;
		case LOST_LAND: break; // Only has side-effects
	}
	return change;
}

void PlayerEvent::applyOther(PlayerEvent::Type self, int player, Game& game) {
	switch(self) {
		case EXTRA_LAND:
			// TODO find an empty land and assign it to player
			break;
		case LOST_LAND:
			/* TWEAK Supposedly, this should be the most recent land
			 * the player got, but we don't track that---they lose a
			 * random one. This shouldn't matter too much. */
			// TODO find a player's land and remove it from player
			break;
		default: break; // NOP
	}
}
