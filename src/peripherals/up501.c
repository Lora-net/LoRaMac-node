/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic driver for the GPS receiver UP501

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


/*!
 * FIFO buffers size
 */
//#define FIFO_TX_SIZE								128
#define FIFO_RX_SIZE								128

//uint8_t TxBuffer[FIFO_TX_SIZE];
uint8_t RxBuffer[FIFO_RX_SIZE];

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

static uint32_t PpsCnt = 0;

int8_t NmeaString[128];
uint8_t NmeaStringSize = 0;

void UartIrqNotify( UartNotifyId_t id )
{
    uint8_t data;
    if( id == UART_NOTIFY_RX )
    {
        if( UartGetChar( &Uart1, &data ) == 0 )
        {
            if( ( data == '$' ) || ( NmeaStringSize >= 128 ) )
            {
                NmeaStringSize = 0;
            }

            NmeaString[NmeaStringSize++] = ( int8_t )data;

            if( data == '\n' )
            {
                NmeaString[NmeaStringSize] = '\0';
                UP501ParseGpsData( NmeaString, NmeaStringSize );
                UartDeInit( &Uart1 );
                BlockLowPowerDuringTask ( false );
            }
        }
    }
}

void UP501OnPpsSignal( void )
{
    PpsCnt++;
#if defined( USE_DEBUG_PINS )   
        GpioWrite( &DbgPin1, 0 );
        GpioWrite( &DbgPin1, 1 );
        GpioWrite( &DbgPin1, 0 );
#endif
    if( PpsCnt >= TRIGGER_GPS_CNT )
    {   
        PpsCnt = 0;
#if defined( USE_DEBUG_PINS )   
        GpioWrite( &DbgPin2, 0 );
        GpioWrite( &DbgPin2, 1 );
        GpioWrite( &DbgPin2, 0 );
#endif
        BlockLowPowerDuringTask ( true );
        UartInit( &Uart1, UART_1, UART_TX, UART_RX );
        UartConfig( &Uart1, RX_ONLY, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    }
}

void UP501Init( void )
{
    NmeaStringSize = 0;
    //FifoInit( &Uart1.FifoTx, TxBuffer, FIFO_TX_SIZE );
    FifoInit( &Uart1.FifoRx, RxBuffer, FIFO_RX_SIZE );
    Uart1.IrqNotify = UartIrqNotify;

//    GpioWrite( &GpsPowerEn, 1 );  // power down the GPS
    GpioWrite( &GpsPowerEn, 0 );    // power up the GPS
    GpioInit( &GpsPps, GPS_PPS, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioSetInterrupt( &GpsPps, IRQ_FALLING_EDGE, IRQ_VERY_LOW_PRIORITY, &UP501OnPpsSignal );
}

void UP501ConvertPositionIntoBinary( void )
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

void UP501ConvertPositionFromStringToNumerical( void )
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


uint8_t UP501GetLatestGpsPositionDouble ( double *lati, double *longi )
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

uint8_t UP501GetLatestGpsPositionBinary ( int32_t *latiBin, int32_t *longiBin )
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

uint16_t UP501GetLatestGpsAltitude (void)
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

/*!
 * Calculate the checksum for a NMEA string
 *
 * Skip the first '$' if necessary and calculate checksum until '*' character is
 * reached (or buffSize exceeded).
 *
 * \retval chkPosIdx Position of the checksum in the string
 */
int32_t UP501NmeaChecksum( int8_t *nmeaStr, int32_t nmeaStrSize, int8_t * checksum )
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
static bool UP501NmeaValidateChecksum( int8_t *serialBuff, int32_t buffSize )
{
	int32_t checksumIndex;
	int8_t checksum[2]; // 2 characters to calculate NMEA checksum

	checksumIndex = UP501NmeaChecksum( serialBuff, buffSize, checksum );

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

	// check the checksum per se
	if( ( serialBuff[checksumIndex] == checksum[0] ) && ( serialBuff[checksumIndex + 1] == checksum[1] ) )
    {
		return true;
	}
    else
    {
		return false;
	}
}

uint8_t UP501ParseGpsData( int8_t *rxBuffer, int32_t rxBufferSize )
{
    uint8_t i = 1;
    uint8_t j = 0;
    uint8_t fieldSize = 0;
    
    if( rxBuffer[0] != '$' )
    {
        return FAIL;
    }

    if( UP501NmeaValidateChecksum( rxBuffer, rxBufferSize ) == false )
    {
        return FAIL;
    }

    fieldSize = 0;
    while( rxBuffer[i + fieldSize++] != ',' );
    if( fieldSize > 6 )
    {
        return FAIL;
    }
    for( j = 0; j < fieldSize; j++, i++ )
    {
        NmeaGpsData.NmeaDataType[j] = rxBuffer[i];
    }
    // Parse the GPGGA data 
    if( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPGGA, 5 ) == 0 )
    {  
        // NmeaUtcTime
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 11 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaUtcTime[j] = rxBuffer[i];
        }
        // NmeaLatitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 10 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitude[j] = rxBuffer[i];
        }
        // NmeaLatitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 2 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitudePole[j] = rxBuffer[i];
        }
        // NmeaLongitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 11 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitude[j] = rxBuffer[i];
        }
        // NmeaLongitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 2 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitudePole[j] = rxBuffer[i];
        }
        // NmeaFixQuality
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 2 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaFixQuality[j] = rxBuffer[i];
        }
        // NmeaSatelliteTracked
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 3 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaSatelliteTracked[j] = rxBuffer[i];
        }
        // NmeaHorizontalDilution
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 6 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaHorizontalDilution[j] = rxBuffer[i];
        }
        // NmeaAltitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 8 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaAltitude[j] = rxBuffer[i];
        }
        // NmeaAltitudeUnit
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 2 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaAltitudeUnit[j] = rxBuffer[i];
        }
        // NmeaHeightGeoid
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 8 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaHeightGeoid[j] = rxBuffer[i];
        }
        // NmeaHeightGeoidUnit
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 2 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaHeightGeoidUnit[j] = rxBuffer[i];
        }

        UP501FormatGpsData( );
        return SUCCESS;
    }
    else if ( strncmp( ( const char* )NmeaGpsData.NmeaDataType, ( const char* )NmeaDataTypeGPRMC, 5 ) == 0 )
    {    
        // NmeaUtcTime
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 11 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaUtcTime[j] = rxBuffer[i];
        }
        // NmeaDataStatus
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 2 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaDataStatus[j] = rxBuffer[i];
        }
        // NmeaLatitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 10 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitude[j] = rxBuffer[i];
        }
        // NmeaLatitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 2 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLatitudePole[j] = rxBuffer[i];
        }
        // NmeaLongitude
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 11 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitude[j] = rxBuffer[i];
        }
        // NmeaLongitudePole
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 2 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaLongitudePole[j] = rxBuffer[i];
        }
        // NmeaSpeed
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 8 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaSpeed[j] = rxBuffer[i];
        }
        // NmeaDetectionAngle
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 8 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaDetectionAngle[j] = rxBuffer[i];
        }
        // NmeaDate
        fieldSize = 0;
        while( rxBuffer[i + fieldSize++] != ',' );
        if( fieldSize > 8 )
        {
            return FAIL;
        }
        for( j = 0; j < fieldSize; j++, i++ )
        {
            NmeaGpsData.NmeaDate[j] = rxBuffer[i];
        }

        UP501FormatGpsData( );
        return SUCCESS;
    }
    else
    {
        return FAIL;
    }
}

void UP501FormatGpsData( void )
{
    UP501ConvertPositionFromStringToNumerical( );
    UP501ConvertPositionIntoBinary( );
}

void UP501ResetPosition( void )
{
    Altitude = 0xFFFF;
    Latitude = 0;
    Longitude = 0;
}
