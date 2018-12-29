//
// Created by Frixs on 10.10.2018.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "structs.h"
#include "game_logic.h"
#include "memory.h"
#include "stats.h"
#include "game.h"
#include "constants.h"
#include "server.h"

/// Evalutate game after turn.
/// \param g        The game.
void _game_logic_evaluate(game_t *g) {
    if (!g)
        return;

    char *message = NULL;
    player_t *p = NULL;

    // Check if all players already have selected their choice.
    for (int i = 0; i < g->player_count; ++i)
        if (!g->players[i]->choice)
            return;

    // All players selected their choices.
    // Count score.
    _game_logic_count_score(g);

    // Check winner.
    if ((p =_game_logic_check_winner(g))) {
        message = memory_malloc(sizeof(char) * 256, 0);
        sprintf(message, "%s;set_player_win;%s\n", p->id, p->nickname); // Token message.
        game_multicast(g, message);

        // Turn off the game.
        g->in_progress = 0;
    }

    // Update player data.
    game_send_update_players(g);

    // Sleep due to client-friendly interaction.
    sleep(1);

    message = memory_malloc(sizeof(char) * 256, 0);
    memset(message, 0, strlen(message));
    sprintf(message, "1;do_after_turn\n"); // Token message.
    game_multicast(g, message);

    // Release game semaphore.
    sem_post(&g->sem_on_turn);
}

/// Check player's turn.
/// \param p        The player.
/// \param c        It's choice.
void game_logic_record_turn(player_t *p, int c) {
    if (!p)
        return;

    if (c == ROCK)
        p->choice = ROCK;
    else if (c == PAPER)
        p->choice = PAPER;
    else
        p->choice = SCISSORS;

    // Evaluate game round.
    _game_logic_evaluate(p->game);
}

/// Prepare player to a new turn.
/// \param p    The player.
void game_logic_prepare_player_turn(player_t *p) {
    if (!p)
        return;

    p->choice = 0;
}

void game_logic_prepare_player_on_game_join(player_t *p) {
    if (!p)
        return;

    p->score = 0;
    p->choice = 0;
}

/// Count score of the current turn.
/// \param g        The game.
void _game_logic_count_score(game_t *g) {
    if (g->player_count <= 1)
        return;

    int rock_count = 0;
    int paper_count = 0;
    int scissors_count = 0;
    int player_index = 0;

    // If there are only 2 players.
    if (g->player_count == 2) {
        if (!(player_index = _game_logic_compare_choices(g->players[0]->choice, g->players[1]->choice)))
            return;

        if (player_index == 3)
            return;

        g->players[player_index - 1]->score++;

        return;
    }

    // Player count > 2.
    for (int i = 0; i < g->player_count; ++i) {
        if (g->players[i]->choice == ROCK)
            ++rock_count;
        else if (g->players[i]->choice == PAPER)
            ++paper_count;
        else
            ++scissors_count;
    }

    if (rock_count == paper_count && rock_count == scissors_count) {
        return; // Draw.

    } else if (rock_count == paper_count) {
        // Rock + Paper.
        _game_logic_set_score_to_all_by_choice(g, ROCK);
        _game_logic_set_score_to_all_by_choice(g, PAPER);

    } else if (rock_count == scissors_count) {
        // Rock + Scissors.
        _game_logic_set_score_to_all_by_choice(g, ROCK);
        _game_logic_set_score_to_all_by_choice(g, SCISSORS);

    } else if (paper_count == scissors_count) {
        // Paper + Scissors.
        _game_logic_set_score_to_all_by_choice(g, PAPER);
        _game_logic_set_score_to_all_by_choice(g, SCISSORS);

    } else {
        if (rock_count > paper_count && rock_count > scissors_count) {
            // Rock.
            _game_logic_set_score_to_all_by_choice(g, ROCK);

        } else if (paper_count > rock_count && paper_count > scissors_count) {
            // Paper.
            _game_logic_set_score_to_all_by_choice(g, PAPER);

        } else {
            // Scissors.
            _game_logic_set_score_to_all_by_choice(g, SCISSORS);

        }
    }
}

/// Compare choices which wins.
/// \param c1   Choice #1.
/// \param c2   Choice #2.
/// \return     0 = Error, 1 = c1 won, 2 = c2 won, 3 = Draw.
int _game_logic_compare_choices(choice_t c1, choice_t c2) {
    if (c1 == c2)
        return 3;

    // Logic.
    if (     c1 == ROCK      && c2 == PAPER)
        return 2;

    else if (c1 == ROCK      && c2 == SCISSORS)
        return 1;

    else if (c1 == PAPER     && c2 == ROCK)
        return 1;

    else if (c1 == PAPER     && c2 == SCISSORS)
        return 2;

    else if (c1 == SCISSORS  && c2 == ROCK)
        return 2;

    else if (c1 == SCISSORS  && c2 == PAPER)
        return 1;

    else
        return 0;
}

/// Set choice to all players in the game by choice.
/// \param g        The game.
/// \param c        The choice.
void _game_logic_set_score_to_all_by_choice(game_t *g, choice_t c) {
    if (!g)
        return;

    for (int i = 0; i < g->player_count; ++i) {
        if (g->players[i]->choice == c)
            g->players[i]->score++;
    }
}

/// Check winner.
/// \param g    The game.
/// \return     NULL on failure or player who is the winner of the game.
player_t *_game_logic_check_winner(game_t *g) {
    if (!g)
        return NULL;

    player_t *p = NULL;

    for (int i = 0; i < g->player_count; ++i) {
        if (g->players[i]->score >= g->goal) {
            if (p) {
                g->goal++;
                return NULL;
            }

            p = g->players[i];
        }
    }

    return p;
}
