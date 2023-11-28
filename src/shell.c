#include "shell.h"
#include <stdio.h>
#include "pico/stdlib.h"

char shellBuf[SHELL_BUF_LEN];
int bufPtr = 0;

void shell_init()
{
    for(int i = 0; i < SHELL_BUF_LEN; i++)
        shellBuf[i] = 0;
    printf("OK.\n" SHELL_PROMPT);
}

void shell()
{
    // Read from stdin
    int16_t rxChar = getchar_timeout_us(1);
    if(rxChar == PICO_ERROR_TIMEOUT)
        return;
    shellBuf[bufPtr] = rxChar;

    if(bufPtr != SHELL_BUF_LEN-1) // Last char of the buffer can only be return, backspace, etc
        putchar(shellBuf[bufPtr]);

    if(shellBuf[bufPtr] == '\n') {
        // TODO: Parse and execute command
        printf("OK.\n" SHELL_PROMPT);
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