#ifndef CATS_FW_RADIO_DRIVER_H
#define CATS_FW_RADIO_DRIVER_H

#include <stdint.h>

typedef void (*radio_rx_cb_t)(char* data, uint8_t len);

typedef enum radio_state {
    RADIO_STATE_IDLE,
    RADIO_STATE_RX,
    RADIO_STATE_TX
} radio_state_t;

int radio_start();
int radio_rx_step(uint8_t* buf);
int radio_tx(uint8_t* data, int len);
int radio_start_rx();
void radio_sleep();
radio_state_t radio_get_state();
void radio_set_frequency(uint32_t frequency);
float radio_get_temp();
float radio_get_voltage();
float radio_get_rssi();

#endif // CATS_FW_RADIO_DRIVER_H