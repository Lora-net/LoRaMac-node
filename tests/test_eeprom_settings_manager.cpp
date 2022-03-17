/**
 * @file test_eeprom_settings_manager.cpp
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2022-03-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "eeprom_settings_manager.h"
#include "bsp.h"
#include "geofence.h"
}

TEST_GROUP(test_eeprom_settings_manager){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

/**
 * @brief Check if when you send up a new setting for the lorawan network keys, the CRC changes.
 * 
 */
TEST(test_eeprom_settings_manager, test_eeprom_changed_crc)
{
    // Initialy, we should have a blank EEPROM
    CHECK_EQUAL(197, get_settings_crc());

    update_device_tx_interval_in_eeprom(TX_INTERVAL_EEPROM_ADDRESS, 324);

    /* Get the crc8 and check if set correctly */
    CHECK_EQUAL(23, get_settings_crc());
}

/**
 * @brief Check if CRC remains the same update does not change eeprom
 * 
 */
TEST(test_eeprom_settings_manager, test_eeprom_unchanged_crc)
{
    // Initialy, we should have a blank EEPROM, so will return CRC of default values
    CHECK_EQUAL(197, get_settings_crc());

    // Geofence settings are still default values
    bool geofence_settings[N_POLYGONS] = {true, true, true, true, true,
                                          true, true, true, true, true,
                                          true, true, true, true, true,
                                          true, true, true, true, true,
                                          true, false, false};

    update_geofence_settings_in_eeprom((uint8_t *)geofence_settings, sizeof(geofence_settings));

    /* Get the crc8 and confirm that it has the same CRC */
    CHECK_EQUAL(197, get_settings_crc());
}

/**
 * @brief Check if when we update with the same values, the EEPROM does not change.
 * 
 */
TEST(test_eeprom_settings_manager, test_if_crc_remains_unchanged_after_identical_updates)
{

    update_device_tx_interval_in_eeprom(TX_INTERVAL_EEPROM_ADDRESS, 429);
    /* Get the crc8 and check if set correctly */
    CHECK_EQUAL(157, get_settings_crc());

    update_device_tx_interval_in_eeprom(TX_INTERVAL_EEPROM_ADDRESS, 429);
    /* Get the crc8 and check if set correctly */
    CHECK_EQUAL(157, get_settings_crc());
}
