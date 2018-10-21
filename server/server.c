#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "constants.h"
#include "stats.h"
#include "structs.h"
#include "player.h"
#include "colors.h"
#include "server.h"
#include "memory.h"
#include "game.h"

time_t time_initial, time_current;
player_t *g_player_list;
pthread_mutex_t g_player_list_mutex;
game_t *g_game_list;
pthread_mutex_t g_game_list_mutex;
pthread_t thread_id;

// Struct to be able to set timeout of socket.
struct timeval timeout;

char **_svr_split_message(char *message);

/// Send the message to entered socket and write the message to statistics.
/// \param socket       Socket where the message is sent.
/// \param message      Message.
void svr_send(int socket, char *message) {
    bytes_sent += send(socket, message, strlen(message) * sizeof(char), 0);
    messages_sent++;
}

/// Connects a player to a game.
/// \param player       The player.
/// \param game         The game.
void svr_connect(player_t *player, game_t *game) {
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
    sprintf(message, "%s;set_player;%s;%s\n", player->id, player->color, game->id);

    svr_send(player->socket, message);

    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Player (ID: %s) joined to the game (ID: %s).\n", player->id, game->id);
    write_log(log_message);

    game_send_player_info(game);
    if (game->player_count == PLAYER_COUNT)
        game_broadcast_board_info();

    memory_free(message);
    memory_free(log_message);
}

/// Disconnects the player from its current game.
/// \param player
/// \param game
void svr_disconnect(player_t *player, game_t *game) {
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

    game_broadcast_board_info();

    // Log.
    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Player: %s (ID: %s) has disconnected from the game %s (ID: %s)!\n", player->nickname,
            player->id, game->name, game->id);
    write_log(log_message);
    memory_free(log_message);

    // Send a message back to client.
    message = memory_malloc(sizeof(char) * 256);
    sprintf(message, "%s;disconnect\n", player->id);

    if (player->is_disconnected != 1)
        svr_send(player->socket, message);

    if (game->player_count == 0) {
        game_remove(game);
        memory_free(message);
    } else {
        game_multicast(game, message);
        game_send_player_info(game);
        game_send_current_state_info(game);
    }

    player->color = NULL;
}

/// Check if ID is already in list of players or game instances.
/// \param id
/// \return
int _svr_find_id(char *id) {
    if (!id)
        return -1;

    // Check players.
    pthread_mutex_lock(&g_player_list_mutex);

    if (g_player_list != NULL) {
        player_t *player_ptr = g_player_list;

        do {
            if (strcmp(player_ptr->id, id) == 0) {
                pthread_mutex_unlock(&g_player_list_mutex);
                return 1;
            }

            player_ptr = player_ptr->next_player;
        } while (player_ptr != NULL);

    }

    pthread_mutex_unlock(&g_player_list_mutex);

    // Check games.
    pthread_mutex_lock(&g_game_list_mutex);

    if (g_game_list != NULL) {
        game_t *game_ptr = g_game_list;

        do {
            if (strcmp(game_ptr->id, id) == 0) {
                pthread_mutex_unlock(&g_game_list_mutex);
                return 1;
            }

            game_ptr = game_ptr->next_game;
        } while (game_ptr != NULL);
    }

    pthread_mutex_unlock(&g_game_list_mutex);

    return 0;
}

/// Generates unique ID for players and game instances.
/// \return
char *svr_generate_id() {
    char *id = memory_malloc(sizeof(char) * (19 + 1));

    do {
        sprintf(id, "%d", rand());
    } while (_svr_find_id(id) != 0);

    return id;

}

/// Send a text messsage to all players.
/// \param message  Message to send.
void svr_broadcast(char *message) {
    if (!message)
        return;

    printf("--->>> %s", message);

    pthread_mutex_lock(&g_player_list_mutex);
    if (g_player_list != NULL) {
        player_t *player_ptr = g_player_list;

        do {
            if (player_ptr->is_disconnected != 1)
                svr_send(player_ptr->socket, message);

            player_ptr = player_ptr->next_player;
        } while (player_ptr != NULL);

    }
    pthread_mutex_unlock(&g_player_list_mutex);

    memory_free(message);
}

/// The main data stream to each player.
/// \param arg      Pointer to newly added player.
/// \return         Status code.
void *_svr_serve_recieving(void *arg) {
    player_t *player_ptr = (player_t *) arg;
    int client_sock = player_ptr->socket;
    char *id = player_ptr->id;
    int read_size;
    int timeout = 0;
    char cbuf[1024];
    char *message = NULL;

    while ((read_size = (int) recv(client_sock, cbuf, 1024 * sizeof(char), 0)) !=
           0) { // 0 = closed connection, -1 = unsuccessful, >0 = length of the received message.
        if (read_size == -1) { // Unsuccessful.
            if (player_ptr->game && player_ptr->game->player_on_turn ==
                                    player_ptr) { // If player is in a game and the player is on its turn.
                if (timeout > 0) {
                    // Send a message back to client.
                    message = memory_malloc(sizeof(char) * 256);
                    sprintf(message, "%s;kicked\n", player_ptr->id);
                    svr_send(player_ptr->socket, message);
                    memory_free(message);

                    svr_disconnect(player_ptr, player_ptr->game);
                }

                timeout++;
            } else {
                timeout = 0;
            }

        } else { // Successful.
            bytes_received += read_size;
            messages_received++;

            printf("<<<--- %s", cbuf);
            _svr_process_request(cbuf);
            memset(cbuf, 0, 1024 * sizeof(char));

            timeout = 0;
        }
    }

    // If the connection is lost - svr_disconnect & remove the player.
    player_ptr = player_find(id);
    if (player_ptr) {
        player_ptr->is_disconnected = 1;

        if (player_ptr->game) {
            svr_disconnect(player_ptr, player_ptr->game);
        }

        player_remove(player_ptr);
    }

    return 0;
}

/// Obtaining data from client about player creation. Create a player.
/// \param arg      Client socket.
/// \return         NULL.
void *_svr_connection_handler(void *arg) {
    int client_sock = *(int *) arg;
    char msg[64];
    char *id = NULL;
    char *tokens = NULL;
    player_t *player = NULL;
    char *log_message = NULL;
    char *nickname = NULL;
    char *message = NULL;

    // Fill the msg var with zeros.
    memset(msg, 0, strlen(msg));

    // Count stats.
    bytes_received += recv(client_sock, msg, sizeof(char) * 64, 0);
    messages_received++;

    // Set socket params (receive timeout).
    setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));

    id = strtok(msg, ";"); // Expecting message like "1;nickname;John;".
    if (id) {
        tokens = strtok(NULL, ";");
    } else {
        messages_bad++;
    }

    // If the message from client contains nickname
    if (tokens != NULL && strcmp(tokens, "nickname") == 0) {
        nickname = strtok(NULL, ";");

        if (!nickname) {
            nickname = memory_malloc(sizeof(char) * 10);
            sprintf(nickname, "Player"); // Default player name.
        }

        // Create player.
        player = player_create(client_sock, nickname);

        // Send a message back to client.
        message = memory_malloc(sizeof(char) * 256);
        sprintf(message, "%s;id\n", player->id);
        svr_send(player->socket, message);
        memory_free(message);

        // Add player to the list.
        player_add(player);

        // Create a new thread to solve player data sending separately.
        thread_id = 0;
        pthread_create(&thread_id, NULL, _svr_serve_recieving, (void *) player);

        // Log.
        log_message = memory_malloc(sizeof(char) * 256);
        sprintf(log_message, "\t> Player %s (ID: %s) connected!\n", player->nickname, player->id);
        write_log(log_message);
        memory_free(log_message);
    } else {
        // Log.
        log_message = memory_malloc(sizeof(char) * 256);
        sprintf(log_message, "\t> Player could not be added!\n");
        write_log(log_message);
        memory_free(log_message);

        messages_bad++;
    }

    memory_free(arg);

    return NULL;
}

/// Create a new socket. Listening to client connection.
/// \param arg      Port number.
void *_svr_serve_connection(void *arg) {
    int server_socket,
            client_socket;
    int return_value;
    int *thread_socket = NULL;
    int port = *(int *) arg;
    int flag = 1;
    char *log_message = NULL;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    socklen_t remote_addr_len;

    // Set timeout to 60sec.
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;

    // Create a new server socket.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket <= 0)
        return NULL;

    // Set socket params.
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &flag, sizeof(int));

    // Sets all values to zero.
    memset(&local_addr, 0, sizeof(struct sockaddr_in));

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons((u_short) port);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to a address (address of the current host and port on which the server will run).
    return_value = bind(server_socket, (struct sockaddr *) &local_addr, sizeof(struct sockaddr_in));
    if (return_value == 0) {
        printf("\t> Bind: OK!\n");
    } else {
        printf("\t> Bind: ERROR!\n");
        exit(1);
    }

    // Allow to listen on the socket.
    return_value = listen(server_socket, 5); // 5 recommended.
    if (return_value == 0) {
        printf("\t> Listen: OK!\n");
    } else {
        printf("\t> Listen: ERROR!\n");
        exit(1);
    }

    for (;;) {
        // Block the process until a client connect to the server.
        // Returns a new file descriptor, and all communication on this connection should be done using the new file descriptor.
        client_socket = accept(server_socket, (struct sockaddr *) &remote_addr, &remote_addr_len);

        if (client_socket > 0) {
            thread_socket = memory_malloc(sizeof(int));
            *thread_socket = client_socket;

            if (pthread_create(&thread_id, NULL, (void *) &_svr_connection_handler, (void *) thread_socket)) {
                // Log.
                log_message = memory_malloc(sizeof(char) * 256);
                sprintf(log_message, "\t> Fatal ERROR during creating a new thread!\n");
                write_log(log_message);
                memory_free(log_message);
            }
        } else {
            // Log.
            log_message = memory_malloc(sizeof(char) * 256);
            sprintf(log_message, "\t> Fatal ERROR during socket processing!\n");
            write_log(log_message);
            memory_free(log_message);

            exit(1);
        }
    }
}

/// Process received message from a client.
/// \param message      The message.
void _svr_process_request(char *message) {
    if (!message)
        return;

    char **tokens = _svr_split_message(message);
    char *id;
    player_t *player = NULL;
    game_t *game = NULL;

    if (tokens[0]) {
        id = tokens[0];
        player = player_find(id);

        if (!player) {
            _svr_count_bad_message(message);
            return;
        }

        game = player->game;

    } else {
        _svr_count_bad_message(message);
    }

    // List of events which server accepts from client side.
    if (tokens[1]) {
        if (strcmp(tokens[1], "show_games") == 0) {
            game_broadcast_board_info();
        } else if (strcmp(tokens[1], "new_game") == 0) {
            game_create(player);
        } else {
            _svr_count_bad_message(message);
        }

    } else {
        _svr_count_bad_message(message);
    }

    memory_free(tokens);
}

/// Split a message to tokens.
/// \param message      The message.
/// \return             Split message / tokens.
char **_svr_split_message(char *message) {
    if (!message)
        return NULL;

    char **message_split = memory_malloc(MAX_TOKENS * sizeof(char *));
    int i = 0;

    message_split[i] = strtok(message, ";");

    if (message_split[i] == NULL)
        return message_split;

    i++;
    while (i < MAX_TOKENS) {
        message_split[i] = strtok(NULL, ";");

        if (message_split[i] == NULL)
            return message_split;

        i++;
    }

    return message_split;
}

/// Call this if incorrect message received.
/// \param message      The message.
void _svr_count_bad_message(char *message) {
    printf("\t> Ignored message: \"%s\".\n", message);
    messages_bad++;
}

/// The main method of the application. It creates a server with entered port number. If the port number is not submited the server will be create with the port number 10000.
/// \param argv -
/// \param args -
/// \return Status code of success.
int main(int argv, char *args[]) {
    int port;
    char *log_message = NULL;
    char input[1024];
    time(&time_initial);

    colors_init();

    // Initialize log file.
    FILE *logs = fopen("server.log", "w");
    fclose(logs);

    // Log.
    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Server is starting... /%s", asctime(localtime(&time_initial)));
    write_log(log_message);
    memory_free(log_message);

    // Listen to custom port.
    if (argv > 1) {
        port = atoi(args[1]);

        if (port >= CUSTOM_PORT_LOWEST_POSSIBLE && port <= CUSTOM_PORT_HIGHEST_POSSIBLE) {
            printf("\t> Setting up the port (%d).\n", port);
        } else {
            printf("\t> Setting up the default port.\n");
            port = DEFAULT_PORT;
        }
    } else {
        printf("\t> Setting up the default port (%d).\n", DEFAULT_PORT);
        port = DEFAULT_PORT;
    }

    // Log.
    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Server is running on the port: %d.\n", port);
    write_log(log_message);

    thread_id = 0;
    if (pthread_create(&thread_id, NULL, _svr_serve_connection, (void *) &port) != 0) {
        // Log.
        log_message = memory_malloc(sizeof(char) * 256);
        sprintf(log_message, "\t> Fatal ERROR during creating a new thread!\n");
        write_log(log_message);
        memory_free(log_message);
    }

    while (scanf("%s", input) != -1) {
        if (strcmp(input, "quit") == 0)
            break;
        if (strcmp(input, "info") == 0)
            print_info(stdout);
//        if (strcmp(input, "games") == 0)
//            print_games();
//        if (strcmp(input, "players") == 0)
//            print_players();
    }

    pthread_cancel(thread_id);

    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "1;stop_server\n");
//    svr_broadcast(log_message);

    colors_free();
//    free_players();
//    free_games();

    log_message = memory_malloc(sizeof(char) * 256);
    sprintf(log_message, "\t> Server is shutting down.\n");
    write_log(log_message);
    memory_free(log_message);

    write_stats();

    print_info(stdout);

    return 0;
}