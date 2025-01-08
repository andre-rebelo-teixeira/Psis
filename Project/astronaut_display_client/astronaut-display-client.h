#ifndef __ASTRONAUT_DISPLAY_CLIENT_H__
#define __ASTRONAUT_DISPLAY_CLIENT_H__

#include <ncurses.h>
#include <zmq.h>

#include "messages.h"
#include "utils.h"

void draw_avatar_game(unsigned int scores[8], char grid[20][20], char current_players[8], bool game_over, char);

/**
    * @brief This is the function that must read the socket from the server whenever updates to the display are made and draw them on the screen
    * @param arg Data passed to the function
    * @return void* The return value of the function
 */
void *display_client(void *arg);

void *input_client(void *arg);

client_to_server_message create_message(int key, char player_char);

void deserialize_message(const char *, display_update_message *, size_t);

#endif // __ASTRONAUT_DISPLAY_CLIENT_H__

