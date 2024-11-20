#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>


#define FIFO_NAME "/tmp/fifo_exercise_3"
#define LIB_NAME "./libfuncs.so"


int main() {
    int fd;
    void *handle;
    char *error;

        handle = dlopen(LIB_NAME, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    while ((fd = open(FIFO_NAME, O_RDONLY)) == -1) {
        if (mkfifo(FIFO_NAME, 0666) != 0) {
            perror("Error creating FIFO");
            exit(EXIT_FAILURE);
        } else {
            printf("FIFO created\n");
        }
    }
    printf("FIFO opened\n");

    int n;
    char str[120];

    while (1) {
                n = read(fd, str, sizeof(str));
        if (n <= 0) {
            perror("Error reading from FIFO");
            exit(EXIT_FAILURE);
        }

                str[n - 1] = '\0';

                dlerror();
        int (*func)() = (int (*)())dlsym(handle, str);
        if ((error = dlerror()) != NULL) {
            fprintf(stderr, "Error finding symbol: %s\n", error);
        } else {
                        int result = func();
            printf("%s() = %d\n", str, result);
        }
    }

        close(fd);
    dlclose(handle);
    return 0;
}
