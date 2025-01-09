#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <stdbool.h>
#include "list.h"

#include <ncurses.h>
#include "utils.h"



#define SERVER_ADDRESS "tcp://*:5555" // Server binds to this address
#define ADDRESS "tcp://127.0.0.1:5555"
#define PUBSUB_ADDRESS "tcp://127.0.0.1:5556"
#define SERVERSHUTDOWN_PUBSUBADDRESS "tcp://127.0.0.1:5557"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_PLAYERS 8
#define START_ALIENS_COUNT 85 
#define GRID_SIZE 20
#define MAX_MSG_LEN 256

typedef enum{
    HORIZONTAL=0, 
    VERTICAL, 
} moving_direction;

typedef struct{
    int x;
    int y;
} position;

typedef struct {
    position pos;
    char shot_symbol;
    long long end_time;
} Shot;

typedef enum {
    LEFT_TO_RIGHT=0, 
    RIGHT_TO_LEFT=1, 
    UP_TO_DOWN=2,
    DOWN_TO_UP=3, 
    ERROR=4, 
} firing_direction;

typedef struct {
    int x; 
    int y;
    long long next_move;
} Alien; 

typedef struct {
    int x;
    int y;
    char name;
    int score;
    bool stunned; 
    long long end_stun_time;
    long long next_shot_time;
} Player;

// Game elements
typedef struct {
    Player players[MAX_PLAYERS];
    bool players_name_used[MAX_PLAYERS];
    bool game_over;
    List *aliens; 
    List *shots;
    char grid[GRID_SIZE][GRID_SIZE];
    unsigned int player_count;
    long long last_alien_killed;
} GameState;

/**
 * @brief Draws the border of the game
 * This includes the square border around the game board, as well as the column and row identification number for easier gameplay
 */
void draw_border_with_numbers();

/**
 * @brief Initializes the ncurses library
 */
void init_ncurses();

static inline long long ms_since_epoch() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

#endif