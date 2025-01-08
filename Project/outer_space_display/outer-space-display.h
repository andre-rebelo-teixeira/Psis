#ifndef __OUTER_SPACE_DISPLAY_H__
#define __OUTER_SPACE_DISPLAY_H__

#include "messages.h"
#include "utils.h"

void deserialize_message(const char *buffer, display_update_message *msg, size_t buffer_size);

void draw_avatar_game(unsigned int scores[8], char grid[20][20], char current_players[8], bool game_over);

#endif // __OUTER_SPACE_DISPLAY_H__