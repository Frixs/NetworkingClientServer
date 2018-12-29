//
// Created by Frixs on 9.10.2018.
//

#ifndef SERVER_STRUCTS_H
#define SERVER_STRUCTS_H

#include <semaphore.h>
#include <netinet/in.h>

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
    char *client_addr;
    char *color;
    int score;
    int choice;
    struct theplayer *next;
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
    struct thegame *next;
} game_t;

typedef struct theremoteconnection {
    char *client_address;
    int client_address_len;
    int client_socket;
} remote_connection_t;

#endif //SERVER_STRUCTS_H
