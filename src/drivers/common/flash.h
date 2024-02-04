#pragma once

// Erase a block of flash/EEPROM at the specified offset. 
void flash_erase(int len);
// Write data to flash memory
void flash_write(int len, char* data);
// Read data from flash memory into outData
void flash_read(int len, char* outData);
// Returns the size of flash memory
long flash_size();
// Returns the page size of flash memory
long flash_page_size();
// Returns the sector size of flash memory
long flash_sector_size();