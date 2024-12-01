#include <stdio.h>
#include <zmq.h>
#include "zhelpers.h"

int main(){
    char line[100];
    char dpt_name[100];
    printf("What is your department? (DEEC, DEI, ...)");
    fgets(line, 100, stdin);
    sscanf(line, "%s", dpt_name);
    printf("Hello your Honor, the President of %s\n", dpt_name);

    void *context = zmq_ctx_new ();
    // Connect to the server using ZMQ_REQ
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    char message[100];
    char full_message[200]; // Buffer for department + message
    while(1){

        printf("Please write the message to the students and staff on your buildings! ");
        fgets(message, 100, stdin);

        // Concatenate department name and message
        snprintf(full_message, sizeof(full_message), "%s:%s", dpt_name, message);

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

        printf("Forwarding this message to all: %s", message);
        
    }
}