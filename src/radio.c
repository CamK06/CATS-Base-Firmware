#include "kiss.h"
#include "radio.h"
#include "config.h"
#include "settings.h"

#include "drivers/radio.h"
#include "drivers/gpio.h"
#include "drivers/mcu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cats/packet.h"
#include "cats/error.h"

static uint8_t buf[8193];
static int buf_idx = 0;
static uint32_t last_tick;

extern void print_packet(cats_packet_t* pkt);

int radio_init()
{
    if(!radio_start()) {
	    return 0;
    }
    memset(buf, 0x00, 8193);
    radio_set_frequency(430500000);  
}

void radio_tick()
{
    if(radio_get_state() == RADIO_STATE_IDLE) {
        radio_start_rx();
        return;
    }
    
    int po = radio_poll_interrupt(buf);
    if(po > 0) {
        gpio_write(RX_LED_PIN, GPIO_HIGH);
        buf_idx += po;
    }
    if(radio_get_state() == RADIO_STATE_IDLE) {
        gpio_write(RX_LED_PIN, GPIO_LOW);
        float rssi = radio_get_rssi();

        cats_packet_t* pkt;
        cats_packet_prepare(&pkt);
        if(cats_packet_decode(pkt, buf, buf_idx) != CATS_SUCCESS) {
            buf_idx = 0;
            memset(buf, 8191, 0x00);
            cats_packet_destroy(&pkt);
            gpio_write(RX_LED_PIN, GPIO_LOW);
            return;
        }
        
        printf("RECEIVED [%.1f dBm]:\n", rssi);
        print_packet(pkt);

        // Digipeating
        if(get_var("DIGIPEAT")->val[0] && cats_packet_should_digipeat(pkt, get_var("CALLSIGN")->val, get_var("SSID")->val[0])) {
            cats_route_whisker_t* route;
            const int r = cats_packet_get_route(pkt, &route);
            if(r == CATS_FAIL) {
                buf_idx = 0;
		        cats_packet_destroy(&pkt);
                return;
            }

            cats_route_hop_t* hop = &(route->hops);
            while(hop != NULL) {
                if(hop->hop_type == CATS_ROUTE_FUTURE && (strcmp(hop->callsign, get_var("CALLSIGN")->val) == 0) 
                && hop->ssid == get_var("SSID")->val[0]) {
                    hop->hop_type = CATS_ROUTE_PAST;
                    break;
                }
                hop = hop->next;
            }
            cats_route_add_past_hop(route, get_var("CALLSIGN")->val, get_var("SSID")->val[0], rssi);
            print_packet(pkt);

            uint8_t tx_buf[CATS_MAX_PKT_LEN];
            int tx_len = cats_packet_encode(pkt, tx_buf);
            if(tx_len != CATS_FAIL) {
                radio_tx(tx_buf, tx_len);
                printf("DIGIPEATED\n");
            }
        }
        
        buf_idx = 0;
        memset(buf, 0x00, 8193);
        cats_packet_destroy(&pkt);
    }

    last_tick = mcu_millis(); // This can probably be removed now that we don't use a delay
}

bool radio_send(uint8_t* data, int len)
{
    if(buf_idx > 0) { // We're receiving and there's already data pending in the TX buf
        return false;
    }

    uint16_t l = len;
    memcpy(buf, &l, sizeof(uint16_t));
    memcpy(buf + 2, data, len);

    radio_tx(buf, len + 2);
    memset(buf, 0x00, 8191);
    
    return true;
}
