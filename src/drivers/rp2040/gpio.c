#include "config.h"
#ifdef USE_RP2040
#include "pico/stdlib.h"
#include "gpio.h"

void gpio_setup(int pin)
{
    gpio_init(pin);
}

void gpio_set_mode(int pin, int mode)
{
    gpio_set_dir(pin, mode);
}

int gpio_read(int pin)
{
    return gpio_get(pin);
}

void gpio_write(int pin, int state)
{
    gpio_put(pin, state);
}

#endif