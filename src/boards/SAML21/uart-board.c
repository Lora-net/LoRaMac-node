/*!
 * \file      uart-board.c
 *
 * \brief     Target board UART driver implementation
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
#include <hal_gpio.h>
#include <hal_usart_sync.h>

#include "board.h"
#include "uart-board.h"

struct usart_sync_descriptor Usart0;

void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

    // Clock initialization
    hri_gclk_write_PCHCTRL_reg( GCLK, SERCOM3_GCLK_ID_CORE, CONF_GCLK_SERCOM3_CORE_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );
    hri_gclk_write_PCHCTRL_reg( GCLK, SERCOM3_GCLK_ID_SLOW, CONF_GCLK_SERCOM3_SLOW_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );
    hri_mclk_set_APBCMASK_SERCOM3_bit( MCLK );

    // USART initialization
    usart_sync_init( &Usart0, SERCOM3, ( void * )NULL );

    // UASRT GPIO initialization
    gpio_set_pin_function( tx, PINMUX_PA22C_SERCOM3_PAD0 );
    gpio_set_pin_function( rx, PINMUX_PA23C_SERCOM3_PAD1 );

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
