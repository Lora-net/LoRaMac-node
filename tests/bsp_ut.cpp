#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "bsp.h"
#include "ublox.h"
#include "nvmm.h"
#include "playback.h"
#include "print_utils.h"
#include "geofence.h"
#include "LoRaMac.h"
#include <math.h> /* fmod */
}

void sensor_read_and_printout(uint32_t number_of_readings);

TEST_GROUP(bsp_ut){
    void setup(){

        EEPROM_Wipe(0, EEPROM_SIZE);
}

void teardown()
{
    mock().clear();
}
}
;

TEST(bsp_ut, test_write_read)
{
    uint8_t write_data = 0x88;
    uint8_t flash_address = (uint16_t)0;
    NvmmUpdate(&write_data, 1, flash_address);

    uint8_t read_data = 0;
    NvmmRead(&read_data, 1, flash_address);
    CHECK_EQUAL(write_data, read_data);
}

gps_info_t gps_info_mock = {
    .GPS_UBX_latitude_Float = 53.23,
    .GPS_UBX_longitude_Float = 0.02,
    .unix_time = 1627938039,
    .latest_gps_status = GPS_SUCCESS,

};

TEST(bsp_ut, bsp_init)
{
    mock().expectNCalls(1, "get_latest_gps_info").andReturnValue(&gps_info_mock);

    BSP_sensor_Init();
    mock().checkExpectations();
}

TEST(bsp_ut, bsp_sensor_read)
{
    mock().expectNCalls(1, "get_latest_gps_info").andReturnValue(&gps_info_mock);
    BSP_sensor_Init();
    mock().checkExpectations();

    mock().expectNCalls(2, "get_latest_gps_info").andReturnValue(&gps_info_mock);
    BSP_sensor_Read();
    mock().checkExpectations();
}

TEST(bsp_ut, manage_incoming_instruction_from_ground_out_of_range)
{
    uint8_t test_instruction[] = {0x4e, 0xb3, 0x07, 0x00, 0x75, 0xaf, 0x07, 0x00};
    bool success = manage_incoming_instruction(test_instruction);

    CHECK_FALSE(success);
}

/**
 * @brief Test traversing several countries
 * 
 */
TEST(bsp_ut, multicountry_simulation)
{
    /* Check if after a reset, it sends down the right stuff */
    mock().expectOneCall("get_latest_gps_info").andReturnValue(&gps_info_mock);
    BSP_sensor_Init();
    sensor_read_and_printout(500);

    mock().expectOneCall("get_latest_gps_info").andReturnValue(&gps_info_mock);
    BSP_sensor_Init();
    sensor_read_and_printout(5000);

    FAIL("Fail multi country simulation");
}

void sensor_read_and_printout(uint32_t number_of_readings)
{
    for (int i = 0; i <= number_of_readings; i++)
    {

        float latitude = 53.23;
        int longitude = i % 360 - 180;

        gps_info_t world_trip_mock = {
            .GPS_UBX_latitude_Float = latitude,
            .GPS_UBX_longitude_Float = (float)longitude,
            .GPSaltitude_mm = 12342000,
            .unix_time = 1627938039 + 60 * 60 * i, /* travel one degree longitude every day */
            .latest_gps_status = GPS_SUCCESS,

        };
        mock().expectOneCall("get_latest_gps_info").andReturnValue(&world_trip_mock);
        BSP_sensor_Read();

        PicoTrackerAppData_t data = prepare_tx_buffer();

        // Print out buffer for debug
        printf("Buffer to tx:\n");
        print_bytes(data.Buffer, data.BufferSize);
        printf("tx_str_buffer_len: %d\n\n", data.BufferSize);
    }
}

/**
 * @brief Verify sizes of define
 * 
 */
TEST(bsp_ut, check_sizes)
{
    CHECK_EQUAL(sizeof(eeprom_playback_stats_t), CURRENT_PLAYBACK_INDEX_IN_EEPROM_LEN);
}

/**
 * @brief Verify that attempts to write past positions outside EEPROM range does not happen.
 * MUST PASS
 * 
 */

TEST(bsp_ut, MUST_PASS_check_eeprom_range)
{

    /* WARNING! Ensure this value is less than DATA_EEPROM_BANK2_END. Or else, it will overflow EEPROM */
    uint32_t EEPROM_ADDR_END = PLAYBACK_EEPROM_ADDR_START + PLAYBACK_EEPROM_SIZE;
    CHECK_EQUAL(5933, EEPROM_ADDR_END);
    CHECK_TRUE(EEPROM_ADDR_END < (EEPROM_SIZE - PLAYBACK_EEPROM_PACKET_SIZE * 2)); // ensure there is leeway a the end of the eeprom area to allow a wield overflow read. Its a bug in getting position index 0.
}

/**
 * @brief Check size of eeprom usage of tx interval
 * 
 */
TEST(bsp_ut, MUST_PASS_check_tx_interval_eeprom_size)
{
    CHECK_EQUAL(TX_INTERVAL_EEPROM_LEN, sizeof(tx_interval_eeprom_t));
}

/**
 * @brief Check size of eeprom usage of eeprom_playback_stats_t
 * 
 */
TEST(bsp_ut, MUST_PASS_check_eeprom_playback_stats_t_size)
{
    CHECK_EQUAL(CURRENT_PLAYBACK_INDEX_IN_EEPROM_LEN, sizeof(eeprom_playback_stats_t));
}

/**
 * @brief Ensure enough space is allocated for the Geofence allow/disallow tx mask
 * 
 */
TEST(bsp_ut, MUST_PASS_check_allow_disallow_geofence_settings_size)
{
    CHECK_EQUAL(TX_PERMISSIONS_LEN, sizeof(geofence_settings_t));
}

/**
 * @brief Verify if playback stats are set correctly in the event of eeprom corruption
 * 
 */
extern eeprom_playback_stats_t eeprom_playback_stats;

TEST(bsp_ut, test_playback_stats_corrupt_eeprom)
{
    read_playback_stats_from_eeprom();
    CHECK_EQUAL(0, eeprom_playback_stats.current_EEPROM_index);
    CHECK_EQUAL(0, eeprom_playback_stats.n_playback_positions_saved);
}

/**
 * @brief Verify if playback stats are set correctly in the event that eeprom is NOT corrupted,
 * and there is valid data there.
 * 
 */
TEST(bsp_ut, test_playback_stats_valid_eeprom)
{

    /**
     * @brief Set the eeprom_playback_stats struct
     */
    eeprom_playback_stats.current_EEPROM_index = 90;
    eeprom_playback_stats.n_playback_positions_saved = 10;
    eeprom_playback_stats.Crc32 = 2132531280;
    NvmmUpdate((uint8_t *)&eeprom_playback_stats, CURRENT_PLAYBACK_INDEX_IN_EEPROM_LEN, CURRENT_PLAYBACK_INDEX_IN_EEPROM_ADDR);

    /**
     * @brief Fill the EEPROM with this valid data
     */
    read_playback_stats_from_eeprom();

    /**
     * @brief Verify that it actually reads the valid data as expected
     */
    CHECK_EQUAL(90, eeprom_playback_stats.current_EEPROM_index);
    CHECK_EQUAL(10, eeprom_playback_stats.n_playback_positions_saved);
}

/**
 * @brief Test get EEPROM loramac region success
 */
TEST(bsp_ut, test_get_eeprom_loramac_region_success)
{

    LoRaMacRegion_t target_region = LORAMAC_REGION_CN470;
    NvmmWrite((uint8_t *)&target_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);

    retrieve_eeprom_stored_lorawan_region();

    CHECK_EQUAL(LORAMAC_REGION_CN470, get_current_loramac_region());
}

/**
 * @brief Test get EEPROM loramac region success over russia. Its the last value in LoRaMacRegion_t.
 */
TEST(bsp_ut, test_get_eeprom_loramac_region_success_russia)
{

    LoRaMacRegion_t target_region = LORAMAC_REGION_RU864;
    NvmmWrite((uint8_t *)&target_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);

    retrieve_eeprom_stored_lorawan_region();

    CHECK_EQUAL(LORAMAC_REGION_RU864, get_current_loramac_region());
}

/**
 * @brief Test get EEPROM loramac region fail
 */
TEST(bsp_ut, test_get_eeprom_loramac_region_fail)
{

    uint8_t target_region = 123;
    NvmmWrite((uint8_t *)&target_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);

    retrieve_eeprom_stored_lorawan_region();

    CHECK_EQUAL(LORAMAC_REGION_EU868, get_current_loramac_region());
}

/**
 * @brief Test set EEPROM loramac region success
 */
TEST(bsp_ut, test_set_eeprom_loramac_region_success)
{

    set_current_loramac_region(LORAMAC_REGION_US915);

    set_eeprom_stored_lorwan_region();

    CHECK_EQUAL(LORAMAC_REGION_US915, get_current_loramac_region());
}

TEST(bsp_ut, test_read_write_eeprom_success)
{

    uint32_t value = 1202;
    // Write value to EEPROM
    update_device_tx_interval_in_eeprom(value, TX_INTERVAL_EEPROM_ADDRESS);

    // Check if its correctly read.
    CHECK_EQUAL(value, read_tx_interval_in_eeprom(TX_INTERVAL_EEPROM_ADDRESS, TX_INTERVAL_GPS_FIX_OK));
}

TEST(bsp_ut, test_read_write_eeprom_corruption)
{

    uint32_t value = 987324;

    // Write value to EEPROM
    update_device_tx_interval_in_eeprom(value, GPS_SEARCH_TIME_ADDR);

    /**
     * @brief Wipe out eeprom to simulate CRC error
     */
    EEPROM_Wipe(0, EEPROM_SIZE);

    /**
     * @brief Now that CRC is wrong, it should read the default value of tx interval
     */
    CHECK_EQUAL(TX_INTERVAL_GPS_FIX_OK, read_tx_interval_in_eeprom(GPS_SEARCH_TIME_ADDR, TX_INTERVAL_GPS_FIX_OK));
}
