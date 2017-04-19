/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic low level driver for GPS receiver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

/*!
 * FIFO buffers size
 */
//#define FIFO_TX_SIZE                                128
#define FIFO_RX_SIZE                                128

//uint8_t TxBuffer[FIFO_TX_SIZE];
uint8_t RxBuffer[FIFO_RX_SIZE];

/*!
 * \brief Buffer holding the  raw data received from the gps
 */
uint8_t NmeaString[128];

/*!
 * \brief Maximum number of data byte that we will accept from the GPS
 */
uint8_t NmeaStringSize = 0;

Gpio_t GpsPowerEn;

bool GpsPowerEnInverted = false;

void GpsMcuOnPpsSignal( void )
{
    bool parseData = false;

    GpsPpsHandler( &parseData );

    if( parseData == true )
    {
        UartInit( &Uart1, UART_1, GPS_UART_TX, GPS_UART_RX );
        UartConfig( &Uart1, RX_ONLY, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    }
}

void GpsMcuInvertPpsTrigger( void )
{

}

void GpsMcuInit( void )
{
    Gpio_t ioPin;

    NmeaStringSize = 0;

    switch( BoardGetVersion( ) )
    {
        case BOARD_VERSION_2:
            GpsPowerEnInverted = true;
            break;
        case BOARD_VERSION_3:
            GpsPowerEnInverted = false;
            break;
        default:
            break;
    }
    GpioInit( &GpsPowerEn, GPS_POWER_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    GpioInit( &ioPin, GPS_PPS, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &ioPin, IRQ_FALLING_EDGE, IRQ_VERY_LOW_PRIORITY, &GpsMcuOnPpsSignal );

    FifoInit( &Uart1.FifoRx, RxBuffer, FIFO_RX_SIZE );
    Uart1.IrqNotify = GpsMcuIrqNotify;

    GpsMcuStart( );
}

void GpsMcuStart( void )
{
    if( GpsPowerEnInverted == true )
    {
        GpioWrite( &GpsPowerEn, 0 );    // power up the GPS
    }
    else
    {
        GpioWrite( &GpsPowerEn, 1 );    // power up the GPS
    }
}

void GpsMcuStop( void )
{
    if( GpsPowerEnInverted == true )
    {
        GpioWrite( &GpsPowerEn, 1 );    // power down the GPS
    }
    else
    {
        GpioWrite( &GpsPowerEn, 0 );    // power down the GPS
    }
}

void GpsMcuProcess( void )
{

}

void GpsMcuIrqNotify( UartNotifyId_t id )
{
    uint8_t data;
    if( id == UART_NOTIFY_RX )
    {
        if( UartGetChar( &Uart1, &data ) == 0 )
        {
            if( ( data == '$' ) || ( NmeaStringSize >= 127 ) )
            {
                NmeaStringSize = 0;
            }

            NmeaString[NmeaStringSize++] = ( int8_t )data;

            if( data == '\n' )
            {
                NmeaString[NmeaStringSize++] = '\0';
                GpsParseGpsData( ( int8_t* )NmeaString, NmeaStringSize );
                UartDeInit( &Uart1 );
                BlockLowPowerDuringTask ( false );
            }
        }
    }
}
