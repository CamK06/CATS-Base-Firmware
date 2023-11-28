#include "config.h"
#ifdef USE_RF4463
#ifndef USE_SPI
#error "RF4463 requires SPI"
#endif
#include "radio.h"

int radio_init()
{

}

#endif