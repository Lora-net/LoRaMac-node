/*!
 * \file  sx1509.c
 *
 * \brief SX1509 IO expander driver implementation
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
#include "sx1509.h"

extern I2c_t I2c;

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

LmnStatus_t SX1509Reset( )
{
    if( SX1509Write( RegReset, 0x12 ) == LMN_STATUS_OK )
    {
        if( SX1509Write( RegReset, 0x34 ) == LMN_STATUS_OK )
        {
            return LMN_STATUS_OK;
        }
    }
    return LMN_STATUS_ERROR;
}

LmnStatus_t SX1509Write( uint8_t addr, uint8_t data )
{
    return SX1509WriteBuffer( addr, &data, 1 );
}

LmnStatus_t SX1509WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteMemBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

LmnStatus_t SX1509Read( uint8_t addr, uint8_t *data )
{
    return SX1509ReadBuffer( addr, data, 1 );
}

LmnStatus_t SX1509ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadMemBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void SX1509SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t SX1509GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}
