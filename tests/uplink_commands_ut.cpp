/**
 * @file uplink_commands_ut.cpp
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-10-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "main.h"
#include "NvmDataMgmt.h"
#include "eeprom-board.h"
}

#include "nvm_images.hpp"
#include "string.h"

void set_correct_notify_flags();

TEST_GROUP(uplink_commands){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

#define SIZE_OF_NETWORK_KEYS_T 52
#define LOCATION 12

TEST(uplink_commands, test_keys_set)
{
    CHECK_EQUAL(1, sizeof(registered_devices_t)); // just check the size of the enum
    CHECK_EQUAL(icspace26_us1_us915_device_1, LOCATION);

    uint8_t simulated_buffer[SIZE_OF_NETWORK_KEYS_T + 1] =
        {
            0x47, 0x18, 0xA9, 0x3A, 0x91, 0x7E, 0xB6, 0x1A, 0xFD, 0xB3, 0x78, 0x6E, 0xA0, 0x4E, 0xC3, 0xEE, // FNwkSIntKey_SNwkSIntKey_NwkSEncKey
            0xD3, 0xAB, 0xC3, 0x3C, 0x12, 0xD9, 0x75, 0xF2, 0x78, 0x5F, 0xFA, 0x46, 0xAF, 0x75, 0x95, 0xE2, // AppSKey
            0x99, 0x4E, 0x0C, 0x26,                                                                         // DevAddr for Things Network
            0x03, 0x00, 0x00, 0x00,                                                                         // frame_count of 3
            0xE8, 0x03, 0x00, 0x00,                                                                         // ReceiveDelay1
            0xD0, 0x07, 0x00, 0x00,                                                                         // ReceiveDelay2
            0xD4, 0x09, 0x3E, 0x2F,                                                                         // Crc32
            LOCATION,                                                                                       // device

        };

    LmHandlerAppData_t appData = {
        .Port = 19,
        .BufferSize = SIZE_OF_NETWORK_KEYS_T + 1,
        .Buffer = simulated_buffer,
    };

    LmHandlerRxParams_t LmHandlerRxParams = {
        .Status = LORAMAC_EVENT_INFO_STATUS_OK,
    };

    OnRxData(&appData, &LmHandlerRxParams);

    // Now check the EEPROM if its all correctly set
    uint8_t current_eeprom[SIZE_OF_NETWORK_KEYS_T];
    uint8_t expected_eeprom[SIZE_OF_NETWORK_KEYS_T] = {
        0x47, 0x18, 0xA9, 0x3A, 0x91, 0x7E, 0xB6, 0x1A, 0xFD, 0xB3, 0x78, 0x6E, 0xA0, 0x4E, 0xC3, 0xEE, // FNwkSIntKey_SNwkSIntKey_NwkSEncKey
        0xD3, 0xAB, 0xC3, 0x3C, 0x12, 0xD9, 0x75, 0xF2, 0x78, 0x5F, 0xFA, 0x46, 0xAF, 0x75, 0x95, 0xE2, // AppSKey
        0x99, 0x4E, 0x0C, 0x26,                                                                         // DevAddr for Things Network
        0x03, 0x00, 0x00, 0x00,                                                                         // frame_count of 3
        0xE8, 0x03, 0x00, 0x00,                                                                         // ReceiveDelay1
        0xD0, 0x07, 0x00, 0x00,                                                                         // ReceiveDelay2
        0xD4, 0x09, 0x3E, 0x2F,                                                                         // Crc32
    };

    EepromMcuReadBuffer(LOCATION * SIZE_OF_NETWORK_KEYS_T, current_eeprom, SIZE_OF_NETWORK_KEYS_T);

    MEMCMP_EQUAL(expected_eeprom, current_eeprom, SIZE_OF_NETWORK_KEYS_T); // should fail
}
