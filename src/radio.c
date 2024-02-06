#include "radio.h"
#include "config.h"

#include "drivers/radio.h"
#include "drivers/gpio.h"

uint8_t radioBuf[8191];

int radio_init()
{
    if(!radio_start())
	    return 0;
    
    radio_set_channel(20); // TODO: Change function to set_frequency   
}

void radio_tick()
{
    if(radio_get_state() == RADIO_STATE_IDLE)
        radio_start_rx();
    radio_interrupt(radioBuf);

    int rxLen = radio_rx(radioBuf);
    if(rxLen > 0) { // We've received a packet
	    // Packet is in radioBuf; TODO: handle it.
    }
}

int radio_send(uint8_t* data, int len)
{
    radio_tx(data, len);
}