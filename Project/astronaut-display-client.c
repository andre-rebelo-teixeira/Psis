#include "astronaut-display-client.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    DISPLAY_HANDLER = 0,
    INPUT_HANDLER = 1
} thread_type;

typedef struct
{
    void *socket;
    void *context;
    thread_type type;
    char player_char;
    bool* server_shutdown;
    pthread_t *thread;
} thread_arguments;

// Global Variable to the C file, this will be used to close the display when the client quits the input thread
volatile bool game_closed = false;
volatile int current_score = 0;
pthread_mutex_t mutex;

void deserialize_message(const char *buffer, size_t buffer_size, display_update_message *msg)
{
    size_t offset = 0;

    // Copy the server shutdown flag
    memcpy(&msg->server_shutdown, buffer, sizeof(msg->server_shutdown));
    offset += sizeof(msg->server_shutdown);

    // Copy the game over flag
    memcpy(&msg->game_over, buffer + offset, sizeof(msg->game_over));
    offset += sizeof(msg->game_over);

    // Copy the grid
    memcpy(&msg->grid, buffer + offset, sizeof(msg->grid));
    offset += sizeof(msg->grid);

    // Copy the scores
    memcpy(&msg->scores, buffer + offset, sizeof(msg->scores));
    offset += sizeof(msg->scores);

    // Copy the current players
    memcpy(&msg->current_players, buffer + offset, sizeof(msg->current_players));
    offset += sizeof(msg->current_players);
}

/**
 * @brief Draw the grid of the game, the scores, the players, the aliens and the shots in the outer space display
 *
 * @param scores the scores of the players
 * @param grid the grid of the game
 * @param current_players the current players in the game
 */
void draw_avatar_game(int scores[8], char grid[20][20], char current_players[8], bool game_over, char player_id)
{
    draw_border_with_numbers();

    // Draw grid
    if (!game_over)
    {
        for (int y = 0; y < GRID_SIZE; y++)
        {
            for (int x = 0; x < GRID_SIZE; x++)
            {
                char ch = grid[y][x];
                if (ch >= 'A' && ch <= 'H')
                {
                    attron(COLOR_PAIR(1)); // Astronauts
                }
                else if (ch == '*')
                {
                    attron(COLOR_PAIR(2)); // Aliens
                }
                else if (ch == '-' || ch == '|')
                {
                    attron(COLOR_PAIR(3)); // Laser beams
                }
                if (ch != ' ')
                {
                    mvprintw(y + 2, x + 2, "%c", ch); // Adjusted for border
                }
                attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3));
            }
        }
    }
    else
    {
        mvprintw(10, 10, "G");
        mvprintw(10, 11, "A");
        mvprintw(10, 12, "M");
        mvprintw(10, 13, "E");
        ;

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

    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
        if (scores[i] > scores[highest_score_index])
            highest_score_index = i;
    }

    mvprintw(1, GRID_SIZE + 10, "Player + %c", player_id);
    // Draw score
    mvprintw(2, GRID_SIZE + 10, "score");
    unsigned int num_players_on = 1;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
        if (current_players[i] != ' ')
        {
            if (game_over && i == highest_score_index)
            {
                attron(COLOR_PAIR(1));
            }

            mvprintw(2 + num_players_on++, GRID_SIZE + 10, "%c-%d\n", current_players[i], scores[i]);
            attroff(COLOR_PAIR(1));
        }
    }

    // Draw scores
    attron(COLOR_PAIR(4));
}

client_to_server_message create_message(int key, char player_char)
{
    client_to_server_message msg;
    msg.character = player_char;

    switch (key)
    {
    // Movement keys are pressed
    case KEY_LEFT:
        msg.type = ASTRONAUT_MOVEMENT;
        msg.move = LEFT;
        break;
    case KEY_RIGHT:
        msg.type = ASTRONAUT_MOVEMENT;
        msg.move = RIGHT;
        break;
    case KEY_UP:
        msg.type = ASTRONAUT_MOVEMENT;
        msg.move = UP;
        break;
    case KEY_DOWN:
        msg.type = ASTRONAUT_MOVEMENT;
        msg.move = DOWN;
        break;
    case ' ':
        msg.type = ASTRONAUT_ZAP;
        break;
    case 'q':
    case 'Q':
        msg.type = ASTRONAUT_DISCONNECT;
        break;
    default:
        break;
    }
    return msg;
}

void *input_client(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    thread_arguments *args = (thread_arguments *)arg;
    void *socket = args->socket;
    void *context = args->context;
    thread_type type = args->type;
    char player_char = args->player_char;

    if (type != INPUT_HANDLER)
    {
        perror("Invalid thread type");
        pthread_exit(NULL);
    }

    int message_counter = 0;

    while (!game_closed)
    {
        int key = getch();
        if (*(args->server_shutdown))
        {
            break;
        }


        client_to_server_message msg = create_message(key, player_char);
        server_to_client_message reply;

        zmq_send(socket, &msg, sizeof(msg), 0);
        zmq_recv(socket, &reply, sizeof(reply), 0);

        pthread_mutex_lock(&mutex); // Lock the mutex
        current_score = reply.score;
        pthread_mutex_unlock(&mutex); // Unlock the mutex

        if (msg.type == ASTRONAUT_DISCONNECT)
        {
            pthread_mutex_lock(&mutex); // Lock the mutex
            game_closed = true;
            pthread_mutex_unlock(&mutex); // Unlock the mutex
        }
    }
    printf("Eding input thread\n");
    return NULL;
}

void *display_client(void *arg)
{
    thread_arguments *args = (thread_arguments *)arg;
    void *socket = args->socket;
    void *context = args->context;
    thread_type type = args->type;

    if (type != DISPLAY_HANDLER)
    {
        perror("Invalid thread type");
        pthread_exit(NULL);
    }

    char topic[64];
    char buffer[1024];
    display_update_message msg;

    // Keep in this loop until the input game thread closes the game, in that moment we need to close the display
    int bytes_received = -1;
    while (!game_closed)
    {
        bytes_received = zmq_recv(socket, topic, sizeof(topic), 0);
        // The message we receive is not valid, so we will not advance in reading the data
        if (bytes_received < 0)
        {
            continue;
        }


        bytes_received = zmq_recv(socket, buffer, sizeof(buffer), 0);
        // The message we receive is not valid, so we will not advance in reading the data
        if (bytes_received < 0)
        {
            continue;
        }

        deserialize_message(buffer, bytes_received, &msg);

        clear();
        draw_avatar_game(msg.scores, msg.grid, msg.current_players, msg.game_over, args->player_char);
        refresh();

        if (msg.server_shutdown)
        {
            if (pthread_cancel(*(args->thread)) != 0) {
                perror("Failed to cancel thread");
                exit(1);
            }

            printf("Server shuting down \n");
            break;
        }
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);

    printf("Ending display thread\n");
    return NULL;
}

/**
 * @brief This is the main function for the client, this will only create the sockets and connect the client to the game server. After that it will open two different threads that will assume the role of handling the rest of the code
 */
int main()
{
    bool server_shutdown = false;
    // Initialize the mutex
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        perror("Mutex initialization failed");
        return EXIT_FAILURE;
    }

    // Open PUB SUB connection - This willl be used to receive updates from the game server to update the display
    void *pub_sub_context = zmq_ctx_new();
    void *pub_sub_socket = zmq_socket(pub_sub_context, ZMQ_SUB);
    zmq_connect(pub_sub_socket, PUBSUB_ADDRESS);
    zmq_setsockopt(pub_sub_socket, ZMQ_SUBSCRIBE, "UPDATE", 6);

    thread_arguments display_args;
    display_args.context = pub_sub_context;
    display_args.socket = pub_sub_socket;
    display_args.type = DISPLAY_HANDLER;
    display_args.server_shutdown = &server_shutdown;

    // Open REQ REP connection - this will be use to share the input from the user to the gamer server
    void *req_context = zmq_ctx_new();
    void *req_socket = zmq_socket(req_context, ZMQ_REQ);
    zmq_connect(req_socket, ADDRESS);

    // Connect to client
    client_to_server_message msg; 
    server_to_client_message reply;

    msg.type = ASTRONAUT_CONNECT;
    zmq_send(req_socket, &msg, sizeof(client_to_server_message), 0);
    zmq_recv(req_socket, &reply, sizeof(server_to_client_message), 0);

    // Check if assigned chatacter is valid | Check if server is full
    if (reply.character != ' ')
    {

        thread_arguments input_args;
        input_args.context = req_context;
        input_args.socket = req_socket;
        input_args.type = INPUT_HANDLER;
        input_args.player_char = reply.character;
        input_args.server_shutdown = &server_shutdown;
        display_args.player_char = reply.character;

        pthread_t display_thread, input_thread;
        display_args.thread = &input_thread;
        input_args.thread = NULL;

        // Ncurses setup
        init_ncurses();

        keypad(stdscr, TRUE);

        if (pthread_create(&display_thread, NULL, display_client, &display_args) != 0)
        {
            perror("Failed to create display thread");
            return 1;
        }

        printf("Input thread being created");

        if (pthread_create(&input_thread, NULL, input_client, &input_args) != 0)
        {
            perror("Failed to create input thread");
            return 1;
        }

        // Wait for threads to complete
        pthread_join(input_thread, NULL);
        pthread_join(display_thread, NULL);

        printf("After zmq cleanup\n");
        pthread_mutex_destroy(&mutex);
        endwin();
    }
    else
    {
        printf("SERVER IS FULL!\n");
    }

    // Clean up
    zmq_close(pub_sub_socket);
    zmq_ctx_destroy(pub_sub_context);
    zmq_close(req_socket);
    zmq_ctx_destroy(req_context);

    return 0;
}