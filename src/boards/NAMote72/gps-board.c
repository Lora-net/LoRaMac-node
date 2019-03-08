/*!
 * \file      gps-board.c
 *
 * \brief     Target board GPS driver implementation
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
 */
#include "board-config.h"
#include "board.h"
#include "gpio.h"
#include "gps.h"
#include "uart.h"
#include "lpm-board.h"
#include "rtc-board.h"
#include "gps-board.h"

/*!
 * FIFO buffers size
 */
//#define FIFO_TX_SIZE                                128
#define FIFO_RX_SIZE                                128

//uint8_t TxBuffer[FIFO_TX_SIZE];
static uint8_t RxBuffer[FIFO_RX_SIZE];

/*!
 * \brief Buffer holding the  raw data received from the gps
 */
static uint8_t NmeaString[128];

/*!
 * \brief Maximum number of data byte that we will accept from the GPS
 */
static volatile uint8_t NmeaStringSize = 0;

static Gpio_t GpsPowerEn;
static Gpio_t GpsPps;

static volatile bool GpsPowerEnInverted = false;

extern Uart_t Uart1;

void GpsMcuOnPpsSignal( void* context )
{
    bool parseData = false;

    GpsPpsHandler( &parseData );

    if( parseData == true )
    {
        // Disables lowest power modes
        LpmSetStopMode( LPM_GPS_ID , LPM_DISABLE );

        UartInit( &Uart1, UART_1, GPS_UART_TX, GPS_UART_RX );
        UartConfig( &Uart1, RX_ONLY, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    }
}

void GpsMcuInvertPpsTrigger( void )
{

}

void GpsMcuInit( void )
{
    NmeaStringSize = 0;

    switch( BoardGetVersion( ).Fields.Major )
    {
        case 2:
            GpsPowerEnInverted = true;
            break;
        case 3:
            GpsPowerEnInverted = false;
            break;
        default:
            break;
    }
    GpioInit( &GpsPowerEn, GPS_POWER_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    GpioInit( &GpsPps, GPS_PPS, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &GpsPps, IRQ_FALLING_EDGE, IRQ_VERY_LOW_PRIORITY, &GpsMcuOnPpsSignal );

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
                // Enables lowest power modes
                LpmSetStopMode( LPM_GPS_ID , LPM_ENABLE );
            }
        }
    }
}
