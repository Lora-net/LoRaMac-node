/*!
 * \file  pam7q.c
 *
 * \brief PAM7Q GPS receiver driver implementation
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
#include "utilities.h"
#include "i2c.h"
#include "pam7q.h"

extern I2c_t I2c;

static uint8_t I2cDeviceAddr = 0;

void PAM7QInit( void )
{
    PAM7QSetDeviceAddr( PAM7Q_I2C_ADDRESS );
}

bool PAM7QGetGpsData( uint8_t *nmeaString, uint8_t *nmeaStringSize, uint16_t nmeaMaxStringSize )
{
    uint8_t status;
    uint16_t pendingBytes;
    bool result = false;

    *nmeaStringSize = 0;

    status = PAM7QReadBuffer( MESSAGE_SIZE_1, nmeaString, 2 );

    if( status == LMN_STATUS_OK )
    {
        // build a 16bit number
        pendingBytes = ( uint16_t )( ( nmeaString[0] << 8 ) | nmeaString[1] );

        // check for invalid length
        if( pendingBytes == 0xFFFF )
        {
            pendingBytes = 0;
        }
        // just to buffer size
        if( pendingBytes > ( nmeaMaxStringSize - 1 ) )
        {
            pendingBytes = nmeaMaxStringSize - 1;
        }

        // read pending data from GPS module
        status = PAM7QReadBuffer( PAYLOAD, nmeaString, pendingBytes );

        // make sure the string is terminated
        if( status == LMN_STATUS_OK )
        {
            nmeaString[pendingBytes] = 0x00;

            // copy length indication to argument
            *nmeaStringSize = pendingBytes;

            // return success only if there is data to process
            if( pendingBytes > 0 )
            {
                result = true;
            }
        }
    }
    return result;
}

uint8_t PAM7QGetDeviceAddr( void )
{
    return I2cDeviceAddr;
}

void PAM7QSetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t PAM7QWrite( uint8_t addr, uint8_t data )
{
    return PAM7QWriteBuffer( addr, &data, 1 );
}

uint8_t PAM7QWriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteMemBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

uint8_t PAM7QRead( uint8_t addr, uint8_t *data )
{
    return PAM7QReadBuffer( addr, data, 1 );
}

uint8_t PAM7QReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadMemBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}
