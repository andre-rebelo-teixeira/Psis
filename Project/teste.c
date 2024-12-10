#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#define ADDRESS "tcp://127.0.0.1:5555"

void child_process() {
    // Initialize ZeroMQ context and socket for the child
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_PUSH); // PUSH socket for sending
    if (zmq_connect(socket, ADDRESS) != 0) {
        perror("Child zmq_connect failed");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    while (1) {
        const char *message = "Hello from child!";
        printf("Child sent message\n");
        zmq_send(socket, message, strlen(message), 0); // Send message to parent
        usleep(100000); // Sleep for 0.1 seconds
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    exit(0);
}

void parent_process() {
    // Initialize ZeroMQ context and socket for the parent
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_PULL); // PULL socket for receiving
    if (zmq_bind(socket, ADDRESS) != 0) {
        perror("Parent zmq_bind failed");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    char buffer[256];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        zmq_recv(socket, buffer, sizeof(buffer) - 1, 0); // Receive message from child
        printf("Parent received: %s\n", buffer);
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
}

int main() {
    pid_t pid = fork(); // Create a child process

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        usleep(100000); // Small delay to ensure the parent sets up first
        child_process();
    } else {
        // Parent process
        parent_process();
    }

    return 0;
}
