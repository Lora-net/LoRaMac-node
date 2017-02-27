/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Implements the generic I2C driver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "i2c-board.h"

/*!
 * Flag to indicates if the I2C is initialized
 */
static bool I2cInitialized = false;

void I2cInit( I2c_t *obj, PinNames scl, PinNames sda )
{
    if( I2cInitialized == false )
    {
        I2cInitialized = true;

        I2cMcuInit( obj, scl, sda );
        I2cMcuFormat( obj, MODE_I2C, I2C_DUTY_CYCLE_2, true, I2C_ACK_ADD_7_BIT, 400000 );
    }
}

void I2cDeInit( I2c_t *obj )
{
    I2cInitialized = false;
    I2cMcuDeInit( obj );
}

void I2cResetBus( I2c_t *obj )
{
    I2cInitialized = false;
    I2cInit( obj, I2C_SCL, I2C_SDA );
}

uint8_t I2cWrite( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t data )
{
    if( I2cInitialized == true )
    {
        if( I2cMcuWriteBuffer( obj, deviceAddr, addr, &data, 1 ) == FAIL )
        {
            // if first attempt fails due to an IRQ, try a second time
            if( I2cMcuWriteBuffer( obj, deviceAddr, addr, &data, 1 ) == FAIL )
            {
                return FAIL;
            }
            else
            {
                return SUCCESS;
            }
        }
        else
        {
            return SUCCESS;
        }
    }
    else
    {
        return FAIL;
    }
}

uint8_t I2cWriteBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size )
{
    if( I2cInitialized == true )
    {
        if( I2cMcuWriteBuffer( obj, deviceAddr, addr, buffer, size ) == FAIL )
        {
            // if first attempt fails due to an IRQ, try a second time
            if( I2cMcuWriteBuffer( obj, deviceAddr, addr, buffer, size ) == FAIL )
            {
                return FAIL;
            }
            else
            {
                return SUCCESS;
            }
        }
        else
        {
            return SUCCESS;
        }
    }
    else
    {
        return FAIL;
    }
}

uint8_t I2cRead( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *data )
{
    if( I2cInitialized == true )
    {
        return( I2cMcuReadBuffer( obj, deviceAddr, addr, data, 1 ) );
    }
    else
    {
        return FAIL;
    }
}

uint8_t I2cReadBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size )
{
    if( I2cInitialized == true )
    {
        return( I2cMcuReadBuffer( obj, deviceAddr, addr, buffer, size ) );
    }
    else
    {
        return FAIL;
    }
}
