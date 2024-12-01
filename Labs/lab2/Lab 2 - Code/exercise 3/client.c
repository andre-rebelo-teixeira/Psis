#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_NAME "/tmp/fifo_exercise_3"

/*
    Objective of this program is to read a string from the keyboards, and send them via a FIFO to an execution server

    Each String will correspond to a command (name of a function) that will be executed by the server
*/

int main() {
    int fd = 0;
    while ((fd = open(FIFO_NAME, O_WRONLY)) == -1) {
        if (mkfifo(FIFO_NAME, 0666) != 0) {
            printf("problem creating the fifo\n");
            exit(-1);
        } else {
            printf("fifo created\n");
        }
    } 

    printf("fifo just opened\n");

    char str[100];

    memset(str, 0, 100);

    while(1) {
        printf("What command should be executed in the execution server=:\n");
        fgets(str, 100, stdin);
        // strip the newline character
        str[strlen(str) - 1] = '\0';
        write(fd, str, 100);
        memset(str, 0, 100); // clear the string once it has been sent
    }
    
    return 0;
}