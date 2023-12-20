#include "config.h"
#if defined(USE_RP2040) && defined(USE_SPI)
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "spi.h"

void cspi_init(int baudRate, int port, int bitOrder)
{
    spi_init(port == 0 ? spi0 : spi1, baudRate);
    spi_set_format(port == 0 ? spi0 : spi1, 8, SPI_CPOL_0, SPI_CPHA_0, bitOrder);
    spi_set_slave(port == 0 ? spi0 : spi1, false);
}

void cspi_set_pins(int tx, int rx, int sck, int cs)
{
    gpio_set_function(tx, GPIO_FUNC_SPI);
    gpio_set_function(rx, GPIO_FUNC_SPI);
    gpio_set_function(sck, GPIO_FUNC_SPI);
    //gpio_set_function(cs, GPIO_FUNC_SPI);
}

void cspi_write(int port, uint8_t *data, int len)
{
    spi_write_blocking(port == 0 ? spi0 : spi1, data, len);
}

int cspi_read(int port, uint8_t *outData, int len)
{
    return spi_read_blocking(port == 0 ? spi0 : spi1, 0, outData, len);
}

int cspi_available(int port)
{
    return spi_is_readable(port == 0 ? spi0 : spi1);
}

int cspi_transfer(int port, uint8_t *data, int len)
{
    return spi_write_read_blocking(port == 0 ? spi0 : spi1, data, data, len);
}

uint8_t cspi_byte(int port, uint8_t data)
{
    uint8_t read;
    spi_write_read_blocking(port == 0 ? spi0 : spi1, &data, &read, 1);
    return read;
}

#endif