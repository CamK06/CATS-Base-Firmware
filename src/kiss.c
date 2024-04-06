#include "kiss.h"
#include "error.h"
#include "radio.h"

#include "drivers/serial.h"

#include <string.h>

static char dataBuf[8191];
static char serBuf[8191];
static int bufPtr = 0;
static int bufLen = 0;
static int serBufPtr = 0;
static int serBufLen = 0;

static void reset()
{
    bufLen = 0;
    bufPtr = 0;
    serBufPtr = 0;
    serBufLen = 0;
    memset(dataBuf, 0x00, 8191);
    memset(serBuf, 0x00, 8191);
}

void kiss_tick()
{
    while(serial_available()) {
        char c = serial_read();
        if(bufPtr >= 8191)
            error(ERROR_MISC);

        dataBuf[bufPtr++] = c;
        if(c == KISS_FEND && bufPtr != 0) // End of frame
            break;
    }
    if((bufPtr >= 2 && dataBuf[bufPtr-1] != KISS_FEND) || bufPtr < 2) // Frame is incomplete; don't deframe yet
        return;
    else if(dataBuf[0] != KISS_FEND || ((dataBuf[1] & 0x0F) != KISS_CMD_DATAFRAME)) { // Frame is invalid or a command; reset
        reset();
        return;
    }

    // Deframing (this is messy lol)
    bufLen = bufPtr;
    bufPtr = 0;
    for(int i = 2; i < bufLen; i++) { // Start at 2 to skip over FEND+CMD
        if(dataBuf[i] == KISS_FESC) {
            if(dataBuf[i+1] == KISS_TFEND) // FEND
                dataBuf[bufPtr++] = KISS_FEND;
            else if(dataBuf[i+1] == KISS_TFESC) // FESC
                dataBuf[bufPtr++] = KISS_FESC;
            else { // Something's wrong; reset
                reset();
                return;
            } 

            // Skip over the escaped byte
            i++;
            continue;
        }
        else if(dataBuf[i] == KISS_FEND) {
            bufLen = bufPtr; // New data length (after escaped bytes)
            break;
        }

        dataBuf[bufPtr++] = dataBuf[i];
    }

    // Transmit the data
    radio_send(dataBuf, bufLen);
    reset();
}

void kiss_send(char* buf, int len)
{
    serBuf[serBufPtr++] = KISS_FEND;
    serBuf[serBufPtr++] = KISS_CMD_DATAFRAME;

    for(int i = 0; i < len; i++) {
        if(buf[i] == KISS_FEND || buf[i] == KISS_FESC) {
            serBuf[serBufPtr++] = KISS_FESC;
            if(buf[i] == KISS_FEND)
                serBuf[serBufPtr++] = KISS_TFEND;
            else if(buf[i] == KISS_FESC)
                serBuf[serBufPtr++] = KISS_TFESC;
            continue;
        }
        
        serBuf[serBufPtr++] = buf[i];
    }
    serBuf[serBufPtr++] = KISS_FEND;

    // for(int i = 0; i < serBufPtr; i++)
    //     printf("%X ", serBuf[i]);
    // printf("\n\n");
    serial_write(serBuf, serBufPtr);

    serBufLen = 0;
    serBufPtr = 0;
    memset(serBuf, 0x00, 8191);
}