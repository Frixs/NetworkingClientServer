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
#include "game_logic.h"

/// Create a player struct.
/// \param connection_info  Connection info struct.
/// \param nickname         Player nickname.
/// \return                 Player struct.
player_t *player_create(remote_connection_t *connection_info, char *nickname) {
    player_t *p = memory_malloc(sizeof(player_t), 0);

    p->nickname = memory_malloc(50 * sizeof(char), 0);
    sprintf(p->nickname, "%s", nickname);

    p->client_addr = memory_malloc(connection_info->client_address_len * sizeof(char), 0);
    sprintf(p->client_addr, "%s", connection_info->client_address);

    p->choice = 0;
    p->socket = connection_info->client_socket;
    p->is_disconnected = 0;
    p->id = svr_generate_id();
    p->next = NULL;
    p->game = NULL;

    return p;
}

/// Change player's socket to another.
/// \param player   The player.
/// \param socket   The socket.
void player_change_socket(player_t *player, int socket) {
    player->socket = socket;
}

/// Remove the player from the player list.
/// \param player
void player_remove(player_t *player) {
    if (!player)
        return;

    char *id = memory_malloc(sizeof(char) * (19 + 1), 0);
    strcpy(id, player->id);
    int socket = player->socket;
    int is_disconnected = player->is_disconnected;
    char *log_message = NULL;
    char *message = NULL;
    player_t *previous = NULL;
    player_t *ptr = NULL;

    ptr = g_player_list;

    log_message = memory_malloc(sizeof(char) * 256, 0);
    sprintf(log_message, "\t> Player %s (ID: %s) has disconnected!\n", player->nickname, player->id);

    pthread_mutex_lock(&g_player_list_mutex);

    do {
        if (strcmp(ptr->id, id) == 0) {
            if (!previous) {
                if (ptr->next == NULL) {
                    _player_destroy(ptr);
                    g_player_list = NULL;
                } else {
                    g_player_list = ptr->next;
                    _player_destroy(ptr);
                    ptr = NULL;
                }
            } else {
                previous->next = ptr->next;
                _player_destroy(ptr);
                ptr = NULL;
            }
        }

        if (ptr) {
            previous = ptr;
            ptr = ptr->next;
        }

    } while (ptr);

    pthread_mutex_unlock(&g_player_list_mutex);

    message = memory_malloc(sizeof(char) * 256, 0);
    sprintf(message, "%s;disconnect_player\n", id); // Token message.

    if (is_disconnected != 1)
        svr_send(socket, message, 0);

    write_log(log_message);

    memory_free(log_message, 0);
    memory_free(message, 0);
    memory_free(id, 0);
}

/// Free all the needed memory space to be able to delete a pointer to the player without filled memory with its data. (Delete the player).
/// \param player
void _player_destroy(player_t *player) {
    if (!player)
        return;

    memory_free(player->id, 0);
    memory_free(player->nickname, 0);
    memory_free(player->client_addr, 0);
    memory_free(player, 0);
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

            player_ptr = player_ptr->next;

        } while (player_ptr != NULL);
    }

    pthread_mutex_unlock(&g_player_list_mutex);

    return NULL;
}

/// Find the player in the player list by Client addr.
/// \param id       Client addr.
/// \return         Pointer to player. Returns NULL if it fails.
player_t *player_find_unknown_reconnect(char *client_addr) {
    if (!client_addr)
        return NULL;

    player_t *player_ptr = NULL;

    pthread_mutex_lock(&g_player_list_mutex);

    if (g_player_list != NULL) {
        player_ptr = g_player_list;

        do {
            if (player_ptr->is_disconnected == 1 && strcmp(player_ptr->client_addr, client_addr) == 0) {
                pthread_mutex_unlock(&g_player_list_mutex);
                return player_ptr;
            }

            player_ptr = player_ptr->next;

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

        while (ptr->next != NULL) {
            ptr = ptr->next;
        }

        ptr->next = player;
    }

    pthread_mutex_unlock(&g_player_list_mutex);
}

/// Connects a player to a game.
/// \param player       The player.
/// \param game         The game.
/// \return             Status code. 0 = Success, 1 = Error.
int player_connect_to_game(player_t *player, game_t *game) {
    if (!player || !game)
        return 1;

    int i, j;
    int is_reconnecting = 0;
    char *message = NULL;
    char *log_message = NULL;

    // Check for reconnection.
    for (i = 0; i < game->player_count; ++i) {
        printf("%s - %s\n", player->id, game->players[i]->id);
        if (strcmp(player->id, game->players[i]->id) == 0) {
            is_reconnecting = 1;
            break;
        }
    }

    if (!is_reconnecting) {
        if (game->player_count >= PLAYER_COUNT)
            return 1;

        for (i = 0; i < PLAYER_COUNT; ++i) {
            if (game->players[i] != NULL)
                continue;

            game->players[i] = player;
            player->color = g_color_list[i];
            game->player_count++;
            player->game = game;
            break;
        }

        game_logic_prepare_player_on_game_join(player); // Only for new players /We do not want to reset score to rejoined player f.e.
    }

    message = memory_malloc(sizeof(char) * 256, 0);
    memset(message, 0, strlen(message));
    sprintf(message, "%s;prepare_window_for_game;%s;%s;%d\n", player->id, game->id, game->name,
            game->goal); // Token message.
    svr_send(player->socket, message, 0);

    log_message = memory_malloc(sizeof(char) * 256, 0);
    if (is_reconnecting)
        sprintf(log_message, "\t> Player %s (ID: %s) returned to the game (ID: %s).\n", player->nickname, player->id, game->id);
    else
        sprintf(log_message, "\t> Player %s (ID: %s) joined to the game (ID: %s).\n", player->nickname, player->id, game->id);
    write_log(log_message);

    game_send_update_players(game);
    if (game->player_count == PLAYER_COUNT) {
        game_broadcast_update_games();

        if (game_start(game)) { // If the game cannot start a thread for it.
            log_message = memory_malloc(sizeof(char) * 256, 0);
            sprintf(log_message, "\t> Fatal ERROR during creating a new thread!\n");
            write_log(log_message);
            memory_free(log_message, 0);

            for (j = 0; j < game->player_count; ++j) {
                player_disconnect_from_game(game->players[j], game);
            }
            game_remove(game);
        }
    }

    memory_free(message, 0);
    memory_free(log_message, 0);

    return 0;
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
            player->color = NULL;
            break;
        }
    }

//    if (game->in_progress)
//        sem_post(&game->sem_on_turn);

    game_broadcast_update_games();

    // Log.
    log_message = memory_malloc(sizeof(char) * 256, 0);
    sprintf(log_message, "\t> Player: %s (ID: %s) has disconnected from the game %s (ID: %s)!\n", player->nickname,
            player->id, game->name, game->id);
    write_log(log_message);
    memory_free(log_message, 0);

    // Send a message back to client.
    message = memory_malloc(sizeof(char) * 256, 0);
    sprintf(message, "%s;leave_game\n", player->id); // Token message.

    // Do not disconnect the player who lost a connection.
    if (player->is_disconnected != 1)
        svr_send(player->socket, message, 0);

    // If there is no player, remove the game. Or update information.
    if (game->player_count == 0) {
        game_remove(game);
    } else {
        game_send_update_players(game);
        game_send_current_state_info(game);
    }

    memory_free(message, 0);
}

/// Free al players.
void player_free() {
    while (g_player_list) {
        player_remove(g_player_list);
    }
}

/// Print the player.
void player_print() {
    player_t *ptr = g_player_list;

    printf("========= PLAYER LIST =========\n");

    if (g_player_list) {
        do {
            printf("Nickname: %s (ID: %s)\n", ptr->nickname, ptr->id);
            //printf("DEBUG: Socket: %d, Client addr.: %s, Is Disconnected status: %d.\n", ptr->socket, ptr->client_addr, ptr->is_disconnected);
            ptr = ptr->next;

        } while (ptr);
    }

    printf("===============================\n");
}