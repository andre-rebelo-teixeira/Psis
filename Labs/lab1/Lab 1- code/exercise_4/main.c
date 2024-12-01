#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

int main() {
    int a;
    char line[100];
    char library_name[100];
    void *handle;
    void (*func_1)(void);
    void (*func_2)(void);
    char *error;

    printf("What version of the functions do you want to use?\n");
    printf("\t1 - Normal    (lib1)\n");
    printf("\t2 - Optimized (lib2)\n");
    fgets(line, 100, stdin);
    sscanf(line, "%d", &a);

    // Choose the library based on user input
    if (a == 1) {
        strcpy(library_name, "./liblib1.so");
        printf("Running the normal versions from %s\n", library_name);
    } else if (a == 2) {
        strcpy(library_name, "./liblib2.so");
        printf("Running the optimized versions from %s\n", library_name);
    } else {
        printf("Invalid option. Not running anything.\n");
        exit(-1);
    }

    // Load the selected library
    handle = dlopen(library_name, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(-1);
    }

    // Clear any existing errors
    dlerror();

    // Load func_1 from the loaded library
    func_1 = (void (*)(void)) dlsym(handle, "func_1");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error loading func_1: %s\n", error);
        dlclose(handle);
        exit(-1);
    }

    // Load func_2 from the loaded library
    func_2 = (void (*)(void)) dlsym(handle, "func_2");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error loading func_2: %s\n", error);
        dlclose(handle);
        exit(-1);
    }

    // Call func_1 and func_2
    func_1();
    func_2();

    // Close the library
    dlclose(handle);
    exit(0);
}
