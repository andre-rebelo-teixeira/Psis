#include <ncurses.h>
#include <time.h>
#include <zmq.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <sys/types.h>

#include<unistd.h>

#include "list.h"
#include "messages.h"
#include "utils.h"
#include "game-server.h"



/**
 * @brief Draws the grid of the game, the scores, the players, the aliens and the shots
 *
 * @param state the current state of the game
 */
void draw_game_state(GameState *state) {
    draw_border_with_numbers();


    // Draw grid
    if (!state->game_over) {
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
 * @brief Places players on the grid based on their positions
 *
 * @param state the current state of the game
 */
void draw_players(GameState *state){
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].name != ' ') {
            state->grid[state->players[i].y][state->players[i].x] = state->players[i].name;
        }
    }

    return;
}

/**
 * @brief Places the shots on the grid based on the firing direction
 *
 * @param state the current state of the game
 */
void draw_shots(GameState *state) {
    Node * node = state->shots->head;

    while(node !=  NULL){
        Shot * shot = (Shot *) node->data;
        state->grid[shot->pos.y][shot->pos.x] = shot->shot_symbol;
        node = node->next;
    }

    return;
}

/**
 * @brief Moves aliens randomly in the grid
 *
 * @param state the current state of the game
 */
void move_aliens_at_random(GameState *state) {
    if (state == NULL) {
        fprintf(stderr, "Game state is NULL");
        return;
    }

    if (state->aliens == NULL) {
        fprintf(stderr, "Aliens list is NULL");
        return;
    }

    Node *current = state->aliens->head;

    while(current != NULL) {
        Alien *alien = (Alien *)current->data;
        if (time(NULL) > alien->next_move) {
            int delta_x = get_random_number(-1, 1);
            int delta_y = get_random_number(-1, 1);

            alien->x = min(max(alien->x + delta_x, 2), GRID_SIZE - 2 - 1);
            alien->y = min(max(alien->y + delta_y, 2), GRID_SIZE - 2 - 1);
            alien->next_move = time(NULL) + 1;
        }

        state->grid[alien->y][alien->x] = '*';
        current = current->next;
    }

    return;
}

/**
 * @brief  Cleans up expired shots from the game state
 *
 * @param state the current state of the game
 */
void cleanup_shot(GameState * state){
    Node *node = state->shots->head;
    void (*free_ptr)(void*) = free;
    
    while(node != NULL) {
        Shot* shot = (Shot *)node->data;

        if (time(NULL) > shot->end_time) {
            Node * next_node = node->next;
            remove_node(state->shots, node, free_ptr);
            node = next_node;
        } else {
            node = node->next;
        }
    }
    return;
}

/**
 * @brief This function will handle the astronaut connect message
 * 
 * @param state the current state of the game
 * @param msg the message received from the client
 * @return char the name of the astronaut that was assigned to the player 
 */
char handle_astronaut_connect(GameState* state, message msg) {
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
            state->players[i].next_shot_time = time(NULL);
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
 */
void  handle_astronaut_disconnect(GameState *state, message msg) {
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
            state->players[i].score = -1;
            break;
        }
    }

    return;
}

/**
 * @brief This function will update the position of the shot based on the direction it was fired
 * 
 * @param shot the current position of the shot
 * @param shot_direction the direction in which the shot was fired
 * @return position the updated position of the shot
 */
position update_shot_pos(position shot, firing_direction shot_direction) {
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

/**
 * @brief This function will handle the astronaut move message
 * 
 * @param state the current state of the game
 * @param msg the message received from the client
 */
void handle_astronaut_move(GameState *state, message msg) {
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


    if (p.stunned && time(NULL) < p.end_stun_time) {
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
bool handle_astronaut_zap(GameState *state, message msg) {
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

    if (p.stunned && time(NULL) < p.end_stun_time) {
        return false;
    } else {
        p.stunned = false;
    }

    if (time(NULL) < p.next_shot_time) {
        return false;
    }
    p.next_shot_time = time(NULL) + 3;


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
        s->end_time = time(NULL) + 0.5;

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
            state->players[i].end_stun_time = time(NULL) + 10; // Stun for 5 seconds
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
 * @return message the response message to be sent back to the client
 */
message handle_new_message(GameState* state,  message msg) {
    message response; 
    response.character = msg.character;
    char name = ' ';

    switch (msg.type) {
        case ASTRONAUT_CONNECT:
            name = handle_astronaut_connect(state, msg);
            response.type = RESPONSE;
            response.character = name;  
            break;
        case ASTRONAUT_MOVEMENT:
            if(!state->game_over) 
                handle_astronaut_move(state, msg);
            response.type = RESPONSE;
            response.character = msg.character;
            break;
        case ASTRONAUT_ZAP:
            if (!state->game_over) 
                state->game_over = handle_astronaut_zap(state, msg);
            response.type = RESPONSE;
            response.character = msg.character;
            break;
        case ASTRONAUT_DISCONNECT:
            handle_astronaut_disconnect(state, msg);
            break;
        case OUTER_SPACE_UPDATE:
            break;
        default:
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
        state->players[i].next_shot_time = time(NULL) - 1;
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
        alien->next_move = time(NULL);
        alien->x = get_random_number(2, GRID_SIZE - 2);
        alien->y = get_random_number(2, GRID_SIZE - 2);
        insert_node(state->aliens, alien);
    }

    return state;
}

/**
 * @brief This function will clear the board associated to the game state
 * 
 * @param state the current state of the game
 */
void clear_board(GameState* state){
    for (unsigned int i = 0; i < GRID_SIZE; i++) {
        for (unsigned int j = 0; j < GRID_SIZE; j++) {
            state->grid[i][j] = ' ';
        }
    }
}

/**
 * @brief Serialize the update the message for it to be sent in a buffer to outer-space-display via the publisher
 * 
 * @param msg the message to be serialized
 * @param buffer the buffer where the message will be serialized
 * @param buffer_size the size of the buffer
 */
void serialize_message(const message *msg, char *buffer, size_t *buffer_size) {
    size_t offset = 0;

    memcpy(buffer + offset, &msg->type, sizeof(msg->type));
    offset += sizeof(msg->type);

    memcpy(buffer + offset, &msg->move, sizeof(msg->move));
    offset += sizeof(msg->move);

    memcpy(buffer + offset, &msg->character, sizeof(msg->character));
    offset += sizeof(msg->character);

    memcpy(buffer + offset, &msg->current_players, sizeof(msg->current_players));
    offset += sizeof(msg->scores);

    memcpy(buffer + offset, &msg->scores, sizeof(msg->scores));
    offset += sizeof(msg->scores);

    memcpy(buffer + offset, &msg->grid, sizeof(msg->grid));
    offset += sizeof(msg->grid);

    memcpy(buffer + offset, &msg->game_over, sizeof(msg->game_over));
    offset += sizeof(msg->game_over);

    *buffer_size = offset;
}

/**
 * @brief This function acts as the parent process of the game server. 
 * It is used to handle the game logic and the communication with both the child process, the client and the outer-space-display
 */
void parent_process(){
    // Initialize NCurses
    init_ncurses();

    // Initialize Game state
    GameState * state = init_game() ;

    //// Cleanup
    //endwin();
    //return 0;
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP); // REP socket for responding
    if (zmq_bind(responder, ADDRESS) != 0) {
        perror("Parent responder zmq_bind failed");
        zmq_close(responder);
        zmq_ctx_destroy(context);
        exit(1);
    }

    void *publisher = zmq_socket(context, ZMQ_PUB); // PUB socket for publishing
    if (zmq_bind(publisher, PUBSUB_ADDRESS) != 0) {
        perror("Parent publisher zmq_bind failed");
        zmq_close(publisher);
        zmq_close(responder);
        zmq_ctx_destroy(context);
        exit(1);
    }

    message msg;
    char buffer[1024];
    size_t buffer_size;

    while(1) {
        zmq_recv(responder, &msg, sizeof(message), 0);
        clear_board(state);

        message response = handle_new_message(state, msg);

        for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
            response.scores[i] = state->players[i].score;
            response.current_players[i] = state->players[i].name;
        }

        // Answer to the client
        zmq_send(responder, &response, sizeof(response), 0);

        clear();
        cleanup_shot(state);
        draw_players(state);
        draw_shots(state);
        move_aliens_at_random(state);
        draw_game_state(state);
        refresh();
        

        // Publish the updated game state
        // Create the message with the topic prepended
        msg.type = OUTER_SPACE_UPDATE;
        msg.character = ' ';
        msg.move = UP;
        for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
            msg.scores[i] = state->players[i].score;
            msg.current_players[i] = state->players[i].name;
        }

        memcpy(msg.grid, state->grid, sizeof(state->grid));
        msg.game_over = state->game_over;
        serialize_message(&msg, buffer, &buffer_size);

        // Send the topic and message as multipart
        zmq_send(publisher, "UPDATE", 6, ZMQ_SNDMORE); // Send topic
        zmq_send(publisher, buffer, buffer_size, 0);   // Send serialized message
    }

    // Cleanup
    zmq_close(publisher);
    zmq_ctx_destroy(context);

    exit(0);
}

/**
 * @brief This function acts as the child process of the game server. 
 * It is used to communicate with the parent process and prevent it from blocking the game logic
 */
void child_process() {
    void *context = zmq_ctx_new();
    void *socket =  zmq_socket(context, ZMQ_REQ);

    if (zmq_connect(socket, ADDRESS) != 0) {
        perror("Child zmq_connect failed");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    message msg;
    msg.type = TICK;
    msg.move = UP;
    msg.character = ' ';

    while (1) {
        message response;
        zmq_send(socket, &msg, sizeof(msg), 0); // Send message to parent
        zmq_recv(socket, &response, sizeof(response), 0); // Receive message from parent
        usleep(100000); // Sleep for 0.1 seconds
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    exit(0);
}

int main() {

    pid_t pid = fork(); // Create a child process

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        usleep(100000); // Small delay to ensure the parent sets up first
        child_process();
    } else {
        // Parent process
        parent_process();
    }

    return 0;
}