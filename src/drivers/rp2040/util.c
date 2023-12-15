#include "config.h"
#ifdef USE_RP2040
#include "pico/stdlib.h"
#include "util.h"

void mcu_sleep(int ms)
{
    sleep_ms(ms);
}

#endif