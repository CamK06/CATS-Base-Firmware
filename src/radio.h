#pragma once

#include <stdint.h>

int radio_init();
void radio_tick();
int radio_send(uint8_t* data, int len);