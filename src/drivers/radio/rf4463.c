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
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    //gpio_attach_interrupt(RADIO_IRQ_PIN, GPIO_FALL, si_interrupt_handler);
    cspi_init(1000000, CSPI_PORT0, CSPI_MSB_FIRST);
    cspi_set_pins(RADIO_TX_PIN, RADIO_RX_PIN, RADIO_SCK_PIN, RADIO_CS_PIN);

    // Initialize the RF4463
    radio_poweron();

    // Set the radio parameters
    int len;
    uint8_t cmd;
    uint8_t buf[32];
    for(int i = 0; i < sizeof(catsConfig); i+= len+1) {
        len = catsConfig[i];
        if(i == 0) // Skip the initial powerup command
            continue;
        memcpy(buf, &catsConfig[i+1], len);
        si_cts();
        // Send the command
        gpio_write(RADIO_CS_PIN, GPIO_LOW);
        cspi_write(CSPI_PORT0, buf, len);
        gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    }
    si_cli();

    if(si_check() != 1)
        return 0;

    //si_sti();
    radio_sleep();
    mcu_sleep(100);
    return 1;
}

int radio_tx(uint8_t* data, int len)
{
    // Setup; bring the radio back to an idle state
    if(int_radio_state == RADIO_STATE_RX)
        radio_sleep();
    si_send_command(RF4463_CMD_FIFO_INFO, (uint8_t[]){0x01 | 0x02}, 1);
    //si_cli(); // Probably not needed- test this.

    // Start the transmission
    int fifo_space = 0;
    int chunk_len = len < 128 ? len : 128;
    si_send_command(RF4463_CMD_TX_FIFO_WRITE, data, chunk_len);
    si_enable_tx_int();
    si_cli();
    si_send_command(RF4463_CMD_START_TX, (uint8_t[]){radio_channel, 0x10, len >> 8, len & 0xFF, 0, 0}, 6);
    int_radio_state = RADIO_STATE_TX;
    gpio_write(TX_LED_PIN, GPIO_HIGH);

    // Step through the buffer until TX is finished
    // TODO: Add a timeout(?)
    int tx = chunk_len;
    while(int_radio_state == RADIO_STATE_TX) {
        // No interrupt, wait then check again
        if(si_irq())
            break;
        gpio_write(RX_LED_PIN, GPIO_HIGH);
        fifo_space = si_tx_fifo_space();
        //chunk_len = (len - tx) > fifo_space ? fifo_space : len - tx;
        // if(chunk_len <= 0) {
        //     mcu_sleep(10);
        //     break;
        // }

        // // Send the next chunk
        // //si_send_command(RF4463_CMD_TX_FIFO_WRITE, data, chunk_len);
        // tx += chunk_len;
        // mcu_sleep(10);
    }
    gpio_write(TX_LED_PIN, GPIO_LOW);
    
    // Cleanup
    si_cli();
    radio_sleep(); // This is probably redundant, radio *should* sleep on its own. TEST THIS.
    int_radio_state = RADIO_STATE_IDLE;
    return 0;
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

void si_start_rx(int len)
{
    si_send_command(RF4463_CMD_FIFO_INFO, (uint8_t[]){0x01 | 0x02}, 1);
    si_enable_rx_int();
    si_cli();
    int_radio_state = RADIO_STATE_RX;
    si_send_command(RF4463_CMD_START_RX, (uint8_t[]){radio_channel, 0, len >> 8, len & 0xff, 0x10, 0x10, 0x10}, 7); // TODO: Look into what 0x10 should be- appears to be state to enter after RX? Currently sleep.
}

int si_rx_fifo_len()
{
    uint8_t fifo[4];
    if(si_read_command(RF4463_CMD_FIFO_INFO, fifo, (uint8_t[]){0}, 1) <= 0)
        return 0;
    return fifo[1];
}

int si_tx_fifo_space()
{
    uint8_t fifo[4];
    if(si_read_command(RF4463_CMD_FIFO_INFO, fifo, (uint8_t[]){0}, 1) <= 0)
        return 0;
    return fifo[2];
}

void radio_set_rx_callback(radio_rx_cb_t cb)
{
    rx_Callback = cb;
}

radio_state_t radio_get_state()
{
    return int_radio_state;
}

int si_read_rx_fifo(uint8_t* outData)
{
    if(!si_cts())
        return 0;
    
    int read = 0;
    si_read_command(RF4463_CMD_RX_FIFO_READ, outData, NULL, 0); // TODO: Remove 0xff at index 0
    return read;
}

/*
void si_interrupt_handler(unsigned int gpio, uint32_t evt)
{
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    si_send_command(RF4463_CMD_GET_INT_STATUS, (uint8_t[]){0x00, 0x00, 0x00}, 0);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    gpio_acknowledge_irq(gpio, evt);
}
*/
int si_set_property(uint16_t property, uint8_t* data, uint8_t len)
{
    if(!si_cts())
        return 0;

    uint8_t cmdBuf[4];
    cmdBuf[0] = RF4463_CMD_SET_PROPERTY;
    cmdBuf[1] = property >> 8;
    cmdBuf[2] = len;
    cmdBuf[3] = property && 0xFF;

    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_write(CSPI_PORT0, cmdBuf, 4);
    cspi_write(CSPI_PORT0, data, len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);

    return 1;
}

int si_send_command(uint8_t cmd, uint8_t* data, int len)
{
    if(!si_cts())
        return 0;

    // Send the command
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_write(CSPI_PORT0, (uint8_t[]){cmd}, 1);
    cspi_write(CSPI_PORT0, data, len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);

    return 1;
}

int si_read_command(uint8_t cmd, uint8_t* outData, uint8_t* data, uint8_t len)
{
    if(!si_cts())
        return 0;

    // Send the command to read
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_write(CSPI_PORT0, (uint8_t[]){cmd}, 1);
    //if(len > 0 && data != NULL)
    cspi_write(CSPI_PORT0, data, len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    
    if(!si_cts())
        return 0;
    
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_write(CSPI_PORT0, (uint8_t[]){RF4463_CMD_READ_BUF}, 1);
    int read = cspi_read(CSPI_PORT0, outData, 16);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);

    return read;
}

int si_cts()
{
    int timeout = RF4463_CTS_TIMEOUT;
    while(1) {
        gpio_write(RADIO_CS_PIN, GPIO_LOW);
        cspi_byte(CSPI_PORT0, RF4463_CMD_READ_BUF);
        uint8_t read = cspi_byte(CSPI_PORT0, 0xff);
        if(read == 0xff) {
            gpio_write(RADIO_CS_PIN, GPIO_HIGH);
            gpio_write(TX_LED_PIN, GPIO_HIGH);
            return 1;
        }
        gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    }
    return 0;
}

int si_cli()
{
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    si_send_command(RF4463_CMD_GET_INT_STATUS, (uint8_t[]){0x00, 0x00, 0x00}, 0);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
}

void si_enable_tx_int()
{
    si_set_property(RF4463_PROPERTY_INT_CTL_ENABLE, (uint8_t[]){0x01, 0x20, 0x00}, 3);
}

void si_enable_rx_int()
{
    si_set_property(RF4463_PROPERTY_INT_CTL_ENABLE, (uint8_t[]){0x03, 0x18, 0x00}, 3);
}

int si_irq()
{
    return !gpio_read(RADIO_IRQ_PIN); // Inverse for active low pin
}

// TODO: DO NOT TURN ON *ALL* INTERRUPTS!
/*
int si_sti()
{
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    si_set_property(RF4463_PROPERTY_INT_CTL_ENABLE, (uint8_t[]){0xff, 1, 1}, 1); // PACKET_RX interrupt
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    si_set_property(RF4463_PROPERTY_INT_CTL_PH_ENABLE, (uint8_t[]){0xff, 1, 1}, 1); // PACKET_RX interrupt
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
}
*/
int si_check()
{
    uint8_t buf[16];
    uint16_t partInfo;
    if(si_read_command(RF4463_CMD_PART_INFO, buf, NULL, 0) <= 0)
        return 0;
    partInfo = buf[2]<<8 | buf[3];
    if(partInfo != 0x4463)
        return 0;
    return 1;
}

void radio_poweron()
{
    // Reset the radio
    gpio_write(RADIO_SDN_PIN, GPIO_HIGH);
    mcu_sleep(100);
    gpio_write(RADIO_SDN_PIN, GPIO_LOW);
    mcu_sleep(50);
    gpio_write(RADIO_SDN_PIN, GPIO_HIGH);
    cspi_byte(CSPI_PORT0, RF4463_CMD_POWER_UP);
    gpio_write(RADIO_SDN_PIN, GPIO_LOW);
    mcu_sleep(250);
}

void radio_sleep()
{
    si_send_command(RF4463_CMD_CHANGE_STATE, (uint8_t[]){0x01}, 1);
}

void radio_set_channel(int channel)
{
    radio_channel = channel;
}

#endif
