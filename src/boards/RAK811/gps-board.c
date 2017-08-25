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
PpsTrigger_t PpsTrigger;

void GpsMcuOnPpsSignal( void )
{
#ifdef GPS_POWER_ON
    bool parseData = false;

    GpsPpsHandler( &parseData );

    if( parseData == true )
    {
        UartInit( &GpsUart, GPS_UART, GPS_UART_TX, GPS_UART_RX );
        UartConfig( &GpsUart, RX_ONLY, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    }
#endif
}

void GpsMcuInvertPpsTrigger( void )
{
#ifdef GPS_POWER_ON
    if( PpsTrigger == PpsTriggerIsRising )
    {
        PpsTrigger = PpsTriggerIsFalling;
        GpioSetInterrupt( &GpsPps, IRQ_FALLING_EDGE, IRQ_VERY_LOW_PRIORITY, &GpsMcuOnPpsSignal );
    }
    else
    {
        PpsTrigger = PpsTriggerIsRising;
        GpioSetInterrupt( &GpsPps, IRQ_RISING_EDGE, IRQ_VERY_LOW_PRIORITY, &GpsMcuOnPpsSignal );
    }
#endif
}

void GpsMcuInit( void )
{
    NmeaStringSize = 0;
    PpsTrigger = PpsTriggerIsFalling;

#ifdef GPS_POWER_ON
    GpioInit( &GpsPowerEn, GPS_POWER_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

#ifdef GPS_PPS
    GpioInit( &GpsPps, GPS_PPS, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &GpsPps, IRQ_FALLING_EDGE, IRQ_VERY_LOW_PRIORITY, &GpsMcuOnPpsSignal );
#endif

#ifdef GPS_UART
    FifoInit( &GpsUart.FifoRx, RxBuffer, FIFO_RX_SIZE );
    GpsUart.IrqNotify = GpsMcuIrqNotify;
#endif

    GpsMcuStart( );
}

void GpsMcuStart( void )
{
#ifdef GPS_POWER_ON
    GpioWrite( &GpsPowerEn, 1 );    // power up the GPS
#endif
}

void GpsMcuStop( void )
{
#ifdef GPS_POWER_ON
    GpioWrite( &GpsPowerEn, 0 );    // power down the GPS
#endif
}

void GpsMcuProcess( void )
{

}

void GpsMcuIrqNotify( UartNotifyId_t id )
{
#ifdef GPS_POWER_ON
    uint8_t data;
    if( id == UART_NOTIFY_RX )
    {
        if( UartGetChar( &GpsUart, &data ) == 0 )
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
                UartDeInit( &GpsUart );
                BlockLowPowerDuringTask ( false );
            }
        }
    }
#endif
}
