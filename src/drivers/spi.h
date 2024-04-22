#ifndef CATS_FW_SPI_H
#define CATS_FW_SPI_H
#ifdef USE_SPI

#include <stdint.h>

#define CSPI_PORT0 0
#define CSPI_PORT1 1
#define CSPI_MSB_FIRST 1
#define CSPI_LSB_FIRST 0

// Initialize SPI at the specified baud rate on the specified port (when applicable)
void cspi_init(int baud_rate, int port, int bit_order);
// Set the pins for the SPI port
void cspi_set_pins(int tx, int rx, int sck, int cs);
// Write a buffer to the specified SPI port
// This function is blocking.
void cspi_write(int port, uint8_t *data, int len);
// Read from the specified SPI port to a buffer. Returns number of bytes read
// This function is blocking.
int cspi_read(int port, uint8_t *out_data, int len);
// Is there data available on the port? Returns 1 if true, 0 if false
int cspi_available(int port);
// Send and receive data
// This function is blocking.
int cspi_transfer(int port, uint8_t *data, int len);
// Send a byte and return the received byte
uint8_t cspi_byte(int port, uint8_t data);
#endif // USE_SPI
#endif // CATS_FW_SPI_H