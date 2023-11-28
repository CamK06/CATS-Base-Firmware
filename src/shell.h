#pragma once

#define SHELL_BUF_LEN 256
#define SHELL_PROMPT ">"

// Initialize the shell
void shell_init();
// Cleanly exit the shell
void shell_terminate();
// Step the shell; called in a loop
void shell();