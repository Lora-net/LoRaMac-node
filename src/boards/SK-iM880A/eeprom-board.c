/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Timer objects and scheduling management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

#include "eeprom-board.h"

uint8_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    uint8_t status = FAIL;

    assert_param( ( FLASH_EEPROM_BASE + addr ) >= FLASH_EEPROM_BASE );
    assert_param( buffer != NULL );
    assert_param( size < ( FLASH_EEPROM_END - FLASH_EEPROM_BASE ) );

    if( HAL_FLASHEx_DATAEEPROM_Unlock( ) == HAL_OK )
    {
        for( uint16_t i = 0; i < size; i++ )
        {
            if( HAL_FLASHEx_DATAEEPROM_Program( FLASH_TYPEPROGRAMDATA_BYTE,
                                                ( FLASH_EEPROM_BASE + addr + i ),
                                                  buffer[i] ) != HAL_OK )
            {
                // Failed to write EEPROM
                break;
            }
        }
        status = SUCCESS;
    }

    HAL_FLASHEx_DATAEEPROM_Lock( );
    return status;
}

uint8_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    assert_param( ( FLASH_EEPROM_BASE + addr ) >= FLASH_EEPROM_BASE );
    assert_param( buffer != NULL );
    assert_param( size < ( FLASH_EEPROM_END - FLASH_EEPROM_BASE ) );

    memcpy1( buffer, ( uint8_t* )( FLASH_EEPROM_BASE + addr ), size );
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
