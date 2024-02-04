#include "config.h"
#include "version.h"

#include "gpio.h"
#include "util.h"
#include "radio.h"
#include "shell.h"
#include "serial.h"
#include "settings.h"

#include <pico.h>
#include <pico/bootrom.h>
#include <pico/usb_reset_interface.h>

#include <stdbool.h>

// TODO: USE FREERTOS

int main() {
    // Initialization
    serial_init(115200);
    radio_set_channel(20); // 430.5MHz with current config   TODO: Make this generic so it works with any radio as channels may differ
    settings_load();

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

            int r = radio_init();
            if(r != 1)
                serial_write("Something's fucked\n");
            unsigned char packet_bin[] = {
                0x42, 0x00, 0xa7, 0x57, 0x7e, 0x2b, 0xad, 0xcd, 0xb0, 0x6c, 0xe5, 0x0d,
                0x51, 0x7a, 0x02, 0xce, 0xe5, 0xa2, 0x40, 0xa9, 0x20, 0x1f, 0x83, 0x9a,
                0xcf, 0x46, 0x4b, 0x40, 0x7b, 0x75, 0xd4, 0x40, 0x2d, 0xcc, 0x21, 0x80,
                0xae, 0xde, 0x5d, 0x69, 0x9d, 0xfd, 0x0a, 0x14, 0x8f, 0x35, 0x5b, 0x68,
                0xe9, 0x3c, 0x8f, 0x54, 0x2a, 0x46, 0xb1, 0xbc, 0xc5, 0x41, 0x6c, 0x50,
                0xdf, 0xbd, 0xd5, 0xa1, 0xe1, 0xdd, 0x30, 0x34
            };
            unsigned int packet_bin_len = 68;
            radio_tx(packet_bin, packet_bin_len);
            serial_write("DONE\n");
            while(!serial_available());
            return -1;

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