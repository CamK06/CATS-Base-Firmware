#include "shell.h"
#include "serial.h"
#include <stdint.h>

char shellBuf[SHELL_BUF_LEN];
int bufPtr = 0;

void shell_init()
{
    for(int i = 0; i < SHELL_BUF_LEN; i++)
        shellBuf[i] = 0;
    serial_write("OK.\n" SHELL_PROMPT);
}

void shell()
{
    // Read from stdin
    int16_t rxChar = serial_read(1);
    if(rxChar == -1)
        return;
    shellBuf[bufPtr] = rxChar;

    if(bufPtr != SHELL_BUF_LEN-1) // Last char of the buffer can only be return, backspace, etc
        serial_putchar(shellBuf[bufPtr]);

    if(shellBuf[bufPtr] == '\n') {
        // TODO: Parse and execute command
        serial_write("OK.\n" SHELL_PROMPT);
        bufPtr = 0;
        return;
    }

    // Stop the 
    if(bufPtr < SHELL_BUF_LEN-1)
        bufPtr++;
}

void shell_terminate()
{

}