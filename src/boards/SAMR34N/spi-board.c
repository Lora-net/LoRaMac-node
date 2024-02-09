/*!
 * \file      spi-board.c
 *
 * \brief     Target board SPI driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 */
#include "asf.h"

#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "spi-board.h"

static struct spi_module master;
struct spi_slave_inst    slave;

void SpiInit( Spi_t* obj, SpiId_t spiId, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
    struct spi_config            config_spi_master;
    struct spi_slave_inst_config slave_dev_config;

    obj->SpiId = spiId;

    spi_slave_inst_get_config_defaults( &slave_dev_config );

    slave_dev_config.ss_pin = nss;
    spi_attach_slave( &slave, &slave_dev_config );

    spi_get_config_defaults( &config_spi_master );

    config_spi_master.mode_specific.master.baudrate = CONF_SPI_BAUDRATE;
    config_spi_master.mux_setting                   = SX_RF_SPI_SERCOM_MUX_SETTING;
    config_spi_master.pinmux_pad0                   = SX_RF_SPI_SERCOM_PINMUX_PAD0;
    config_spi_master.pinmux_pad1                   = PINMUX_UNUSED;
    config_spi_master.pinmux_pad2                   = SX_RF_SPI_SERCOM_PINMUX_PAD2;
    config_spi_master.pinmux_pad3                   = SX_RF_SPI_SERCOM_PINMUX_PAD3;

    spi_init( &master, SX_RF_SPI, &config_spi_master );
    spi_enable( &master );
}

void SpiDeInit( Spi_t* obj )
{
    spi_disable( &master );
}

uint16_t SpiInOut( Spi_t* obj, uint16_t outData )
{
    uint16_t in_data = 0;

    spi_select_slave( &master, &slave, true );

    /* Write the byte in the transceiver data register */
    while( !spi_is_ready_to_write( &master ) )
        ;

    spi_write( &master, outData );

    while( !spi_is_write_complete( &master ) )
        ;
    while( !spi_is_ready_to_read( &master ) )
        ;

    spi_read( &master, &in_data );

    spi_select_slave( &master, &slave, false );

    return in_data;
}
