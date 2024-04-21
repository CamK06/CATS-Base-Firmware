#ifndef CATS_COMMANDS_H
#define CATS_COMMANDS_H

#include "shell.h"

int cmd_store_settings(int argc, char* argv[]);
int cmd_list(int argc, char* argv[]);
int cmd_get(int argc, char* argv[]);
int cmd_set(int argc, char* argv[]);
int cmd_help(int argc, char* argv[]);
int cmd_ver(int argc, char* argv[]);
int cmd_reboot(int argc, char* argv[]);

#endif