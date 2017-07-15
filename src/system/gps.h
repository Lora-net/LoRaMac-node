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
#ifndef __GPS_H__
#define __GPS_H__

/* Structure to handle the GPS parsed data in ASCII */
typedef struct
{
    char NmeaDataType[6];           /* "GP" + 3 letters sentence type */
    char NmeaUtcTime[11];           /* "hhmmss.sss" */
    char NmeaDataStatus[2];         /* "V" or "A" */
    char NmeaLatitude[10];          /* "ddmm.mmmm" */
    char NmeaLatitudePole[2];       /* "N" or "S" */
    char NmeaLongitude[11];         /* "dddmm.mmmm" */
    char NmeaLongitudePole[2];      /* "E" or "W" */
    char NmeaFixQuality[2];         /* "0" -> "8" */
    char NmeaSatelliteTracked[3];   /* "00" -> "99" */
    char NmeaHorizontalDilution[6]; /* "00.00" -> "99.99" */
    char NmeaAltitude[8];           /* "-9999.9" -> "17999.9" */
    char NmeaAltitudeUnit[2];       /* "M" */
    char NmeaHeightGeoid[8];        /* "-9999.9" -> "17999.9" */
    char NmeaHeightGeoidUnit[2];    /* "M" */
    char NmeaSpeed[8];              /* "000.000" */
    char NmeaDetectionAngle[8];     /* "000.000" */
    char NmeaDate[7];               /* "DDMMYY" */
}tNmeaGpsData;

extern tNmeaGpsData NmeaGpsData;

/*!
 * NMEA cordinate strings are in [d]ddmm.mmmm format
 * to convert it to an integer number, we need an integer capable of storing
 * a value of magnitude 10000*60*180, and an extra bit for a sign.
 * i.e. ceil(log(10000*60*180)/log(2))[value]+1[sign] bits = 27 + 1 bits = 28 bits
 */
typedef int32_t tGpsIntegerCoord;
#include <inttypes.h>
#define GPS_PRINT_FORMAT_FLAG PRId32
#define GPS_SUB_MINUTE_FACTOR  10000
#define GPS_SUB_DEGREE_FACTOR 600000

/*!
 * \brief Initializes the handling of the GPS receiver
 */
void GpsInit( void );

/*!
 * \brief Switch ON the GPS
 */
void GpsStart( void );

/*!
 * \brief Switch OFF the GPS
 */
void GpsStop( void );

/*!
 * Updates the GPS status
 */
void GpsProcess( void );

/*!
 * \brief PPS signal handling function
 */
void GpsPpsHandler( bool *parseData );

/*!
 * \brief PPS signal handling function
 *
 * \retval ppsDetected State of PPS signal.
 */
bool GpsGetPpsDetectedState( void );

/*!
 * \brief Indicates if GPS has fix
 *
 * \retval hasFix
 */
bool GpsHasFix( void );

/*!
 * \brief Converts the latest Position (latitude and Longitude) from ASCII into
 *        tGpsIntegerCoord numerical format
 */
void GpsConvertPositionFromStringToNumericalGpsIntegerCoord( void );

/*!
 * \brief Gets the latest Position (latitude and Longitude) as two tGpsIntegerCoord
 *        values if available
 *
 * \param [OUT] lati Latitude value
 * \param [OUT] longi Longitude value
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t GpsGetLatestGpsPositionGpsIntegerCoord( tGpsIntegerCoord *lati, tGpsIntegerCoord *longi );

/*!
 * \brief Gets the latest Position (latitude and Longitude) as two binary values
 *        if available
 *
 * \param [OUT] latiBin Latitude value
 * \param [OUT] longiBin Longitude value
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t GpsGetLatestGpsPositionBinary ( int32_t *latiBin, int32_t *longiBin );

/*!
 * \brief Parses the NMEA sentence.
 *
 * \remark Only parses GPGGA, GPRMC and GPGLL sentences
 *
 * \param [IN] rxBuffer Data buffer to be parsed
 * \param [IN] rxBufferSize Size of data buffer
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t GpsParseGpsData( int8_t *rxBuffer, int32_t rxBufferSize );

/*!
 * \brief Returns the latest altitude from the parsed NMEA sentence
 *
 * \retval altitude
 */
int16_t GpsGetLatestGpsAltitude( void );

/*!
 * \brief Format GPS data into numeric and binary formats
 */
void GpsFormatGpsData( void );

/*!
 * \brief Resets the GPS position variables
 */
void GpsResetPosition( void );

#endif  // __GPS_H__
