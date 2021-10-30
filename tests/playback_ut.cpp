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

time_pos_fix_t retrieve_eeprom_time_pos(uint16_t time_pos_index)
{

    time_pos_fix_t time_pos_fix = {0};

    return time_pos_fix;
}

TEST(Playback, prepare_tx_buffer_test)
{
    sensor_t sensor_data = {};
    time_pos_fix_t current_position = {};
    uint16_t earliest_timepos_index = 10;

    init_playback(&sensor_data, &current_position, &retrieve_eeprom_time_pos, earliest_timepos_index);

    PicoTrackerAppData_t PicoTrackerAppData = prepare_tx_buffer();

    CHECK_EQUAL(0x00, PicoTrackerAppData.Buffer[5]);
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

TEST(Playback, check_if_gps_playback_struct_is_set_correctly)
{
    PicoTrackerAppData_t PicoTrackerAppData = setup_passing_through_regions();

    CHECK_EQUAL(32, current_sensor_data_ptr->no_load_solar_voltage);
    CHECK_EQUAL(23, current_sensor_data_ptr->load_solar_voltage);
    CHECK_EQUAL(1, current_sensor_data_ptr->days_of_playback);
    CHECK_EQUAL(12, current_sensor_data_ptr->temperature);
    CHECK_EQUAL(14, current_sensor_data_ptr->reset_count);
    CHECK_EQUAL(16, current_sensor_data_ptr->sats);
    teardown_n_positions_mock();
}

TEST(Playback, run_app_through_few_positions)
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
