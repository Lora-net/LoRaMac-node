/**
 * @file      i2c-board.c
 *
 * @brief     Target board I2C driver implementation
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 * Revised BSD License
 * Copyright The Things Industries B.V 2020. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Things Industries B.V nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE THINGS INDUSTRIES B.V BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <peripheral_clk_config.h>
#include <hal_gpio.h>
#include <hal_i2c_m_sync.h>

#include "board.h"
#include "i2c-board.h"

struct i2c_m_sync_desc I2C_INSTANCE;

void I2cMcuInit( I2c_t* obj, I2cId_t i2cId, PinNames scl, PinNames sda )
{
    obj->I2cId = i2cId;

    // Clock initialization
    hri_gclk_write_PCHCTRL_reg( GCLK, SERCOM1_GCLK_ID_CORE,
                                CONF_GCLK_SERCOM1_CORE_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );
    hri_gclk_write_PCHCTRL_reg( GCLK, SERCOM1_GCLK_ID_SLOW,
                                CONF_GCLK_SERCOM1_SLOW_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );

    hri_mclk_set_APBCMASK_SERCOM1_bit( MCLK );

    // I2c initialization
    i2c_m_sync_init( &I2C_INSTANCE, SERCOM1 );

    gpio_set_pin_function( sda, PINMUX_PA16C_SERCOM1_PAD0 );
    gpio_set_pin_function( scl, PINMUX_PA17C_SERCOM1_PAD1 );

    i2c_m_sync_enable( &I2C_INSTANCE );
}

void I2cMcuDeInit( I2c_t* obj )
{
    // Left empty
}

void I2cMcuFormat( I2c_t* obj, I2cMode mode, I2cDutyCycle dutyCycle, bool I2cAckEnable, I2cAckAddrMode AckAddrMode,
                   uint32_t I2cFrequency )
{
    // configured via hpl_sercom_config.h
    return;
}

uint8_t I2cMcuWriteBuffer( I2c_t *obj, uint8_t deviceAddr, uint8_t *buffer, uint16_t size )
{
    i2c_m_sync_set_slaveaddr( &I2C_INSTANCE, deviceAddr, I2C_M_SEVEN );
    if( io_write( &I2C_INSTANCE.io, buffer, size ) == size )
    {
        return 1;  // ok
    }
    else
    {
        return 0;  // something went wrong
    }
}

uint8_t I2cMcuReadBuffer( I2c_t *obj, uint8_t deviceAddr, uint8_t *buffer, uint16_t size )
{
    i2c_m_sync_set_slaveaddr( &I2C_INSTANCE, deviceAddr, I2C_M_SEVEN );
    if( io_read( &I2C_INSTANCE.io, buffer, size ) == size )
    {
        return 1;  // ok
    }
    else
    {
        return 0;  // something went wrong
    }
}

uint8_t I2cMcuWriteMemBuffer( I2c_t* obj, uint8_t deviceAddr, uint16_t addr, uint8_t* buffer, uint16_t size )
{
    i2c_m_sync_set_slaveaddr( &I2C_INSTANCE, deviceAddr, I2C_M_SEVEN );
    if( i2c_m_sync_cmd_write( &I2C_INSTANCE, addr, buffer, size ) == size )
    {
        return 1;  // ok
    }
    else
    {
        return 0;  // something went wrong
    }
}

uint8_t I2cMcuReadMemBuffer( I2c_t* obj, uint8_t deviceAddr, uint16_t addr, uint8_t* buffer, uint16_t size )
{
    i2c_m_sync_set_slaveaddr( &I2C_INSTANCE, deviceAddr, I2C_M_SEVEN );
    if( i2c_m_sync_cmd_read( &I2C_INSTANCE, addr, buffer, size ) == size )
    {
        return 1;  // ok
    }
    else
    {
        return 0;  // something went wrong
    }
}