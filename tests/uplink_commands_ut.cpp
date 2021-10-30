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
#include "callbacks.h"
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

#define LOCATION 12

/**
 * @brief Verifies the size of the network keys. VERY IMPORTANT TO PASS
 * 
 * 
 */
TEST(uplink_commands, verify_sizeof_network_keys_struct)
{
    CHECK_EQUAL(SIZE_OF_NETWORK_KEYS_T, sizeof(network_keys_t)); // just check the size of the enum
}

/**
 * @brief Verify location of specify registration keys
 * 
 */
TEST(uplink_commands, verify_location_of_specify_registration_keys)
{
    CHECK_EQUAL(icspace26_us1_us915_device_1, LOCATION);
}

/**
 * @brief Test whether the tracker can accept a message from ground telling it to 
 * change the keys in the EEPROM. The test verifies if correctly parses the message
 * from ground, and the checks if correctly written to EEPROM.
 * 
 */
TEST(uplink_commands, test_keys_set)
{

    /**
     * @brief Simulate a downlink from ground, requesting a change for the icspace26_us1_us915_device_1
     * keys
     * 
     */
    uint8_t simulated_downlink_from_ground[SIZE_OF_NETWORK_KEYS_T + 1] =
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

    /**
     * @brief Fill the required structs for the downlink processor
     */
    LmHandlerAppData_t appData = {
        .Port = 19,
        .BufferSize = SIZE_OF_NETWORK_KEYS_T + 1,
        .Buffer = simulated_downlink_from_ground,
    };

    LmHandlerRxParams_t LmHandlerRxParams = {
        .Status = LORAMAC_EVENT_INFO_STATUS_OK,
    };

    /**
     * @brief Handle the downlink from ground
     */
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

    /**
     * @brief Read the eeprom to check if its correctly set
     * 
     * @param SIZE_OF_NETWORK_KEYS_T 
     */
    EepromMcuReadBuffer(LOCATION * SIZE_OF_NETWORK_KEYS_T, current_eeprom, SIZE_OF_NETWORK_KEYS_T);

    MEMCMP_EQUAL(expected_eeprom, current_eeprom, SIZE_OF_NETWORK_KEYS_T); // should fail
}

/**
 * @brief Test whether the tracker correctly sets the telemetry flag to indicate
 * eeprom values have changed. It should send set eeprom changed true flag, as the 
 * eeprom was all set to zero prior to setting these bytes
 */
TEST(uplink_commands, test_eeprom_changed_ack)
{
    /**
     * @brief Simulate a downlink from ground, requesting a change for the icspace26_us1_us915_device_1
     * keys
     * 
     */
    uint8_t simulated_downlink_from_ground[SIZE_OF_NETWORK_KEYS_T + 1] =
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

    /**
     * @brief Fill the required structs for the downlink processor
     */
    LmHandlerAppData_t appData = {
        .Port = 19,
        .BufferSize = SIZE_OF_NETWORK_KEYS_T + 1,
        .Buffer = simulated_downlink_from_ground,
    };

    LmHandlerRxParams_t LmHandlerRxParams = {
        .Status = LORAMAC_EVENT_INFO_STATUS_OK,
    };

    /**
     * @brief Handle the downlink from ground
     */
    OnRxData(&appData, &LmHandlerRxParams);

    /**
     * Now verify if the bitfield flags have been set
     */

    sensor_t current_sensor_data = get_current_sensor_data();

    /* Get bitfield flags and check if correctly set */
    /* Expect eeprom changed flag to be set: 0b00000010 = 0x02 */
    CHECK_EQUAL(0x2, current_sensor_data.status_bitfields);
}
