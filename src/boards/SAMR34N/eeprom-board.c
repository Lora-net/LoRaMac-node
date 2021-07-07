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
#include "asf.h"

#include "utilities.h"
#include "eeprom-board.h"

LmnStatus_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    return ( rww_eeprom_emulator_write_buffer( addr, buffer, size ) == STATUS_OK ) ? LMN_STATUS_OK : LMN_STATUS_ERROR;
}

LmnStatus_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    return ( rww_eeprom_emulator_read_buffer( addr, buffer, size ) == STATUS_OK ) ? LMN_STATUS_OK : LMN_STATUS_ERROR;
}

void EepromMcuSetDeviceAddr( uint8_t addr )
{
    while( 1 )
    {
    }
}

LmnStatus_t EepromMcuGetDeviceAddr( void )
{
    while( 1 )
    {
    }
//    return 0;
}
