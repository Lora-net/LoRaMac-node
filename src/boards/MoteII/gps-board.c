/*
  ______                              _
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

#define NMEA_MAX_BUFFER_SIZE                        256
/*!
 * \brief Buffer holding the  raw data received from the gps
 */
static uint8_t NmeaString[NMEA_MAX_BUFFER_SIZE];

/*!
 * \brief Maximum number of data byte that we will accept from the GPS
 */
static uint8_t NmeaStringSize = 0;

Gpio_t GpsPowerEn;

void GpsMcuOnPpsSignal( void )
{
    bool parseData = false;

    GpsPpsHandler( &parseData );

    if( parseData == true )
    {
        BlockLowPowerDuringTask ( false );
    }
}

void GpsMcuInvertPpsTrigger( void )
{
    // The PPS trigger polarity inversion is not required.
}

void GpsMcuInit( void )
{
    Gpio_t gpsPps;
    NmeaStringSize = 0;

    GpioInit( &GpsPowerEn, GPS_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &gpsPps, GPS_PPS, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &gpsPps, IRQ_FALLING_EDGE, IRQ_VERY_LOW_PRIORITY, &GpsMcuOnPpsSignal );

    PAM7QInit( );
    GpsMcuStart( );
}

void GpsMcuStart( void )
{
    GpioWrite( &GpsPowerEn, 0 );    // power up the GPS
}

void GpsMcuStop( void )
{
    GpioWrite( &GpsPowerEn, 1 );    // power down the GPS
}

void GpsMcuProcess( void )
{
    if( GpsGetPpsDetectedState( ) == true )
    {
        if( PAM7QGetGpsData( NmeaString, &NmeaStringSize, NMEA_MAX_BUFFER_SIZE ) == true )
        {
            uint16_t i;
            uint16_t size = 0;
            for( i = 0; i < NmeaStringSize; )
            {
                size = 0;
                if( NmeaString[i] == '$' )
                {
                    for( size = 0; size < ( NmeaStringSize - i ); size++ )
                    {
                        if( NmeaString[i + size] == '\n' )
                        {
                            break;
                        }
                    }
                    GpsParseGpsData( ( int8_t* )NmeaString + i, size );
                }
                i += size;
                if( size == 0 )
                {
                    i++;
                }
            }
        }
    }
}
