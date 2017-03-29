/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Driver for the SX1509 IO expander

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __PAM7Q_H__
#define __PAM7Q_H__

void PAM7QInit( void );

uint8_t PAM7QGetDeviceAddr( void );

void PAM7QSetDeviceAddr( uint8_t addr );

bool PAM7QGetGpsData( uint8_t *nmeaString, uint8_t *nmeaStringSize, uint16_t nmeaMaxStringSize );

uint8_t PAM7QWrite( uint8_t addr, uint8_t data );

uint8_t PAM7QWriteBuffer( uint8_t addr, uint8_t *data, uint8_t size );

uint8_t PAM7QRead( uint8_t addr, uint8_t *data );

uint8_t PAM7QReadBuffer( uint8_t addr, uint8_t *data, uint8_t size );

void GpsMcuOnPpsSignal( void );

/*
 * MPL3115A2 I2C address
 */
#define PAM7Q_I2C_ADDRESS                       0x42

#define MESSAGE_SIZE_1                          0xFD
#define MESSAGE_SIZE_2                          0xFE

#define PAYLOAD                                 0xFF


#endif  // __PAM7Q_H__

