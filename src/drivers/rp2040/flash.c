#include "config.h"
#ifdef USE_RP2040
#include "flash.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

inline uint32_t *flash_settings_addr()
{
    extern uint32_t ADDR_FLASH_SETTINGS[];
    return ADDR_FLASH_SETTINGS;
}

void flash_erase(int len)
{
    uint32_t ints = save_and_disable_interrupts();
    uint32_t len_aligned = (len + FLASH_SECTOR_SIZE - 1) & ~(FLASH_SECTOR_SIZE - 1);
    flash_range_erase((uint32_t)flash_settings_addr()-XIP_BASE, len_aligned);
    restore_interrupts(ints);
}

void flash_write(int len, char* data)
{
    uint32_t ints = save_and_disable_interrupts();
    uint32_t len_aligned = (len + FLASH_PAGE_SIZE - 1) & ~(FLASH_PAGE_SIZE - 1);
    flash_range_program((uint32_t)flash_settings_addr()-XIP_BASE, data, len_aligned);
    restore_interrupts(ints);
}

void flash_read(int len, char* outData)
{
    uint32_t ints = save_and_disable_interrupts();
    memcpy(outData, flash_settings_addr(), len);
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