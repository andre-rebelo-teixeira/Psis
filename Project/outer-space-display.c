#include <zmq.h>
#include <ncurses.h>
#include <string.h>

#include "messages.h"
#include "utils.h"

void deserialize_message(const char *buffer, size_t buffer_size, message *msg) {
    size_t offset = 0;

    memcpy(&msg->type, buffer + offset, sizeof(msg->type));
    offset += sizeof(msg->type);

    memcpy(&msg->move, buffer + offset, sizeof(msg->move));
    offset += sizeof(msg->move);

    memcpy(&msg->character, buffer + offset, sizeof(msg->character));
    offset += sizeof(msg->character);

    memcpy(&msg->current_players, buffer + offset, sizeof(msg->current_players));
    offset += sizeof(msg->scores);

    memcpy(&msg->scores, buffer + offset, sizeof(msg->scores));
    offset += sizeof(msg->scores);

    memcpy(&msg->grid, buffer + offset, sizeof(msg->grid));
    offset += sizeof(msg->grid);
}

void draw_avatar_game(int scores[8], char grid[20][20], char current_players[8]) {
    draw_border_with_numbers();

    // Draw grid
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
    

    // Draw score
    mvprintw(2, GRID_SIZE + 10,  "score");
    unsigned int num_players_on = 1;
    for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
        if (current_players[i] != ' ') {
            mvprintw(2 + num_players_on++, GRID_SIZE + 10,  "%c-%d\n", current_players[i], scores[i]);
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
    printf("Connected to the server\n");

    // Subscribe to topic
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "UPDATE", 6);

    char topic[64];
    char buffer[1024];

    while (1) {
        zmq_recv(subscriber, topic, sizeof(topic), 0);
        
        int bytes_received = zmq_recv(subscriber, buffer, sizeof(buffer), 0);

        if (bytes_received > 0) {
            // Deserialize and process the message
            message msg;
            deserialize_message(buffer, bytes_received, &msg);
            
            clear();
            draw_avatar_game(msg.scores, msg.grid, msg.current_players);
            refresh();
        }

    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}