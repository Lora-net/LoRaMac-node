/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Driver for the MAG3110 Magnetometer

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "mag3110.h"

static uint8_t I2cDeviceAddr = 0;
static bool MAG3110Initialized = false;

uint8_t MAG3110Init( void )
{
    uint8_t regVal = 0;

    MAG3110SetDeviceAddr( MAG3110_I2C_ADDRESS );

    if( MAG3110Initialized == false )
    {
        MAG3110Initialized = true;

        MAG3110Read( MAG3110_ID, &regVal );
        if( regVal != 0xC4 )   // Fixed Device ID Number = 0xC4
        {
            return FAIL;
        }

        MAG3110Reset( );
    }
    return SUCCESS;
}

uint8_t MAG3110Reset( void )
{
    if( MAG3110Write( 0x11, 0x10 ) == SUCCESS ) // Reset the MAG3110 with CTRL_REG2
    {
        return SUCCESS;
    }
    return FAIL;
}

uint8_t MAG3110Write( uint8_t addr, uint8_t data )
{
    return MAG3110WriteBuffer( addr, &data, 1 );
}

uint8_t MAG3110WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

uint8_t MAG3110Read( uint8_t addr, uint8_t *data )
{
    return MAG3110ReadBuffer( addr, data, 1 );
}

uint8_t MAG3110ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void MAG3110SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t MAG3110GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}
