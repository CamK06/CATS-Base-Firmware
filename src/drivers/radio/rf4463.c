#include "config.h"
#include "rf4463.h"

#ifdef USE_RF4463
#ifndef USE_SPI
#error "RF4463 requires SPI"
#endif

#include "drivers/serial.h"
#include "drivers/radio.h"
#include "drivers/gpio.h"
#include "drivers/spi.h"
#include "drivers/mcu.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define VCXO_FREQ 30000000

static radio_state_t int_radio_state = RADIO_STATE_IDLE;
static int rx_idx = 0;

int radio_start()
{
    // Initialize GPIO + SPI
    gpio_setup(RADIO_SDN_PIN);
    gpio_setup(RADIO_IRQ_PIN);
    gpio_setup(RADIO_CS_PIN);
    gpio_set_mode(RADIO_SDN_PIN, GPIO_OUTPUT);
    gpio_set_mode(RADIO_CS_PIN, GPIO_OUTPUT);
    gpio_set_mode(RADIO_IRQ_PIN, GPIO_INPUT);
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_init(10000000, CSPI_PORT0, CSPI_MSB_FIRST);
    cspi_set_pins(RADIO_TX_PIN, RADIO_RX_PIN, RADIO_SCK_PIN, RADIO_CS_PIN);

    // Initialize the RF4463
    si_poweron();

    // Set the radio parameters
    si_load_config(cats_config, sizeof(cats_config));
    si_send_command((uint8_t[]){RF4463_CMD_SET_PROPERTY, 0x01, 2, 0x00, 0b00000001, 0b00110011}, 6);
    radio_sleep();
    return 1;
}

int radio_tx(uint8_t* data, int len)
{
    // Setup; bring the radio back to an idle state
    if(int_radio_state == RADIO_STATE_RX) {
        radio_sleep();
    }
    si_send_command((uint8_t[]){RF4463_CMD_FIFO_INFO, 0x01 | 0x02}, 2);
    si_send_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0, 0, 0xFF}, 4);
    si_cli();

    // Start the transmission
    int fifo_space = 128;
    int chunk_len = (len < fifo_space) ? len : fifo_space;
    uint8_t tx_buf[128];
    memcpy(tx_buf, data, chunk_len);

    // Write the first chunk into the TX FIFO
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_byte(CSPI_PORT0, RF4463_CMD_TX_FIFO_WRITE);
    cspi_write(CSPI_PORT0, tx_buf, chunk_len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    
    // Start transmitting
    gpio_write(TX_LED_PIN, GPIO_HIGH);
    si_send_command((uint8_t[]){RF4463_CMD_START_TX, 20, 0b00010000, (uint8_t)((uint16_t)len >> 8), (uint8_t)len, 0, 0}, 6);
    int_radio_state = RADIO_STATE_TX;

    // Step through the buffer until TX is finished
    int tx = chunk_len;
    while(int_radio_state == RADIO_STATE_TX) {
        fifo_space = si_tx_fifo_space();
        if(fifo_space <= 0) {
            continue;
        }

        chunk_len = ((len - tx) < fifo_space) ? (len - tx) : fifo_space;
        if(chunk_len == 0) {
            break;
        }
        memcpy(tx_buf, data + tx, chunk_len);

        if(si_fifo_underflow_pending()) {
            break;
        }

        // Send the next chunk
        gpio_write(RADIO_CS_PIN, GPIO_LOW);
        cspi_byte(CSPI_PORT0, RF4463_CMD_TX_FIFO_WRITE);
        cspi_write(CSPI_PORT0, tx_buf, chunk_len);
        gpio_write(RADIO_CS_PIN, GPIO_HIGH);
        
        tx += chunk_len;
        if(si_packet_sent_pending()) {
            break;
        }
    }
    
    // Cleanup
    while(!si_packet_sent_pending()) {
        mcu_sleep(1);
    }
    int_radio_state = RADIO_STATE_IDLE;
    gpio_write(TX_LED_PIN, GPIO_LOW);
    return 0;
}

int si_rx_step(uint8_t* buf)
{
    const int fifo_len = si_rx_fifo_len();
    if(fifo_len == 0) {
        return 0;
    }
    if(si_fifo_underflow_pending()) {
        return 0;
    }

    uint8_t data[fifo_len];
    memset(data, 0x00, fifo_len);

    // Read from the RX fifo into data
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_byte(CSPI_PORT0, RF4463_CMD_RX_FIFO_READ);
    const int read = cspi_transfer(CSPI_PORT0, data, fifo_len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);

    // Copy the received chunk to the output buffer
    memcpy(buf + rx_idx, data, fifo_len);
    rx_idx += fifo_len;
    if(int_radio_state == RADIO_STATE_IDLE) {
        rx_idx = 0;
    }
    return fifo_len;
}

int radio_rx_step(uint8_t* buf)
{
    if(si_fifo_underflow_pending()) {
        return 0;
    }

    if(int_radio_state == RADIO_STATE_RX) {
        if(si_packet_rx_pending()) {
            int_radio_state = RADIO_STATE_IDLE;
        }
        
        return si_rx_step(buf);
    }
}

int radio_start_rx()
{
    int_radio_state = RADIO_STATE_RX;
    const int len = 0; // Keeping this as it may be used in the future
    si_send_command((uint8_t[]){RF4463_CMD_FIFO_INFO, 0x01 | 0x02}, 2);
    si_send_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0, 0, 0xFF}, 4);
    si_cli();
    si_send_command((uint8_t[]){RF4463_CMD_START_RX, 0, 0, (len >> 8), len, 0b00001000, 0b00000001, 0b00001000}, 8);
    rx_idx = 0;
    return 0;
}

void radio_sleep()
{
    int_radio_state = RADIO_STATE_IDLE;
    si_send_command((uint8_t[]){RF4463_CMD_CHANGE_STATE, 0x01}, 2);
}

radio_state_t radio_get_state()
{
    //rf4463_state_t state = si_get_state();
    return int_radio_state;
}

void radio_set_frequency(uint32_t frequency)
{
    int outdiv;
    int band;

    if(frequency < 177000000) {
        outdiv = 24;
        band = 5;
    }
    else if(frequency < 239000000) {
        outdiv = 16;
        band = 4;
    }
    else if(frequency < 353000000) {
        outdiv = 12;
        band = 3;
    }
    else if(frequency < 525000000) {
        outdiv = 8;
        band = 2;
    }
    else if(frequency < 705000000) {
        outdiv = 6;
        band = 1;
    }
    else {
        outdiv = 4;
        band = 0;
    }

    const float f_pfd = 2 * VCXO_FREQ / outdiv;
    const uint8_t n = (frequency / f_pfd - 1);
    const float ratio = (float)frequency / (float)f_pfd;
    const float rest = ratio - (float)n;
    const uint32_t m = (uint32_t)(rest * 524288.0f);

    si_send_command((uint8_t[]){RF4463_CMD_SET_PROPERTY, 0x20, 0x01, 0x51, 8 + band}, 5);
    si_send_command((uint8_t[]){RF4463_CMD_SET_PROPERTY, 0x40, 0x04, 0x00, n, ((m >> 16) & 0xFF),
        ((m >> 8) & 0xFF), (m & 0xFF)}, 8);
}

float radio_get_temp()
{
    uint16_t result = si_get_adc(RF4463_ADC_CONV_TEMP, RF4463_ADC_SPEED << 4, 4);
    return 899.0f * result / 4096 - 293.0f;
}

float radio_get_voltage()
{
    uint16_t result = si_get_adc(RF4463_ADC_CONV_VOLTS, RF4463_ADC_SPEED << 4, 2);
    return 3 * result / 1280;
}

float radio_get_rssi()
{
    uint8_t frr = si_read_frr(0);
    return (float)frr / 2.0f - 136.0f;
}

// Internal functions

void si_load_config(const uint8_t* config, int len)
{
    // Set the radio parameters
    int cmd_len;
    uint8_t cmd;
    uint8_t buf[32];
    for(int i = 0; i < len; i += cmd_len + 1) {
        cmd_len = config[i];
        if(i == 0) {
            continue;
        }
        memcpy(buf, config + i + 1, cmd_len);
        si_send_command(buf, cmd_len);
    }
    si_cli();
}

int si_set_property(uint16_t property, uint8_t* data, uint8_t len)
{
    uint8_t buf[len + 4];
    memcpy(buf + 4, data, len);
    buf[0] = RF4463_CMD_SET_PROPERTY;
    buf[1] = property >> 8;
    buf[2] = len;
    buf[3] = property && 0xFF;

    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_write(CSPI_PORT0, buf, len + 4);
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

    while(1) {
        gpio_write(RADIO_CS_PIN, GPIO_LOW);
        cspi_byte(CSPI_PORT0, RF4463_CMD_READ_BUF);
        if(cspi_byte(CSPI_PORT0, 0xFF) != 0xFF) {
            gpio_write(RADIO_CS_PIN, GPIO_HIGH);
            continue;
        }
        
        gpio_write(RADIO_CS_PIN, GPIO_HIGH);
        break;
    }
    return 1;
}

int si_read_command(uint8_t* cmd, uint8_t len, uint8_t* out_data, uint8_t out_len)
{
    memset(out_data, 0xff, out_len);
    uint8_t buf[len];
    memcpy(buf, cmd, len);

    // Send the command to read
    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_transfer(CSPI_PORT0, buf, len);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    
    int read = 0;
    while(1) {
        gpio_write(RADIO_CS_PIN, GPIO_LOW);
        cspi_byte(CSPI_PORT0, RF4463_CMD_READ_BUF);
        if(cspi_byte(CSPI_PORT0, 0xFF) != 0xFF) {
            gpio_write(RADIO_CS_PIN, GPIO_HIGH);
            continue;
        }
        read = cspi_transfer(CSPI_PORT0, out_data, out_len);
        gpio_write(RADIO_CS_PIN, GPIO_HIGH);
        break;
    }

    return read;
}

int si_get_state()
{
    uint8_t resp[2];
    if(si_read_command((uint8_t[]){RF4463_CMD_REQUEST_DEVICE_STATE}, 1, resp, 2) <= 0) {
        return 0;
    }
    return resp[0];
}

int si_rx_fifo_len()
{
    uint8_t fifo[2];
    if(si_read_command((uint8_t[]){RF4463_CMD_FIFO_INFO, 0x00}, 2, fifo, 2) <= 0) {
        return 0;
    }
    return fifo[0];
}

int si_tx_fifo_space()
{
    uint8_t fifo[2];
    if(si_read_command((uint8_t[]){RF4463_CMD_FIFO_INFO, 0x00}, 2, fifo, 2) <= 0) {
        return 0;
    }
    return fifo[1];
}

int si_packet_sent_pending()
{
    if(!si_irq()) {
        return 0;
    }
    uint8_t outData[4];
    si_read_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0xFF ^ (1 << 5), 0xFF, 0xFF}, 4, outData, 3);
    return (outData[2] & (1 << 5)) != 0;
}

int si_packet_rx_pending()
{
    if(!si_irq()) {
        return 0;
    }
    uint8_t outData[4];
    si_read_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0xFF ^ (1 << 4), 0xFF, 0xFF}, 4, outData, 3);
    return (outData[2] & (1 << 4)) != 0;
}

int si_fifo_underflow_pending()
{
    if(!si_irq()) {
        return 0;
    }
    uint8_t outData[8];
    si_read_command((uint8_t[]){RF4463_CMD_GET_INT_STATUS, 0xFF, 0xFF, 0xFF ^ (1 << 5)}, 4, outData, 7);
    return (outData[6] & (1 << 5)) != 0;
}

uint16_t si_get_adc(uint8_t en, uint8_t cfg, size_t part) 
{
    uint8_t out[8];
    si_read_command((uint8_t[]){RF4463_CMD_GET_ADC_READING, en, cfg}, 3, out, 8);
    return ((uint16_t)out[part] << 8) | (uint16_t)out[part + 1];
}

uint8_t si_read_frr(size_t part) 
{
    uint8_t out[4];
    memset(out, 0xff, 4);

    gpio_write(RADIO_CS_PIN, GPIO_LOW);
    cspi_byte(CSPI_PORT0, RF4463_CMD_FAST_RESPONSE_A);
    cspi_transfer(CSPI_PORT0, out, 4);
    gpio_write(RADIO_CS_PIN, GPIO_HIGH);

    return out[part];
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
    const int timeout = RF4463_CTS_TIMEOUT;
    while(1) {
        gpio_write(RADIO_CS_PIN, GPIO_LOW);
        cspi_byte(CSPI_PORT0, RF4463_CMD_READ_BUF);
        uint8_t read = cspi_byte(CSPI_PORT0, 0x00);
        if(read == 0xff) {
            gpio_write(RADIO_CS_PIN, GPIO_HIGH);
            return 1;
        }
        gpio_write(RADIO_CS_PIN, GPIO_HIGH);
    }
    return 1;
}

int si_check()
{
    uint8_t buf[9];
    uint16_t part_info;
    if(si_read_command((uint8_t[]){RF4463_CMD_PART_INFO}, 1, buf, 9) <= 0) {
        return 0;
    }
    part_info = buf[2]<<8 | buf[3];
    if(part_info != 0x4463) {
        return 0;
    }

    return 1;
}

void si_poweron()
{
    uint8_t buf[7];
    memcpy(buf, radio_config + 1, 7);

    // Reset the radio
    gpio_write(RADIO_SDN_PIN, GPIO_HIGH);
    mcu_sleep(100);
    gpio_write(RADIO_SDN_PIN, GPIO_LOW);
    mcu_sleep(5);

    si_send_command(buf, 7);
    mcu_sleep(250);
}

#endif
