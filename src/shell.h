#ifndef CATS_SHELL_H
#define CATS_SHELL_H

#define SHELL_OK 0
#define SHELL_FAIL -1

typedef struct shell_cmd {
    char cmd[16]; // Command name
    char desc[255]; // Description
    char usage[255];
    int minArgs; // Minimum number of args, else error
    int (*fun)(int argc, char* argv[]);
} shell_cmd_t;

// Initialize the shell
void shell_init();
// Step the shell; called in a loop
void shell_tick();

#endif
