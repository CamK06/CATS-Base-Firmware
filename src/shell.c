#include "shell.h"
#include "commands.h"
#include "config.h"
#include "version.h"

#include "drivers/serial.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern const shell_cmd_t commands[];
extern const int cmd_count;

extern char serial_buf[];
extern int serial_buf_ptr;

void shell_init()
{
    serial_write_str(DEVICE_NAME "\n");
    serial_write_str("Firmware Version: " CATS_FW_VERSION "\n");
    serial_write_str("Build: " BUILD_STR "\n");
    serial_putchar('>');
}

void shell_char_in()
{
    char c = serial_buf[serial_buf_ptr - 1];
    serial_putchar(c);
    if(c == 0x7f && serial_buf_ptr >= 2) { // Backspace/delete
        serial_write_str("\b \b"); // this is kinda ghetto
        serial_buf[--serial_buf_ptr] = 0;
        serial_buf[--serial_buf_ptr] = 0;
        return;
    }

    if(c != '\r') { // Don't execute command if last byte isn't enter/return
        return;
    }
    serial_putchar('\n');
    serial_buf_ptr = 0;

    // This is a stupid hack, please do this properly later
    int argc = 1;
    for(int i = 0; i < strlen(serial_buf); i++) {
        if(serial_buf[i] == ' ') {
            argc++;
        }
    }
    char* argv[argc];
    argc = 0;

    char* token = strtok(serial_buf, " ");
    while(token != NULL) {
        for(int i = 0; i < strlen(token); i++) { // Commands cannot have a newline
            if(token[i] == '\n' || token[i] == '\r') {
                token[i] = '\0';
            }
        }
        
        argv[argc] = malloc(strlen(token));
        if(argv[argc] == NULL) {
            return; // Malloc fail
        }
        strcpy(argv[argc], token);
        token = strtok(NULL, " ");
        argc++;
    }

    if(argv[0] == NULL) { // No command
        for(int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        serial_putchar('>');
        return;
    }

    for(int i = 0; i < cmd_count; i++) {
        if(strcmp(commands[i].cmd, argv[0]) == 0) {
            if(argc < commands[i].min_args + 1) {
                printf("Usage: %s %s\n", commands[i].cmd, commands[i].usage);
                serial_write_str("FAIL\n");
                break;
            }
            
            int r = commands[i].fun(argc, argv);
            if(r == SHELL_OK) {
                serial_write_str("OK\n");
            }
            else {
                serial_write_str("FAIL\n");
            }
        }
    }

    for(int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    serial_putchar('>');
}
