/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Generic GPIO driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __UP501_H__
#define __UP501_H__

/* Structure to handle the GPS parsed data in ASCII */
typedef struct
{
    char NmeaDataType[6];
    char NmeaUtcTime[11];
    char NmeaDataStatus[2];
    char NmeaLatitude[10];
    char NmeaLatitudePole[2];
    char NmeaLongitude[11];
    char NmeaLongitudePole[2];
    char NmeaFixQuality[2];
    char NmeaSatelliteTracked[3];
    char NmeaHorizontalDilution[4];
    char NmeaAltitude[8];
    char NmeaAltitudeUnit[2];
    char NmeaHeightGeoid[8];
    char NmeaHeightGeoidUnit[2];
    char NmeaSpeed[8];
    char NmeaDetectionAngle[6];
    char NmeaDate[8];
}tNmeaGpsData;

extern tNmeaGpsData NmeaGpsData;

/*!
 * \brief Init the handling of the GPS receiver
 */
void up501Init( void );

/*!
 * \brief 
 */
void OnPpsSignal( void );

/*!
 * \brief 
 */
void OnUartRxTrigger( void );

/*!
 * \brief Convert the latest Position (latitude and Longitude) into a binary number
 */
void up501ConvertPositionIntoBinary( void );

/*!
 * \brief Convert the latest Position (latitude and Longitude) from ASCII into DMS numerical format
 */
void up501ConvertPositionFromStringToNumerical( void );

/*!
 * \brief Get the latest Position (latitude and Longitude) as two double values if available
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t up501GetLatestGpsPositionDouble ( double *lati, double *longi );

/*!
 * \brief Get the latest Position (latitude and Longitude) as two binary values if available
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t up501GetLatestGpsPositionBinary ( int32_t *latiBin, int32_t *longiBin );

/*!
 * \brief This is the NMEA packet parsing function, setup to parse only the GPGGA and GPRMC packets
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t up501ParseGpsData( char *RxBuffer );

/*!
 * \brief Return the latest Altitude from the NMEA parsed data
 *
 * \retval Altitude
 */
uint16_t up501GetLatestGpsAltitude (void);

/*!
 * \brief Convert position form a binary format into decimal format
 */
void up501ConvertBinaryToPosition( int32_t LatitudeBinary, int32_t LongitudeBinary );

/*!
 * \brief Format GPS data so that they can be sent
 */
void up501FormatGpsData( void );

/*!
 * \brief Reset the GPS position internally so we lose the values if we lose the satellite connection
 */
void up501ResetPosition( void );

#endif  // __UP501_H__
