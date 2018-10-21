//
// Created by Frixs on 8.10.2018.
//

#ifndef SERVER_MAIN_H
#define SERVER_MAIN_H

extern player_t *g_player_list;
extern pthread_mutex_t g_player_list_mutex;
extern game_t *g_game_list;
extern pthread_mutex_t g_game_list_mutex;

void svr_send(int socket, char *message);
void svr_connect(player_t *player, game_t *game);
void svr_disconnect(player_t *player, game_t *game);
int _svr_find_id(char *id);
char *svr_generate_id();
void svr_broadcast(char *message);
void *_svr_serve_recieving(void *arg);
void *_svr_connection_handler(void *arg);
void *_svr_serve_connection(void *arg);
void _svr_process_request(char *message);
char **_svr_split_message(char *message);
void _svr_count_bad_message(char *message);

#endif //SERVER_MAIN_H
