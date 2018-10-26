//
// Created by Frixs on 10.10.2018.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "structs.h"
#include "game_logic.h"
#include "memory.h"
#include "stats.h"
#include "game.h"

/// Evalutate game after turn.
/// \param g        The game.
void game_logic_evaluate(game_t *g) {
    // TODO;
    printf("TODO\n");
}

/// Check player's turn.
/// \param p        The player.
/// \param c        It's choice.
void game_logic_record_turn(player_t *p, int c) {
    int bCheck = 1;
    char *message = NULL;

    if (c == ROCK) {
        p->choice = ROCK;

    } else if (c == PAPER) {
        p->choice = PAPER;

    } else {
        p->choice = SCISSORS;

    }

    // Check if all players already have selected their choice.
    for (int i = 0; i < p->game->player_count; ++i) {
        if (!p->game->players[i]->choice) {
            bCheck = 0;
            break;
        }
    }

    // If some players did not select their choices yet.
    if (!bCheck)
        return;

    // TODO: Count score.

    // All players selected their choices.
    message = memory_malloc(sizeof(char) * 256);
    memset(message, 0, strlen(message));
    sprintf(message, "1;evaluate_game\n"); // Token message.
    game_multicast(p->game, message);
}

/// Prepare player to a new turn.
/// \param p    The player.
void game_logic_prepare_player_turn(player_t *p) {
    p->choice = 0;
}

void game_logic_prepare_player_on_game_join(player_t *p) {
    p->score = 0;
    p->choice = 0;
}
