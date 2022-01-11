/*!
 * \file  sx9500.c
 *
 * \brief SX9500 proximity sensor driver implementation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdbool.h>
#include "utilities.h"
#include "i2c.h"
#include "sx9500.h"

extern I2c_t I2c;

static uint8_t I2cDeviceAddr = 0;

static bool SX9500Initialized = false;

LmnStatus_t SX9500Init( void )
{
    uint8_t regVal = 0;

    SX9500SetDeviceAddr( SX9500_I2C_ADDRESS );

    if( SX9500Initialized == false )
    {
        SX9500Initialized = true;

        SX9500Read( SX9500_REG_PROXCTRL0, &regVal );
        if( regVal != 0x0F )
        {
            return LMN_STATUS_ERROR;
        }

        SX9500Reset( );
    }
    return LMN_STATUS_OK;
}

LmnStatus_t SX9500Reset( )
{
    if( SX9500Write( SX9500_REG_RESET, SX9500_RESET_CMD ) == LMN_STATUS_OK )
    {
        return LMN_STATUS_OK;
    }
    return LMN_STATUS_ERROR;
}

LmnStatus_t SX9500Write( uint8_t addr, uint8_t data )
{
    return SX9500WriteBuffer( addr, &data, 1 );
}

LmnStatus_t SX9500WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteMemBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

LmnStatus_t SX9500Read( uint8_t addr, uint8_t *data )
{
    return SX9500ReadBuffer( addr, data, 1 );
}

LmnStatus_t SX9500ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadMemBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void SX9500SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t SX9500GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}

void SX9500LockUntilDetection( void )
{
    uint8_t val = 0;

    SX9500Write( SX9500_REG_RESET, SX9500_RESET_CMD );
    SX9500Read( SX9500_REG_IRQSRC, &val );
    SX9500Read( SX9500_REG_STAT, &val );

    SX9500Write( SX9500_REG_PROXCTRL0, 0x0F );
    SX9500Write( SX9500_REG_PROXCTRL1, 0x43 );
    SX9500Write( SX9500_REG_PROXCTRL2, 0x77 );
    SX9500Write( SX9500_REG_PROXCTRL3, 0x01 );
    SX9500Write( SX9500_REG_PROXCTRL4, 0x30 );
    SX9500Write( SX9500_REG_PROXCTRL5, 0x0F );
    SX9500Write( SX9500_REG_PROXCTRL6, 0x04 );
    SX9500Write( SX9500_REG_PROXCTRL7, 0x40 );
    SX9500Write( SX9500_REG_PROXCTRL8, 0x00 );
    SX9500Write( SX9500_REG_IRQMSK, 0x60 );

    val = 0;

    while( ( val & 0xF0 ) == 0x00 )
    {
        SX9500Read( SX9500_REG_STAT, &val );
    }

    SX9500Read( SX9500_REG_STAT, &val );
    SX9500Read( SX9500_REG_IRQSRC, &val );
}
