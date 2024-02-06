#pragma once

#include <stdint.h>

// Sleep for the specified number of milliseconds
void mcu_sleep(int ms);
// Soft reset (supported MCUs only)
// Returns -1 if not supported
int mcu_reset();
// Reboot into flashing mode (supported MCUs only)
// Returns -1 if not supported
int mcu_flash();
// Milliseconds since bootup
uint32_t mcu_millis();