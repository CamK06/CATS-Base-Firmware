#include "shell.h"
#include "config.h"
#include "version.h"
#include "settings.h"

#include "drivers/serial.h"
#include "drivers/radio.h"
#include "drivers/gpio.h"
#include "drivers/mcu.h"

#include <pico.h>
#include <pico/bootrom.h>
#include <pico/usb_reset_interface.h>

#include <stdbool.h>

// TODO: USE FREERTOS

typedef enum errorType {
    ERROR_RADIO,
    ERROR_MISC
} error_type_t;

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
        else
            gpio_write(USB_LED_PIN, s);
        s = !s;
        mcu_sleep(500);
    }
}

int main() {
    // Initialization
    serial_init(115200);
    radio_set_channel(20); // 430.5MHz with current config   TODO: Make this generic so it works with any radio as channels may differ
    //settings_load();
    if(!radio_init())
        error(ERROR_RADIO);

    // GPIO Setup
    gpio_setup(USB_LED_PIN);
    gpio_setup(TX_LED_PIN);
    gpio_setup(RX_LED_PIN);
    gpio_set_mode(USB_LED_PIN, GPIO_OUTPUT);
    gpio_set_mode(TX_LED_PIN, GPIO_OUTPUT);
    gpio_set_mode(RX_LED_PIN, GPIO_OUTPUT);

    // Startup LED pattern (can ya tell I just got bored and carried away?)
    uint8_t leds[] = { USB_LED_PIN, TX_LED_PIN, RX_LED_PIN };
    for(int i = 0; i < sizeof(leds)*2; i++) {
        gpio_write(leds[i >= sizeof(leds) ? i-sizeof(leds) : i], i >= sizeof(leds) ? GPIO_LOW : GPIO_HIGH);
        mcu_sleep(64);
    }
    for(int i = sizeof(leds)*2; i >= 0; i--) {
        gpio_write(leds[i >= sizeof(leds) ? i-sizeof(leds) : i], i >= sizeof(leds) ? GPIO_HIGH : GPIO_LOW);
        mcu_sleep(64);
    }

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