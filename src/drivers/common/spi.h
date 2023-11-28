#pragma once
#ifdef USE_SPI

#define CSPI_PORT0 0
#define CSPI_PORT1 1

// Initialize SPI at the specified baud rate on the specified port (when applicable)
void cspi_init(int baudRate, int port);
// Set the pins for the SPI port
void cspi_set_pins(int tx, int rx, int sck, int cs);
// Write a buffer to the specified SPI port
// This function is blocking.
void cspi_write(int port, uint8_t *data, int len);
// Read from the specified SPI port to a buffer
// This function is blocking.
void cspi_read(int port, uint8_t *outData, int len);
// Is there data available on the port? Returns 1 if true, 0 if false
int cspi_available(int port);
#endif