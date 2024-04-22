#ifndef CATS_FW_FLASH_H
#define CATS_FW_FLASH_H

void flash_erase(int len);
void flash_write(int len, char* data);
void flash_read(int len, char* out_data);

#endif // CATS_FW_FLASH_H