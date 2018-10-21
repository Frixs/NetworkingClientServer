cmake_minimum_required(VERSION 3.9)
project(server C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -pthread)

add_executable(server server.c stats.c stats.h constants.h server.h player.c player.h structs.h colors.c colors.h game.c game.h game_logic.c game_logic.h memory.c memory.h)