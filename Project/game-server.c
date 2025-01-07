#include <ncurses.h>
#include <time.h>
#include <zmq.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <pthread.h>

#include <sys/types.h>

#include<unistd.h>

#include "list.h"
#include "messages.h"
#include "score_message.pb-c.h"
#include "utils.h"
#include "game-server.h"


pthread_mutex_t game_state_mutex;
bool server_shutdown = false;
typedef struct {
    bool *server_shutdown;
    GameState *state; 
    void *zmq_context;
} thread_args;


/** 
 * @brief This function populates the grid of the state according to the current position of alliens, shots and players, in this order, meaning that if a player is in the same position as an shot, the player will be drawn on top of the alien 
 *
 * @param state the current state of the game
 *
 * @return void
 */
void populate_grid_from_gamestate(GameState *state){
    // Clean the previous grid to avoid having elements in the previous position
    for (unsigned int i = 0; i < GRID_SIZE; i++) {
        for (unsigned int j = 0; j < GRID_SIZE; j++) {
            state->grid[i][j] = ' ';
        }
    }

    // Draw all the aliens in the correct position in the grid
    Node *current = state->shots->head;
    while(current != NULL) {
        Shot *shot = (Shot *)current->data;
        state->grid[shot->pos.y][shot->pos.x] = shot->shot_symbol;
        current = current->next;
    }

    // Draw all the aliens in the correct position in the grid
    current = state->aliens->head;
    while(current != NULL) {
        Alien *alien = (Alien *)current->data;
        state->grid[alien->y][alien->x] = '*';
        current = current->next;
    }

    // Draw all players in the correct position in the grid
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].name != ' ') {
            if (state->players[i].stunned) {
                state->grid[state->players[i].y][state->players[i].x] = 'a' + state->players[i].name - 'A';
            } else {
                state->grid[state->players[i].y][state->players[i].x] = state->players[i].name;
            }
        }
    }
}

/**
 * @brief  This function will update the game state, removing all the shots that have expired 
 *
 * @param state the current state of the game
 * @param current_time the current time of the game
 *
 * @return void
 */
void update_game_state(GameState *state, long long current_time) {
    // Remove expired shots
    Node * current = state->shots->head;
    while (current != NULL) {
        Shot *shot = (Shot *)current->data;
        if (current_time > shot->end_time) {
            Node *next = current->next;
            remove_node(state->shots, current, free);
            current = next;
        } else {
            current = current->next;
        }
    }



    // Repopulate Aliens if needed
    // Only repopulate if the less kill has been made more than 10 seconds ago
    if(state->last_alien_killed + 10 * 1000 < current_time && state->aliens->size < 256) {
        unsigned int delta = state->aliens->size * 0.1 + 1;
        state->last_alien_killed = current_time;

        // Here we are limiting the number of aliens to 256 even tho it is not specified in the requirements, but not only do we avoid memory issues, but we also would fall in the problem of having more aliens than space in the board for them 
        while(delta > 0 && state->aliens->size < 256) {
            delta--;
            Alien *alien = (Alien *) calloc(1, sizeof(Alien));
            if (alien == NULL) {
                perror("Failed to allocate memory for alien");
                exit(EXIT_FAILURE);
            }
            alien->x = get_random_number(2, GRID_SIZE - 2 - 1);
            alien->y = get_random_number(2, GRID_SIZE - 2 - 1);
            alien->next_move = current_time + 1;
            insert_node(state->aliens, alien);
        }
    }
    

    // Remove Stuneed status from players
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].stunned && current_time > state->players[i].end_stun_time) {
            state->players[i].stunned = false;
        }
    }

    return;
}

/**
 * @brief Draws the grid of the game, the scores, the players, the aliens and the shots
 *
 * @param state the current state of the game
 *
 * @return void
 */
void draw_game_state(GameState *state) {
    draw_border_with_numbers();

    // Draw grid
    if (!state->game_over) {
        mvprintw(1, GRID_SIZE + 10, "Number of aliens alive %d", state->aliens->size);
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
                if (ch != ' '){
                    mvprintw(y + 2, x + 2, "%c", ch); // Adjusted for border
                }
                attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3));
            }
        }
    } else {
        mvprintw(10, 10, "G");
        mvprintw(10, 11, "A");
        mvprintw(10, 12, "M");
        mvprintw(10, 13, "E");;

        mvprintw(12, 11, "H");
        mvprintw(12, 12, "A");
        mvprintw(12, 13, "S");

        mvprintw(14, 10, "E");
        mvprintw(14, 11, "N");
        mvprintw(14, 12, "D");
        mvprintw(14, 13, "E");
        mvprintw(14, 14, "D");
     // ended    
    }

    unsigned int highest_score_index = 0;
    
    for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].score > state->players[highest_score_index].score)
            highest_score_index = i;    
    } 
    

    // Draw score
    mvprintw(2, GRID_SIZE + 10,  "score");
    unsigned int num_players_on = 1;
    for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].name != ' ') {
            if  (state->game_over && i == highest_score_index) {
                attron(COLOR_PAIR(1));
            }

            mvprintw(2 + num_players_on++, GRID_SIZE + 10,  "%c-%d\n", state->players[i].name, state->players[i].score);
            attroff(COLOR_PAIR(1));
        }
    }


    // Draw scores
    attron(COLOR_PAIR(4));
}

/**
 * @brief This function will handle the astronaut connect message
 * 
 * @param state the current state of the game
 * @param msg the message received from the client
 *
 * @return char the name of the astronaut that was assigned to the player 
 */
char handle_astronaut_connect(GameState* state, client_to_server_message msg) {
    if (state == NULL) {
        fprintf(stderr, "Game state is NULL");
        return ' ';
    }

    if (msg.type != ASTRONAUT_CONNECT) {
        fprintf(stderr, "Invalid message type");
        return ' ';
    }

    if (state->player_count >= MAX_PLAYERS) {
        fprintf(stderr, "Maximum number of players reached");
        return ' ';
    }

    char name = 'A';

    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players_name_used[i]) {
            break;
        }
        name += 1;
    }


    // Find the first empty slot in the players array to add the new player
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        // If name of the player is an empty space, then the slot is empty
        if (state->players[i].name == ' ') {
            state->players[i].name = name;
            position pos = get_start_position(name);
            state->players[i].x = pos.x;
            state->players[i].y = pos.y;
            state->players_name_used[i] = true;
            state->players[i].next_shot_time = ms_since_epoch();
            state->player_count++;
            break;
        }
    }
    return name;
}

/**
 * @brief This function will handle the astronaut disconnect message
 * 
 * @param state the current state of the game
 * @param msg the message received from the client
 *
 * @return void
 */
void  handle_astronaut_disconnect(GameState *state, client_to_server_message msg) {
    if (state == NULL) {
        fprintf(stderr, "Game state is NULL");
        return;
    }

    if (msg.type != ASTRONAUT_DISCONNECT) {
        fprintf(stderr, "Invalid message type");
        return; 
    }


    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].name == msg.character) {
            state->players[i].name = ' ';
            state->players_name_used[i] = false;
            state->player_count--;
            state->players[i].x = -1;
            state->players[i].y = -1;
            state->players[i].score = 0;
            break;
        }
    }

    return;
}


/**
 * @brief This function will handle the astronaut move message
 * 
 * @param state the current state of the game
 * @param msg the message received from the client
 *
 * @return void   
 */
void handle_astronaut_move(GameState *state, client_to_server_message msg) {
    if (state == NULL) {
        fprintf(stderr, "Game state is NULL");
        return;
    }

    if (msg.type != ASTRONAUT_MOVEMENT) {
        fprintf(stderr, "Invalid message type");
        return;
    }

    Player p;
    unsigned int  j = 0;

    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].name == msg.character) {
            j = i;
            p = state->players[i];
            break;
        }
    }


    if (p.stunned && ms_since_epoch() < p.end_stun_time) {
        return;
    } else {
        p.stunned = false;
    }

    moving_direction direction = get_moving_direction(p.name);

    if (direction == HORIZONTAL) {
        if (msg.move == LEFT) {
            p.x--; 
        }
        else if (msg.move == RIGHT){
            p.x++;
        }
        p.x = min(max(p.x, 2), 17);
    }
    else if (direction == VERTICAL) {
        if (msg.move == UP){
            p.y--;
        } else if (msg.move == DOWN) {
            p.y++;
        }
        p.y = min(max(p.y, 2), 17);
    }

    state->players[j] = p;

    return;
}

/**
 * @brief This function will handle the astronaut zap message
 * 
 * @param state the current state of the game
 * @param msg the message received from the client
 * 
 * @return A boolean that is true if the game has ended -> This will mean the astronauts can no longer move or shot
 */
bool handle_astronaut_zap(GameState *state, client_to_server_message msg) {
    if (state == NULL) {
        fprintf(stderr, "Game state is NULL");
        return false;
    }

    if (msg.type != ASTRONAUT_ZAP) {
        fprintf(stderr, "Invalid message type");
        return false;
    }

    Player p;
    unsigned int j = 0;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].name == msg.character) {
            j = i;
            p = state->players[i];
            break;
        }
    }

    if (p.stunned && ms_since_epoch() < p.end_stun_time) {
        return false;
    } else {
        p.stunned = false;
    }

    if (ms_since_epoch() < p.next_shot_time) {
        return false;
    }
    p.next_shot_time = ms_since_epoch() + 3 * 1000;


    firing_direction direction = get_firing_direction(p.name);

    position shot = {p.x, p.y};
    shot = update_shot_pos(shot, direction);
    bool zapped_player[8] = {false};

    while (shot.x >= 0 && shot.x < GRID_SIZE && shot.y >= 0 && shot.y < GRID_SIZE) {
        
        Shot* s = (Shot *) calloc(1, sizeof(Shot));
        if (s == NULL) {
            perror("Failled to allocate memory for shot");
            exit(EXIT_FAILURE);
        }
        s->pos.x = shot.x;
        s->pos.y = shot.y;
        s->end_time = ms_since_epoch() + 0.5 * 1000;

        if (direction == LEFT_TO_RIGHT || direction == RIGHT_TO_LEFT) {
            s->shot_symbol = '-';
        } else {
            s->shot_symbol = '|';
        }

        insert_node(state->shots, s);

        // Check if the shot hit another player
        for (unsigned int i = 0; i < MAX_PLAYERS; i++){
            if (state->players[i].x == shot.x && state->players[i].y == shot.y) {
                zapped_player[i] = true;
            }
        }

        // Check if the shot it another alien
        Node *current = state->aliens->head;
        void (*free_ptr)(void*) = free;


        while(current != NULL) {
            Alien *alien = (Alien *)current->data;

            if (alien->x == shot.x && alien->y == shot.y) {
                state->last_alien_killed = ms_since_epoch();
                // Remove the alien from the list
                Node * next = current->next;
                remove_node(state->aliens, current, free_ptr);
                p.score++;

                current = next;
            } else {
                current = current->next;
            }
        }
        
        shot = update_shot_pos(shot, direction);
    }

    state->players[j] = p;

    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (zapped_player[i]) {
            state->players[i].stunned = true;
            state->players[i].end_stun_time = ms_since_epoch() + 10 * 1000; // Stun for 5 seconds
        }
    }

    if (state->aliens->size > 0) {
        return false;
    } 
    return true;
}

/**
 * @brief This function will handle the new message received from the client
 * 
 * @param state the current state of the game
 * @param msg the message received from the client
 *
 * @return message the response message to be sent back to the client
 */
server_to_client_message handle_new_message(GameState* state,  client_to_server_message msg) {
    server_to_client_message response; 
    response.character = msg.character;
    response.game_over = state->game_over;
    response.score = 0;
    response.status = true;
    char name = ' ';

    switch (msg.type) {
        case ASTRONAUT_CONNECT:
            response.character = handle_astronaut_connect(state, msg);
            if (response.character == ' ') {
                response.status = false;
            }

            break;
        case ASTRONAUT_MOVEMENT:
            if(!state->game_over) {
                handle_astronaut_move(state, msg);
            }
            break;
        case ASTRONAUT_ZAP:
            if (!state->game_over){
                state->game_over = handle_astronaut_zap(state, msg);
                response.score = state->players[name - 'A'].score;
            }
            break;
        case ASTRONAUT_DISCONNECT:
            handle_astronaut_disconnect(state, msg);
            break;
        case TICK: 
        case LAST_TICK:
            break;
        default:
            response.status = false;
            break;
    }

    return response;
}

/**
 * @brief Initialize the game state
 *
 * @return GameState* the created game state
 */
GameState* init_game(){ 
    GameState* state = (GameState*)calloc(1, sizeof(GameState));

    if (state == NULL) {
        perror("Failed allocation of memory in init game");
        return NULL;
    }

    state->player_count = 0;
    state->game_over = false;
    // Initialize an empty grid
    memset(state->grid, ' ', sizeof(state->grid));

    // Initialize list of players 
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].x = 0;
        state->players[i].y = 0;
        state->players[i].score = 0;
        state->players[i].name = ' ';
        state->players[i].stunned = false;
        state->players[i].end_stun_time = 0;
        state->players[i].next_shot_time = ms_since_epoch();
    }

    // Initialize aliens
    state->aliens = create_list();
    if (state->aliens == NULL) {
        perror("Failed to create list of aliens");
        return NULL;
    }

    state->shots = create_list();
    
    if (state->shots == NULL) {
        perror("Failed to create shots list");
        return NULL;
    }

    // Initialize the correct number os aliens in the board 
    for (unsigned int i = 0; i < START_ALIENS_COUNT; i++) {
        Alien *alien = (Alien *)calloc(1, sizeof(Alien));

        if (alien == NULL) {
            perror("Failed to allocate alien");
        }
        alien->next_move = ms_since_epoch();
        alien->x = get_random_number(2, GRID_SIZE - 2);
        alien->y = get_random_number(2, GRID_SIZE - 2);
        insert_node(state->aliens, alien);
    }

    state->last_alien_killed = ms_since_epoch();

    return state;
}

/**
 * @brief Serialize the update the message for it to be sent in a buffer to outer-space-display via the publisher
 * 
 * @param msg the message to be serialized
 * @param buffer the buffer where the message will be serialized
 * @param buffer_size the size of the buffer
 *
 * @return void
 */
void serialize_message(display_update_message *msg, char *buffer, size_t *buffer_size) {
    size_t offset = 0;


    // Copy the server shutdown flag 
    memcpy(buffer, &msg->server_shutdown, sizeof(msg->server_shutdown));
    offset += sizeof(msg->server_shutdown);


    // Copy the game over flag
    memcpy(buffer + offset, &msg->game_over, sizeof(msg->game_over));
    offset += sizeof(msg->game_over);

    // Copy the grid
    memcpy(buffer + offset, &msg->grid, sizeof(msg->grid));
    offset += sizeof(msg->grid);

    // Copy the scores
    memcpy(buffer + offset, &msg->scores, sizeof(msg->scores));
    offset += sizeof(msg->scores);
    
    // Copy the current players
    memcpy(buffer + offset, &msg->current_players, sizeof(msg->current_players));
    offset += sizeof(msg->current_players);

    *buffer_size = offset;
}

/**
 * @brief This function acts as the parent process of the game server. 
 * It is used to handle the game logic and the communication with both the child process, the client and the outer-space-display
 */
void *game_handler(void *arg){
    thread_args *args = (thread_args *)arg;
    GameState *state = args->state;
    
    client_to_server_message req;
    char buffer[1024];
    size_t buffer_size;

    // Open ZMQ context and socket
    void *context = args->zmq_context;
    void *socket = zmq_socket(context, ZMQ_REP); 

    // Bind the socket to the address
    if (zmq_bind(socket, ADDRESS) != 0) {
        perror("Parent responder zmq_bind failed");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    // Open PUB SUB connection
    void *pub_sub_socket = zmq_socket(context, ZMQ_PUB);
    // Bind the socket to the address
    if (zmq_bind(pub_sub_socket, PUBSUB_ADDRESS) != 0) {
        perror("Parent publisher zmq_bind failed");
        zmq_close(socket);
        zmq_close(pub_sub_socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    while(true) {
        client_to_server_message req;
        // Receive messages from an existing client or a new one
        zmq_recv(socket, &req, sizeof(client_to_server_message), 0);

        // process and create the response message to the client
        server_to_client_message rep = handle_new_message(state, req);

        // Answer to the client
        zmq_send(socket, &rep, sizeof(server_to_client_message), 0);

        // ncurses function to clear the full screen allowing for a full repaint of the game
        clear();

        // Lock the mutex to ensure no race conflicts in the game state  
        pthread_mutex_lock(&game_state_mutex);

        // Update the game state
        update_game_state(state, ms_since_epoch());
        populate_grid_from_gamestate(state);

        // Unlock the mutex
        pthread_mutex_unlock(&game_state_mutex);

        draw_game_state(state);

        // Refresh the screen with the new repainted game
        refresh();

        display_update_message update; 

        memcpy(update.grid, state->grid, sizeof(state->grid));
        update.game_over = state->game_over;
        update.server_shutdown =  false;
        for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].name != ' ') {
                update.current_players[i] = state->players[i].name;
                update.scores[i] = state->players[i].score;
            }
        }

        if (req.type == LAST_TICK) {
            update.server_shutdown = true;
        }

        serialize_message(&update, buffer, &buffer_size);

        // Send the topic and message as multipart
        zmq_send(pub_sub_socket, "UPDATE", 6, ZMQ_SNDMORE); // Send topic
        zmq_send(pub_sub_socket, buffer, buffer_size, 0);

        if (req.type == LAST_TICK) {
            break;
        }
    }

    pthread_exit(NULL);

    // Cleanup
    zmq_close(socket);
    zmq_close(pub_sub_socket);
    zmq_ctx_destroy(context);

}

void *alien_handler(void *arg) {
    thread_args *args = (thread_args *)arg;
    GameState *state = args->state;
    Node * current = NULL;

    while(*(args->server_shutdown) == false) {
        long long current_time = ms_since_epoch();
        // Move the aliens
        pthread_mutex_lock(&game_state_mutex);
        current = state->aliens->head;

        while(current != NULL) {
            Alien *alien = (Alien *)current->data;
            if (current_time > alien->next_move) {
                int delta_x = get_random_number(-1, 1);
                int delta_y = get_random_number(-1, 1);

                int randomness_coefficient = get_random_number(-100, 100); // This value if used to ensure the aliens won't all move at the same time, and instead of moving at fixed rate of 1 square per second, they time interval they will have between moves will be between 0.9s and 1.1s making the game seem more dynamic

                alien->x = min(max(alien->x + delta_x, 2), GRID_SIZE - 2 - 1);
                alien->y = min(max(alien->y + delta_y, 2), GRID_SIZE - 2 - 1);
                alien->next_move = current_time + 1 * 1000 +  randomness_coefficient;
            }

            state->grid[alien->y][alien->x] = '*';
            current = current->next;
        }
        pthread_mutex_unlock(&game_state_mutex);
        usleep(10000); // Sleep for 0.5 seconds
    }

    pthread_exit(NULL);
}

void *tick_handler(void *arg) {
    void *context = zmq_ctx_new();
    void *socket =  zmq_socket(context, ZMQ_REQ);

    thread_args *args = (thread_args *)arg;

    if (zmq_connect(socket, ADDRESS) != 0) {
        perror("Child zmq_connect failed");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    client_to_server_message msg;
    msg.character = ' ';
    msg.move = UP;
    msg.type = TICK;

    server_to_client_message response;

    while (true) {
        server_to_client_message response;
        zmq_send(socket, &msg, sizeof(client_to_server_message), 0); // Send message to parent
        zmq_recv(socket, &response, sizeof(server_to_client_message), 0); // Receive message from parent

        if (!response.status) {
            fprintf(stderr, "Tick Messages are not being processed correctly in the game server, something is wrong\n");
        }
        usleep(100000); // Sleep for 0.1 seconds

        if (*(args->server_shutdown)) {
            if (msg.type == LAST_TICK) {
                break;
            }
            msg.type = LAST_TICK;
            
        }
    }

    pthread_exit(NULL);
}

void* keyboard_handler(void* arg) {
    thread_args* args = (thread_args*)arg;
    int ch;
    position pos;
    char buffer[1024];
    size_t buffer_size;
   
    void *context = args->zmq_context;
    void *socket = zmq_socket(context, ZMQ_PUB);

    if (zmq_bind(socket, SERVERSHUTDOWN_PUBSUBADDRESS) != 0) {
        perror("Parent publisher zmq_bind failed");
        zmq_close(socket);
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    while (1) {
        ch = getch();
        
        if (ch == 'q' || ch == 'Q') {
            *(args->server_shutdown) = true;
            break;
        }
        else if (ch == 'r' || ch == 'R') {
            pthread_mutex_lock(&game_state_mutex);
            
            // Save player names and their usage status
            Player saved_players[MAX_PLAYERS];
            bool saved_names_used[MAX_PLAYERS];
            unsigned int saved_player_count = args->state->player_count;
            
            for (int i = 0; i < MAX_PLAYERS; i++) {
                saved_players[i] = args->state->players[i];
                saved_names_used[i] = args->state->players_name_used[i];
            }

            // Reset game state
            GameState* new_state = init_game();

            // Restore player names and their initial positions
            new_state->player_count = saved_player_count;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                new_state->players[i].name = saved_players[i].name;
                pos = get_start_position(new_state->players[i].name);
                new_state->players[i].x = pos.x;
                new_state->players[i].y = pos.y;
                new_state->players_name_used[i] = saved_names_used[i];
            }

            memcpy(args->state, new_state, sizeof(GameState));
            free(new_state);

            pthread_mutex_unlock(&game_state_mutex);
        }

    }

    pthread_exit(NULL);
}

int main() {
    GameState *state = init_game();     

    void * context = zmq_ctx_new();


    thread_args game_handler_args, alien_handler_args, tick_handler_args, keyboard_handler_args;

    game_handler_args.state = state;
    game_handler_args.zmq_context = context;
    game_handler_args.server_shutdown = &server_shutdown;

    alien_handler_args.state = state;
    alien_handler_args.zmq_context = context;
    alien_handler_args.server_shutdown = &server_shutdown;

    tick_handler_args.state = state;
    tick_handler_args.zmq_context = context;
    tick_handler_args.server_shutdown = &server_shutdown;

    keyboard_handler_args.state = state;
    keyboard_handler_args.zmq_context = context;
    keyboard_handler_args.server_shutdown = &server_shutdown;

    pthread_t keyboard_thread, game_handler_thread, alien_movement_thread, tick_thread;

    if (pthread_mutex_init(&game_state_mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        return EXIT_FAILURE;
    }

    init_ncurses();

    if (pthread_create(&keyboard_thread, NULL, keyboard_handler, &keyboard_handler_args) != 0) {
        perror("Failed to create keyboard handler thread");
        return 1;
    }

    if (pthread_create(&game_handler_thread, NULL, (void *)game_handler, &game_handler_args) != 0) {
        perror("Failed to create game handler thread");
        return 1;
    }

    if (pthread_create(&alien_movement_thread, NULL, (void *)alien_handler, &alien_handler_args) != 0) {
        perror("Failed to create alien movement thread");
        return 1;
    }

    if (pthread_create(&tick_thread, NULL, (void *)tick_handler, &tick_handler_args) != 0) {
        perror("Failed to create tick thread");
        return 1;
    }

    pthread_join(keyboard_thread, NULL);
    pthread_join(game_handler_thread, NULL);
    pthread_join(alien_movement_thread, NULL);
    pthread_join(tick_thread, NULL);


    endwin();  // Clean up ncurses
    pthread_mutex_destroy(&game_state_mutex);
    free(state);

    return 0;
}
