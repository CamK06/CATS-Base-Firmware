#include "config.h"
#ifdef USE_RP2040
#include "flash.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

void flash_erase(int offset, int len)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(offset, len); // TODO: Handle sector alignment
    restore_interrupts(ints);
}

void flash_write(int offset, int len, char* data)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(offset, data, len);
    restore_interrupts(ints);
}

void flash_read(int offset, int len, char* outData)
{
    uint32_t ints = save_and_disable_interrupts();
    memcpy(outData, (char*)(XIP_BASE + offset), len);
    restore_interrupts(ints);
}

long flash_size()
{
    return PICO_FLASH_SIZE_BYTES;
}

long flash_page_size()
{
    return FLASH_PAGE_SIZE;
}

long flash_sector_size()
{
    return FLASH_SECTOR_SIZE;
}

#endif