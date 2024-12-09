#include <ncurses.h>
#include <time.h>
#include <zmq.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "list.h"
#include<unistd.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_PLAYERS 8
#define START_ALIENS_COUNT 4 
#define GRID_SIZE 20
#define MAX_MSG_LEN 256
typedef struct {
    int x; 
    int y;
} Alien; 

typedef struct {
    int x;
    int y;
    int fd; 
    int score;
    bool stunned; 
    time_t end_stun_time;
} Player;

// Game elements
typedef struct {
    Player players[MAX_PLAYERS];
    List *aliens; 
    char grid[GRID_SIZE][GRID_SIZE];
} GameState;

static inline unsigned int get_random_number(unsigned int min, unsigned int max)
{
    return min + (rand() % (max - min + 1));
}


void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    curs_set(0); // Hide cursor
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Astronauts
    init_pair(2, COLOR_RED, COLOR_BLACK);   // Aliens
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);// Laser beams
    init_pair(4, COLOR_WHITE, COLOR_BLACK); // Borders and text
}

void draw_border_with_numbers() {
    // Enable the desired color pair for the border
    attron(COLOR_PAIR(4));
    char border_number[] = "01234567890123456789";
    char vertical_border[] = "+------------------+";

    // Top border with column numbers
    mvprintw(0, 2, "%s", border_number); // Print column numbers above the top border
    mvprintw(1, 2, "%s", vertical_border);                // Top-left corner

    // Left and right borders with row numbers
    for (unsigned int row = 0; row < GRID_SIZE; row++) {
        mvprintw(row + 3, 0, "%c", border_number[row]); // Print row numbers to the left
        mvprintw(row + 3, 1, "|");                     // Draw left border
        mvprintw(row + 3, GRID_SIZE + 2, "|");         // Draw right border
    }

    // Bottom border with column numbers
    mvprintw(GRID_SIZE + 3, 1, "+"); // Bottom-left corner
    for (int col = 0; col < GRID_SIZE; col++) {
        mvprintw(GRID_SIZE + 3, col + 2, "-"); // Bottom border line
    }
    mvprintw(GRID_SIZE + 3, GRID_SIZE + 2, "+"); // Bottom-right corner
    mvprintw(GRID_SIZE + 4, 2, "%s", border_number); // Print column numbers below the bottom border

    // Turn off the color pair
    attroff(COLOR_PAIR(4));
}



void draw_game_state(GameState *state) {
    draw_border_with_numbers();

    // Draw grid
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            char ch = state->grid[y][x];
            if (ch >= 'A' && ch <= 'H') {
                attron(COLOR_PAIR(1)); // Astronauts
            } else if (ch == '*') {
                attron(COLOR_PAIR(2)); // Aliens
            } else if (ch == '-' || ch == '|') {
                attron(COLOR_PAIR(3)); // Laser beams
            }
            mvprintw(y + 1, x + 2, "%c", ch); // Adjusted for border
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3));
        }
    }

    // Draw scores
    attron(COLOR_PAIR(4));
}

void update_game_state(GameState *state, const char *message) {
    if (strncmp(message, "UPDATE", 6) == 0) {
        char *token = strtok((char *)message, "|");
        while (token != NULL) {
            if (strncmp(token, "UPDATE", 6) == 0) {
                // Parse grid update: UPDATE|<row>,<col>,<char>
                char *update = strtok(NULL, "|");
                if (update) {
                    int row, col;
                    char ch;
                    sscanf(update, "%d,%d,%c", &row, &col, &ch);
                    state->grid[row][col] = ch;
                }
            } else if (strncmp(token, "SCORES", 6) == 0) {
                // Parse scores: SCORES|A=10,B=20,...
                char *scores = strtok(NULL, "|");
                if (scores) {
                    char astronaut;
                    int score, idx;
                    char *score_token = strtok(scores, ",");
                    while (score_token != NULL) {
                        sscanf(score_token, "%c=%d", &astronaut, &score);
                        idx = astronaut - 'A';
                        score_token = strtok(NULL, ",");
                    }
                }
            }
            token = strtok(NULL, "|");
        }
    }
}

GameState* move_aliens_at_random(GameState *state) {
    if (state == NULL) {
        fprintf(stderr, "Game state is NULL");
        return NULL;
    }

    if (state->aliens == NULL) {
        fprintf(stderr, "Aliens list is NULL");
        return state;
    }

    // Move the aliens to random positions
    for (unsigned int i = 2; i < GRID_SIZE - 2; i++) {
        for (unsigned int j = 2; j < GRID_SIZE - 2; j++) {
            state->grid[i][j] = ' ';
        }
    } 

    Node *current = state->aliens->head;

    while(current != NULL) {
        Alien *alien = (Alien *)current->data;
        int delta_x = get_random_number(-1, 1);
        int delta_y = get_random_number(-1, 1);

        alien->x = min(max(alien->x + delta_x, 2), GRID_SIZE - 2 - 1);
        alien->y = min(max(alien->y + delta_y, 2), GRID_SIZE - 2 - 1);
        state->grid[alien->y][alien->x] = '*';
        current = current->next;
    }

    return state;
}


GameState* init_game() {
    GameState* state = (GameState*)calloc(1, sizeof(GameState));

    if (state == NULL) {
        perror("Failed allocation of memory in init game");
        return NULL;
    }

    // Initialize an empty grid
    memset(state->grid, ' ', sizeof(state->grid));

    // Initialize list of players 
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].x = 0;
        state->players[i].y = 0;
        state->players[i].fd = -1;
        state->players[i].score = 0;
        state->players[i].stunned = false;
        state->players[i].end_stun_time = 0;
    }

    // Initialize aliens
    state->aliens = create_list();
    if (state->aliens == NULL) {
        perror("Failed to create list of aliens");
        return NULL;
    }

    // Initialize the correct number os aliens in the board 
    for (unsigned int i = 0; i < START_ALIENS_COUNT; i++) {
        Alien *alien = (Alien *)calloc(1, sizeof(Alien));

        if (alien == NULL) {
            perror("Failed to allocate alien");
        }

        alien->x = get_random_number(2, GRID_SIZE - 2);
        alien->y = get_random_number(2, GRID_SIZE - 2);
        insert_node(state->aliens, alien);
    }

    return state;
}

int main() {
    // Initialize NCurses
    init_ncurses();
    
    // Initialize Game state
    GameState * state = init_game() ;

    // Initialize game state
    // ZeroMQ context and socket
    //void *context = zmq_ctx_new();
    //void *socket = zmq_socket(context, ZMQ_SUB);
    //zmq_connect(socket, "tcp://localhost:5555");
    //zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);

    //char buffer[MAX_MSG_LEN];

    while (1) {
        // Receive a message from the server
        //zmq_recv(socket, buffer, MAX_MSG_LEN - 1, 0);
        
        //buffer[MAX_MSG_LEN - 1] = '\0'; // Null-terminate the message

        // Update the game state
        //update_game_state(&state, buffer);

        // Redraw the screen
        clear();
        state = move_aliens_at_random(state);
        draw_game_state(state);
        sleep(1);
        refresh();
        
    }

    //// Cleanup
    //endwin();
    //return 0;
}
