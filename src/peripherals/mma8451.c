/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Driver for the MMA8451 Accelerometer

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "mma8451.h"

static uint8_t I2cDeviceAddr = 0;

static bool MMA8451Initialized = false;

uint8_t MMA8451Init( void )
{
    uint8_t regVal = 0;

    MMA8451SetDeviceAddr( MMA8451_I2C_ADDRESS );

    if( MMA8451Initialized == false )
    {   
        MMA8451Initialized = true;
            
        MMA8451Read( MMA8451_ID, &regVal );
        if( regVal != 0x1A )   // Fixed Device ID Number = 0x1A 
        {
            return FAIL;
        }
        MMA8451Reset( );
    }
    return SUCCESS;
}


uint8_t MMA8451Reset( )
{
    if( MMA8451Write( 0x2B, 0x40 ) == SUCCESS ) // Reset the MMA8451 with CTRL_REG2
    {
        return SUCCESS;
    }
    return FAIL;
}

uint8_t MMA8451Write( uint8_t addr, uint8_t data )
{
    return MMA8451WriteBuffer( addr, &data, 1 );
}

uint8_t MMA8451WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

uint8_t MMA8451Read( uint8_t addr, uint8_t *data )
{
    return MMA8451ReadBuffer( addr, data, 1 );
}

uint8_t MMA8451ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void MMA8451SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t MMA8451GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}
