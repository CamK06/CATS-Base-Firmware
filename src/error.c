#include "error.h"
#include "config.h"

#include "drivers/gpio.h"
#include "drivers/mcu.h"

void error(error_type_t err) {
    gpio_setup(USB_LED_PIN);
    gpio_setup(TX_LED_PIN);
    gpio_setup(RX_LED_PIN);
    gpio_set_mode(USB_LED_PIN, GPIO_OUTPUT);
    gpio_set_mode(TX_LED_PIN, GPIO_OUTPUT);
    gpio_set_mode(RX_LED_PIN, GPIO_OUTPUT);
    int s = 0;
    while(1) {
        if(err == ERROR_RADIO) {
            gpio_write(TX_LED_PIN, s);
            gpio_write(RX_LED_PIN, s);
        }
        else {
            gpio_write(USB_LED_PIN, s);
        }
        s = !s;
        mcu_sleep(500);
    }
}