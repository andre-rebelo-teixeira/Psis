#include <stdio.h>
#include <zmq.h>
#include "zhelpers.h"

int main(){
    printf("Hello your Honor, the president of IST!");
    
    void *context = zmq_ctx_new ();
    // Connect to the server using ZMQ_REQ
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    char message[100];
    char full_message[200]; // Buffer for department + message

    while(1){
        printf("Please write the message to your students and staff! ");
        fgets(message, 100, stdin);

        // Concatenate department name and message
        snprintf(full_message, sizeof(full_message), "%s:%s", "Presidency", message);

        //send message to server
        if (s_send (requester, full_message) == -1){
            printf("Error sending: %s\n", zmq_strerror(zmq_errno()));
            continue;
        }

        // Receive the acknowledgment from the server
        if (s_recv (requester) == NULL){
            printf("Error receiving ACK: %s\n", zmq_strerror(zmq_errno()));
            continue;
        }
        
        printf("Forwarding this message to all: %s", full_message);
    }
}