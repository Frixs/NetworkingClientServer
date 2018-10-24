//
// Created by Frixs on 9.10.2018.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "structs.h"
#include "server.h"
#include "player.h"
#include "memory.h"
#include "stats.h"
#include "constants.h"
#include "game.h"
#include "colors.h"

/// Create a player struct.
/// \param socket       Socket number.
/// \param nickname     Player nickname.
/// \return             Player struct.
player_t *player_create(int socket, char *nickname) {
    player_t *p = memory_malloc(sizeof(player_t));

    p->nickname = memory_malloc(50 * sizeof(char));
    sprintf(p->nickname, "%s", nickname);

    p->socket               = socket;
    p->is_disconnected      = 0;
    p->id                   = svr_generate_id();
    p->next_player = NULL;
    p->game                 = NULL;

    return p;
}

/// Remove the player from the player list.
/// \param player
void player_remove(player_t *player) {
    if (!player)
        return;

    char *id = memory_malloc(sizeof(char) * (19 + 1));
    strcpy(id, player->id);
    int socket = player->socket;
    int is_disconnected = player->is_disconnected;
    char *log_message = NULL;
    char *message = NULL;

    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Player %s (ID: %s) has disconnected!\n", player->nickname, player->id);

    pthread_mutex_lock(&g_player_list_mutex);

    player_t *player_list_ptr = g_player_list;
    player_t *previous_player = NULL;

    do {
        if (strcmp(player_list_ptr->id, id) == 0) {
            if (!previous_player) {
                if (player_list_ptr->next_player == NULL) {
                    _player_destroy(player_list_ptr);
                    g_player_list = NULL;
                } else {
                    g_player_list = player_list_ptr->next_player;
                    _player_destroy(player_list_ptr);
                    player_list_ptr = NULL;
                }
            } else {
                previous_player->next_player = player_list_ptr->next_player;
                _player_destroy(player_list_ptr);
                g_player_list = NULL;
            }
        }

        previous_player = player_list_ptr;
        player_list_ptr = player_list_ptr->next_player;

    } while (player_list_ptr != NULL);

    pthread_mutex_unlock(&g_player_list_mutex);

    message = memory_malloc(sizeof(char) * 256);
    sprintf(message, "%s;disconnect_player\n", id); // Token message.

    if (is_disconnected != 1)
        svr_send(socket, message);

    write_log(log_message);

    memory_free(log_message);
    memory_free(message);
    memory_free(id);
}

/// Free all the needed memory space to be able to delete a pointer to the player without filled memory with its data. (Delete the player).
/// \param player
void _player_destroy(player_t *player) {
    memory_free(player->id);
    memory_free(player->nickname);
    memory_free(player);
}

/// Find the player in the player list by ID.
/// \param id       Player ID.
/// \return         Pointer to player. Returns NULL if it fails.
player_t *player_find(char *id) {
    if (!id)
        return NULL;

    player_t *player_ptr = NULL;

    pthread_mutex_lock(&g_player_list_mutex);

    if (g_player_list != NULL) {
        player_ptr = g_player_list;

        do {
            if (strcmp(player_ptr->id, id) == 0) {
                pthread_mutex_unlock(&g_player_list_mutex);
                return player_ptr;
            }

            player_ptr = player_ptr->next_player;

        } while (player_ptr != NULL);
    }

    pthread_mutex_unlock(&g_player_list_mutex);

    return NULL;
}

/// Add a new player into player list.
/// \param hrac     Player to be added.
void player_add(player_t *player) {
    if (!player)
        return;

    pthread_mutex_lock(&g_player_list_mutex);

    if (g_player_list == NULL) {
        // There is no other player, let's add a new (the first) one.
        g_player_list = player;
    } else {
        // There are already some players created, let's add a new one at the end of the player list.
        player_t *ptr = g_player_list;

        while (ptr->next_player != NULL) {
            ptr = ptr->next_player;
        }

        ptr->next_player = player;
    }

    pthread_mutex_unlock(&g_player_list_mutex);
}

/// Connects a player to a game.
/// \param player       The player.
/// \param game         The game.
void player_connect_to_game(player_t *player, game_t *game) {
    if (!player || !game)
        return;

    int i;
    char *message = NULL;
    char *log_message = NULL;

    if (game->player_count >= PLAYER_COUNT)
        return;

    for (i = 0; i < PLAYER_COUNT; ++i) {
        if (game->players[i] != NULL)
            continue;

        game->players[i] = player;
        player->color = g_color_list[i];
        game->player_count++;
        player->game = game;
        break;
    }

    message = memory_malloc(sizeof(char) * 256);
    memset(message, 0, strlen(message));
    sprintf(message, "%s;prepare_window_for_game;%s;%s;%d\n", player->id, game->id, game->name, game->goal); // Token message.

    svr_send(player->socket, message);

    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Player (ID: %s) joined to the game (ID: %s).\n", player->id, game->id);
    write_log(log_message);

    game_send_update_players(game);
    if (game->player_count == PLAYER_COUNT)
        game_broadcast_update_games();

    memory_free(message);
    memory_free(log_message);
}

/// Disconnects a player from its current game.
/// \param player       The player.
/// \param game         The game.
void player_disconnect_from_game(player_t *player, game_t *game) {
    if (!player || !game)
        return;

    int i;
    char *log_message = NULL;
    char *message = NULL;

    for (i = 0; i < PLAYER_COUNT; i++) {
        if (game->players[i] == player) {
            game->players[i] = NULL;
            game->player_count--;
            player->game = NULL;
            break;
        }
    }

    if (player == game->player_on_turn) {
        sem_post(&game->sem_play);
    }

    game_broadcast_update_games();

    // Log.
    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Player: %s (ID: %s) has disconnected from the game %s (ID: %s)!\n", player->nickname,
            player->id, game->name, game->id);
    write_log(log_message);
    memory_free(log_message);

    // Send a message back to client.
    message = memory_malloc(sizeof(char) * 256);
    sprintf(message, "%s;disconnect_player_from_game\n", player->id); // Token message.

    if (player->is_disconnected != 1)
        svr_send(player->socket, message);

    if (game->player_count == 0) {
        game_remove(game);
        memory_free(message);
    } else {
        game_multicast(game, message);
        game_send_update_players(game);
        game_send_current_state_info(game);
    }

    player->color = NULL;
}