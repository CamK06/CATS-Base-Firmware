#include "radio.h"
#include "config.h"

#include "drivers/radio.h"
#include "drivers/gpio.h"
#include "drivers/mcu.h"

#include <stdio.h>
#include <string.h>

#include "cats/packet.h"
#include "cats/error.h"

uint8_t radioBuf[8193];
uint32_t lastTick;
int bufIdx = 0;

int radio_init()
{
    if(!radio_start())
	    return 0;
    memset(radioBuf, 0x00, 8193);
    radio_set_channel(20); // TODO: Change function to set_frequency   
}

void radio_tick()
{
    if(mcu_millis()-lastTick < 100)
        return;
        
    if(radio_get_state() == RADIO_STATE_IDLE) {
        radio_start_rx();
        return;
    }
    
    int po = radio_poll_interrupt(radioBuf);
    if(po > 0) {
        bufIdx += po;
    }
    if(radio_get_state() == RADIO_STATE_IDLE) {
        printf("\n\nPKT LEN: %d\n", bufIdx);
        for(int i = 0; i < bufIdx; i++)
            printf("%x ", radioBuf[i]);
        printf("\n\n");

        cats_packet_t* pkt;
        cats_packet_prepare(&pkt);
        if(cats_packet_from_buf(pkt, radioBuf, bufIdx) != CATS_SUCCESS) {
            printf("Failed to decode packet!!\n");
            printf("%s\n", cats_error_str);
            bufIdx = 0;
            memset(radioBuf, 8191, 0x00);
            return;
        }

        char callsign[255];
        uint8_t ssid;
        uint16_t icon;
        cats_packet_get_identification(pkt, callsign, &ssid, &icon);

        char comment[1024];
        cats_packet_get_comment(pkt, comment);
        
        printf("%s-%d: \n", callsign, ssid);

        bufIdx = 0;
        memset(radioBuf, 8191, 0x00);
        //uint16_t rxLen = radio_rx(radioBuf+2);
    }

    //int rxLen = radio_rx(radioBuf);
    //if(rxLen > 0) { // We've received a packet
	    //for(int i = 0; i < rxLen; i++)
        //    printf("%X ", radioBuf[i]);
        //printf("\n\n");
        //printf("%s\n", radioBuf);
    //}
    //printf("%d ", po);
    lastTick = mcu_millis();
}

int radio_send(uint8_t* data, int len)
{
    uint16_t l = len;
    memcpy(radioBuf, &l, sizeof(uint16_t));
    memcpy(radioBuf+2, data, len);
    radio_tx(radioBuf, len+2);
    memset(radioBuf, 0x00, 8191);
}