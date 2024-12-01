#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>

#include "data_struct.h"

#define SERVER_CLIENT_FIFO "/tmp/server_client_fifo"
#define CLIENT_SERVER_FIFO "/tmp/client_server_fifo"


int main() {
    char func_name[256];
    int client_server_fifo = -1, server_client_fifo = -1;
    char read_buffer[256], result_buffer[256];
    char * error;
    int (*no_arg_func)() = NULL;
    int (*arg_func)(int) = NULL;
    int result = -1;
    message_type message;

    memset(read_buffer, 0, sizeof(read_buffer));
    memset(result_buffer, 0, sizeof(result_buffer));

    // Create FIFOs if they do not exist
    if (mkfifo(CLIENT_SERVER_FIFO, 0666) == -1 && errno != EEXIST) {
        perror("Error creating CLIENT_SERVER_FIFO");
        exit(EXIT_FAILURE);
    }

    if (mkfifo(SERVER_CLIENT_FIFO, 0666) == -1 && errno != EEXIST) {
        perror("Error creating SERVER_CLIENT_FIFO");
        exit(EXIT_FAILURE);
    }

    // Open FIFOs
    client_server_fifo = open(CLIENT_SERVER_FIFO, O_RDONLY);
    if (client_server_fifo == -1) {
        perror("Error opening CLIENT_SERVER_FIFO");
        exit(EXIT_FAILURE);
    }

    server_client_fifo = open(SERVER_CLIENT_FIFO, O_WRONLY);
    if (server_client_fifo == -1) {
        perror("Error opening SERVER_CLIENT_FIFO");
        close(client_server_fifo);
        exit(EXIT_FAILURE);
    }

    printf("FIFOs successfully opened. Ready to communicate.\n");

    // Load the shared library
    void *handle = dlopen("./libfuncs.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        close(client_server_fifo);
        close(server_client_fifo);
        exit(EXIT_FAILURE);
    }

    printf("Shared library loaded successfully.\n");

    // Communication loop
    while (1) {
        memset(func_name, 0, sizeof(func_name));
        if (read(client_server_fifo, &message,sizeof(message_type)) <= 0) {
            perror("Error reading from CLIENT_SERVER_FIFO");
            break;
        }

        printf("Name %s, type %d, arg %d\n", message.f_name, message.funct_type, message.arg);

        if(message.funct_type == 0) {
            no_arg_func = (int (*)())dlsym(handle, message.f_name);
            if ((error = dlerror()) != NULL) {
                fprintf(stderr, "Error finding symbol: %s\n", error);
            } else {
                result = no_arg_func();
            }

        } else {
            arg_func = (int (*)(int))dlsym(handle, message.f_name);

            if ((error = dlerror()) != NULL) {
                fprintf(stderr, "Error finding symbol: %s\n", error);
            } else {
                result = arg_func(message.arg);
            }
        } 

        printf("Result of the function %s: %d\n", func_name, result);

        memset(result_buffer, 0, sizeof(result_buffer));
        snprintf(result_buffer, sizeof(result_buffer), "%d\n", result);

        // Write the result back to the client
        if (write(server_client_fifo, &result_buffer, strlen(result_buffer)) == -1) {
            perror("Error writing to SERVER_CLIENT_FIFO");
            break;
        }

        memset(func_name, 0, sizeof(func_name));
    }

    close(client_server_fifo);
    close(server_client_fifo);
    dlclose(handle);

    unlink(CLIENT_SERVER_FIFO);
    unlink(SERVER_CLIENT_FIFO);

    return 0;
}
