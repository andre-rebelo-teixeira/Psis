#include <zmq.h>
#include <ncurses.h>
#include "messages.h"

int main(){
    int n = 0;
    int key;
    message msg; // Message to be sent to the server

    // ZeroMQ TCP connection
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, "tcp://localhost:5555");

    // Send connection message to the server
    msg.type = ASTRONAUT_CONNECT;

    // Ncurses setup
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);

    while(1){
    	key = getch();		
        n++;
        switch (key){
            // Movement keys are pressed
            case KEY_LEFT:
                mvprintw(0,0,"%d Left arrow is pressed", n);
                msg.move = LEFT;
                break;

            case KEY_RIGHT:
                mvprintw(0,0,"%d Right arrow is pressed", n);
                msg.move = RIGHT;
                break;

            case KEY_UP:
                mvprintw(0,0,"%d Up arrow is pressed", n);
                msg.move = UP;
                break;

            case KEY_DOWN:
                mvprintw(0,0,"%d Down arrow is pressed", n);
                msg.move = DOWN;
                break;

            // Shooting key is pressed
            case ' ':
                mvprintw(0,0,"%d Space is pressed", n);
                msg.type = ASTRONAUT_ZAP;
                break;

            // Exit key is pressed
            case 'q':
            case 'Q':
                mvprintw(0,0,"%d q/Q is pressed", n);
                msg.type = ASTRONAUT_DISCONNECT;
                break;

            default:
                key = 'x'; 
                break;
        }
    }

    endwin(); // End ncurses
    // ZeroMQ cleanup
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}