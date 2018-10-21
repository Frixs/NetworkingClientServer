//
// Created by Frixs on 9.10.2018.
//

#define COLORS_STATUS_COUNT 1

#ifndef SERVER_COLORS_H
#define SERVER_COLORS_H

extern char *g_color_list[PLAYER_COUNT + COLORS_STATUS_COUNT];

void colors_init();
void colors_free();

#endif //SERVER_COLORS_H
