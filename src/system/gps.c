/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic driver for any GPS receiver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "gps.h"

#define TRIGGER_GPS_CNT                             10

/* Various type of NMEA data we can receive with the Gps to get a position fix*/
const char NmeaDataTypeGPGGA[] = "GPGGA"; /* Fix information */
const char NmeaDataTypeGPRMC[] = "GPRMC"; /* Recommended minimum data for gps */
const char NmeaDataTypeGPGLL[] = "GPGLL"; /* Lat/Lon data */

tNmeaGpsData NmeaGpsData;

static bool HasFix = false;
static tGpsIntegerCoord Latitude = 0;
static tGpsIntegerCoord Longitude = 0;

static int16_t Altitude = 0xFFFF;

static uint32_t PpsCnt = 0;

bool PpsDetected = false;

void GpsPpsHandler( bool *parseData )
{
    PpsDetected = true;
    PpsCnt++;
    *parseData = false;

    if( PpsCnt >= TRIGGER_GPS_CNT )
    {
        PpsCnt = 0;
        BlockLowPowerDuringTask ( true );
        *parseData = true;
    }
}

void GpsInit( void )
{
    PpsDetected = false;
    GpsMcuInit( );
}

void GpsStart( void )
{
    GpsMcuStart( );
}

void GpsStop( void )
{
    GpsMcuStop( );
}

void GpsProcess( void )
{
    GpsMcuProcess( );
}

bool GpsGetPpsDetectedState( void )
{
    bool state = false;

    BoardDisableIrq( );
    state = PpsDetected;
    PpsDetected = false;
    BoardEnableIrq( );
    return state;
}

bool GpsHasFix( void )
{
    return HasFix;
}

void GpsConvertPositionFromStringToNumericalGpsIntegerCoord( void )
{
    int i;

    uint8_t  valueTmp1;
    uint8_t  valueTmp2;
    uint16_t valueTmp3;

    // Convert the latitude from ASCII to uint8_t values
    for( i = 0 ; i < sizeof( NmeaGpsData.NmeaLatitude ) ; i++ )
    {
        NmeaGpsData.NmeaLatitude[i] = NmeaGpsData.NmeaLatitude[i] & 0xF;
    }
    // Convert latitude from ddmm.mmmm format into tGpsIntegerCoord
    valueTmp1 = NmeaGpsData.NmeaLatitude[0] * 10 + NmeaGpsData.NmeaLatitude[1];
    valueTmp2 = NmeaGpsData.NmeaLatitude[2] * 10 + NmeaGpsData.NmeaLatitude[3];
    valueTmp3 = ( uint16_t )NmeaGpsData.NmeaLatitude[5] * 1000 +
                ( uint16_t )NmeaGpsData.NmeaLatitude[6] * 100 +
                NmeaGpsData.NmeaLatitude[7] * 10 + NmeaGpsData.NmeaLatitude[8];

    Latitude = ( tGpsIntegerCoord )valueTmp1 * GPS_SUB_DEGREE_FACTOR +
               ( tGpsIntegerCoord )valueTmp2 * GPS_SUB_MINUTE_FACTOR +
                                    valueTmp3;

    if( NmeaGpsData.NmeaLatitudePole[0] == 'S' )
    {
        Latitude *= -1;
    }

    // Convert the longitude from ASCII to uint8_t values
    for( i = 0 ; i < sizeof( NmeaGpsData.NmeaLongitude ) ; i++ )
    {
        NmeaGpsData.NmeaLongitude[i] = NmeaGpsData.NmeaLongitude[i] & 0xF;
    }
    // Convert latitude from dddmm.mmmm format into tGpsIntegerCoord
    valueTmp1 = NmeaGpsData.NmeaLatitude[0] * 100 + NmeaGpsData.NmeaLatitude[1] * 10 + NmeaGpsData.NmeaLatitude[2];
    valueTmp2 = NmeaGpsData.NmeaLatitude[3] * 10 + NmeaGpsData.NmeaLatitude[4];
    valueTmp3 = ( uint16_t )NmeaGpsData.NmeaLatitude[6] * 1000 +
                ( uint16_t )NmeaGpsData.NmeaLatitude[7] * 100 +
                NmeaGpsData.NmeaLatitude[8] * 10 + NmeaGpsData.NmeaLatitude[9];

    Longitude = ( tGpsIntegerCoord )valueTmp1 * GPS_SUB_DEGREE_FACTOR +
                ( tGpsIntegerCoord )valueTmp2 * GPS_SUB_MINUTE_FACTOR +
                                    valueTmp3;

    if( NmeaGpsData.NmeaLongitudePole[0] == 'W' )
    {
        Longitude *= -1;
    }
}

uint8_t GpsGetLatestGpsPositionGpsIntegerCoord( tGpsIntegerCoord *lati, tGpsIntegerCoord *longi )
{
    uint8_t status = FAIL;
    if( HasFix == true )
    {
        status = SUCCESS;
    }
    else
    {
        GpsResetPosition( );
    }
    *lati = Latitude;
    *longi = Longitude;
    return status;
}

uint8_t GpsGetLatestGpsPositionBinary( int32_t *latiBin, int32_t *longiBin )
{
    /* Value used for the conversion of the position from tGpsIntegerCoord to binary */
    const int32_t MaxNorthPosition = 8388607;       // 2^23 - 1
    const int32_t MaxSouthPosition = 8388608;       // -2^23
    const int32_t MaxEastPosition = 8388607;        // 2^23 - 1
    const int32_t MaxWestPosition = 8388608;        // -2^23

    uint8_t status = FAIL;

    BoardDisableIrq( );
    if( HasFix == true )
    {
        status = SUCCESS;
    }
    else
    {
        GpsResetPosition( );
    }

    if( Latitude >= 0)
    {
      *latiBin = ( int32_t )INT_CLOSEST_TO_DIV_ANY_SIGNS( ( ( int64_t )Latitude * MaxNorthPosition ), ( GPS_SUB_DEGREE_FACTOR * 90 ) );
    }
    else
    {
      *latiBin = ( int32_t )INT_CLOSEST_TO_DIV_ANY_SIGNS( ( ( int64_t )Latitude * MaxSouthPosition ), ( GPS_SUB_DEGREE_FACTOR * 90 ) );
    }

    if( Longitude >= 0)
    {
      *latiBin = ( int32_t )INT_CLOSEST_TO_DIV_ANY_SIGNS( ( ( int64_t )Longitude * MaxEastPosition ), ( GPS_SUB_DEGREE_FACTOR * 180 ) );
    }
    else
    {
      *latiBin = ( int32_t )INT_CLOSEST_TO_DIV_ANY_SIGNS( ( ( int64_t )Longitude * MaxWestPosition ), ( GPS_SUB_DEGREE_FACTOR * 180 ) );
    }

    BoardEnableIrq( );
    return status;
}

int16_t GpsGetLatestGpsAltitude( void )
{
    BoardDisableIrq( );
    if( HasFix == true )
    {
        Altitude = atoi( NmeaGpsData.NmeaAltitude );
    }
    else
    {
        Altitude = 0xFFFF;
    }
    BoardEnableIrq( );

    return Altitude;
}

/*!
 * Calculates the checksum for a NMEA sentence
 *
 * Skip the first '$' if necessary and calculate checksum until '*' character is
 * reached (or buffSize exceeded).
 *
 * \retval chkPosIdx Position of the checksum in the sentence
 */
int32_t GpsNmeaChecksum( int8_t *nmeaStr, int32_t nmeaStrSize, int8_t * checksum )
{
    int i = 0;
    uint8_t checkNum = 0;

    // Check input parameters
    if( ( nmeaStr == NULL ) || ( checksum == NULL ) || ( nmeaStrSize <= 1 ) )
    {
        return -1;
    }

    // Skip the first '$' if necessary
    if( nmeaStr[i] == '$' )
    {
        i += 1;
    }

    // XOR until '*' or max length is reached
    while( nmeaStr[i] != '*' )
    {
        checkNum ^= nmeaStr[i];
        i += 1;
        if( i >= nmeaStrSize )
        {
            return -1;
        }
    }

    // Convert checksum value to 2 hexadecimal characters
    checksum[0] = Nibble2HexChar( checkNum / 16 ); // upper nibble
    checksum[1] = Nibble2HexChar( checkNum % 16 ); // lower nibble

    return i + 1;
}

/*!
 * Calculate the checksum of a NMEA frame and compare it to the checksum that is
 * present at the end of it.
 * Return true if it matches
 */
static bool GpsNmeaValidateChecksum( int8_t *serialBuff, int32_t buffSize )
{
    int32_t checksumIndex;
    int8_t checksum[2]; // 2 characters to calculate NMEA checksum

    checksumIndex = GpsNmeaChecksum( serialBuff, buffSize, checksum );

    // could we calculate a verification checksum ?
    if( checksumIndex < 0 )
    {
        return false;
    }

    // check if there are enough char in the serial buffer to read checksum
    if( checksumIndex >= ( buffSize - 2 ) )
    {
        return false;
    }

    // check the checksum
    if( ( serialBuff[checksumIndex] == checksum[0] ) && ( serialBuff[checksumIndex + 1] == checksum[1] ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t GpsParseGpsData( int8_t *rxBuffer, int32_t rxBufferSize )
{
    uint8_t i = 1;
    uint8_t j = 0;
    uint8_t fieldSize = 0;

    if( rxBuffer[0] != '$' )
    {
        GpsMcuInvertPpsTrigger( );
        return FAIL;
    }

    if( GpsNmeaValidateChecksum( rxBuffer, rxBufferSize ) == false )
    {
        return FAIL;
    }

    // NmeaDataType
    fieldSize = 0;
    while( rxBuffer[i + fieldSize++] != ',' )
    {
        if( fieldSize > sizeof( NmeaGpsData.NmeaDataType ) )
        {
            return FAIL;
        }
    }
    memset(&( NmeaGpsData.NmeaDataType ), 0, sizeof( NmeaGpsData.NmeaDataType ));
    for( j = 0; j < fieldSize; j++, i++ )
    {
        NmeaGpsData.NmeaDataType[j] = rxBuffer[i];
    }
    // Parse the GPGGA data
    if( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPGGA, 5 ) == 0 )
    {
        // NmeaUtcTime
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaUtcTime ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaUtcTime ), 0, sizeof( NmeaGpsData.NmeaUtcTime ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaUtcTime[j] = rxBuffer[i];
        }
        // NmeaLatitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLatitude ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLatitude ), 0, sizeof( NmeaGpsData.NmeaLatitude ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitude[j] = rxBuffer[i];
        }
        // NmeaLatitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLatitudePole ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLatitudePole ), 0, sizeof( NmeaGpsData.NmeaLatitudePole ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitudePole[j] = rxBuffer[i];
        }
        // NmeaLongitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLongitude ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLongitude ), 0, sizeof( NmeaGpsData.NmeaLongitude ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitude[j] = rxBuffer[i];
        }
        // NmeaLongitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLongitudePole ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLongitude ), 0, sizeof( NmeaGpsData.NmeaLongitudePole ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitudePole[j] = rxBuffer[i];
        }
        // NmeaFixQuality
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaFixQuality ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaFixQuality ), 0, sizeof( NmeaGpsData.NmeaFixQuality ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaFixQuality[j] = rxBuffer[i];
        }
        // NmeaSatelliteTracked
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaSatelliteTracked ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaSatelliteTracked ), 0, sizeof( NmeaGpsData.NmeaSatelliteTracked ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaSatelliteTracked[j] = rxBuffer[i];
        }
        // NmeaHorizontalDilution
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaHorizontalDilution ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaHorizontalDilution ), 0, sizeof( NmeaGpsData.NmeaHorizontalDilution ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaHorizontalDilution[j] = rxBuffer[i];
        }
        // NmeaAltitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaAltitude ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaAltitude ), 0, sizeof( NmeaGpsData.NmeaAltitude ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaAltitude[j] = rxBuffer[i];
        }
        // NmeaAltitudeUnit
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaAltitudeUnit ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaAltitudeUnit ), 0, sizeof( NmeaGpsData.NmeaAltitudeUnit ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaAltitudeUnit[j] = rxBuffer[i];
        }
        // NmeaHeightGeoid
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaHeightGeoid ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaHeightGeoid ), 0, sizeof( NmeaGpsData.NmeaHeightGeoid ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaHeightGeoid[j] = rxBuffer[i];
        }
        // NmeaHeightGeoidUnit
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaHeightGeoidUnit ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaHeightGeoidUnit ), 0, sizeof( NmeaGpsData.NmeaHeightGeoidUnit ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaHeightGeoidUnit[j] = rxBuffer[i];
        }

        GpsFormatGpsData( );
        return SUCCESS;
    }
    else if ( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPRMC, 5 ) == 0 )
    {
        // NmeaUtcTime
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaUtcTime ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaUtcTime ), 0, sizeof( NmeaGpsData.NmeaUtcTime ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaUtcTime[j] = rxBuffer[i];
        }
        // NmeaDataStatus
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaDataStatus ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaDataStatus ), 0, sizeof( NmeaGpsData.NmeaDataStatus ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaDataStatus[j] = rxBuffer[i];
        }
        // NmeaLatitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLatitude ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLatitude ), 0, sizeof( NmeaGpsData.NmeaLatitude ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitude[j] = rxBuffer[i];
        }
        // NmeaLatitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLatitudePole ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLatitudePole ), 0, sizeof( NmeaGpsData.NmeaLatitudePole ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitudePole[j] = rxBuffer[i];
        }
        // NmeaLongitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLongitude ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLongitude ), 0, sizeof( NmeaGpsData.NmeaLongitude ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitude[j] = rxBuffer[i];
        }
        // NmeaLongitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLongitudePole ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLongitudePole ), 0, sizeof( NmeaGpsData.NmeaLongitudePole ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitudePole[j] = rxBuffer[i];
        }
        // NmeaSpeed
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaSpeed ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaSpeed ), 0, sizeof( NmeaGpsData.NmeaSpeed ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaSpeed[j] = rxBuffer[i];
        }
        // NmeaDetectionAngle
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaDetectionAngle ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaDetectionAngle ), 0, sizeof( NmeaGpsData.NmeaDetectionAngle ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaDetectionAngle[j] = rxBuffer[i];
        }
        // NmeaDate
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaDate ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaDate ), 0, sizeof( NmeaGpsData.NmeaDate ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaDate[j] = rxBuffer[i];
        }

        GpsFormatGpsData( );
        return SUCCESS;
    }
    else if ( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPGLL, 5 ) == 0 )
    {
        // NmeaLatitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLatitude ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLatitude ), 0, sizeof( NmeaGpsData.NmeaLatitude ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitude[j] = rxBuffer[i];
        }
        // NmeaLatitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLatitudePole ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLatitudePole ), 0, sizeof( NmeaGpsData.NmeaLatitudePole ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitudePole[j] = rxBuffer[i];
        }
        // NmeaLongitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLongitude ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLongitude ), 0, sizeof( NmeaGpsData.NmeaLongitude ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitude[j] = rxBuffer[i];
        }
        // NmeaLongitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaLongitudePole ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaLongitudePole ), 0, sizeof( NmeaGpsData.NmeaLongitudePole ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitudePole[j] = rxBuffer[i];
        }
        // NmeaUtcTime
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaUtcTime ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaUtcTime ), 0, sizeof( NmeaGpsData.NmeaUtcTime ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaUtcTime[j] = rxBuffer[i];
        }
        // NmeaDataStatus
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' )
        {
            if( fieldSize > sizeof( NmeaGpsData.NmeaDataStatus ) )
            {
                return FAIL;
            }
        }
        memset(&( NmeaGpsData.NmeaDataStatus ), 0, sizeof( NmeaGpsData.NmeaDataStatus ));
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaDataStatus[j] = rxBuffer[i];
        }

        GpsFormatGpsData( );
        return SUCCESS;
    }
    else
    {
        return FAIL;
    }
}

void GpsFormatGpsData( void )
{
    if( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPGGA, 5 ) == 0 )
    {
        HasFix = ( NmeaGpsData.NmeaFixQuality[0] > '0' ) ? true : false;
    }
    else if( ( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPRMC, 5 ) == 0 )
             ||
             ( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPGLL, 5 ) == 0 ) )
    {
        HasFix = ( NmeaGpsData.NmeaDataStatus[0] == 'A' ) ? true : false;
    }
    GpsConvertPositionFromStringToNumericalGpsIntegerCoord( );
}

void GpsResetPosition( void )
{
    Altitude = 0xFFFF;
    Latitude = 0;
    Longitude = 0;
}
