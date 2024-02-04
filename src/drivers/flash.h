#pragma once

// Erase a block of flash/EEPROM at the specified offset. 
void flash_erase(int len);
// Write data to flash memory
void flash_write(int len, char* data);
// Read data from flash memory into outData
void flash_read(int len, char* outData);