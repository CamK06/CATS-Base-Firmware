#pragma once

#include <stdint.h>

#define GPIO_INPUT 0
#define GPIO_OUTPUT 1
#define GPIO_HIGH 1
#define GPIO_LOW 0
#define GPIO_RISE 8
#define GPIO_FALL 4

typedef void (*gpio_irq_cb_t)(unsigned int gpio, uint32_t evt);

// Initialize a GPIO pin
void gpio_setup(int pin);
// Set the I/O direction of a GPIO pin. Mode is either GPIO_INPUT or GPIO_OUTPUT
void gpio_set_mode(int pin, int mode);
// Read the state of an input pin. Returns 0 for LOW, 1 for HIGH
int gpio_read(int pin);
// Set a pin state. States are either GPIO_HIGH or GPIO_LOW
void gpio_write(int pin, int state);
// Attach an interrupt handler to a pin
void gpio_attach_interrupt(int pin, int mode, gpio_irq_cb_t cb);
// Remove an interrupt handler from the specified pin
void gpio_detach_interrupt(int pin);