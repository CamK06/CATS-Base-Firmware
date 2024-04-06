#include "gps.h"
#include "config.h"

#include "drivers/uart.h"

#include "lwgps/lwgps.h"

#define NMEA_MAX_LEN 82

static uint8_t gps_buf[128];
static int buf_ptr = 0;
extern lwgps_t hgps;

void gps_init()
{
    cuart_set_pins(GPS_UART_TX, GPS_UART_RX);
    cuart_init(CUART_PORT1, 9600);
    lwgps_init(&hgps);
}

void gps_tick()
{
    while(cuart_available(CUART_PORT1)) {
        gps_buf[buf_ptr] = cuart_read_byte(CUART_PORT1);
        buf_ptr++;
        if(buf_ptr > NMEA_MAX_LEN) {
            buf_ptr = 0; // Something's wrong, discard the data
        }

        if(gps_buf[buf_ptr - 1] == '\n') {
            lwgps_process(&hgps, gps_buf, buf_ptr);
            buf_ptr = 0;
        }
    }
}