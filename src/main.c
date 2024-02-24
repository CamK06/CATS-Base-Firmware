#include "shell.h"
#include "radio.h"
#include "config.h"
#include "version.h"
#include "settings.h"

#include "drivers/serial.h"
#include "drivers/gpio.h"
#include "drivers/mcu.h"

#include <cats/packet.h>
#include <cats/error.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
        //uint8_t txBuf[] = {14, 0, 'V', 'E', '3', 'K', 'C', 'N', ' ', 'B', 'E', 'A', 'C', 'O', 'N', '\0'};
        
        serial_write("Beaconing!\n");
        // uint8_t* txBuf = NULL;
        // cats_packet_t* pkt;
        // cats_packet_prepare(&pkt);
        // cats_packet_add_identification(pkt, "VE3KCN", 17, 0);
        // cats_packet_add_comment(pkt, "Hello libCATS World!");
        // cats_packet_add_gps(pkt, 43.392281, -80.351509, 5, 0, 0, 0);
        // int len = cats_packet_build(pkt, &txBuf);
        uint8_t txBuf[CATS_MAX_PKT_LEN];


        uint8_t* buf = NULL;
        cats_packet_t* pkt;
        cats_packet_prepare(&pkt);
        cats_packet_add_identification(pkt, "VE3KCN", 17, 1);
        cats_packet_add_comment(pkt, get_var("STATUS")->val);
        cats_packet_add_gps(pkt, 43.389933, -80.347411, 5, 0, 0, 0);
        uint16_t len = cats_packet_build(pkt, &buf);
        printf("%d bytes: ", len);
        for(int i = 0; i < len; i++)
            printf("%X ", buf[i]);
        printf("\n");

        //memcpy(txBuf, &len, sizeof(uint16_t));
        memcpy(txBuf, buf, len);
        radio_send(buf, len);

        free(buf);
        free(pkt);

        lastBeaconTx = mcu_millis();
    }
}

int main() {
    // Initialization
    serial_init(115200);
    settings_load();
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
