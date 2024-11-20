#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
    unsigned int size = 1;

    for (unsigned int i = 1; i < argc; i++) {
        size += strlen(argv[i]) - 1; // remove the null terminator
    }

    char *str = (char * ) calloc(size, sizeof(char));

    for (unsigned int i = 1; i < argc; i++) {
        for (unsigned int j = 0; j < strlen(argv[i]); j++) {
            str[strlen(str)] = argv[i][j];
        }
    }
    printf("%s\n" , str);

    return 0;
}