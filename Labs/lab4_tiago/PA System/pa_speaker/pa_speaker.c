#include <stdio.h>
#include <string.h>
#include <zmq.h>
#include "zhelpers.h"

int main() {
    char line[100];
    char dpt_name[100];
    printf("What is the department of this building? (DEEC, DEI, ...): ");
    fgets(line, sizeof(line), stdin);
    sscanf(line, "%s", dpt_name);
    printf("We will broadcast all messages from the president of IST and %s\n", dpt_name);

    void *context = zmq_ctx_new();

    // Connect to the server using ZMQ_SUB
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5556");

    // Subscribe to topics
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "Presidency", strlen("Presidency"));
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, dpt_name, strlen(dpt_name));

    while (1) {
        // Receive messages
        char *buffer = s_recv(subscriber);
        if (buffer == NULL) {
            printf("Error receiving: %s\n", zmq_strerror(zmq_errno()));
            continue;
        }

        // Split the message into department and actual message
        char *topic = strtok(buffer, ":");
        char *content = strtok(NULL, ":");

        if (topic && content) {
            printf("Message from %s - %s\n", topic, content);
        } else {
            printf("Malformed message: %s\n", buffer);
        }

        free(buffer); // free the buffer allocated by s_recv
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}
