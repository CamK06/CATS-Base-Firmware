#include "version.h"
#include "config.h"

#include "gpio.h"
#include "serial.h"
#include "radio.h"
#include "settings.h"
#include "shell.h"
#include "util.h"
#include "cats/cats.h"
#include <string.h>
#include <stdbool.h>

#include <pico.h>
#include <pico/usb_reset_interface.h>
#include <pico/bootrom.h>

// TODO: USE FREERTOS

int main() {
    // Initialization
    //settings_load();
    serial_init(115200);
    radio_init();
    radio_set_channel(20); // 430.5MHz with current config   TODO: Make this generic so it works with any radio as channels may differ

    // GPIO Setup
    gpio_setup(USB_LED_PIN);
    gpio_setup(TX_LED_PIN);
    gpio_setup(RX_LED_PIN);
    gpio_set_mode(USB_LED_PIN, GPIO_OUTPUT);
    gpio_set_mode(TX_LED_PIN, GPIO_OUTPUT);
    gpio_set_mode(RX_LED_PIN, GPIO_OUTPUT);

    // Main Loop
    // TODO: Use interrupts instead of polling?
    bool usbConnected = false;
    while(true) {
        if(serial_connected() && !usbConnected) {
            usbConnected = true;
            serial_write(DEVICE_NAME "\n");
            serial_write("Firmware Version: " VERSION "\n");
            serial_write("Build: " BUILD_STR "\n");
            gpio_write(USB_LED_PIN, usbConnected);
            serial_putchar('>');
            continue;
        } else if(!serial_connected() && usbConnected) {
            usbConnected = false;
            shell_terminate();
            gpio_write(USB_LED_PIN, usbConnected);
            reset_usb_boot(0, 0);
            continue;
        }
        if(serial_available())
            shell();
        mcu_sleep(1);
    }
}