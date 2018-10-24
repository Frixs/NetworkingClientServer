//
// Created by Frixs on 9.10.2018.
//

#ifndef SERVER_PLAYER_H
#define SERVER_PLAYER_H

player_t *player_create(int socket, char *nickname);
void player_remove(player_t *player);
void _player_destroy(player_t *player);
player_t *player_find(char *id);
void player_add(player_t *player);
void player_connect_to_game(player_t *player, game_t *game);
void player_disconnect_from_game(player_t *player, game_t *game);
//void player_print();

#endif //SERVER_PLAYER_H
