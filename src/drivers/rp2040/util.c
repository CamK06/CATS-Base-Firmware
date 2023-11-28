#include "config.h"
#ifdef USE_RP2040
#include "pico/stdlib.h"
#include "util.h"

void mcuSleep(int ms)
{
    sleep_ms(ms);
}

#endif