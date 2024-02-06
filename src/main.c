#include "shell.h"
#include "radio.h"
#include "config.h"
#include "version.h"
#include "settings.h"

#include "drivers/serial.h"
#include "drivers/gpio.h"
#include "drivers/mcu.h"

uint32_t lastBeaconTx;

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

void beacon_tick()
{
    if(!get_var("BEACON")->val[0])
        return;
    if(mcu_millis() - lastBeaconTx >= var_val_int(get_var("BEACON_INTERVAL"))*1000) {
        radio_send("This is a test packet being transmitted at a regular interval!", 63);
        lastBeaconTx = mcu_millis();
    }
}

int main() {
    // Initialization
    serial_init(115200);
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
    uint8_t usbConnected = 0;
    while(1) {
        if(serial_connected() && !usbConnected) {
            usbConnected = 1;
	        shell_init();
            gpio_write(USB_LED_PIN, usbConnected);
        } else if(!serial_connected() && usbConnected) {
            usbConnected = 0;
            gpio_write(USB_LED_PIN, usbConnected);
            mcu_flash();
        }
        
	    radio_tick();
	    shell_tick();
	    beacon_tick();
        mcu_sleep(1);
    }
}
