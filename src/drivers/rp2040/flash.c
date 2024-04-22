#include "config.h"
#ifdef USE_RP2040
#include "drivers/flash.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

extern uint32_t ADDR_FLASH_SETTINGS[];

void flash_erase(int len)
{
    const uint32_t ints = save_and_disable_interrupts();
    const uint32_t len_aligned = (len + FLASH_SECTOR_SIZE - 1) & ~(FLASH_SECTOR_SIZE - 1);
    flash_range_erase((uint32_t)ADDR_FLASH_SETTINGS - XIP_BASE, len_aligned);
    restore_interrupts(ints);
}

void flash_write(int len, char* data)
{
    const uint32_t ints = save_and_disable_interrupts();
    const uint32_t len_aligned = (len + FLASH_PAGE_SIZE - 1) & ~(FLASH_PAGE_SIZE - 1);
    flash_range_program((uint32_t)ADDR_FLASH_SETTINGS - XIP_BASE, data, len_aligned);
    restore_interrupts(ints);
}

void flash_read(int len, char* out_data)
{
    const uint32_t ints = save_and_disable_interrupts();
    memcpy(out_data, ADDR_FLASH_SETTINGS, len);
    restore_interrupts(ints);
}

#endif