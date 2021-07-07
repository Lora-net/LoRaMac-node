/*!
 * \file      i2c-board.c
 *
 * \brief     Target board I2C driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 */
#include "asf.h"

#include "utilities.h"
#include "board-config.h"
#include "i2c-board.h"

static struct i2c_master_module i2c_master_instance;

void I2cMcuInit( I2c_t* obj, I2cId_t i2cId, PinNames scl, PinNames sda )
{
    obj->I2cId = i2cId;

    /* init i2c master in full speed mode*/
    struct i2c_master_config config_i2c;

    i2c_master_get_config_defaults( &config_i2c );

    config_i2c.buffer_timeout = 10000;
    config_i2c.baud_rate      = I2C_MASTER_BAUD_RATE_400KHZ;

    /* Change pins */
    config_i2c.pinmux_pad0 = PINMUX_PA16C_SERCOM1_PAD0;
    config_i2c.pinmux_pad1 = PINMUX_PA17C_SERCOM1_PAD1;

    /* Initialize and enable device with config */
    i2c_master_disable( &i2c_master_instance );

    i2c_master_init( &i2c_master_instance, SERCOM1, &config_i2c );

    i2c_master_enable( &i2c_master_instance );
}

void I2cMcuDeInit( I2c_t* obj )
{
    // Left empty
}

void I2cMcuFormat( I2c_t* obj, I2cMode mode, I2cDutyCycle dutyCycle, bool I2cAckEnable, I2cAckAddrMode AckAddrMode,
                   uint32_t I2cFrequency )
{
    // configured via i2c_master.h defaults
}

LmnStatus_t I2cMcuWriteBuffer( I2c_t* obj, uint8_t deviceAddr, uint8_t* buffer, uint16_t size )
{
    uint32_t                 timeout_cycles = 1000;
    struct i2c_master_packet master_packet  = {
        .address         = deviceAddr,
        .data_length     = size,
        .data            = buffer,
        .ten_bit_address = false,
        .high_speed      = false,
        .hs_master_code  = 0x0,
    };

    while( i2c_master_write_packet_wait( &i2c_master_instance, &master_packet ) != STATUS_OK )
    {
        if( timeout_cycles-- == 0 )
        {
            return LMN_STATUS_ERROR;
        }
    }

    if( timeout_cycles > 0 )
    {
        return LMN_STATUS_OK;
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cMcuReadBuffer( I2c_t* obj, uint8_t deviceAddr, uint8_t* buffer, uint16_t size )
{
    uint32_t                 timeout_cycles = 1000;
    struct i2c_master_packet master_packet  = {
        .address         = deviceAddr,
        .data_length     = size,
        .data            = buffer,
        .ten_bit_address = false,
        .high_speed      = false,
        .hs_master_code  = 0x0,
    };

    timeout_cycles = 1000;
    while( i2c_master_read_packet_wait( &i2c_master_instance, &master_packet ) != STATUS_OK )
    {
        if( timeout_cycles-- == 0 )
        {
            return LMN_STATUS_ERROR;
        }
    }

    if( timeout_cycles > 0 )
    {
        return LMN_STATUS_OK;
    }
    else
    {
        return LMN_STATUS_ERROR;
    }
}

LmnStatus_t I2cMcuWriteMemBuffer( I2c_t* obj, uint8_t deviceAddr, uint16_t addr, uint8_t* buffer, uint16_t size )
{
    uint8_t                  local_addr     = ( uint8_t ) addr;
    uint32_t                 timeout_cycles = 1000;
    struct i2c_master_packet master_packet  = {
        .address         = deviceAddr,
        .data_length     = 1,
        .data            = &local_addr,
        .ten_bit_address = false,
        .high_speed      = false,
        .hs_master_code  = 0x0,
    };

    while( i2c_master_write_packet_wait_no_stop( &i2c_master_instance, &master_packet ) != STATUS_OK )
    {
        if( timeout_cycles-- == 0 )
        {
            return LMN_STATUS_ERROR;
        }
    }

    timeout_cycles            = 1000;
    master_packet.data_length = size;
    master_packet.data        = buffer;
    while( i2c_master_write_packet_wait( &i2c_master_instance, &master_packet ) != STATUS_OK )
    {
        if( timeout_cycles-- == 0 )
        {
            return LMN_STATUS_ERROR;
        }
    }
    return LMN_STATUS_OK;
}

LmnStatus_t I2cMcuReadMemBuffer( I2c_t* obj, uint8_t deviceAddr, uint16_t addr, uint8_t* buffer, uint16_t size )
{
    uint8_t  local_addr     = ( uint8_t ) addr;
    uint32_t timeout_cycles = 1000;

    struct i2c_master_packet master_packet = {
        .address         = deviceAddr,
        .data_length     = 1,
        .data            = &local_addr,
        .ten_bit_address = false,
        .high_speed      = false,
        .hs_master_code  = 0x0,
    };

    while( i2c_master_write_packet_wait_no_stop( &i2c_master_instance, &master_packet ) != STATUS_OK )
    {
        if( timeout_cycles-- == 0 )
        {
            return LMN_STATUS_ERROR;
        }
    }

    timeout_cycles            = 1000;
    master_packet.data_length = size;
    master_packet.data        = buffer;
    while( i2c_master_read_packet_wait( &i2c_master_instance, &master_packet ) != STATUS_OK )
    {
        if( timeout_cycles-- == 0 )
        {
            return LMN_STATUS_ERROR;
        }
    }
    return LMN_STATUS_OK;
}
