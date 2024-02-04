#include "config.h"
#ifdef USE_RP2040
#include "flash.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

extern uint32_t ADDR_FLASH_SETTINGS[];

void flash_erase(int len)
{
    uint32_t ints = save_and_disable_interrupts();
    uint32_t len_aligned = (len + FLASH_SECTOR_SIZE - 1) & ~(FLASH_SECTOR_SIZE - 1);
    flash_range_erase((uint32_t)ADDR_FLASH_SETTINGS-XIP_BASE, len_aligned);
    restore_interrupts(ints);
}

void flash_write(int len, char* data)
{
    uint32_t ints = save_and_disable_interrupts();
    uint32_t len_aligned = (len + FLASH_PAGE_SIZE - 1) & ~(FLASH_PAGE_SIZE - 1);
    flash_range_program((uint32_t)ADDR_FLASH_SETTINGS-XIP_BASE, data, len_aligned);
    restore_interrupts(ints);
}

void flash_read(int len, char* outData)
{
    uint32_t ints = save_and_disable_interrupts();
    memcpy(outData, ADDR_FLASH_SETTINGS, len);
    restore_interrupts(ints);
}

#endif