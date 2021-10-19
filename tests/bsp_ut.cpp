#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "bsp.h"
#include "ublox.h"
#include "nvmm.h"
#include "playback.h"
#include "print_utils.h"

#include <math.h> /* fmod */
}

extern uint8_t simulated_flash[EEPROM_SIZE];

void sensor_read_and_printout(uint32_t number_of_readings);

TEST_GROUP(bsp_ut){
    void setup(){}

    void teardown(){
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
    mock().expectNCalls(2, "get_latest_gps_info").andReturnValue(&gps_info_mock);

    BSP_sensor_Init();
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
            .GPSaltitude = 12342000,
            .GPS_UBX_latitude = latitude * 1e7,
            .GPS_UBX_longitude = longitude * 1e7,
            .unix_time = 1627938039 + 60 * 60 * i, /* travel one degree longitude every day */
            .latest_gps_status = GPS_SUCCESS,

        };
        mock().expectOneCall("get_latest_gps_info").andReturnValue(&world_trip_mock);
        BSP_sensor_Read();

        // printf("EEPROM: ");
        // print_bytes(simulated_flash, EEPROM_SIZE);

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

