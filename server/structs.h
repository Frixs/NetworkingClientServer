//
// Created by Frixs on 9.10.2018.
//

#ifndef SERVER_STRUCTS_H
#define SERVER_STRUCTS_H

#include <semaphore.h>

typedef enum thechoice {
    ROCK        = 1,
    PAPER       = 2,
    SCISSORS    = 3,
} choice_t;

typedef struct theplayer {
    int socket;
    char *id;
    int is_disconnected;
    char *nickname;
    char *color;
    int score;
    int choice;
    struct theplayer *next_player;
    struct thegame *game;

} player_t;

typedef struct thegame {
    char *id;
    char *name;
    int goal;
    player_t *players[2];
    int player_count;
    int in_progress;
    pthread_t thread;
    sem_t sem_on_turn;
    struct thegame *next_game;
} game_t;

#endif //SERVER_STRUCTS_H
