/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Timer objects and scheduling management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __EEPROM_H__
#define __EEPROM_H__

uint8_t EepromWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size );
uint8_t EepromReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size );
void EepromSetDeviceAddr( uint8_t addr );
uint8_t EepromGetDeviceAddr( void );


#endif // __EEPROM_H__

