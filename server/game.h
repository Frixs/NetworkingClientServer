//
// Created by Frixs on 10.10.2018.
//

#ifndef SERVER_GAME_H
#define SERVER_GAME_H

//game_t *game_find(char *id);
void game_broadcast_update_games();
void game_send_update_players(game_t *game);
void game_send_current_state_info(game_t *game);
void game_create(player_t *player, int goal);
void game_add(game_t *game);
void game_remove(game_t *game);
void game_destroy(game_t *game);
void game_multicast(game_t *game, char *message);
//void game_free();
//void game_print();

#endif //SERVER_GAME_H
