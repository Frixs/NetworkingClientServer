//
// Created by Frixs on 10.10.2018.
//

#ifndef SERVER_GAME_LOGIC_H
#define SERVER_GAME_LOGIC_H

void _game_logic_evaluate(game_t *g);
void game_logic_record_turn(player_t *p, int c);
void game_logic_prepare_player_turn(player_t *p);
void game_logic_prepare_player_on_game_join(player_t *p);
void _game_logic_count_score(game_t *g);
int _game_logic_compare_choices(choice_t c1, choice_t c2);
void _game_logic_set_score_to_all_by_choice(game_t *g, choice_t c);
player_t *_game_logic_check_winner(game_t *g);

#endif //SERVER_GAME_LOGIC_H
