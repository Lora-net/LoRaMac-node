#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "playback.h"
#include "struct.h"
}

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

TEST(Playback, init_playback_test)
{
    sensor_t sensor_data = {};
    time_pos_fix_t current_position = {};
    uint16_t earliest_timepos_index = 10;

    init_playback(&sensor_data, &current_position, &retrieve_eeprom_time_pos, earliest_timepos_index);
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

TEST(Playback, fill_positions_to_send_buffer_test)
{
    sensor_t sensor_data = {};
    time_pos_fix_t current_position = {};
    uint16_t earliest_timepos_index = 10;

    init_playback(&sensor_data, &current_position, &retrieve_eeprom_time_pos, earliest_timepos_index);

    fill_positions_to_send_buffer(); // call whenever a new position fix acquired

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


/**
 * @brief This will fail if the previous test playback_key_info_test1 
 * is run with shared memory, but passes when run individually(e.g. via
 * cpputest explorer)
 * 
 */
TEST(Playback, playback_key_info_test2)
{
    playback_key_info_t *playback_key_info_ptr;

    playback_key_info_ptr = get_playback_key_info_ptr();

    playback_key_info_ptr->n_positions_saved_since_boot += 1;

    CHECK_EQUAL(1, playback_key_info_ptr->n_positions_saved_since_boot);
}
