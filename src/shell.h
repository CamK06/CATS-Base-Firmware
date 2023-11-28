#pragma once

#define SHELL_BUF_LEN 256
#define SHELL_PROMPT ">"

void shell_init();
void shell_terminate();
void shell();