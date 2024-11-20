#include <string.h>
#include <stdio.h>

int main(int argc, char ** argv) {
    char  str[1024];
    for (int i = 0; i < 1024; i++) {
        str[i] = '\0';
    }
    
    for (int i = 1; i < argc; i++) {
        strcat(str, argv[i]);
    }

    printf("%s\n" , str);

    return 0;
}