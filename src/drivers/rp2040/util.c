#include "config.h"
#ifdef USE_RP2040
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"

#include "util.h"

void mcu_sleep(int ms)
{
    sleep_ms(ms);
}

int mcu_reset()
{
    watchdog_reboot(0, SRAM_END, 10);
    return 0;
}

int mcu_flash()
{
    reset_usb_boot(0, 0);
    return 0;
}

#endif