#include "config.h"
#ifdef USE_RP2040
#include "pico/stdlib.h"
#include "drivers/gpio.h"

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

void gpio_attach_interrupt(int pin, int mode, gpio_irq_cb_t cb)
{
    gpio_set_irq_enabled_with_callback(pin, mode, true, cb);
}

void gpio_detach_interrupt(int pin)
{
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);
}

#endif