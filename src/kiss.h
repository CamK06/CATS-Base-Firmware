#ifndef CATS_FW_KISS_H
#define CATS_FW_KISS_H

#define KISS_FEND 0xC0
#define KISS_FESC 0xDB
#define KISS_TFEND 0xDC
#define KISS_TFESC 0xDD

#define KISS_CMD_DATAFRAME 0x00
#define KISS_CMD_TXDELAY 0x01
#define KISS_CMD_PERSIST 0x02
#define KISS_CMD_SLOT_TIME 0x03
#define KISS_CMD_TX_TAIL 0x04
#define KISS_CMD_DUPLEX 0x05
#define KISS_CMD_SET_HARDWARE 0x06
#define KISS_CMD_RETURN 0xff

void kiss_tick();
void kiss_send(char* buf, int len);

#endif // CATS_KISS_H