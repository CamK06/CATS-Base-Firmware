#include "version.h"
#include "config.h"

#include "gpio.h"
#include "serial.h"
#include "radio.h"
#include "settings.h"
#include "shell.h"
#include "util.h"

int main() {
    // Initialization
    settings_load();
    radio_init();
    serial_init(115200);
    
    // GPIO Setup
    gpio_setup(USB_LED_PIN);
    gpio_setup(RX_LED_PIN);
    gpio_set_mode(USB_LED_PIN, GPIO_OUTPUT);
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
            shell_init();
            gpio_write(USB_LED_PIN, usbConnected);
            continue;
        } else if(!serial_connected() && usbConnected) {
            usbConnected = false;
            shell_terminate();
            gpio_write(USB_LED_PIN, usbConnected);
            continue;
        }
        if(usbConnected)
            shell();
        mcuSleep(1);
    }
}