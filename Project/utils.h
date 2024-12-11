#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include "list.h"


#define SERVER_ADDRESS "tcp://*:5555" // Server binds to this address
#define ADDRESS "tcp://127.0.0.1:5555"
#define PUBSUB_ADDRESS "tcp://127.0.0.1:5556"

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
    time_t end_time;
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
    time_t next_move;
} Alien; 

typedef struct {
    int x;
    int y;
    char name;
    int score;
    bool stunned; 
    time_t end_stun_time;
    time_t next_shot_time;
} Player;

// Game elements
typedef struct {
    Player players[MAX_PLAYERS];
    bool players_name_used[MAX_PLAYERS];
    unsigned int player_count;
    List *aliens; 
    List *shots;
    char grid[GRID_SIZE][GRID_SIZE];
} GameState;

void draw_game_state(GameState *state);
void init_ncurses();
void draw_border_with_numbers();

#endif