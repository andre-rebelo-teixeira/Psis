#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "data_struct.h"

#define SERVER_CLIENT_FIFO "/tmp/server_client_fifo"
#define CLIENT_SERVER_FIFO "/tmp/client_server_fifo"

enum func_type {
    NO_ARG_FUNC = 0, 
    ARG_FUNC 
} func_type;

int main() {
    char func_name[256];
    int client_server_fifo = -1, server_client_fifo = -1;

    message_type message;

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
    client_server_fifo = open(CLIENT_SERVER_FIFO, O_WRONLY);
    if (client_server_fifo == -1) {
        perror("Error opening CLIENT_SERVER_FIFO");
        exit(EXIT_FAILURE);
    }

    server_client_fifo = open(SERVER_CLIENT_FIFO, O_RDONLY);
    if (server_client_fifo == -1) {
        perror("Error opening SERVER_CLIENT_FIFO");
        close(client_server_fifo);
        exit(EXIT_FAILURE);
    }

    printf("FIFOs successfully opened. Ready to communicate.\n");

    // Communication loop
    while (1) {
        char str[5];
        printf("Does the function take an argument? (y/n): ");

        if (fgets(str, sizeof(str), stdin) != NULL) {
            if (str[0] == 'y') {
                message.funct_type = ARG_FUNC;
            } else {
                message.funct_type = NO_ARG_FUNC;
            }
        }

        printf("Enter the function name: ");
        if (fgets(func_name, sizeof(func_name), stdin) != NULL) {
            func_name[strcspn(func_name, "\n")] = '\0'; // Remove newline character
            strcpy(message.f_name, func_name);
        }

        memset(&func_name, 0, sizeof(func_name));
        if (message.funct_type == ARG_FUNC) {
            printf("Enter the argument: ");
            if (fgets(func_name, sizeof(func_name), stdin) != NULL) {
                func_name[strcspn(func_name, "\n")] = '\0'; // Remove newline character
                message.arg = atoi(func_name);

            }
        }

        if (write(client_server_fifo, (void *)&message, sizeof(message)) == -1) {
            perror("Error writing to CLIENT_SERVER_FIFO");
            break;
        }

        // Clear the buffer and read the response
        memset(str, 0, sizeof(str));
        if (read(server_client_fifo, str, sizeof(str)) > 0) {
            printf("Result: %s\n", str);
        } else {
            perror("Error reading from SERVER_CLIENT_FIFO");
            break;
        }
    }

    close(client_server_fifo);
    close(server_client_fifo);

    unlink(CLIENT_SERVER_FIFO);
    unlink(SERVER_CLIENT_FIFO);

    return 0;
}
