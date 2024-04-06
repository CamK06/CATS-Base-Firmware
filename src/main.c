#include "gps.h"
#include "kiss.h"
#include "shell.h"
#include "radio.h"
#include "error.h"
#include "config.h"
#include "version.h"
#include "settings.h"

#include "drivers/serial.h"
#include "drivers/gpio.h"
#include "drivers/uart.h"
#include "drivers/mcu.h"

#include "cats/error.h"
#include "cats/packet.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "lwgps/lwgps.h"

uint32_t lastBeaconTx;
lwgps_t hgps;

void print_packet(cats_packet_t* pkt)
{
    cats_whisker_data_t* data;
    if(cats_packet_get_identification(pkt, (cats_ident_whisker_t**)&data) == CATS_SUCCESS) {
        printf("IDENT: \t%s-%d [ICON: %d]\n", 
                data->identification.callsign, 
                data->identification.ssid, 
                data->identification.icon
        );
    }
    if(cats_packet_get_route(pkt, (cats_route_whisker_t**)&data) == CATS_SUCCESS) {
        printf("ROUTE: \t(MAX %d) ", data->route.max_digipeats);
        cats_route_hop_t* hop = &(data->route.hops);
        while(hop != NULL) {
            if(hop->hop_type == CATS_ROUTE_INET) {
                printf("[NET]");
            }
            else if(hop->hop_type == CATS_ROUTE_FUTURE) {
                printf("%s-%d*", hop->callsign, hop->ssid);
            }
            else if(hop->hop_type == CATS_ROUTE_PAST) {
                printf("%s-%d [%d dBm]", hop->callsign, hop->ssid, hop->rssi);
            }
            if(hop->next != NULL) {
                printf(" -> ");
            }
            hop = hop->next;
        }
        printf("\n");
    }
    char comment[CATS_MAX_PKT_LEN];
    if(cats_packet_get_comment(pkt, comment) == CATS_SUCCESS) {
        printf("CMNT: \t'%s'\n", comment);
    }
    if(cats_packet_get_gps(pkt, (cats_gps_whisker_t**)&data) == CATS_SUCCESS) {
        printf("GPS: \t(%.4f, %.4f) +/- %d m, v = %.2f m/s [N %.2f] deg\nALT: \t%.2f m\n",
                data->gps.latitude,
                data->gps.longitude,
                data->gps.max_error,
                data->gps.speed,
                data->gps.heading,
                data->gps.altitude
        );
    }
    if(cats_packet_get_nodeinfo(pkt, (cats_nodeinfo_whisker_t**)&data) == CATS_SUCCESS) {
        if(data->node_info.hardware_id.enabled && data->node_info.software_id.enabled) {
            printf("HW: \t0x%x SW: 0x%x\n", data->node_info.hardware_id.val, data->node_info.software_id.val);
        }
        else if(data->node_info.hardware_id.enabled) {
            printf("HW: \t0x%x\n", data->node_info.hardware_id.val);
        }
        else if(data->node_info.software_id.enabled) {
            printf("SW: \t0x%x\n", data->node_info.software_id.val);
        }

        if(data->node_info.uptime.enabled) {
            printf("UTIME: \t%d s\n", data->node_info.uptime.val);
        }
        if(data->node_info.ant_height.enabled) {
            printf("VERT: \t%d m\n", data->node_info.ant_height.val);
        }
        if(data->node_info.ant_gain.enabled) {
            printf("GAIN: \t%.2f dBi\n", data->node_info.ant_gain.val);
        }
        if(data->node_info.tx_power.enabled) {
            printf("TXP: \t%d dBm\n", data->node_info.tx_power.val);
        }
        if(data->node_info.voltage.enabled) {
            printf("VOLTS: \t%d V\n", data->node_info.voltage.val);
        }
        if(data->node_info.temperature.enabled) {
            printf("TEMP: \t%d C\n", data->node_info.temperature.val);
        }
    }

    printf("\n");
}

void beacon_tick()
{
    if(!get_var("BEACON")->val[0])
        return;
    if(mcu_millis() - lastBeaconTx >= var_val_int(get_var("BEACON_INTERVAL"))*1000) {
        uint8_t txBuf[CATS_MAX_PKT_LEN];

        cats_route_whisker_t route = cats_route_new(3);
        cats_packet_t* pkt;
        cats_packet_prepare(&pkt);
        cats_packet_add_identification(pkt, get_var("CALLSIGN")->val, get_var("SSID")->val[0], 1);
        cats_packet_add_comment(pkt, get_var("STATUS")->val);
        //cats_packet_add_gps(pkt, 43.389933, -80.347411, 5, 0, 0, 0);
        if(hgps.is_valid) {
            cats_packet_add_gps(pkt, hgps.latitude, hgps.longitude, hgps.altitude, hgps.variation, hgps.course, hgps.speed);
        }

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
        
        printf("BEACONING:\n");
        print_packet(pkt);

        uint16_t len = cats_packet_encode(pkt, txBuf);
        /*
        printf("%d bytes: ", len);
        for(int i = 0; i < len; i++)
            printf("%X ", txBuf[i]);
        printf("\n");
        */
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

    gps_init();

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
    int uartPtr = 0;
    uint8_t uartBuf[128];
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
	    gps_tick();
        beacon_tick();

        mcu_sleep(1);
    }
}
