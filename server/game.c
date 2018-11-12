//
// Created by Frixs on 10.10.2018.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "structs.h"
#include "game.h"
#include "server.h"
#include "constants.h"
#include "memory.h"
#include "stats.h"
#include "player.h"
#include "game_logic.h"

pthread_t thread_id;

/// Find the game in the game list.
/// \param id       Id of the game.
/// \return         The game struct or NULL.
game_t *game_find(char *id) {
    pthread_mutex_lock(&g_game_list_mutex);

    if (g_game_list != NULL) {
        game_t *game_ptr = g_game_list;

        do {
            if (strcmp(game_ptr->id, id) == 0) {
                pthread_mutex_unlock(&g_game_list_mutex);
                return game_ptr;
            }

            game_ptr = game_ptr->next;

        } while (game_ptr != NULL);
    }

    pthread_mutex_unlock(&g_game_list_mutex);
}

/// Broadcast information about available games to all players.
void game_broadcast_update_games() {
    game_t *game_list_ptr = g_game_list;
    char *message = NULL;

    message = memory_malloc(sizeof(char) * 1024);
    memset(message, 0, strlen(message));
    sprintf(message, "1;update_games"); // Token message.

    pthread_mutex_lock(&g_game_list_mutex);

    if (g_game_list != NULL) {
        do {
            if (game_list_ptr->player_count < PLAYER_COUNT)
                sprintf(message, "%s;%s;%s;%d", message, game_list_ptr->name, game_list_ptr->id, game_list_ptr->goal);

            game_list_ptr = game_list_ptr->next;

        } while (game_list_ptr != NULL);
    }

    pthread_mutex_unlock(&g_game_list_mutex);

    strcat(message, "\n");
    svr_broadcast(message);
}

/// Send information about all players who playing the current game.
/// \param game     The game.
void game_send_update_players(game_t *game) {
    if (!game)
        return;

    int i;
    char *message = NULL;
    player_t *player = NULL;

    message = memory_malloc(sizeof(char) * 1024);
    memset(message, 0, strlen(message));
    sprintf(message, "1;update_players"); // Token message.

    for (i = 0; i < PLAYER_COUNT; ++i) {
        player = game->players[i];

        if (player)
            sprintf(message, "%s;%s;%s;%s;%d;%d", message, player->id, player->nickname, player->color, player->score, player->choice);
    }

    strcat(message, "\n");
    game_multicast(game, message);
}

/// Send information about state/status of the game.
/// \param game     The game.
void game_send_current_state_info(game_t *game) {
    if (!game)
        return;

    int i;
    int state = 0;
    char *message = NULL;
    player_t *player = NULL;

    message = memory_malloc(sizeof(char) * 256);
    memset(message, 0, strlen(message));

    for (i = 0; i < PLAYER_COUNT; ++i) {
        if (game->players[i]) {
            player = game->players[i];
            break;
        }
    }

    if (game->player_count == 1) {
        state = 1;

        if (game->in_progress) { // If the game is in progress and there is only 1 player.
            state = 0;

            sprintf(message, "%s;set_player_win;%s\n", player->id, player->nickname); // Token message.
            game_multicast(game, message);

            // Turn off the game.
            game->in_progress = 0;

            // Release game semaphore.
            sem_post(&game->sem_on_turn);
        }
    } else if (game->player_count < PLAYER_COUNT) {
        state = 1;
    }

    if (player != NULL && state > 0) {
        sprintf(message, "%s;game_state;%d\n", player->id, state); // Token message.
        game_multicast(game, message);
    }
}

/// Creates the game.
/// \param player       Creator of the game.
/// \param goal         The goal.
void game_create(player_t *player, int goal) {
    if (!player)
        return;

    int i;
    char *log_message = NULL;

    game_t *game = memory_malloc(sizeof(game_t));

    game->id = svr_generate_id();

    game->name = memory_malloc(sizeof(char) * (19 + 5 + 1));
    strcpy(game->name, "game-");
    strcat(game->name, game->id);

    game->next = NULL;
    game->player_count = 0;

    if (goal > 0)
        game->goal = goal;
    else
        game->goal = GOAL_DEFAULT;

    sem_init(&(game->sem_on_turn), 0, 0);
    game->in_progress = 0;

    for (i = 0; i < PLAYER_COUNT; ++i)
        game->players[i] = NULL;

    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Game created (ID: %s)!\n", game->id);
    write_log(log_message);

    game_add(game);
    player_connect_to_game(player, game);
    game_broadcast_update_games();

    memory_free(log_message);
}

/// It adds the game to the game list.
/// \param game     The game.
void game_add(game_t *game) {
    if (!game)
        return;

    pthread_mutex_lock(&g_game_list_mutex);

    if (g_game_list == NULL) {
        g_game_list = game;
    } else {
        game_t *game_list_ptr = g_game_list;

        while (game_list_ptr->next != NULL) {
            game_list_ptr = game_list_ptr->next;
        }

        game_list_ptr->next = game;
    }

    pthread_mutex_unlock(&g_game_list_mutex);
}

/// It removes the game from the game list.
/// \param game     The game.
void game_remove(game_t *game) {
    if (!game)
        return;

    char *id = game->id;
    game_t *ptr = NULL;
    game_t *previous = NULL;
    char *log_message = NULL;

    ptr = g_game_list;

    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Game removed (ID: %s)!\n", game->id);

    pthread_mutex_lock(&g_game_list_mutex);

    do {
        if (strcmp(ptr->id, id) == 0) {
            if (!previous) {
                if (ptr->next == NULL) {
                    _game_destroy(ptr);
                    g_game_list = NULL;
                } else {
                    g_game_list = ptr->next;
                    _game_destroy(ptr);
                    ptr = NULL;
                }
            } else {
                previous->next = ptr->next;
                _game_destroy(ptr);
                ptr = NULL;
            }
        }

        if (ptr) {
            previous = ptr;
            ptr = ptr->next;
        }

    } while (ptr);

    pthread_mutex_unlock(&g_game_list_mutex);

    write_log(log_message);
    game_broadcast_update_games();

    memory_free(log_message);
}

/// Free all the needed memory space to be able to delete a pointer to the game without filled memory with its data. (Delete the game).
/// \param game
void _game_destroy(game_t *game) {
    if (!game)
        return;

    memory_free(game->id);
    memory_free(game->name);
    sem_destroy(&(game->sem_on_turn));
    memory_free(game);
}

/// Start the game thread.
/// \param game     The game.
/// \return         Status code. 0 = success. 1 = error.
int game_start(game_t *game) {
    char *log_message = NULL;

    thread_id = 0;

    if (!game || pthread_create(&thread_id, NULL, _game_serve, (void *) game)) {
        // Log.
        log_message = memory_malloc(sizeof(char) * 256);
        sprintf(log_message, "\t> Fatal ERROR during creating a new thread!\n");
        write_log(log_message);
        memory_free(log_message);
        return 1;
    }

    game->thread = thread_id;
    game->in_progress = 1;

    return 0;
}

/// Send message to all players of the game.
/// \param game         The game.
/// \param message      The message.
void game_multicast(game_t *game, char *message) {
    if (!game || !message)
        return;

    int i;

//    printf("--->>> %s", message);

    for (i = 0; i < PLAYER_COUNT; ++i)
        if (game->players[i] && game->players[i]->is_disconnected != 1)
            svr_send(game->players[i]->socket, message);

    memory_free(message);
}

/// Serve the game.
/// \param arg      The args.
/// \return         -
void *_game_serve(void *arg) {
    if (!arg)
        return NULL;

    game_t *game = NULL;
    char *message = NULL;

    game = (game_t *) arg;

    while (game->in_progress && game->player_count == PLAYER_COUNT) {
        for (int i = 0; i < game->player_count; ++i) {
            game_logic_prepare_player_turn(game->players[i]);
            // Update player data.
            game_send_update_players(game);

            message = memory_malloc(sizeof(char) * 256);
            memset(message, 0, strlen(message));
            sprintf(message, "%s;on_turn\n", game->players[i]->id);
            svr_send(game->players[i]->socket, message);
            memory_free(message);
        }

        // Wait until all players play.
        sem_wait(&game->sem_on_turn);
    }

    // Disconnect all players.
    for (int j = 0; j < PLAYER_COUNT; ++j) {
        if (!game->player_count)
            break;

        player_disconnect_from_game(game->players[j], game);
    }

    game->in_progress = 0;

    return NULL;
}

/// Free all games.
void game_free() {
    while (g_game_list) {
        game_remove(g_game_list);
    }
}

/// Print the game.
void game_print() {
    game_t *ptr = g_game_list;

    printf("========== GAME LIST ==========\n");

    if (g_game_list) {
        do {
            printf("Name: %s (ID: %s)\n", ptr->name, ptr->id);
            ptr = ptr->next;

        } while (ptr);
    }

    printf("===============================\n");
}