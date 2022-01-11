/*!
 * \file  uart-board.c
 *
 * \brief Target board UART driver implementation
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
#include <hal_gpio.h>
#include <hal_usart_sync.h>

#include "board.h"
#include "uart-board.h"

struct usart_sync_descriptor Usart0;

void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

    // Clock initialization
    hri_gclk_write_PCHCTRL_reg( GCLK, SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );
    hri_gclk_write_PCHCTRL_reg( GCLK, SERCOM0_GCLK_ID_SLOW, CONF_GCLK_SERCOM0_SLOW_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );
    hri_mclk_set_APBCMASK_SERCOM0_bit( MCLK );

    // USART initialization
    usart_sync_init( &Usart0, SERCOM0, ( void * )NULL );

    // UASRT GPIO initialization
    gpio_set_pin_function( tx, PINMUX_PA04D_SERCOM0_PAD0 );
    gpio_set_pin_function( rx, PINMUX_PA05D_SERCOM0_PAD1 );

    usart_sync_enable( &Usart0 );
}

void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    usart_sync_set_baud_rate( &Usart0, baudrate );
}

void UartMcuDeInit( Uart_t *obj )
{

}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
    if( io_write( &Usart0.io, &data, 1 ) == 0 )
    {
        return 1; // Busy
    }
    return 0; // OK
}

uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data )
{
    if( io_read( &Usart0.io, data, 1 ) == 1 )
    {
        return 0; // OK
    }
    else
    {
        return 1; // Busy
    }
}

uint8_t UartMcuPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size )
{
    if( io_write( &Usart0.io, buffer, size ) == 0 )
    {
        return 1; //Error
    }
    return 0; // OK
}

uint8_t UartMcuGetBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size, uint16_t *nbReadBytes )
{
    *nbReadBytes = io_read( &Usart0.io, buffer, size );
    if( *nbReadBytes == 0 )
    {
        return 1; // Empty
    }
    return 0; // OK
}
