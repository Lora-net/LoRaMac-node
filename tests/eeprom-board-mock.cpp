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
#include "nvm_images.hpp"

uint8_t simulated_flash[EEPROM_SIZE];

LmnStatus_t EepromMcuWriteBuffer(uint16_t addr, uint8_t *buffer, uint16_t size)
{
    memcpy(&simulated_flash[(int)addr], buffer, size);

    return LMN_STATUS_OK;
}

LmnStatus_t EepromMcuReadBuffer(uint16_t addr, uint8_t *buffer, uint16_t size)
{
    memcpy(buffer, &simulated_flash[(int)addr], size);

    return LMN_STATUS_OK;
}

void EepromMcuSetDeviceAddr(uint8_t addr)
{
}

LmnStatus_t EepromMcuGetDeviceAddr(void)
{
    return LMN_STATUS_OK;
}

void fake_eeprom_set(void)
{
    memcpy(simulated_flash, proper_compressed_nvm_eeprom_image, EEPROM_SIZE);
}

void fake_eeprom_set_target_image(uint8_t *target_image)
{
    memcpy(simulated_flash, target_image, EEPROM_SIZE);
}

uint32_t deserialize_uint32(unsigned char *buffer, uint32_t loc);
bool HAL_FLASHEx_DATAEEPROM_Program(uint32_t TypeProgram, uint32_t Address, uint32_t Data);
/** @defgroup FLASHEx_Type_Program_Data FLASHEx Type Program Data
  * @{
  */
#define FLASH_TYPEPROGRAMDATA_BYTE ((uint32_t)0x00U)     /*!<Program byte (8-bit) at a specified address.*/
#define FLASH_TYPEPROGRAMDATA_HALFWORD ((uint32_t)0x01U) /*!<Program a half-word (16-bit) at a specified address.*/
#define FLASH_TYPEPROGRAMDATA_WORD ((uint32_t)0x02U)     /*!<Program a word (32-bit) at a specified address.*/
#define DATA_EEPROM_BASE ((uint32_t)0x08080000U)

uint32_t deserialize_uint32(unsigned char *buffer, uint32_t loc)
{
    uint32_t value = 0;

    value |= buffer[loc + 3] << 24;
    value |= buffer[loc + 2] << 16;
    value |= buffer[loc + 1] << 8;
    value |= buffer[loc + 0];
    return value;
}

/**
 * @brief Write words(4 bytes) at a time
 * 
 * @param addr 
 * @param buffer 
 * @param size 
 * @return LmnStatus_t 
 */
LmnStatus_t EepromMcuWriteBufferWord(uint16_t addr, uint8_t *buffer, uint16_t size)
{
    LmnStatus_t status = LMN_STATUS_ERROR;

    for (uint16_t i = 0; i < size; i += 4)
    {
        uint32_t write = deserialize_uint32(buffer, i);
        if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, (addr + i), write) != true)
        {
            // Failed to write EEPROM
            break;
        }
    }
    status = LMN_STATUS_OK;

    return status;
}

bool HAL_FLASHEx_DATAEEPROM_Program(uint32_t TypeProgram, uint32_t Address, uint32_t Data)
{

    if (TypeProgram == FLASH_TYPEPROGRAMDATA_WORD)
    {
        /* Program word (32-bit) at a specified address.*/
        memcpy(&simulated_flash[Address], &Data, sizeof(uint32_t));
    }
    else if (TypeProgram == FLASH_TYPEPROGRAMDATA_HALFWORD)
    {
        /* Program halfword (16-bit) at a specified address.*/
        memcpy(&simulated_flash[Address], &Data, sizeof(uint16_t));
    }
    else if (TypeProgram == FLASH_TYPEPROGRAMDATA_BYTE)
    {
        /* Program byte (8-bit) at a specified address.*/
        memcpy(&simulated_flash[Address], &Data, sizeof(uint8_t));
    }
    else
    {
        return false;
    }
    return true;
}