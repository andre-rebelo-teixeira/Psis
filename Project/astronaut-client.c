#include <zmq.h>
#include <ncurses.h>

#include "messages.h"
#include "utils.h"

int main(){
    int n = 0;
    int key;
    message msg; // Message to be sent to the server
    message reply; // Reply from the server
    char character; // Character of the player

    // ZeroMQ TCP connection
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, ADDRESS);

    // Send connection message to the server
    msg.type = ASTRONAUT_CONNECT;
    zmq_send(socket, &msg, sizeof(msg), 0);
    zmq_recv(socket, &reply, sizeof(reply), 0);
    character = reply.character;
    int initial_game_over = reply.game_over;

    // Check if assigned chatacter is valid | Check if server is full
    if(character != ' ') {

        // Ncurses setup
        init_ncurses();
        keypad(stdscr, TRUE);


        while(1){

            refresh();

            key = getch();		
            switch (key){
                // Movement keys are pressed
                case KEY_LEFT:
                    msg.type = ASTRONAUT_MOVEMENT;
                    msg.move = LEFT;
                    msg.character = character;
                    break;

                case KEY_RIGHT:
                    msg.type = ASTRONAUT_MOVEMENT;
                    msg.move = RIGHT;
                    msg.character = character;
                    break;

                case KEY_UP:
                    msg.type = ASTRONAUT_MOVEMENT;
                    msg.move = UP;
                    msg.character = character;
                    break;

                case KEY_DOWN:
                    msg.type = ASTRONAUT_MOVEMENT;
                    msg.move = DOWN;
                    msg.character = character;
                    break;

                // Shooting key is pressed
                case ' ':
                    msg.type = ASTRONAUT_ZAP;
                    msg.character = character;
                    break;

                // Exit key is pressed
                case 'q':
                case 'Q':
                    msg.type = ASTRONAUT_DISCONNECT;
                    msg.character = character;
                    break;

                default:
                    msg.type = TICK;
                    break;
            }

            // Send the message to the server
            zmq_send(socket, &msg, sizeof(msg), 0);
            zmq_recv(socket, &reply, sizeof(reply), 0);

            for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
                if (reply.current_players[i] == character) {
                    mvprintw(1, 0, "Score: %d\n", reply.scores[i]);
                }
            }

            if  (msg.type == ASTRONAUT_DISCONNECT) {
                break;
            }

            if (reply.game_over) {
                mvprintw(0, 0, "Game has ended\n");
            }

        }
    
        endwin();

    }else{
        printf("SERVER IS FULL!\n");
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}