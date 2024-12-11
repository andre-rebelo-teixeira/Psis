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

    memcpy(&msg->scores, buffer + offset, sizeof(msg->scores));
    offset += sizeof(msg->scores);

    memcpy(&msg->grid, buffer + offset, sizeof(msg->grid));
    offset += sizeof(msg->grid);
}

void draw_avatar_game(int scores[8], char grid[20][20]) {
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
        if (state->players[i].name != ' ') {
            mvprintw(2 + num_players_on++, GRID_SIZE + 10,  "%c-%d\n", state->players[i].name, state->players[i].score);
        }
    }


    // Draw scores
    attron(COLOR_PAIR(4));
}

int main(){
    // ZeroMQ TCP connection
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, PUBSUB_ADDRESS);
    printf("Connected to the server\n");

    // Subscribe to topic
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "UPDATE", 6);

    init_ncurses();

    char topic[64];
    char buffer[1024];

    while (1) {
        zmq_recv(subscriber, topic, sizeof(topic), 0);
        printf("Received topic: %s\n", topic);

        int bytes_received = zmq_recv(subscriber, buffer, sizeof(buffer), 0);

        if (bytes_received > 0) {
            // Deserialize and process the message
            message msg;
            deserialize_message(buffer, bytes_received, &msg);
            /*
            printf("Message type: %d\n", msg.type);
            printf("Move: %d\n", msg.move);
            printf("Character: %c\n", msg.character);
            printf("Scores: ");
            for (int i = 0; i < 8; i++) {
                printf("%u ", msg.scores[i]);
            }
            printf("\nGrid:\n");
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 20; j++) {
                    printf("%c ", msg.grid[i][j]);
                }
                printf("\n");
            }
            */
        }

    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}