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

static inline unsigned int get_random_number(unsigned int min, unsigned int max)
{
    return min + (rand() % (max - min + 1));
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
            if (ch != ' '){
                mvprintw(y + 2, x + 2, "%c", ch); // Adjusted for border
            }
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3));
        }
    }
    

    // Draw score
    mvprintw(2, GRID_SIZE + 10,  "score");
    unsigned int num_players_on = 1;
    for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].name != ' ') {
            mvprintw(2 + num_players_on++, GRID_SIZE + 10,  "%c-%d\n", state->players[i].name, state->players[i].score);
        }
    }


    // Draw scores
    attron(COLOR_PAIR(4));
}

void draw_players(GameState *state){
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].name != ' ') {
            state->grid[state->players[i].y][state->players[i].x] = state->players[i].name;
        }
    }

    return;
}

void draw_shots(GameState *state) {
    Node * node = state->shots->head;

    while(node !=  NULL){
        Shot * shot = (Shot *) node->data;
        state->grid[shot->pos.y][shot->pos.x] = shot->shot_symbol;
        node = node->next;
    }

    return;
}

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
 * @brief Return the default starting position of an astronaut in the grid
 * This position is well defined according to the astronaut name, meaning that each astronaut will always start from the same position. This is not the most correct way to do it, since it removed randomness in the game, but overall was the fastest way to implement it, since this way whenever a astronaut connect, we dont need to find on of the 8 allowed row's/col's from where astronauts can shoot at aliens 
 * 
 * @param astronaut_name  should be a letter with values between A and H
 * @return position the position in the grid array of the state where we should input the astronaut. If the astronaut name is invalid, the x and y values will be -1
 */
position get_start_position(char astronaut_name) {
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
 * @brief Get the firing direction object
 * 
 * @param astronaut_name 
 * @return firing_direction 
 */
firing_direction get_firing_direction(char astronaut_name){
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

/**
 * @brief Get the firing direction object
 * 
 * @param astronaut_name 
 * @return firing_direction 
 */
moving_direction  get_moving_direction(char astronaut_name){
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
 * @brief This function will handle the astronaut connect message
 * 
 * @param state 
 * @param msg 
 * @return GameState* 
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

    //printf("Passed all the checks, now will search a name");

    char name = 'A';

    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players_name_used[i]) {
            break;
        }
        name += 1;
    }

    //printf("Name after search is %c\n", name);

    // Find the first empty slot in the players array to add the new player
    for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
        // If name of the player is an empty space, then the slot is empty
        if (state->players[i].name == ' ') {
            state->players[i].name = name;
            position pos = get_start_position(name);
            state->players[i].x = pos.x;
            state->players[i].y = pos.y;
            state->players_name_used[i] = true;
            state->players[i].next_shot_time = time(NULL) - 1;
            state->player_count++;
            break;
        }
    }
    return name;
}

void  handle_astronaut_disconnect(GameState *state, message msg) {
    if (state == NULL) {
        fprintf(stderr, "Game state is NULL");
        return;
    }

    if (msg.type != ASTRONAUT_DISCONNECT) {
        fprintf(stderr, "Invalid message type");
        return; 
    }

    //printf("Passed hanndle disconnect checks - %c\n", msg.character);

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

    //printf("Original pos %d - %d\n", p.x, p.y);
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
    //printf(" final pos %d - %d %d %d\n", p.x, p.y, direction, msg.move);

    return;
}

void handle_astronaut_zap(GameState *state, message msg) {
    if (state == NULL) {
        fprintf(stderr, "Game state is NULL");
        return;
    }

    if (msg.type != ASTRONAUT_ZAP) {
        fprintf(stderr, "Invalid message type");
        return;
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
        return;
    } else {
        p.stunned = false;
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
}

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
            handle_astronaut_move(state, msg);
            response.type = RESPONSE;
            response.character = msg.character;
            break;
        case ASTRONAUT_ZAP:
            handle_astronaut_zap(state, msg);
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

GameState* init_game(){ 
    GameState* state = (GameState*)calloc(1, sizeof(GameState));

    if (state == NULL) {
        perror("Failed allocation of memory in init game");
        return NULL;
    }

    state->player_count = 0;
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

void clear_board(GameState* state){
    for (unsigned int i = 0; i < GRID_SIZE; i++) {
        for (unsigned int j = 0; j < GRID_SIZE; j++) {
            state->grid[i][j] = ' ';
        }
    }
}

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

    *buffer_size = offset;
}

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
        // printf("message received - %d\n", msg.type);

        message response = handle_new_message(state, msg);

        for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
            response.scores[i] = state->players[i].score;
        }

        //purintf("%c - %d\n", response.character, response.type);
        // Answer to the client
        zmq_send(responder, &response, sizeof(response), 0);

        //printf("Message if of type %d \n", msg.type);
        
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
        usleep(1000000); // Sleep for 0.1 seconds



    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    exit(0);
}

int main() {
    // Initi
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