/*!
 * \file  mag3110.c
 *
 * \brief MAG3110 Magnetometer driver implementation
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
#include "mag3110.h"

extern I2c_t I2c;

static uint8_t I2cDeviceAddr = 0;
static bool MAG3110Initialized = false;

LmnStatus_t MAG3110Init( void )
{
    uint8_t regVal = 0;

    MAG3110SetDeviceAddr( MAG3110_I2C_ADDRESS );

    if( MAG3110Initialized == false )
    {
        MAG3110Initialized = true;

        MAG3110Read( MAG3110_ID, &regVal );
        if( regVal != 0xC4 )   // Fixed Device ID Number = 0xC4
        {
            return LMN_STATUS_ERROR;
        }

        MAG3110Reset( );
    }
    return LMN_STATUS_OK;
}

LmnStatus_t MAG3110Reset( void )
{
    if( MAG3110Write( 0x11, 0x10 ) == LMN_STATUS_OK ) // Reset the MAG3110 with CTRL_REG2
    {
        return LMN_STATUS_OK;
    }
    return LMN_STATUS_ERROR;
}

LmnStatus_t MAG3110Write( uint8_t addr, uint8_t data )
{
    return MAG3110WriteBuffer( addr, &data, 1 );
}

LmnStatus_t MAG3110WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteMemBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

LmnStatus_t MAG3110Read( uint8_t addr, uint8_t *data )
{
    return MAG3110ReadBuffer( addr, data, 1 );
}

LmnStatus_t MAG3110ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadMemBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void MAG3110SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t MAG3110GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}
