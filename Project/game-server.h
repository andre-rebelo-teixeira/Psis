#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <ncurses.h>
#include <time.h>
#include <zmq.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "list.h"
#include "messages.h"
#include "score_message.pb-c.h"
#include "utils.h"

// Function declarations
void random_move_aliens(GameState *state, time_t current_time);

void populate_grid_from_gamestate(GameState *state);

void update_game_state(GameState *state, time_t current_time);

// Drawing functions
void draw_shots(GameState *state);
void clear_board(GameState *state);
void draw_game_state(GameState *state);

// Game logic functions
GameState* init_game();
bool handle_astronaut_zap(GameState *state, message msg);
message handle_new_message(GameState *state, message msg);
void handle_astronaut_move(GameState *state, message msg);
char handle_astronaut_connect(GameState *state, message msg);
void handle_astronaut_disconnect(GameState *state, message msg);

void serialize_message(const message *msg, char *buffer, size_t *buffer_size);
void serialize_score_message(message* msg, uint8_t** buffer, size_t* size);

// Process separation functions

void *game_handler(void *arg);
void *alien_handler(void *arg);

// Function definition
// 
// Multiple function are define in the header file since they are simple enough to be defined in the header, they are initialization function

/**
 * @brief Get a random number inside a range
 * This function is inline since is a very simple function that is called multiple time during the code, making it faster
 * 
 * @param min 
 * @param max 
 * @return unsigned int 
 */
static inline unsigned int get_random_number(unsigned int min, unsigned int max) {
    return min + (rand() % (max - min + 1));
};

/**
 * @brief Return the default starting position of an astronaut in the grid
 * This position is well defined according to the astronaut name, meaning that each astronaut will always start from the same position. This is not the most correct way to do it, since it removed randomness in the game, but overall was the fastest way to implement it, since this way whenever a astronaut connect, we dont need to find on of the 8 allowed row's/col's from where astronauts can shoot at aliens 
 * 
 * @param astronaut_name the name of the player (should be a letter with values between A and H)
 * @return position the position in the grid array of the state where we should input the astronaut (if the astronaut name is invalid, the x and y values will be -1)
 */
static inline position get_start_position(char astronaut_name) {
    position pos;
    pos.x = -1; 
    pos.y = -1;

    // Fastest and more efficient way to implement this, since we don't need sequential for loops to find the astronaut
    switch (astronaut_name){
        case 'A':
            pos.x = 0;
            pos.y = GRID_SIZE / 2; 
            break;
        case 'B':
            pos.x = GRID_SIZE / 2;
            pos.y = 0;
            break;
        case 'C':
            pos.x = GRID_SIZE - 1;
            pos.y  = GRID_SIZE / 2;
            break;
        case 'D':
            pos.x = GRID_SIZE / 2;
            pos.y = GRID_SIZE - 1;
            break;
        case 'E':
            pos.x = 1;
            pos.y = GRID_SIZE / 2;
            break;
        case 'F':
            pos.x = GRID_SIZE / 2;
            pos.y = 1;
            break;
        case 'G':   
            pos.x = GRID_SIZE - 2;
            pos.y = GRID_SIZE / 2;
            break;
        case 'H':
            pos.x = GRID_SIZE / 2;
            pos.y = GRID_SIZE - 2;
            break;
        default:
            break;
    }

    return pos;
}

/**
 * @brief Get the moving direction object
 * 
 * @param astronaut_name the name of the player (should be a letter with values between A and H)
 * @return moving_direction the direction in which the astronaut can move
 */
static inline firing_direction get_firing_direction(char astronaut_name){
    firing_direction direction = ERROR;

    switch (astronaut_name) {
        case 'A':
        case 'E':
            direction = LEFT_TO_RIGHT;
            break;
        case 'B':
        case 'F':
            direction = UP_TO_DOWN;
            break;
        case 'C':
        case 'G':
            direction = RIGHT_TO_LEFT;
            break;
        case 'D':
        case 'H':
            direction = DOWN_TO_UP;
            break;
        default:
            break;
    }

    return direction;

}

static inline moving_direction get_moving_direction(char astronaut_name){
    moving_direction direction = HORIZONTAL;

    switch (astronaut_name) {
        case 'A':
        case 'E':
        case 'C':
        case 'G':
        direction = VERTICAL;
            break;
        case 'B':
        case 'F':
        case 'D':
        case 'H':
            direction = HORIZONTAL;
            break;
        default:
            break;
    }

    return direction;
}

/**
 * @brief This function will update the position of the shot based on the direction it was fired
 * 
 * @param shot the current position of the shot
 * @param shot_direction the direction in which the shot was fired
 * @return position the updated position of the shot
 */
static inline position update_shot_pos(position shot, firing_direction shot_direction) {
    switch (shot_direction) {
        case LEFT_TO_RIGHT:
            shot.x += 1;
            break;
        case RIGHT_TO_LEFT:
            shot.x -= 1;
            break;
        case UP_TO_DOWN:
            shot.y += 1;
            break;
        case DOWN_TO_UP:
            shot.y -= 1;
            break;
        default:
            break;
    }
    return shot;
}
#endif // GAME_SERVER_H
