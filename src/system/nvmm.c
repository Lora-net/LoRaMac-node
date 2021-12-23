/*!
 * \file      nvmm.c
 *
 * \brief     None volatile memory management module
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
 *              (C)2013-2020 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 */

#include <stdint.h>

#include "utilities.h"
#include "eeprom-board.h"
#include "nvmm.h"
#include <string.h>
#include "print_utils.h"
#include <stdio.h>

static void eeprom_write_workaround(uint16_t offset);
static bool NvmmUpdateByte(uint8_t *to_write_byte, uint16_t offset);


uint16_t NvmmWrite( uint8_t* src, uint16_t size, uint16_t offset )
{
    eeprom_write_workaround( offset );

    if( EepromMcuWriteBuffer( offset, src, size ) == LMN_STATUS_OK )
    {
        return size;
    }
    return 0;
}

uint16_t NvmmUpdate(uint8_t *src, uint16_t size, uint16_t offset)
{
    uint16_t counter = 0;

    for (uint16_t i = 0; i < size; i++)
    {

        bool ret;
        ret = NvmmUpdateByte((uint8_t *)&src[i], i + offset);

        if (ret == true)
        {
            counter++;
        }
    }

    return counter;
}

bool NvmmUpdateByte(uint8_t *to_write_byte, uint16_t offset)
{
    uint8_t existing_byte;
    EepromMcuReadBuffer(offset, &existing_byte, 1);

    if (existing_byte != *to_write_byte)
    {
        /** 
         * Write twice as workaround. For some reason, it does not
         * write the first time.
         */
        EepromMcuWriteBuffer(offset, to_write_byte, 1);
        EepromMcuWriteBuffer(offset, to_write_byte, 1);
        return true;
    }
    else
    {
        return false;
    }
}


uint16_t NvmmRead( uint8_t* dest, uint16_t size, uint16_t offset )
{
    if( EepromMcuReadBuffer( offset, dest, size ) == LMN_STATUS_OK )
    {
        return size;
    }
    return 0;
}

bool is_crc_correct(uint16_t size, void *input_struct)
{
    uint8_t *p = input_struct;
    uint8_t data = 0;
    uint32_t calculatedCrc32 = 0;
    uint32_t readCrc32 = 0;

    memcpy(&readCrc32, &p[size - sizeof(readCrc32)], sizeof(readCrc32));

    // Calculate crc
    calculatedCrc32 = Crc32Init();

    for (uint16_t i = 0; i < (size - sizeof(readCrc32)); i++)
    {
        data = p[i];
        calculatedCrc32 = Crc32Update(calculatedCrc32, &data, 1);
    }
    calculatedCrc32 = Crc32Finalize(calculatedCrc32);

    if (calculatedCrc32 != readCrc32)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool NvmmCrc32Check( uint16_t size, uint16_t offset )
{
    uint8_t data = 0;
    uint32_t calculatedCrc32 = 0;
    uint32_t readCrc32 = 0;

    if( NvmmRead( ( uint8_t* ) &readCrc32, sizeof( readCrc32 ),
                  ( offset + ( size - sizeof( readCrc32 ) ) ) ) == sizeof( readCrc32 ) )
    {
        // Calculate crc
        calculatedCrc32 = Crc32Init( );
        for( uint16_t i = 0; i < ( size - sizeof( readCrc32 ) ); i++ )
        {
            if( NvmmRead( &data, 1, offset + i ) != 1 )
            {
                return false;
            }
            calculatedCrc32 = Crc32Update( calculatedCrc32, &data, 1 );
        }
        calculatedCrc32 = Crc32Finalize( calculatedCrc32 );

        if( calculatedCrc32 != readCrc32 )
        {
            return false;
        }
    }
    return true;
}

bool NvmmReset( uint16_t size, uint16_t offset )
{
    uint32_t crc32 = 0;

    if( EepromMcuWriteBuffer( offset + size - sizeof( crc32 ),
                              ( uint8_t* ) &crc32, sizeof( crc32 ) ) == LMN_STATUS_OK )
    {
        return true;
    }
    return false;
}

#define EEPROM_SIZE 0x17FFUL

bool EEPROM_Wipe(void)
{

    uint32_t eeprom_size = EEPROM_SIZE;
    uint8_t reset_value = 0;

    while (eeprom_size--)
    {

        if (EepromMcuWriteBuffer(eeprom_size, &reset_value, sizeof(reset_value)) != LMN_STATUS_OK)
        {
            return false;
        }
    }

    return true;
}

#define DATA_EEPROM_BASE ((uint32_t)0x08080000U)
bool EEPROM_Dump(void)
{
    print_bytes((uint8_t *)(DATA_EEPROM_BASE), EEPROM_SIZE);

    return true;
}

typedef struct test_s
{
    int32_t num1;
    int32_t num2;

} test_t;


test_t write_data = {
    .num1 = 0x2323,
    .num2 = 0xAAAA};

test_t read_data = {
    .num1 = 23,
    .num2 = 34};

int eeprom_read_write_test()
{
    printf("Doing read write test eeprom\n");

    uint32_t addr = 0;

    NvmmUpdate((uint8_t *)&write_data, sizeof(test_t), addr);
    int read = NvmmRead((uint8_t *)&read_data, sizeof(test_t), addr);

    int res = memcmp(&write_data, &read_data, sizeof(test_t));
    printf("Comparison result %d\n", res);
    printf("Write data:(%d bytes)", sizeof(test_t));
    print_bytes(&write_data, sizeof(test_t));
    printf("Read data:(%d bytes)", read);
    print_bytes(&read_data, sizeof(test_t));
    printf("\n");

    return res;
}

/**
 * @brief It seems like eeprom write does not work unless some garbage is written to it first. 
 * Wierd.
 * 
 */
static void eeprom_write_workaround(uint16_t offset)
{
    uint8_t garbage_value = 0xff;
    EepromMcuWriteBuffer(offset, &garbage_value, 1);
}
