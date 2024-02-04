#include "shell.h"
#include "serial.h"
#include "commands.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char cmdBuf[255];
int bufPtr = 0;

void shell()
{
    // Readability? pffffttt what's that?
    while(serial_available()) {
        char c = serial_read();
        if(bufPtr >= 254 && c != '\r' && c != 0x7f) // Don't overflow; only allow return and delete
            continue;
        serial_putchar(c);
        if(c == 0x7f && bufPtr >= 1) { // Backspace/delete
            serial_write("\b \b"); // this is kinda ghetto
            cmdBuf[--bufPtr] = 0;
            continue;
        }
        cmdBuf[bufPtr++] = c;
        if(c == '\r')
            break;
    }
    if(cmdBuf[bufPtr-1] != '\r')
        return;
    serial_putchar('\n');
    bufPtr = 0;

    // This is a stupid hack, please do this properly later
    int argc = 1;
    for(int i = 0; i < strlen(cmdBuf); i++)
        if(cmdBuf[i] == ' ') argc++;
    char* argv[argc];
    argc = 0;

    char* token = strtok(cmdBuf, " ");
    while(token != NULL) {
        for(int i = 0; i < strlen(token); i++) // Commands cannot have a newline
            if(token[i] == '\n' || token[i] == '\r')
                token[i] = '\0';
        
        argv[argc] = malloc(strlen(token));
        if(argv[argc] == NULL)
            return; // Malloc fail
        strcpy(argv[argc], token);
        token = strtok(NULL, " ");
        argc++;
    }

    if(argv[0] == NULL)
        cmdCount = 0; // Stupid hack to skip the for loop so a prompt is printed
    for(int i = 0; i < cmdCount; i++) {
        if(strcmp(commands[i].cmd, argv[0]) == 0) {
            if(argc < commands[i].minArgs+1) {
                printf("Usage: %s %s\n", commands[i].cmd, commands[i].usage);
                serial_write("FAIL\n");
                break;
            }
            
            int r = commands[i].fun(argc, argv);
            if(r == SHELL_OK)
                serial_write("OK\n");
            else
                serial_write("FAIL\n");
        }
    }
    for(int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    serial_putchar('>');
}

void shell_terminate()
{

}