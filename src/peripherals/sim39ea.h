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
#ifndef __SIM39EA_H__
#define __SIM39EA_H__

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
    char NmeaHorizontalDilution[6];
    char NmeaAltitude[8];
    char NmeaAltitudeUnit[2];
    char NmeaHeightGeoid[8];
    char NmeaHeightGeoidUnit[2];
    char NmeaSpeed[8];
    char NmeaDetectionAngle[8];
    char NmeaDate[8];
}tNmeaGpsData;

extern tNmeaGpsData NmeaGpsData;

/*!
 * \brief Init the handling of the GPS receiver
 */
void SIM39EAInit( void );

/*!
 * \brief 
 */
void SIM39EAOnPpsSignal( void );

/*!
 * \brief Convert the latest Position (latitude and Longitude) into a binary number
 */
void SIM39EAConvertPositionIntoBinary( void );

/*!
 * \brief Convert the latest Position (latitude and Longitude) from ASCII into DMS numerical format
 */
void SIM39EAConvertPositionFromStringToNumerical( void );

/*!
 * \brief Get the latest Position (latitude and Longitude) as two double values if available
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t SIM39EAGetLatestGpsPositionDouble ( double *lati, double *longi );

/*!
 * \brief Get the latest Position (latitude and Longitude) as two binary values if available
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t SIM39EAGetLatestGpsPositionBinary ( int32_t *latiBin, int32_t *longiBin );

/*!
 * \brief This is the NMEA packet parsing function, setup to parse only the GPGGA and GPRMC packets
 *
 * \retval status [SUCCESS, FAIL]
 */
uint8_t SIM39EAParseGpsData( int8_t *rxBuffer, int32_t rxBufferSize );

/*!
 * \brief Return the latest Altitude from the NMEA parsed data
 *
 * \retval Altitude
 */
uint16_t SIM39EAGetLatestGpsAltitude (void);

/*!
 * \brief Convert position form a binary format into decimal format
 */
void SIM39EAConvertBinaryToPosition( int32_t LatitudeBinary, int32_t LongitudeBinary );

/*!
 * \brief Format GPS data so that they can be sent
 */
void SIM39EAFormatGpsData( void );

/*!
 * \brief Reset the GPS position internally so we lose the values if we lose the satellite connection
 */
void SIM39EAResetPosition( void );

#endif  // __SIM39EA_H__
