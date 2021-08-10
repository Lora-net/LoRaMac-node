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
 * 
 * Modified by Medad Newman for unittesting
 */

#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "utilities.h"
#include "eeprom-board.h"
#include <string.h>
#include "bsp.h"
}


uint8_t simulated_flash[EEPROM_SIZE];

LmnStatus_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    memcpy(&simulated_flash[(int)addr], buffer, size);

    return LMN_STATUS_OK;
}

LmnStatus_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    memcpy(buffer, &simulated_flash[(int)addr], size);

    return LMN_STATUS_OK;
}

void EepromMcuSetDeviceAddr( uint8_t addr )
{
}

LmnStatus_t EepromMcuGetDeviceAddr( void )
{
    return LMN_STATUS_OK;
}
