#ifndef CATS_FW_SERIAL_H
#define CATS_FW_SERIAL_H

// Initialize serial I/O. Returns 0 if initialization succeeded, -1 if it failed.
int serial_init(int baudRate);
// Write a character to serial.
void serial_putchar(char c);
// Write a string to serial.
void serial_write_str(const char* text);
// Write raw data to serial.
void serial_write(const char* buf, int len);
// Is there data available on the port? Returns 1 if true, 0 if false
int serial_available();
// Read one character from serial. Returns -1 if there is no character
char serial_read();
// Is the PC connected/using the terminal? Returns 1 if true, 0 if false
int serial_connected();

#endif // CATS_FW_SERIAL_H