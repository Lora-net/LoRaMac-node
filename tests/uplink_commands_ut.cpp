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
#include "nvmm.h"
#include "geofence.h"
#include "eeprom_settings_manager.h"
}

#include "nvm_images.hpp"
#include "string.h"

void set_correct_notify_flags();

LmHandlerRxParams_t LmHandlerRxParams = {
    .Status = LORAMAC_EVENT_INFO_STATUS_OK,
};

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
TEST(uplink_commands, MUST_PASS_verify_sizeof_network_keys_struct)
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
TEST(uplink_commands, test_eeprom_keys_set_correctly)
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
 * @brief Test if an uplink to poll a specific time range of past position can return the
 * ack if the date range is available in EEPROM. We expect it to nack here because the
 * simulated EEPROM has no past data in it.
 */
IGNORE_TEST(uplink_commands, test_get_time_range_of_past_positions_nak)
{
    /**
     * @brief Simulate a downlink from ground, requesting a past position
     * range from 2021-10-15 10:30:01 to 2021-10-15 11:55:01
     */

    uint8_t length_of_uplink = 8;
    uint8_t simulated_downlink_from_ground[length_of_uplink] =
        {
            0x72, 0x5b, 0x0e, 0x00, // '2021-10-15 10:30:01'
            0x1d, 0x5b, 0x0e, 0x00, // '2021-10-15 11:55:01'
        };

    /**
     * @brief Fill the required structs for the downlink processor
     */
    LmHandlerAppData_t appData = {
        .Port = 18,
        .BufferSize = length_of_uplink,
        .Buffer = simulated_downlink_from_ground,
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
    /* Expect eeprom changed flag to be set: 0b00000001 = 0x01 */
    CHECK_EQUAL(0x01, current_sensor_data.status_bitfields);
}

/**
 * @brief Test if an uplink to poll a specific time range of past position can return the
 * ack if the date range is available in EEPROM. We expect it to ack here as the request does
 * match the data in the EEPROM
 */

extern eeprom_playback_stats_t eeprom_playback_stats;

IGNORE_TEST(uplink_commands, test_get_time_range_of_past_positions_ack)
{
    /**
     * @brief Set the EEPROM to have some past data
     * 
     */

    eeprom_playback_stats = {
        .current_EEPROM_index = 27 * PLAYBACK_EEPROM_PACKET_SIZE,
        .n_playback_positions_saved = 27,
    };

    uint16_t past_data_size = 9 * 27;
    EepromMcuWriteBuffer(PLAYBACK_EEPROM_ADDR_START, past_saved_data, past_data_size);

    /**
     * @brief Simulate a downlink from ground, requesting a past position
     * range from 2021-11-4 15:30:01 to 2021-11-4 19:30:01
     */

    uint8_t length_of_uplink = 8;
    uint8_t simulated_downlink_from_ground[length_of_uplink] =
        {
            0xf5, 0xcd, 0x0e, 0x00, // 2021-11-4 15:30:01
            0x05, 0xcd, 0x0e, 0x00, // 2021-11-4 19:30:01
        };

    /**
     * @brief Fill the required structs for the downlink processor
     */
    LmHandlerAppData_t appData = {
        .Port = 18,
        .BufferSize = length_of_uplink,
        .Buffer = simulated_downlink_from_ground,
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
    /* Expect eeprom changed flag to be set: 0b00000100 = 0x04 */
    CHECK_EQUAL(0x04, current_sensor_data.status_bitfields);
}

/**
 * @brief Check if can successfully set tx interval 
 */
TEST(uplink_commands, test_set_tx_interval_success)
{
    /**
     * @brief Simulate a downlink from ground, requesting a past position
     * range from 2021-11-4 15:30:01 to 2021-11-4 19:30:01
     */

    uint32_t target_interval = 15000;

    uint8_t length_of_uplink = 4;
    uint8_t simulated_downlink_from_ground[length_of_uplink];

    /**
     * @brief Set the simulated downlink
     * 
     */
    memcpy(simulated_downlink_from_ground, &target_interval, sizeof(uint32_t));

    /**
     * @brief Fill the required structs for the downlink processor
     */
    LmHandlerAppData_t appData = {
        .Port = 20,
        .BufferSize = length_of_uplink,
        .Buffer = simulated_downlink_from_ground,
    };

    /**
     * @brief Handle the downlink from ground
     */
    OnRxData(&appData, &LmHandlerRxParams);

    /**
     * Now verify if the bitfield flags have been set
     */

    sensor_t current_sensor_data = get_current_sensor_data();

    /**
     * @brief Check if EEPROM stored value has correctly been
     */
    CHECK_EQUAL(target_interval, read_tx_interval_in_eeprom(TX_INTERVAL_EEPROM_ADDRESS, TX_INTERVAL_GPS_FIX_OK));
}

/**
 * @brief Verify that the Geofence mask can be changed in EEPROM
 * 
 */
TEST(uplink_commands, verify_geofence_mask_is_changed_correctly)
{

    /**
     * @brief Simulate a downlink from ground containing a message to enable
     * transmissions over Ukraine
     * 
     */
    uint8_t length_of_uplink = N_POLYGONS * sizeof(bool);
    uint8_t simulated_downlink_from_ground[length_of_uplink];

    bool instructions[N_POLYGONS] = {
        true, true, true, true, true, true,
        true, true, true, true, true, true,
        true, true, true, true, true, true,
        true, true, true, true, false};

    /**
     * @brief Set the simulated downlink
     * 
     */
    memcpy(simulated_downlink_from_ground, instructions, length_of_uplink);

    /**
     * @brief Fill the required structs for the downlink processor
     */
    LmHandlerAppData_t appData = {
        .Port = 22,
        .BufferSize = length_of_uplink,
        .Buffer = simulated_downlink_from_ground,
    };

    /**
     * @brief Handle the downlink from ground
     */
    OnRxData(&appData, &LmHandlerRxParams);

    bool values[N_POLYGONS];
    read_geofence_settings_in_eeprom(values);

    update_geofence_position(50.4501, 30.5234); // Ukraine
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}
