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

static char cmd_buf[255];
static int buf_ptr = 0;

void shell_init()
{
    memset(cmd_buf, 0x00, 255);
    buf_ptr = 0;
    serial_write_str(DEVICE_NAME "\n");
    serial_write_str("Firmware Version: " CATS_FW_VERSION "\n");
    serial_write_str("Build: " BUILD_STR "\n");
    serial_putchar('>');
}

void shell_tick()
{
    // Readability? pffffttt what's that?
    while(serial_available()) {
        char c = serial_read();
        if(buf_ptr >= 254 && c != '\r' && c != 0x7f) { // Don't overflow; only allow return and delete
            continue;
        }
        serial_putchar(c);
        if(c == 0x7f && buf_ptr >= 1) { // Backspace/delete
            serial_write_str("\b \b"); // this is kinda ghetto
            cmd_buf[--buf_ptr] = 0;
            continue;
        }
        cmd_buf[buf_ptr++] = c;
        if(c == '\r') {
            break;
        }
    }
    if(cmd_buf[buf_ptr - 1] != '\r') {
        return;
    }
    serial_putchar('\n');
    buf_ptr = 0;

    // This is a stupid hack, please do this properly later
    int argc = 1;
    for(int i = 0; i < strlen(cmd_buf); i++) {
        if(cmd_buf[i] == ' ') {
            argc++;
        }
    }
    char* argv[argc];
    argc = 0;

    char* token = strtok(cmd_buf, " ");
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
