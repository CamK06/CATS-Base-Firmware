#ifndef CATS_FW_RADIO_H
#define CATS_FW_RADIO_H

#include <stdint.h>
#include <stdbool.h>

int radio_init();
void radio_tick();
bool radio_send(uint8_t* data, int len);

#endif // CATS_FW_RADIO_H