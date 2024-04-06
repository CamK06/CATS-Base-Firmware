#include "config.h"
#if defined(USE_RP2040) && defined(USE_UART)
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "drivers/uart.h"

void cuart_init(int port, int baud_rate)
{
    uart_init(port == 0 ? uart0 : uart1, baud_rate);
}

void cuart_set_pins(int tx, int rx)
{
    gpio_set_function(tx, GPIO_FUNC_UART);
    gpio_set_function(rx, GPIO_FUNC_UART);
}

int cuart_available(int port)
{
    return uart_is_readable(port == 0 ? uart0 : uart1);
}

uint8_t cuart_read_byte(int port)
{
    return uart_getc(port == 0 ? uart0 : uart1);
}

#endif