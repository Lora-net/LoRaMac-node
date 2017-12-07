/*!
 * \file      eeprom-board.c
 *
 * \brief     Target board EEPROM driver implementation
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
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "utilities.h"
#include "i2c-board.h"
#include "eeprom-board.h"

#define DEVICE_I2C_ADDRESS                          0xA8

static uint8_t I2cDeviceAddr = DEVICE_I2C_ADDRESS;

#define EE_PAGE_SIZE                                64

extern I2c_t I2c;

uint8_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    uint8_t nbPage = 0;
    uint8_t nbBytes = 0;
    uint8_t nbBytesRemaining = 0;
    uint16_t lAddr = 0;

    lAddr = addr % EE_PAGE_SIZE;
    nbBytesRemaining = EE_PAGE_SIZE - lAddr;
    nbPage =  size / EE_PAGE_SIZE;
    nbBytes = size % EE_PAGE_SIZE;

    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_16 );
    /*!< If lAddr is EE_PAGE_SIZE aligned  */
    if( lAddr == 0 )
    {
        /*!< If size < EE_PAGE_SIZE */
        if( nbPage == 0 )
        {
            if( I2cWriteBuffer( &I2c, I2cDeviceAddr, addr, buffer, size ) == FAIL )
            {
                I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                return FAIL;
            }
            if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
            {
                I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                return FAIL;
            }
        }
        /*!< If size > EE_PAGE_SIZE */
        else
        {
            while( nbPage-- )
            {
                if( I2cWriteBuffer( &I2c, I2cDeviceAddr, addr, buffer, EE_PAGE_SIZE ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                addr +=  EE_PAGE_SIZE;
                buffer += EE_PAGE_SIZE;
            }

            if( nbBytes != 0 )
            {
                if( I2cWriteBuffer( &I2c, I2cDeviceAddr, addr, buffer, nbBytes ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
            }
        }
    }
    /*!< If addr is not EE_PAGE_SIZE aligned  */
    else
    {
        /*!< If size < EE_PAGE_SIZE */
        if( nbPage== 0 )
        {
            /*!< If the number of data to be written is more than the remaining space
            in the current page: */
            if ( size > nbBytesRemaining )
            {
                if( I2cWriteBuffer( &I2c, I2cDeviceAddr, addr, buffer, nbBytesRemaining ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }

                if( I2cWriteBuffer( &I2c, I2cDeviceAddr, ( addr + nbBytesRemaining ),
                                                   ( uint8_t* )( buffer + nbBytesRemaining ),
                                                   ( size - nbBytesRemaining ) ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
            }
            else
            {
                if( I2cWriteBuffer( &I2c, I2cDeviceAddr, addr, buffer, nbBytes ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
            }
        }
        /*!< If size > EE_PAGE_SIZE */
        else
        {
            size -= nbBytesRemaining;
            nbPage =  size / EE_PAGE_SIZE;
            nbBytes = size % EE_PAGE_SIZE;

            if( nbBytesRemaining != 0 )
            {
                if( I2cWriteBuffer( &I2c, I2cDeviceAddr, addr, buffer, nbBytesRemaining ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                addr += nbBytesRemaining;
                buffer += nbBytesRemaining;
            }

            while( nbPage-- )
            {
                if( I2cWriteBuffer( &I2c, I2cDeviceAddr, addr, buffer, EE_PAGE_SIZE ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                addr +=  EE_PAGE_SIZE;
                buffer += EE_PAGE_SIZE;
            }
            if( nbBytes != 0 )
            {
                if( I2cWriteBuffer( &I2c, I2cDeviceAddr, addr, buffer, nbBytes ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
                if( I2cMcuWaitStandbyState( &I2c, I2cDeviceAddr ) == FAIL )
                {
                    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
                    return FAIL;
                }
            }
        }
    }
    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );
    return SUCCESS;
}

uint8_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    uint8_t status = FAIL;

    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_16 );

    status = I2cReadBuffer( &I2c, I2cDeviceAddr, addr, buffer, size );

    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_8 );

    return status;
}

void EepromMcuSetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t EepromMcuGetDeviceAddr( void )
{
    return I2cDeviceAddr;
}
