/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Driver for the SX9500 proximity sensor

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "sx9500.h"

static uint8_t I2cDeviceAddr = 0;

static bool sx9500Initialized = false;

uint8_t sx9500Init( void )
{
    uint8_t reg_val = 0;

    sx9500SetDeviceAddr( SX9500_I2C_ADDRESS );

    if( sx9500Initialized == false )
    {   
        sx9500Initialized = true;
        
        sx9500Read( 0x06, &reg_val );
        if( reg_val != 0x0F )
        {
            return FAIL;
        }
    
        sx9500Reset( );
    }
    return SUCCESS;
}

uint8_t sx9500Reset( )
{
    if( sx9500Write( 0x7F, 0xDE ) == SUCCESS )
    {
        return SUCCESS;
    }
    return FAIL;
}

uint8_t sx9500Write( uint8_t addr, uint8_t data )
{
    return sx9500WriteBuffer( addr, &data, 1 );
}

uint8_t sx9500WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

uint8_t sx9500Read( uint8_t addr, uint8_t *data )
{
    return sx9500ReadBuffer( addr, data, 1 );
}

uint8_t sx9500ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void sx9500SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t sx9500GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}

void sx9500LockUntilDetection( void )
{
    uint8_t val = 0;

    sx9500Write( 0x7F, 0xDE );
    sx9500Read( 0x00, &val );
    sx9500Read( 0x01, &val );

    sx9500Write( 0x06, 0x0F );
    sx9500Write( 0x07, 0x43 );
    sx9500Write( 0x08, 0x77 );
    sx9500Write( 0x09, 0x01 );
    sx9500Write( 0x0A, 0x30 );
    sx9500Write( 0x0B, 0x0F );
    sx9500Write( 0x0C, 0x04 );
    sx9500Write( 0x0D, 0x40 );
    sx9500Write( 0x0E, 0x00 );
    sx9500Write( 0x03, 0x60 );  
           
    val = 0;
                    
    while( ( val & 0xF0 ) == 0x00 )
    {
        sx9500Read( 0x01, &val );
    }

    sx9500Read( 0x01, &val );
    sx9500Read( 0x00, &val ); 
}
