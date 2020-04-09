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
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */
#include <peripheral_clk_config.h>
#include <hal_spi_m_sync.h>
#include <hal_gpio.h>
#include "spi-board.h"

struct spi_m_sync_descriptor Spi0;

void SpiInit( Spi_t *obj, SpiId_t spiId, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
    hri_gclk_write_PCHCTRL_reg( GCLK, SERCOM4_GCLK_ID_CORE, CONF_GCLK_SERCOM4_CORE_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );
    hri_gclk_write_PCHCTRL_reg( GCLK, SERCOM4_GCLK_ID_SLOW, CONF_GCLK_SERCOM4_SLOW_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );

    hri_mclk_set_APBCMASK_SERCOM4_bit( MCLK );

    spi_m_sync_init( &Spi0, SERCOM4 );

    hri_sercomspi_wait_for_sync( SERCOM4, SERCOM_SPI_SYNCBUSY_SWRST );
    hri_sercomspi_set_CTRLA_SWRST_bit( SERCOM4 );
    hri_sercomspi_wait_for_sync( SERCOM4, SERCOM_SPI_SYNCBUSY_SWRST );
    // 0x0001000C DOPO=1 MODE=3
    hri_sercomspi_write_CTRLA_reg( SERCOM4, SERCOM_SPI_CTRLA_MODE( 3 ) | SERCOM_SPI_CTRLA_DOPO( 1 ) );
    // 0x00020000 RXEN
    hri_sercomspi_write_CTRLB_reg( SERCOM4, SERCOM_SPI_CTRLB_RXEN );
    hri_sercomspi_write_BAUD_reg( SERCOM4, ( ( float )CONF_GCLK_SERCOM4_CORE_FREQUENCY / ( float )( 2 * 1000000 ) ) - 1 );
    hri_sercomspi_write_DBGCTRL_reg( SERCOM4, 0 );

    // Set pin direction to input. MISO
    gpio_set_pin_direction( miso, GPIO_DIRECTION_IN );
    gpio_set_pin_pull_mode( miso, GPIO_PULL_OFF );
    gpio_set_pin_function( miso, PINMUX_PC19F_SERCOM4_PAD0 );

    // Set pin direction to output. MOSI
    gpio_set_pin_direction( mosi, GPIO_DIRECTION_OUT );
    gpio_set_pin_level( mosi, false );
    gpio_set_pin_function( mosi, PINMUX_PB30F_SERCOM4_PAD2 );

    // Set pin direction to output. CLK
    gpio_set_pin_direction( sclk, GPIO_DIRECTION_OUT );
    gpio_set_pin_level( sclk,  false );
    gpio_set_pin_function( sclk, PINMUX_PC18F_SERCOM4_PAD3 );

    hri_sercomspi_set_CTRLA_ENABLE_bit( SERCOM4 );
}

void SpiDeInit( Spi_t *obj )
{
    GpioInit( &obj->Mosi, obj->Mosi.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Miso, obj->Miso.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, obj->Sclk.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Nss, obj->Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    // Wait for bus idle (ready to write)
    while( ( SERCOM_SPI_INTFLAG_DRE & hri_sercomspi_read_INTFLAG_reg( SERCOM4 ) ) == 0 )
    {

    }
    hri_sercomspi_clear_INTFLAG_reg( SERCOM4, SERCOM_SPI_INTFLAG_DRE );

    // Write byte
    hri_sercomspi_write_DATA_reg( SERCOM4, outData );

    // Wait for ready to read
    while( ( SERCOM_SPI_INTFLAG_RXC & hri_sercomspi_read_INTFLAG_reg( SERCOM4 ) ) == 0 )
    {

    }
    hri_sercomspi_clear_INTFLAG_reg( SERCOM4, SERCOM_SPI_INTFLAG_RXC );

    // Read byte
    outData = ( uint16_t )hri_sercomspi_read_DATA_reg( SERCOM4 );

    return outData;
}
