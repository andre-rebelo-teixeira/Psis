#include <zmq.h>
#include <ncurses.h>
#include <string.h>

#include "outer-space-display.h"

/**
 * @brief Deserialize the update the message for it to be obtained from the buffer
 * 
 * @param buffer the buffer where the message will be deserialized
 * @param buffer_size the size of the buffer
 * @param msg the message to be deserialized
 */
void deserialize_message(const char *buffer, display_update_message *msg, size_t buffer_size) {
    size_t offset = 0;

    if (buffer  == NULL || msg == NULL) {
        return;
    }

    // Copy the server shutdown flag
    if (sizeof(msg->server_shutdown) + offset < buffer_size) {
        memcpy(&msg->server_shutdown, buffer, sizeof(msg->server_shutdown));
        offset += sizeof(msg->server_shutdown);
    }

    // Copy the game over flag
    if (sizeof(msg->game_over) + offset < buffer_size) {
        memcpy(&msg->game_over, buffer + offset, sizeof(msg->game_over));
        offset += sizeof(msg->game_over);
    }

    // Copy the grid
    if (sizeof(msg->grid) + offset < buffer_size) {
        memcpy(&msg->grid, buffer + offset, sizeof(msg->grid));
        offset += sizeof(msg->grid);
    }

    if (sizeof(msg->scores) + offset < buffer_size) {
        memcpy(&msg->scores, buffer + offset, sizeof(msg->scores));
        offset += sizeof(msg->scores);
    }

    // Copy the current players
    if (sizeof(msg->current_players) + offset < buffer_size){
        memcpy(&msg->current_players, buffer + offset, sizeof(msg->current_players));
        offset += sizeof(msg->current_players);
    }
}

/**
 * @brief Draw the grid of the game, the scores, the players, the aliens and the shots in the outer space display
 * 
 * @param scores the scores of the players
 * @param grid the grid of the game
 * @param current_players the current players in the game
 */
void draw_avatar_game(unsigned int scores[8], char grid[20][20], char current_players[8], bool game_over) {
    draw_border_with_numbers();

    // Draw grid
    if (!game_over){
        for (int y = 0; y < GRID_SIZE; y++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                char ch = grid[y][x];
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
    }

    unsigned int highest_score_index = 0;
    
    for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (scores[i] > scores[highest_score_index])
            highest_score_index = i;    
    } 
    

    // Draw score
    mvprintw(2, GRID_SIZE + 10,  "score");
    unsigned int num_players_on = 1;
    for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (current_players[i] != ' ') {
            if  (game_over && i == highest_score_index) {
                attron(COLOR_PAIR(1));
            }

            mvprintw(2 + num_players_on++, GRID_SIZE + 10,  "%c-%d\n", current_players[i], scores[i]);
            attroff(COLOR_PAIR(1));
        }
    }


    // Draw scores
    attron(COLOR_PAIR(4));
}

int main(){
    init_ncurses();
    // ZeroMQ TCP connection
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, PUBSUB_ADDRESS);
    zmq_connect(subscriber, SERVERSHUTDOWN_PUBSUBADDRESS);

    // Subscribe to topic
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "UPDATE", 6);

    char topic[64];
    char buffer[1024];
    display_update_message msg;
    msg.game_over = false;

    while (!msg.game_over) {
        zmq_recv(subscriber, topic, sizeof(topic), 0);
        
        int bytes_received = zmq_recv(subscriber, buffer, sizeof(display_update_message), 0);

        if (bytes_received > 0) {
            // Deserialize and process the message
            deserialize_message(buffer, &msg, bytes_received);
            
            clear();
            draw_avatar_game(msg.scores, msg.grid, msg.current_players, msg.game_over);
            refresh();

            if (msg.server_shutdown) {
                printf("Server Shutdown \n");
                break;
            }
        }
    }

    endwin();
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}