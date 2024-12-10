#include <zmq.h>
#include <ncurses.h>

#include "messages.h"
#include "game-server.h"

int main(){
    // ZeroMQ TCP connection
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, ADDRESS);
    printf("Connected to the server\n");

    // Subscribe to topic
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "Presidency", strlen("Presidency"));

    while (1) {
        
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}