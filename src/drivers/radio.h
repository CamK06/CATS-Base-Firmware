#pragma once

#include <stdint.h>

typedef void (*radio_rx_cb_t)(char* data, uint8_t len);

typedef enum radio_state {
    RADIO_STATE_IDLE,
    RADIO_STATE_RX,
    RADIO_STATE_TX
} radio_state_t;

// Initialize the radio. Returns 0 if initialization succeeded, -1 if failed
int radio_init();
// Transmit a packet. Returns 0 if the command was successfully sent, -1 if failed, *NOT* if the transmission itself succeeded or failed
int radio_tx(uint8_t* data, int len);
// Enable the radio
void radio_poweron();
// Put the radio to sleep
void radio_sleep();
// Get the current state of the radio
radio_state_t radio_get_state();
// Set the radio channel
void radio_set_channel(int channel);