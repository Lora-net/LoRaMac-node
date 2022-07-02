#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "playback.h"
#include "struct.h"
#include "print_utils.h"
#include "main.h"
#include "geofence.h"
}

#include "gps_mock_utils.hpp"
#include "eeprom-board-mock.hpp"
#include "nvm_images.hpp"

TEST_GROUP(Playback){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

TEST(Playback, bytes_to_long)
{
    uint8_t buff1[4] = {0xbd, 0x40, 0x07, 0x00};
    uint32_t asd = extractLong_from_buff(0, buff1);
    CHECK_EQUAL((uint32_t)475325, asd);
}
TEST(Playback, process_playback_instructions_valid)
{
    bool success = process_playback_instructions(6, 12);
    CHECK_TRUE(success);
}

TEST(Playback, process_playback_instructions_invalid)
{
    bool success = process_playback_instructions(12, 4);
    CHECK_FALSE(success);
}


time_pos_fix_t time_pos_fixs[18] =
    {
        {
            .minutes_since_epoch = 9832,
            .latitude_encoded = 312,
            .longitude_encoded = 44,
            .altitude_encoded = 123,
        },
        {
            .minutes_since_epoch = 212,
            .latitude_encoded = 322,
            .longitude_encoded = 45,
            .altitude_encoded = 124,
        },
        {
            .minutes_since_epoch = 234324,
            .latitude_encoded = 332,
            .longitude_encoded = 46,
            .altitude_encoded = 125,
        },
        {
            .minutes_since_epoch = 93845,
            .latitude_encoded = 342,
            .longitude_encoded = 47,
            .altitude_encoded = 126,
        },
        {
            .minutes_since_epoch = 398,
            .latitude_encoded = 352,
            .longitude_encoded = 48,
            .altitude_encoded = 127,
        },
        {
            .minutes_since_epoch = 23123,
            .latitude_encoded = 362,
            .longitude_encoded = 49,
            .altitude_encoded = 128,
        },
        {
            .minutes_since_epoch = 324234234,
            .latitude_encoded = 372,
            .longitude_encoded = 50,
            .altitude_encoded = 129,
        },
        {
            .minutes_since_epoch = 2213,
            .latitude_encoded = 382,
            .longitude_encoded = 51,
            .altitude_encoded = 1210,
        },
        {
            .minutes_since_epoch = 943,
            .latitude_encoded = 392,
            .longitude_encoded = 52,
            .altitude_encoded = 1223,
        },
        {
            .minutes_since_epoch = 78,
            .latitude_encoded = 402,
            .longitude_encoded = 53,
            .altitude_encoded = 1233,
        },
        {
            .minutes_since_epoch = 21312,
            .latitude_encoded = 412,
            .longitude_encoded = 54,
            .altitude_encoded = 1237,
        },

};

time_pos_fix_t retrieve_eeprom_time_pos(uint16_t time_pos_index)
{

    return time_pos_fixs[time_pos_index];
}

IGNORE_TEST(Playback, prepare_tx_buffer_test)
{

    time_pos_fix_t current_position =
        {
            .minutes_since_epoch = 4533,
            .latitude_encoded = 52,
            .longitude_encoded = 2,
            .altitude_encoded = 232,
        };

    uint16_t earliest_timepos_index = 10;

    init_playback(&current_position, &retrieve_eeprom_time_pos, earliest_timepos_index);

    fill_positions_to_send_buffer(); // must be called before preparing buffer
    PicoTrackerAppData_t PicoTrackerAppData = prepare_tx_buffer();

    uint8_t tx_string_size = PicoTrackerAppData.BufferSize;

    uint8_t expected_tx_string[tx_string_size] =
        {
            0x73, 0x80, 0x00, 0x00, 0x00,                   // sensor and debug data
            0x34, 0x00, 0x02, 0x00, 0xE8, 0x00,             // latitude, longitude, altitude
            0x74, 0x01, 0x32, 0x00, 0x81, 0x00, 0xBB, 0xA5, // past position - time 1
            0x88, 0x01, 0x34, 0x00, 0xC7, 0x04, 0x06, 0x0E, // past position - time 2
            0x42, 0x01, 0x2D, 0x00, 0x7C, 0x00, 0xE1, 0x10, // past position - time 3
            0x56, 0x01, 0x2F, 0x00, 0x7E, 0x00, 0x20, 0xA3, // past position - time 4
            0x38, 0x01, 0x2C, 0x00, 0x7B, 0x00, 0x4D, 0xEB, // past position - time 5
            0x6A, 0x01, 0x31, 0x00, 0x80, 0x00, 0x62, 0xB7, // past position - time 6
            0x4C, 0x01, 0x2E, 0x00, 0x7D, 0x00, 0x61, 0x7E, // past position - time 7
            0x7E, 0x01, 0x33, 0x00, 0xBA, 0x04, 0x10, 0x09, // past position - time 8
            0x60, 0x01, 0x30, 0x00, 0x7F, 0x00, 0x27, 0x10, // past position - time 9
            0x92, 0x01, 0x35, 0x00, 0xD1, 0x04, 0x67, 0x11, // past position - time 10
            0x74, 0x01, 0x32, 0x00, 0x81, 0x00, 0xBB, 0xA5, // past position - time 11
            0x88, 0x01, 0x34, 0x00, 0xC7, 0x04, 0x06, 0x0E, // past position - time 12
            0x42, 0x01, 0x2D, 0x00, 0x7C, 0x00, 0xE1, 0x10, // past position - time 13
        };

    MEMCMP_EQUAL(expected_tx_string, PicoTrackerAppData.Buffer, tx_string_size);
}

TEST(Playback, playback_key_info_test1)
{
    playback_key_info_t *playback_key_info_ptr;

    playback_key_info_ptr = get_playback_key_info_ptr();

    playback_key_info_ptr->n_positions_saved_since_boot += 1;

    CHECK_EQUAL(1, playback_key_info_ptr->n_positions_saved_since_boot);
}

PicoTrackerAppData_t setup_passing_through_regions()
{
    srand(0);
    fake_eeprom_set_target_image(EEPROM_dump_with_34_stored_pos_fixes);

    prepare_n_position_mocks(10000, 24);
    // setup_board();

    int region_switches;

    region_switches = 1;
    while (region_switches--)
    {
        //loop();
    }

    playback_key_info_t *playback_key_info_ptr = get_playback_key_info_ptr();

    playback_key_info_ptr->n_positions_saved_since_boot = 0;

    // setup_board();

    region_switches = 1;
    while (region_switches--)
    {
        //loop();
    }

    // setup_board();

    /* fill up the buffer to send down */
    fill_positions_to_send_buffer();
    PicoTrackerAppData_t PicoTrackerAppData = prepare_tx_buffer();

    print_bytes(PicoTrackerAppData.Buffer, PicoTrackerAppData.BufferSize);

    return PicoTrackerAppData;
}

extern sensor_t *current_sensor_data_ptr;

IGNORE_TEST(Playback, check_if_gps_playback_struct_is_set_correctly)
{
    PicoTrackerAppData_t PicoTrackerAppData = setup_passing_through_regions();

    sensor_t current_sensor_data = get_current_sensor_data();

    CHECK_EQUAL(32, current_sensor_data.no_load_solar_voltage);
    CHECK_EQUAL(23, current_sensor_data.load_solar_voltage);
    CHECK_EQUAL(1, current_sensor_data.days_of_playback);
    CHECK_EQUAL(12, current_sensor_data.temperature);
    CHECK_EQUAL(14, current_sensor_data.reset_count);
    CHECK_EQUAL(16, current_sensor_data.sats);
    teardown_n_positions_mock();
}

IGNORE_TEST(Playback, run_app_through_few_positions)
{

    PicoTrackerAppData_t PicoTrackerAppData = setup_passing_through_regions();

    uint8_t expected_tx_buffer[] = {
        0x71, 0x41, 0x00, 0x86, 0x0C,
        0xBA, 0x1F, 0xB8, 0xEA, 0x52, 0xBC,
        0xAB, 0x1E, 0xDD, 0xFF, 0x6B, 0x01, 0x94, 0x02,
        0xAB, 0x1E, 0xDD, 0xFF, 0xF2, 0x01, 0x85, 0x02,
        0xAB, 0x1E, 0xDD, 0xFF, 0x5A, 0x01, 0x9B, 0x02,
        0xAB, 0x1E, 0xDD, 0xFF, 0x6B, 0x01, 0x90, 0x02,
        0xAB, 0x1E, 0xDD, 0xFF, 0xBA, 0x01, 0x25, 0x07,
        0xAB, 0x1E, 0xDD, 0xFF, 0x48, 0x01, 0xA2, 0x02,
        0xAB, 0x1E, 0xDD, 0xFF, 0x09, 0x01, 0x34, 0x07,
        0xAB, 0x1E, 0xDD, 0xFF, 0x8C, 0x01, 0x86, 0x02,
        0xAB, 0x1E, 0xDD, 0xFF, 0x5F, 0x01, 0x96, 0x02,
        0xBA, 0x1F, 0x32, 0xA3, 0x52, 0xBC, 0x2C, 0x01,
        0xAB, 0x1E, 0xDD, 0xFF, 0xC8, 0x00, 0x8A, 0x02,
        0xAB, 0x1E, 0xDD, 0xFF, 0xBF, 0x00, 0x84, 0x02,
        0xAB, 0x1E, 0xDD, 0xFF, 0xD8, 0x00, 0x8C, 0x02};

    MEMCMP_EQUAL(expected_tx_buffer, PicoTrackerAppData.Buffer, PicoTrackerAppData.BufferSize);
    teardown_n_positions_mock();
};
