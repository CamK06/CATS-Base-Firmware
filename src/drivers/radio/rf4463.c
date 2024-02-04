#include "config.h"
#ifdef USE_RF4463
#ifndef USE_SPI
#error "RF4463 requires SPI"
#endif
#include "radio.h"
#include "rf4463.h"
#include "spi.h"
#include "gpio.h"
#include "util.h"
#include <string.h>
#include <stdio.h>
#include "serial.h"

#include <stdlib.h>
#include "pico/stdlib.h"

radio_state_t int_radio_state = RADIO_STATE_IDLE;
radio_rx_cb_t rx_Callback;
int radio_channel = 0;

int radio_init()
{
    // Initialize GPIO + SPI
    gpio_setup(RADIO_SDN_PIN);
    gpio_setup(RADIO_IRQ_PIN);
    gpio_setup(RADIO_CS_PIN);
    gpio_set_mode(RADIO_SDN_PIN, GPIO_OUTPUT);
    gpio_set_mode(RADIO_CS_PIN, GPIO_OUTPUT);
    gpio_set_mode(RADIO_IRQ_PIN, GPIO_INPUT);
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_init(1000000, CSPI_PORT0, CSPI_MSB_FIRST);
    cspi_set_pins(RADIO_TX_PIN, RADIO_RX_PIN, RADIO_SCK_PIN, RADIO_CS_PIN);

    // Initialize the RF4463
    radio_poweron();

    // Set the radio parameters
    int len;
    uint8_t cmd;
    uint8_t buf[32];
    for(int i = 0; i < sizeof(radioConfig); i+= len+1) {
        len = radioConfig[i];
        if(i == 0)
            continue;
        memcpy(buf, radioConfig+i+1, len);
        si_send_command(buf, len);
    }
    si_cli();
    radio_sleep();
    return 1;
}

int radio_tx(uint8_t* data, int len)
{
    // Setup; bring the radio back to an idle state
    if(int_radio_state == RADIO_STATE_RX)
        radio_sleep();
    si_send_command((uint8_t[]){RF4463_CMD_FIFO_INFO, 0x01 | 0x02}, 2);
    si_send_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0, 0, 0xFF}, 4);
    si_cli();

    // Start the transmission
    int fifo_space = 128;
    int chunk_len = (len < fifo_space) ? len : fifo_space;
    printf("INITIAL LEN %d", chunk_len);
    uint8_t txBuf[128];
    memcpy(txBuf, data, chunk_len);

    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_byte(CSPI_PORT0, RF4463_CMD_TX_FIFO_WRITE);
    cspi_transfer(CSPI_PORT0, txBuf, chunk_len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    
    gpio_write(TX_LED_PIN, GPIO_HIGH);
    si_send_command((uint8_t[]){RF4463_CMD_START_TX, 20, 0b00010000, (uint8_t)((uint16_t)len >> 8), (uint8_t)len, 0, 0}, 6);
    int_radio_state = RADIO_STATE_TX;
    return;

    // Step through the buffer until TX is finished
    // TODO: Add a timeout(?)
    int tx = chunk_len;
    while(int_radio_state == RADIO_STATE_TX) {
        if(si_irq()) {
            si_cli();
            break;
        }

        fifo_space = si_tx_fifo_space();
        if(fifo_space <= 0)
            continue;

        chunk_len = ((len - tx) < fifo_space) ? (len - tx) : fifo_space;
        printf("LEN: %d  FIFO: %d  LEN-TX:%d\n", chunk_len, fifo_space, len-tx);
        if(chunk_len <= 0) {
            mcu_sleep(16);
            continue;
        }
        memcpy(txBuf, data+tx, chunk_len);

        if(si_fifo_underflow_pending()) {
            printf("FIFO UNDERFLOW\n");
            break;
        }

        // Send the next chunk
        gpio_write(RADIO_CS_PIN, GPIO_LOW);
        cspi_byte(CSPI_PORT0, RF4463_CMD_TX_FIFO_WRITE);
        cspi_transfer(CSPI_PORT0, txBuf, chunk_len);
        gpio_write(RADIO_CS_PIN, GPIO_HIGH);
        tx += chunk_len;
        mcu_sleep(16);

        if(si_packet_sent_pending())
            break;
    }
    
    // Cleanup
    // CLI and sleep may be harmful as the radio should already sleep on its own after TX...
    //si_cli();
    //radio_sleep();
    int_radio_state = RADIO_STATE_IDLE;
    gpio_write(TX_LED_PIN, GPIO_LOW);
    return 0;
}

void radio_poweron()
{
    uint8_t buf[7];
    memcpy(buf, radioConfig+1, 7);

    // Reset the radio
    gpio_write(RADIO_SDN_PIN, GPIO_HIGH);
    mcu_sleep(100);
    gpio_write(RADIO_SDN_PIN, GPIO_LOW);
    mcu_sleep(5);

    si_send_command(buf, 7);
    mcu_sleep(250);
}

void radio_sleep()
{
    si_send_command((uint8_t[]){RF4463_CMD_CHANGE_STATE, 0x01}, 2);
}

void radio_set_channel(int channel)
{
    radio_channel = channel;
}

void radio_set_rx_callback(radio_rx_cb_t cb)
{
    rx_Callback = cb;
}

radio_state_t radio_get_state()
{
    return int_radio_state;
}

int si_set_property(uint16_t property, uint8_t* data, uint8_t len)
{
    if(!si_cts())
        return 0;

    uint8_t buf[len+4];
    memcpy(buf+4, data, len);
    buf[0] = RF4463_CMD_SET_PROPERTY;
    buf[1] = property >> 8;
    buf[2] = len;
    buf[3] = property && 0xFF;

    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_transfer(CSPI_PORT0, buf, len+4);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);

    return 1;
}

int si_send_command(uint8_t* cmd, int len)
{
    uint8_t buf[len];
    memcpy(buf, cmd, len);

    // Send the command
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_write(CSPI_PORT0, buf, len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);

    si_cts();
    return 1;
}

int si_read_command(uint8_t* cmd, uint8_t len, uint8_t* outData, uint8_t outLen)
{
    if(!si_cts())
        return 0;

    // Zero the output buffer so only 0x00 is sent to the module
    // This may not be necessary, TEST THIS.
    for(int i = 0; i < outLen; i++)
        outData[i] = 0x00;
    uint8_t buf[len];
    memcpy(buf, cmd, len);

    // Send the command to read
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_transfer(CSPI_PORT0, buf, len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_byte(CSPI_PORT0, RF4463_CMD_READ_BUF);
    int read = cspi_transfer(CSPI_PORT0, outData, outLen);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);

    si_cts();

    return read;
}

int si_read_rx_fifo(uint8_t* outData)
{
    if(!si_cts())
        return 0;
    
    int read = si_rx_fifo_len();
    read = si_read_command((uint8_t[]){RF4463_CMD_RX_FIFO_READ}, 1, outData, read); // TODO: Remove 0xff CTS at index 0 of read data
    return read;
}

void si_start_rx(int len)
{
    si_send_command((uint8_t[]){RF4463_CMD_FIFO_INFO, 0x01 | 0x02}, 2);
    si_enable_rx_int();
    si_cli();
    int_radio_state = RADIO_STATE_RX;
    si_send_command((uint8_t[]){RF4463_CMD_START_RX, radio_channel, 0, len >> 8, len & 0xff, 0x10, 0x10, 0x10}, 8); // TODO: Look into what 0x10 should be- appears to be state to enter after RX? Currently sleep.
}

int si_rx_packet(uint8_t* outData, int len)
{
    // Receive the packet
    si_start_rx(len);
    while(!si_irq())
        mcu_sleep(10);
    int rx = si_read_rx_fifo(outData);
    
    // Cleanup
    si_cli();
    radio_sleep();
    int_radio_state = RADIO_STATE_IDLE;
    return rx;
}

int si_rx_fifo_len()
{
    uint8_t fifo[4];
    if(si_read_command((uint8_t[]){RF4463_CMD_FIFO_INFO, 0x00}, 2, fifo, 4) <= 0)
        return 0;
    return fifo[1];
}

int si_tx_fifo_space()
{
    uint8_t fifo[4];
    if(si_read_command((uint8_t[]){RF4463_CMD_FIFO_INFO, 0x00}, 2, fifo, 4) <= 0)
        return 0;
    return fifo[2];
}

int si_packet_sent_pending()
{
    uint8_t outData[4];
    si_read_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0xFF ^ (1 << 5), 0xFF, 0xFF}, 4, outData, 4);
    return (outData[3] & (1 << 5)) != 0;
}

int si_packet_rx_pending()
{
    uint8_t outData[4];
    si_read_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0xFF ^ (1 << 4), 0xFF, 0xFF}, 4, outData, 4);
    return (outData[2] & (1 << 4)) != 0;
}

int si_fifo_underflow_pending()
{
    uint8_t outData[8];
    si_read_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0xFF, 0xFF, 0xFF ^ (1 << 5)}, 4, outData, 7);
    return (outData[7] & (1 << 5)) != 0;
}

void si_enable_tx_int()
{
    si_set_property(RF4463_PROPERTY_INT_CTL_ENABLE, (uint8_t[]){0x01, 0x20, 0x00}, 3);
}

void si_enable_rx_int()
{
    //si_set_property(RF4463_PROPERTY_INT_CTL_ENABLE, (uint8_t[]){0x03, 0x18, 0x00}, 3);
}

int si_cli()
{
    si_send_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0x00, 0x00, 0x00}, 1);
}

int si_irq()
{
    return !gpio_read(RADIO_IRQ_PIN); // Inverse for active low pin
}

int si_cts()
{
    int timeout = RF4463_CTS_TIMEOUT;
    //while(gpio_read(RADIO_GP1_PIN) == 0)
    //    mcu_sleep(1);
    //serial_write("CTS OK\n");
    while(1) {
        gpio_write(RADIO_CS_PIN, GPIO_LOW);
        cspi_byte(CSPI_PORT0, RF4463_CMD_READ_BUF);
        uint8_t read = cspi_byte(CSPI_PORT0, 0x00);
        if(read == 0xff) {
            gpio_write(RADIO_CS_PIN, GPIO_HIGH);
            return 1;
        }
        gpio_write(RADIO_CS_PIN, GPIO_HIGH);
        //mcu_sleep(20);
    }
    //mcu_sleep(50);
    return 1;
}

int si_check()
{
    uint8_t buf[9];
    uint16_t partInfo;
    if(si_read_command((uint8_t[]){RF4463_CMD_PART_INFO}, 1, buf, 9) <= 0)
        return 0;
    partInfo = buf[2]<<8 | buf[3];
    if(partInfo != 0x4463)
        return 0;

    return 1;
}

#endif
