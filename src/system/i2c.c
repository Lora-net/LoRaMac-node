/*!
 * \file  i2c.c
 *
 * \brief I2C driver implementation
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
#include "i2c-board.h"

/*!
 * Flag to indicates if the I2C is initialized
 */
static bool I2cInitialized = false;

void I2cInit( I2c_t *obj, I2cId_t i2cId, PinNames scl, PinNames sda )
{
    if( I2cInitialized == false )
    {
        I2cInitialized = true;

        I2cMcuInit( obj, i2cId, scl, sda );
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
    I2cMcuResetBus( obj );
}

LmnStatus_t I2cWrite( I2c_t *obj, uint8_t deviceAddr, uint8_t data )
{
    if( I2cInitialized == true )
    {
        if( I2cMcuWriteBuffer( obj, deviceAddr, &data, 1 ) == LMN_STATUS_ERROR )
        {
            // if first attempt fails due to an IRQ, try a second time
            if( I2cMcuWriteBuffer( obj, deviceAddr, &data, 1 ) == LMN_STATUS_ERROR )
            {
                return LMN_STATUS_ERROR;
            }
            else
            {
                return LMN_STATUS_OK;
            }
        }
        else
        {
            return LMN_STATUS_OK;
        }
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cWriteBuffer( I2c_t *obj, uint8_t deviceAddr, uint8_t *buffer, uint16_t size )
{
    if( I2cInitialized == true )
    {
        if( I2cMcuWriteBuffer( obj, deviceAddr, buffer, size ) == LMN_STATUS_ERROR )
        {
            // if first attempt fails due to an IRQ, try a second time
            if( I2cMcuWriteBuffer( obj, deviceAddr, buffer, size ) == LMN_STATUS_ERROR )
            {
                return LMN_STATUS_ERROR;
            }
            else
            {
                return LMN_STATUS_OK;
            }
        }
        else
        {
            return LMN_STATUS_OK;
        }
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cWriteMem( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t data )
{
    if( I2cInitialized == true )
    {
        if( I2cMcuWriteMemBuffer( obj, deviceAddr, addr, &data, 1 ) == LMN_STATUS_ERROR )
        {
            // if first attempt fails due to an IRQ, try a second time
            if( I2cMcuWriteMemBuffer( obj, deviceAddr, addr, &data, 1 ) == LMN_STATUS_ERROR )
            {
                return LMN_STATUS_ERROR;
            }
            else
            {
                return LMN_STATUS_OK;
            }
        }
        else
        {
            return LMN_STATUS_OK;
        }
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cWriteMemBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size )
{
    if( I2cInitialized == true )
    {
        if( I2cMcuWriteMemBuffer( obj, deviceAddr, addr, buffer, size ) == LMN_STATUS_ERROR )
        {
            // if first attempt fails due to an IRQ, try a second time
            if( I2cMcuWriteMemBuffer( obj, deviceAddr, addr, buffer, size ) == LMN_STATUS_ERROR )
            {
                return LMN_STATUS_ERROR;
            }
            else
            {
                return LMN_STATUS_OK;
            }
        }
        else
        {
            return LMN_STATUS_OK;
        }
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cRead( I2c_t *obj, uint8_t deviceAddr, uint8_t *data )
{
    if( I2cInitialized == true )
    {
        return( I2cMcuReadBuffer( obj, deviceAddr, data, 1 ) );
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cReadBuffer( I2c_t *obj, uint8_t deviceAddr, uint8_t *buffer, uint16_t size )
{
    if( I2cInitialized == true )
    {
        return( I2cMcuReadBuffer( obj, deviceAddr, buffer, size ) );
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cReadMem( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *data )
{
    if( I2cInitialized == true )
    {
        return( I2cMcuReadMemBuffer( obj, deviceAddr, addr, data, 1 ) );
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cReadMemBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size )
{
    if( I2cInitialized == true )
    {
        return( I2cMcuReadMemBuffer( obj, deviceAddr, addr, buffer, size ) );
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}
