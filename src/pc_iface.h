#ifndef CATS_FW_PC_IFACE_H
#define CATS_FW_PC_IFACE_H

#include "cats/packet.h"

void pc_iface_char_in();
void pc_iface_send(cats_packet_t* pkt, float rssi);

#endif // CATS_FW_PC_IFACE_H