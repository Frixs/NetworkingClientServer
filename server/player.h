//
// Created by Frixs on 9.10.2018.
//

#ifndef SERVER_PLAYER_H
#define SERVER_PLAYER_H

player_t *player_create(int socket, char *nickname);
void player_remove(player_t *player);
void player_destroy(player_t *player);
player_t *player_find(char *id);
void player_add(player_t *player);
//void player_print();

#endif //SERVER_PLAYER_H
