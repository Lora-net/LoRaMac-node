/*!
 * \file      spi-board.c
 *
 * \brief     Target board SPI driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Alistair Francis <alistair@alistair23.me>
 */
#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "spi-board.h"
#include "libtock/lora_phy.h"

void SpiInit( Spi_t *obj, SpiId_t spiId, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
}

void SpiDeInit( Spi_t *obj )
{
}

void SpiFormat( Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave )
{
    lora_phy_set_polarity(cpol);
    lora_phy_set_phase(cpha);
}

void SpiFrequency( Spi_t *obj, uint32_t hz )
{
    lora_phy_set_rate(hz);
}

#define BUF_SIZE 1

char rbuf[BUF_SIZE];
char wbuf[BUF_SIZE];

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    wbuf[0] = outData & 0xFF;

    lora_phy_read_write_sync(wbuf, rbuf, BUF_SIZE);

    return rbuf[0];
}
