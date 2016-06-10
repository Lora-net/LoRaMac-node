/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Driver for the SX1509 IO expander

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "sx1509.h"

static uint8_t I2cDeviceAddr = 0;

static bool SX1509Initialized = false;

void SX1509Init( void )
{
    if( SX1509Initialized == false )
    {   
        SX1509SetDeviceAddr( SX1509_I2C_ADDRESS );
        SX1509Initialized = true;
        
        SX1509Reset( );
    }
}

uint8_t SX1509Reset( )
{
    if( SX1509Write( RegReset, 0x12 ) == SUCCESS )
    {
        if( SX1509Write( RegReset, 0x34 ) == SUCCESS )
        {
            return SUCCESS;
        }
    }
    return FAIL;
}

uint8_t SX1509Write( uint8_t addr, uint8_t data )
{
    return SX1509WriteBuffer( addr, &data, 1 );
}

uint8_t SX1509WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

uint8_t SX1509Read( uint8_t addr, uint8_t *data )
{
    return SX1509ReadBuffer( addr, data, 1 );
}

uint8_t SX1509ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void SX1509SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t SX1509GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}
