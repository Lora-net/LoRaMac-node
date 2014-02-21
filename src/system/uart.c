/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Uart basic implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "uart-board.h"

/*!
 * Flag to indicates if the UART is initialized 
 */
static bool UartInitialized = false;

void UartInit( Uart_t *obj, PinNames tx, PinNames rx )
{
    if( UartInitialized == false )
    {
        UartInitialized = true;

        UartMcuInit( obj, tx, rx );
        UartMcuFormat( obj, RX_ONLY, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
        UartMcuIrqInit( obj );
    }
}

void UartDeInit( Uart_t *obj )
{
    UartInitialized = false;
    UartMcuDeInit( obj );
}

