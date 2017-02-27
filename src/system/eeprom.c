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
#include "board.h"

#include "eeprom-board.h"

uint8_t EepromWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    return EepromMcuWriteBuffer( addr, buffer, size );
}

uint8_t EepromReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    return EepromMcuReadBuffer( addr, buffer, size );
}

void EepromSetDeviceAddr( uint8_t addr )
{
    EepromMcuSetDeviceAddr( addr );
}

uint8_t EepromGetDeviceAddr( void )
{
    return EepromMcuGetDeviceAddr( );
}
