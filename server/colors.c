//
// Created by Frixs on 9.10.2018.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "constants.h"
#include "colors.h"
#include "memory.h"

char *g_color_list[PLAYER_COUNT + COLORS_STATUS_COUNT];

/// Initialize colors.
void colors_init() {
    g_color_list[0] = memory_malloc(sizeof(char) * 7);
    strcpy(g_color_list[0], "0000FF");

    g_color_list[1] = memory_malloc(sizeof(char) * 7);
    strcpy(g_color_list[1], "FF0000");

    g_color_list[2] = memory_malloc(sizeof(char) * 7);
    strcpy(g_color_list[2], "505050"); // DC-ing color.
}

/// Free memory.
void colors_free() {
    int i;
    for (i = 0; i < PLAYER_COUNT; i++) {
        memory_free(g_color_list[i]);
        g_color_list[i] = NULL;
    }
}