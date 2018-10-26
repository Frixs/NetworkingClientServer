//
// Created by Frixs on 10.10.2018.
//

#ifndef SERVER_GAME_LOGIC_H
#define SERVER_GAME_LOGIC_H

void game_logic_evaluate(game_t *g);
void game_logic_record_turn(player_t *p, int c);
void game_logic_prepare_player_turn(player_t *p);
void game_logic_prepare_player_on_game_join(player_t *p);

#endif //SERVER_GAME_LOGIC_H
