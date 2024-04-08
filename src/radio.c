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

#include "cats/packet.h"
#include "cats/error.h"

static uint8_t rx_buf[8193];
static uint8_t tx_buf[8193];
static int rx_idx = 0;
static int tx_len = 0;
static uint32_t last_tick;

extern void print_packet(cats_packet_t* pkt);

int radio_init()
{
    if(!radio_start()) {
	    return 0;
    }
    memset(rx_buf, 0x00, 8193);
    memset(tx_buf, 0x00, 8193);
    radio_set_channel(20); // TODO: Change function to set_frequency   
}

void radio_tick()
{
    if(radio_get_state() == RADIO_STATE_IDLE) {
        radio_start_rx();
        return;
    }
    
    int po = radio_poll_interrupt(rx_buf);
    if(po > 0) {
        gpio_write(RX_LED_PIN, GPIO_HIGH);
        rx_idx += po;
    }
    if(radio_get_state() == RADIO_STATE_IDLE) {
        gpio_write(RX_LED_PIN, GPIO_LOW);
        cats_packet_t* pkt;
        cats_packet_prepare(&pkt);
        if(cats_packet_decode(pkt, rx_buf, rx_idx) != CATS_SUCCESS) {
            rx_idx = 0;
            memset(rx_buf, 8191, 0x00);
            cats_packet_destroy(&pkt);
            gpio_write(RX_LED_PIN, GPIO_LOW);
            return;
        }
        
        printf("RECEIVED:\n");
        print_packet(pkt);

        // Digipeating
        if(get_var("DIGIPEAT")->val[0] && cats_packet_should_digipeat(pkt, get_var("CALLSIGN")->val, get_var("SSID")->val[0])) {
            cats_route_whisker_t* route;
            const int r = cats_packet_get_route(pkt, &route);
            if(r == CATS_FAIL) {
                rx_idx = 0;
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
            cats_route_add_past_hop(route, get_var("CALLSIGN")->val, get_var("SSID")->val[0], 0);

            tx_len = cats_packet_encode(pkt, tx_buf + 2);

            if(tx_len != CATS_FAIL) {
                memcpy(tx_buf, &tx_len, sizeof(uint16_t));
                tx_len += 2;

                radio_tx(tx_buf, tx_len);
                memset(tx_buf, 0x00, 8191);
                tx_len = 0;
                printf("DIGIPEATED\n");
            }
        }
        
        rx_idx = 0;
        memset(rx_buf, 0x00, 8193);
        cats_packet_destroy(&pkt);
    }

    last_tick = mcu_millis(); // This can probably be removed now that we don't use a delay
}

int radio_send(uint8_t* data, int len)
{
    if(rx_idx > 0 && tx_len > 0) { // We're receiving and there's already data pending in the TX buf
        return -1;
    }

    uint16_t l = len;
    memcpy(tx_buf, &l, sizeof(uint16_t));
    memcpy(tx_buf+2, data, len);
    tx_len = len+2;

    if(rx_idx > 0) { // We're actively receiving data
        return 0;
    }

    radio_tx(tx_buf, tx_len);
    memset(tx_buf, 0x00, 8191);
    tx_len = 0;
}
