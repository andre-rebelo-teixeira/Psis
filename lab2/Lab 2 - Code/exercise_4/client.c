#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define SERVER_CLIENT_FIFO "/tmp/server_client_fifo"
#define CLIENT_SERVER_FIFO "/tmp/client_server_fifo"

int main() {
    char str[100];

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
    int client_server_fifo = open(CLIENT_SERVER_FIFO, O_WRONLY);
    if (client_server_fifo == -1) {
        perror("Error opening CLIENT_SERVER_FIFO");
        exit(EXIT_FAILURE);
    }

    int server_client_fifo = open(SERVER_CLIENT_FIFO, O_RDONLY);
    if (server_client_fifo == -1) {
        perror("Error opening SERVER_CLIENT_FIFO");
        close(client_server_fifo);
        exit(EXIT_FAILURE);
    }

    printf("FIFOs successfully opened. Ready to communicate.\n");

    // Communication loop
    while (1) {
        printf("Enter a command to execute on the server: ");
        if (fgets(str, sizeof(str), stdin) != NULL) {
            str[strcspn(str, "\n")] = '\0'; // Remove newline character
            if (write(client_server_fifo, str, strlen(str) + 1) == -1) {
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
        } else {
            fprintf(stderr, "Error reading input\n");
            break;
        }
    }

    close(client_server_fifo);
    close(server_client_fifo);

    unlink(CLIENT_SERVER_FIFO);
    unlink(SERVER_CLIENT_FIFO);

    return 0;
}
