/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Generic GPIO driver implementation

Comment: Relies on the specific board GPIO implementation as well as on
         IO expander driver implementation if one is available on the target
         board.

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "up501.h"


#define TRIGGER_GPS_CNT                         10

/* Various type of NMEA data we can receive with the UP501 */
const char NmeaDataTypeGPGGA[] = "GPGGA";
const char NmeaDataTypeGPGSA[] = "GPGSA";
const char NmeaDataTypeGPGSV[] = "GPGSV";
const char NmeaDataTypeGPRMC[] = "GPRMC";

/* Value used for the convertion of the postion from DMS to decimal */
const int32_t MaxNorthPosition = 8388607;       // 2^23 - 1
const int32_t MaxSouthPosition = 8388608;       // -2^23
const int32_t MaxEastPosition = 8388607;        // 2^23 - 1    
const int32_t MaxWestPosition = 8388608;        // -2^23

tNmeaGpsData NmeaGpsData;

static double Latitude = 0;
static double Longitude = 0;

static int32_t LatitudeBinary = 0;
static int32_t LongitudeBinary = 0;

static uint16_t Altitude = 0xFFFF;

static uint32_t cnt = 0;

void OnPpsSignal( void )
{
    cnt++;
#if defined( USE_DEBUG_PINS )   
        GpioWrite( &DbgPin1, 0 );
        GpioWrite( &DbgPin1, 1 );
        GpioWrite( &DbgPin1, 0 );
#endif
    if( cnt >= TRIGGER_GPS_CNT )
    {   
        cnt = 0;
#if defined( USE_DEBUG_PINS )   
        GpioWrite( &DbgPin2, 0 );
        GpioWrite( &DbgPin2, 1 );
        GpioWrite( &DbgPin2, 0 );
#endif
        BlockLowPowerDuringTask ( true );
        UartInit( &Uart, UART_TX, UART_RX );
        USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );
    }
}

void up501Init( void )
{
//    GpioWrite( &GpsPowerEn, 1 );  // power down the GPS
    GpioWrite( &GpsPowerEn, 0 );    // power up the GPS
    GpioInit( &GpsPps, GPS_PPS, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &GpsPps, IRQ_RISING_EDGE, IRQ_VERY_LOW_PRIORITY, &OnPpsSignal );
}

void up501ConvertPositionIntoBinary( void )
{
    long double temp;

    if( Latitude >= 0 ) // North
    {    
        temp = Latitude * MaxNorthPosition;
        LatitudeBinary = temp / 90;
    }
    else                // South
    {    
        temp = Latitude * MaxSouthPosition;
        LatitudeBinary = temp / 90;
    }

    if( Longitude >= 0 ) // East
    {    
        temp = Longitude * MaxEastPosition;
        LongitudeBinary = temp / 180;
    }
    else                // West
    {    
        temp = Longitude * MaxWestPosition;
        LongitudeBinary = temp / 180;
    }
}

void up501ConvertPositionFromStringToNumerical( void )
{
    int i;

    double valueTmp1;
    double valueTmp2;
    double valueTmp3;
    double valueTmp4;

    // Convert the latitude from ASCII to uint8_t values
    for( i = 0 ; i < 10 ; i++ )
    {
        NmeaGpsData.NmeaLatitude[i] = NmeaGpsData.NmeaLatitude[i] & 0xF;  
    }
    // Convert latitude from degree/minute/second (DMS) format into decimal
    valueTmp1 = ( double )NmeaGpsData.NmeaLatitude[0] * 10.0 + ( double )NmeaGpsData.NmeaLatitude[1];
    valueTmp2 = ( double )NmeaGpsData.NmeaLatitude[2] * 10.0 + ( double )NmeaGpsData.NmeaLatitude[3];
    valueTmp3 = ( double )NmeaGpsData.NmeaLatitude[5] * 1000.0 + ( double )NmeaGpsData.NmeaLatitude[6] * 100.0 + 
                ( double )NmeaGpsData.NmeaLatitude[7] * 10.0 + ( double )NmeaGpsData.NmeaLatitude[8];
                
    Latitude = valueTmp1 + ( ( valueTmp2 + ( valueTmp3 * 0.0001 ) ) / 60.0 ); 
    
    if( NmeaGpsData.NmeaLatitudePole[0] == 'S' )
    {
        Latitude *= -1;
    }
 
    // Convert the longitude from ASCII to uint8_t values
    for( i = 0 ; i < 10 ; i++ )
    {
        NmeaGpsData.NmeaLongitude[i] = NmeaGpsData.NmeaLongitude[i] & 0xF;  
    }
    // Convert longitude from degree/minute/second (DMS) format into decimal
    valueTmp1 = ( double )NmeaGpsData.NmeaLongitude[0] * 100.0 + ( double )NmeaGpsData.NmeaLongitude[1] * 10.0 + ( double )NmeaGpsData.NmeaLongitude[2];
    valueTmp2 = ( double )NmeaGpsData.NmeaLongitude[3] * 10.0 + ( double )NmeaGpsData.NmeaLongitude[4];
    valueTmp3 = ( double )NmeaGpsData.NmeaLongitude[6] * 1000.0 + ( double )NmeaGpsData.NmeaLongitude[7] * 100;
    valueTmp4 = ( double )NmeaGpsData.NmeaLongitude[8] * 10.0 + ( double )NmeaGpsData.NmeaLongitude[9];
    
    Longitude = valueTmp1 + ( valueTmp2 / 60.0 ) + ( ( ( valueTmp3 + valueTmp4 ) * 0.0001 ) / 60.0 );
    
    if( NmeaGpsData.NmeaLongitudePole[0] == 'W' )
    {
        Longitude *= -1;
    }
}


uint8_t up501GetLatestGpsPositionDouble ( double *lati, double *longi )
{
    if( ( Latitude != 0 ) || ( Longitude != 0 ) )
    {    
        *lati = Latitude;
        *longi = Longitude;
        return SUCCESS;
    }
    else
    {
        return FAIL;
    }  
}

uint8_t up501GetLatestGpsPositionBinary ( int32_t *latiBin, int32_t *longiBin )
{
    if( ( Latitude != 0 ) || ( Longitude != 0 ) )
    {    
        *latiBin = LatitudeBinary;
        *longiBin = LongitudeBinary;
        return SUCCESS;
    }
    else
    {
        *latiBin = 0;
        *longiBin = 0;
        return FAIL;
    }  
}

uint16_t up501GetLatestGpsAltitude (void)
{
    if( ( Latitude != 0 ) || ( Longitude != 0 ) )
    {    
        Altitude = atoi( NmeaGpsData.NmeaAltitude );
    }
    else
    {
        Altitude = 0xFFFF;
    }
    
    return Altitude;
}

uint8_t up501ParseGpsData( char *RxBuffer )
{
    uint8_t i = 0;
    uint8_t j = 0;

    while( RxBuffer[i] != ',' )     // Global Positioning System Fix Data
    {
        if( RxBuffer[i] != '$' )
        {        
            NmeaGpsData.NmeaDataType[j] = RxBuffer[i];
            j++;
        }  
        i++;
    }
    
    // Parse the GPGGA data 
    if( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPGGA, 5 ) == 0 )
    {  
        Latitude = 0;
        Longitude = 0;
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaUtcTime
        {
            NmeaGpsData.NmeaUtcTime[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaLatitude
        {
            NmeaGpsData.NmeaLatitude[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaLatitudePole
        {
            NmeaGpsData.NmeaLatitudePole[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaLongitude 
        {
            NmeaGpsData.NmeaLongitude[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaLongitudePole
        {
            NmeaGpsData.NmeaLongitudePole[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaFixQuality
        {
            NmeaGpsData.NmeaFixQuality[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaSatelliteTracked
        {
            NmeaGpsData.NmeaSatelliteTracked[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaHorizontalDilution
        {
            NmeaGpsData.NmeaHorizontalDilution[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaAltitude
        {
            NmeaGpsData.NmeaAltitude[j] = RxBuffer[i];
            i++;
            j++;
        }
    
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaAltitudeUnit
        {
            NmeaGpsData.NmeaAltitudeUnit[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaHeightGeoid
        {
            NmeaGpsData.NmeaHeightGeoid[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // NmeaHeightGeoidUnit
        {
            NmeaGpsData.NmeaHeightGeoidUnit[j] = RxBuffer[i];
            i++;
            j++;
        }
        up501FormatGpsData( );
        return SUCCESS;
    }
    else if ( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPRMC, 5 ) == 0 )
    {    
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // UTC
        {
            NmeaGpsData.NmeaUtcTime[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // Status
        {
            NmeaGpsData.NmeaDataStatus[j] = RxBuffer[i];
            i++;
            j++;
        }
        j = 0;
        while( RxBuffer[i] != ',' )     // Latitude
        {
            NmeaGpsData.NmeaLatitude[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // Latitude pole
        {
            NmeaGpsData.NmeaLatitudePole[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // Longitude 
        {
            NmeaGpsData.NmeaLongitude[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // Longitude side
        {
            NmeaGpsData.NmeaLongitudePole[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // Speed
        {
            NmeaGpsData.NmeaSpeed[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // Angle
        {
            NmeaGpsData.NmeaDetectionAngle[j] = RxBuffer[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while( RxBuffer[i] != ',' )     // Date
        {
            NmeaGpsData.NmeaDate[j] = RxBuffer[i];
            i++;
            j++;
        } 
        up501FormatGpsData( );
        return SUCCESS;
    }
    else
    {
        return FAIL;
    }
}

void up501FormatGpsData( void )
{
    up501ConvertPositionFromStringToNumerical( );
    up501ConvertPositionIntoBinary( );
}

void up501ResetPosition( void )
{
    Altitude = 0xFFFF;
    Latitude = 0;
    Longitude = 0;
}
