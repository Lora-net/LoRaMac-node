/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Uart basic implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "uart-board.h"

#include "uart.h"

/*!
 * Flag to indicates if the UART is initialized 
 */
static bool UartInitialized = false;

/*!
 * Number of times the UartPutBuffer will try to send the buffer before
 * returning ERROR
 */
#define TX_BUFFER_RETRY_COUNT                       10

void UartInit( Uart_t *obj, PinNames tx, PinNames rx )
{
    if( UartInitialized == false )
    {
        UartInitialized = true;

        UartMcuInit( obj, tx, rx );
        UartMcuFormat( obj, RX_ONLY, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    }
}

void UartDeInit( Uart_t *obj )
{
    UartInitialized = false;
    UartMcuDeInit( obj );
}

uint8_t UartPutChar( Uart_t *obj, uint8_t data )
{
    return UartMcuPutChar( obj, data );
}

uint8_t UartGetChar( Uart_t *obj, uint8_t *data )
{
    return UartMcuGetChar( obj, data );
}

uint8_t UartPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size )
{
    uint8_t retryCount;
    uint16_t i;

    for( i = 0; i < size; i++ )
    {
        retryCount = 0;
        while( UartPutChar( obj, buffer[i] ) != 0 )
        {
            retryCount++;

            // Exit if something goes terribly wrong
            if( retryCount > TX_BUFFER_RETRY_COUNT )
            {
                return 1; // Error
            }
        }
    }
    return 0; // OK
}

uint8_t UartGetBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size, uint16_t *nbReadBytes )
{
    uint16_t localSize = 0;

    while( localSize < size )
    {
        if( UartGetChar( obj, buffer + localSize ) == 0 )
        {
            localSize++;
        }
        else
        {
            break;
        }
    }
    
    *nbReadBytes = localSize;
    
    if( localSize == 0 )
    {
        return 1; // Empty
    }
    return 0; // OK
}

