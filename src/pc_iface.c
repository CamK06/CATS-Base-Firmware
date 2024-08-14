#include "pc_iface.h"
#include "radio.h"

#include "drivers/mcu.h"
#include "drivers/serial.h"

#define BUILD_RADIO_IFACE
#include "cats/error.h"
#include "cats/packet.h"
#include "cats/radio_iface.h"

#include <string.h>
#include <stdio.h>

#define SERIAL_BUF_LEN 10000

extern char serial_buf[];
extern int serial_buf_ptr;

static const uint8_t CBOR_BEGIN[3] = { 0xd9, 0xd9, 0xf7 };
static uint32_t last_rx = -1;

static void reset_serial_buf()
{
    memset(serial_buf, 0x00, SERIAL_BUF_LEN);
    serial_buf_ptr = 0;
    last_rx = -1;
}

void pc_iface_char_in()
{
    // 3 second serial RX Timeout
    if(mcu_millis() - last_rx >= 3000 && last_rx != -1) {
        printf("RX Timeout!\n");
        reset_serial_buf();
    }
    last_rx = mcu_millis();

    if(serial_buf_ptr < 3) {
        return;
    }

    if(memcmp(serial_buf, CBOR_BEGIN, 3) != 0) {
        reset_serial_buf();
        return;
    }

    float rssi = 0;
    int r = cats_radio_iface_decode(serial_buf, serial_buf_ptr, &rssi);
    if(r == CATS_FAIL) {
        return;
    }
    serial_buf_ptr = r;

    cats_packet_t* pkt;
    cats_packet_prepare(&pkt);
    if(cats_packet_semi_decode(pkt, serial_buf, serial_buf_ptr) == CATS_FAIL) { // Failed to decode packet
        cats_packet_destroy(&pkt);
        reset_serial_buf();
        printf("Failed to semi decode the CATS packet? wtf??");
        return;
    }
    reset_serial_buf();

    r = cats_packet_encode(pkt, serial_buf);
    radio_send(serial_buf, r);
    cats_packet_destroy(&pkt);
    reset_serial_buf();
}