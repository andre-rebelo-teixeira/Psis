#include <zmq.h>
#include <ncurses.h>

#include "messages.h"
#include "game-server.h"

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
    printf("Connected to the server\n");


    // Send connection message to the server
    msg.type = ASTRONAUT_CONNECT;
    zmq_send(socket, &msg, sizeof(msg), 0);
    zmq_recv(socket, &reply, sizeof(reply), 0);
    printf("Received reply: %c\n", reply.character);
    character = reply.character;
    

    // Ncurses setup
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    int message_counter = 0;

    while(1){
    	key = getch();		
        n++;
        switch (key){
            // Movement keys are pressed
            case KEY_LEFT:
                msg.type = ASTRONAUT_MOVEMENT;
                msg.move = LEFT;
                msg.character = character;
                // Send the message to the server
                zmq_send(socket, &msg, sizeof(msg), 0);
                zmq_recv(socket, &reply, sizeof(reply), 0);
                mvprintw(0, 0, "Received reply: %c %d\n", reply.character, message_counter++);
                break;

            case KEY_RIGHT:
                msg.type = ASTRONAUT_MOVEMENT;
                msg.move = RIGHT;
                msg.character = character;
                // Send the message to the server
                zmq_send(socket, &msg, sizeof(msg), 0);
                zmq_recv(socket, &reply, sizeof(reply), 0);
                mvprintw(0, 0, "Received reply: %c %d\n", reply.character, message_counter++);
                break;

            case KEY_UP:
                msg.type = ASTRONAUT_MOVEMENT;
                msg.move = UP;
                msg.character = character;
                // Send the message to the server
                zmq_send(socket, &msg, sizeof(msg), 0);
                zmq_recv(socket, &reply, sizeof(reply), 0);
                mvprintw(0, 0, "Received reply: %c %d\n", reply.character, message_counter++);
                break;

            case KEY_DOWN:
                msg.type = ASTRONAUT_MOVEMENT;
                msg.move = DOWN;
                msg.character = character;
                // Send the message to the server
                zmq_send(socket, &msg, sizeof(msg), 0);
                zmq_recv(socket, &reply, sizeof(reply), 0);
                mvprintw(0, 0, "Received reply: %c %d\n", reply.character, message_counter++);
                break;

            // Shooting key is pressed
            case ' ':
                msg.type = ASTRONAUT_ZAP;
                msg.character = character;
                // Send the message to the server
                zmq_send(socket, &msg, sizeof(msg), 0);
                zmq_recv(socket, &reply, sizeof(reply), 0);
                mvprintw(0, 0, "Received reply: %c %d\n", reply.character, message_counter++);
                break;

            // Exit key is pressed
            case 'q':
            case 'Q':
                msg.type = ASTRONAUT_DISCONNECT;
                msg.character = character;
                // Send the message to the server
                zmq_send(socket, &msg, sizeof(msg), 0);
                zmq_recv(socket, &reply, sizeof(reply), 0);
                mvprintw(0, 0, "Received reply: %c\n", reply.character);
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