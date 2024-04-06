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

static uint8_t rxBuf[8193];
static uint8_t txBuf[8193];
static int rxIdx = 0;
static int txLen = 0;
static uint32_t lastTick;

extern void print_packet(cats_packet_t* pkt);

int radio_init()
{
    if(!radio_start())
	    return 0;
    memset(rxBuf, 0x00, 8193);
    memset(txBuf, 0x00, 8193);
    radio_set_channel(20); // TODO: Change function to set_frequency   
}

void radio_tick()
{
    if(radio_get_state() == RADIO_STATE_IDLE) {
        radio_start_rx();
        return;
    }
    
    int po = radio_poll_interrupt(rxBuf);
    if(po > 0) {
        gpio_write(RX_LED_PIN, GPIO_HIGH);
        rxIdx += po;
    }
    if(radio_get_state() == RADIO_STATE_IDLE) {
        gpio_write(RX_LED_PIN, GPIO_LOW);

        printf("\n\nPKT LEN: %d\n", rxIdx);
        for(int i = 0; i < rxIdx; i++)
            printf("%x ", rxBuf[i]);
        printf("\n\n");
        
        cats_packet_t* pkt;
        cats_packet_prepare(&pkt);
        if(cats_packet_decode(pkt, rxBuf, rxIdx) != CATS_SUCCESS) {
            printf("Failed to decode packet!!\n");
            printf("%s\n", cats_error_str);
            printf("\n\nPKT LEN: %d\n", rxIdx);
            for(int i = 0; i < rxIdx; i++)
                printf("%x ", rxBuf[i]);
            printf("\n\n");
            rxIdx = 0;
            memset(rxBuf, 8191, 0x00);
            gpio_write(RX_LED_PIN, GPIO_LOW);
            return;
        }
        
        printf("RECEIVED:\n");
        print_packet(pkt);

        // Digipeating
        if(get_var("DIGIPEAT")->val[0] && cats_packet_should_digipeat(pkt, get_var("CALLSIGN")->val, get_var("SSID")->val[0])) {
            int shouldDigipeat = 0;
            cats_route_whisker_t* route;
            int r = cats_packet_get_route(pkt, &route);
            if(r == CATS_FAIL) {
                rxIdx = 0;
		        // TODO: Free pkt!
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

            txLen = cats_packet_encode(pkt, txBuf + 2);

            if(txLen != CATS_FAIL) {
                uint16_t l = txLen;
                memcpy(txBuf, &txLen, sizeof(uint16_t));
                txLen = txLen+2;

                radio_tx(txBuf, txLen);
                memset(txBuf, 0x00, 8191);
                txLen = 0;
                printf("DIGIPEATED\n");
            }
        }

        // if(txLen > 0) { // There's data pending in the TX buf
        //     mcu_sleep(25);
        //     radio_tx(txBuf, txLen);
        //     memset(txBuf, 0x00, 8191);
        //     txLen = 0;
        // }
        
        rxIdx = 0;
        memset(rxBuf, 0x00, 8193);
        //free(pkt->whiskers);
        //free(pkt);
    }

    lastTick = mcu_millis(); // This can probably be removed now that we don't use a delay
}

int radio_send(uint8_t* data, int len)
{
    if(rxIdx > 0 && txLen > 0) // We're receiving and there's already data pending in the TX buf
        return -1;

    uint16_t l = len;
    memcpy(txBuf, &l, sizeof(uint16_t));
    memcpy(txBuf+2, data, len);
    txLen = len+2;

    if(rxIdx > 0) // We're actively receiving data
        return 0;

    radio_tx(txBuf, txLen);
    memset(txBuf, 0x00, 8191);
    txLen = 0;
}
