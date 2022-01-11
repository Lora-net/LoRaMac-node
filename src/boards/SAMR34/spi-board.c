/*!
 * \file  spi-board.c
 *
 * \brief Target board SPI driver implementation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright TWTG on behalf of Microchip/Atmel (c)2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
