#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "zhelpers.h"

int main(void) {
    void *context = zmq_ctx_new();

    // Bind a ZMQ_REP socket
    void *responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(responder, "tcp://*:5555");
    assert(rc == 0);

    // Bind a ZMQ_PUB socket
    void *publisher = zmq_socket(context, ZMQ_PUB);
    rc = zmq_bind(publisher, "tcp://*:5556");
    assert(rc == 0);

    while (1) {
        // Receive message from the client
        char *buffer = s_recv(responder);
        if (buffer == NULL) {
            printf("Error receiving: %s\n", zmq_strerror(zmq_errno()));
            continue;
        }

        char *delimiter_pos = strchr(buffer, ':');

        // Split the message into department and actual message
        char *dpt = strtok(buffer, ":");
        char *s = strtok(NULL, ":");

        // Restore the original buffer
        *delimiter_pos = ':';

        if (dpt && s) {
            printf("Department: %s, Message: %s\n", dpt, s);

            // Publish the message to all subscribers
            s_send(publisher, buffer);
        } else {
            printf("Malformed message received: %s\n", buffer);
        }

        // Send an acknowledgment back to the client
        s_send(responder, "OK");

        free(buffer); // Free the buffer allocated by s_recv
    }

    zmq_close(responder);
    zmq_close(publisher);
    zmq_ctx_destroy(context);
    return 0;
}
