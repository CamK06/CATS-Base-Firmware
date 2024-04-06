#pragma once
#ifdef USE_UART

#include <stdint.h>

#define CUART_PORT0 0
#define CUART_PORT1 1

void cuart_init(int port, int baud_rate);
void cuart_set_pins(int tx, int rx);
int cuart_available(int port);
uint8_t cuart_read_byte(int port);

#endif