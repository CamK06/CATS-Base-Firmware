#include "kiss.h"
#include "shell.h"
#include "radio.h"
#include "error.h"
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
#include <stdbool.h>

uint32_t lastBeaconTx;

void beacon_tick()
{
    if(!get_var("BEACON")->val[0])
        return;
    if(mcu_millis() - lastBeaconTx >= var_val_int(get_var("BEACON_INTERVAL"))*1000) {
        serial_write_str("Beaconing!\n");
        uint8_t txBuf[CATS_MAX_PKT_LEN];

        cats_route_whisker_t route = cats_route_new(3);
        cats_packet_t* pkt;
        cats_packet_prepare(&pkt);
        cats_packet_add_identification(pkt, get_var("CALLSIGN")->val, get_var("SSID")->val[0], 1);
        cats_packet_add_comment(pkt, get_var("STATUS")->val);
        //cats_packet_add_gps(pkt, 43.389933, -80.347411, 5, 0, 0, 0);
        cats_packet_add_gps(pkt, 43.387078, -80.336053, 5, 0, 0, 0);

        cats_nodeinfo_whisker_t info;
        info.ant_gain.enabled = false;
        info.ant_height.enabled = false;
        info.battery_level.enabled = false;
        info.software_id.enabled = true;
        info.temperature.enabled = false;
        info.voltage.enabled = false;
        info.tx_power.enabled = true;
        info.uptime.enabled = true;
        info.hardware_id.enabled = true;
        info.tx_power.val = 30;
        info.uptime.val = mcu_millis() / 1000;
        info.hardware_id.val = DEVICE_HWID;
        info.software_id.val = 1;
        cats_packet_add_nodeinfo(pkt, info);
        cats_packet_add_route(pkt, route);
        
        uint16_t len = cats_packet_encode(pkt, txBuf);
        printf("%d bytes: ", len);
        for(int i = 0; i < len; i++)
            printf("%X ", txBuf[i]);
        printf("\n");

        //memcpy(txBuf, &len, sizeof(uint16_t));
        //memcpy(txBuf, buf, len);
        radio_send(txBuf, len);

        cats_packet_destroy(&pkt);

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
