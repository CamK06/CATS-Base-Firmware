#include "config.h"
#ifdef USE_RP2040
#include "pico/stdlib.h"
#include "tusb.h"
#include "drivers/serial.h"

int serial_init(int baudRate)
{
    // TODO: Properly initialize UART if enabled
    stdio_init_all();
}

void serial_putchar(char c)
{
    putchar(c);
}

void serial_write(const char* text)
{
    printf(text);
}


char serial_read()
{
    char c = getchar_timeout_us(1);
    if(c == PICO_ERROR_TIMEOUT)
        return -1;
    return c;
}

int serial_available()
{
    return tud_cdc_available();
    // TODO: Implement; use tud_cdc_available()?
}

int serial_connected()
{
    return tud_cdc_connected();
}

#endif