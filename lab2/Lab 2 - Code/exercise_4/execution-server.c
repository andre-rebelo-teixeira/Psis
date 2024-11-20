#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>

#define SERVER_CLIENT_FIFO "/tmp/server_client_fifo"
#define CLIENT_SERVER_FIFO "/tmp/client_server_fifo"

int main() {
    char func_name[256];

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
    int client_server_fifo = open(CLIENT_SERVER_FIFO, O_RDONLY);
    if (client_server_fifo == -1) {
        perror("Error opening CLIENT_SERVER_FIFO");
        exit(EXIT_FAILURE);
    }

    int server_client_fifo = open(SERVER_CLIENT_FIFO, O_WRONLY);
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

    char result_str[256];
    // Communication loop
    while (1) {
        memset(func_name, 0, sizeof(func_name));
        if (read(client_server_fifo, func_name, sizeof(func_name)) <= 0) {
            perror("Error reading from CLIENT_SERVER_FIFO");
            break;
        }

        // Lookup the function in the library
        int (*func)();
        char *error;
        func = (int (*)())dlsym(handle, func_name);

        int result = -1; // Default result in case of error
        if ((error = dlerror()) != NULL) {
            fprintf(stderr, "Error finding symbol: %s\n", error);
        } else {
            result = func();
        }

        printf("Result of the function %s: %d\n", func_name, result);

        memset(result_str, 0, sizeof(result_str));
        snprintf(result_str, sizeof(result_str), "%d\n", result);

        // Write the result back to the client
        if (write(server_client_fifo, &result_str, strlen(result_str)) == -1) {
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
