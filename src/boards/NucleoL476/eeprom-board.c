/*!
 * \file      eeprom-board.c
 *
 * \brief     Target board EEPROM driver implementation
 * 
 * \remark    WARNING This driver shouldn't be used in production code.
 *                    Please prefer a driver such the one provided on
 *                    X_CUBE_EEPROM package.
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
#include "stm32l4xx.h"
#include "utilities.h"
#include "eeprom-board.h"

#define DATA_EEPROM_BASE    ( ( uint32_t )0x080F9800U )
#define DATA_EEPROM_END     ( ( uint32_t )DATA_EEPROM_BASE + 2048 )

/*!
 * \brief Erase a page of Flash. Here used to Erase EEPROM region.
 *
 * \param [in]  page          address of page to erase
 * \param [in]  banks         address of banks to erase
 */
static void FlashPageErase( uint32_t page, uint32_t banks )
{
    // Check the parameters
    assert_param( IS_FLASH_PAGE( page ) );
    assert_param( IS_FLASH_BANK_EXCLUSIVE( banks ) );

    if( ( banks & FLASH_BANK_1 ) != RESET )
    {
        CLEAR_BIT( FLASH->CR, FLASH_CR_BKER );
    }
    else
    {
        SET_BIT( FLASH->CR, FLASH_CR_BKER );
    }

    // Proceed to erase the page
    MODIFY_REG( FLASH->CR, FLASH_CR_PNB, ( page << 3 ) );
    SET_BIT( FLASH->CR, FLASH_CR_PER );
    SET_BIT( FLASH->CR, FLASH_CR_STRT );
}

uint8_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    uint8_t status = FAIL;
    uint64_t *flash = ( uint64_t* )buffer;

    assert_param( ( DATA_EEPROM_BASE + addr ) >= DATA_EEPROM_BASE );
    assert_param( buffer != NULL );
    assert_param( size < ( DATA_EEPROM_END - DATA_EEPROM_BASE ) );

    if( HAL_FLASH_Unlock( ) == HAL_OK )
    {
        // Page size equal to 2048. Uses last page of flash bank 2
        FlashPageErase( 255, FLASH_BANK_2 );

        WRITE_REG( FLASH->CR, FLASH_CR_OPTLOCK_Msk );

        for( uint16_t i = 0; i < size; i++ )
        {
            if( HAL_FLASH_Program( FLASH_TYPEPROGRAM_DOUBLEWORD,
                                   ( DATA_EEPROM_BASE + addr + ( 8 * i ) ),
                                   flash[i] ) != HAL_OK )
            {
                // Failed to write EEPROM
                break;
            }
        }
        status = SUCCESS;
    }

    HAL_FLASH_Lock( );
    return status;
}

uint8_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    assert_param( ( DATA_EEPROM_BASE + addr ) >= DATA_EEPROM_BASE );
    assert_param( buffer != NULL );
    assert_param( size < ( DATA_EEPROM_END - DATA_EEPROM_BASE ) );

    memcpy1( buffer, ( uint8_t* )( DATA_EEPROM_BASE + addr ), size );
    return SUCCESS;
}

void EepromMcuSetDeviceAddr( uint8_t addr )
{
    assert_param( FAIL );
}

uint8_t EepromMcuGetDeviceAddr( void )
{
    assert_param( FAIL );
    return 0;
}
