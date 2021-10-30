/**
 * @file position_time_encoder_ut.cpp
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-10-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "position_time_encoder.h"
}

TEST_GROUP(test_position_time_encoder){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

/**
 * @brief Check if encode and decode works for given gps_info_t gps_info
 * 
 * @param gps_info 
 */
void check_encode_decode(gps_info_t gps_info)
{
    time_pos_fix_t time_pos_fix = encode_time_pos(gps_info);
    gps_info_t ret_gps_info = decode_time_pos(time_pos_fix);

    DOUBLES_EQUAL(gps_info.GPS_UBX_latitude_Float, ret_gps_info.GPS_UBX_latitude_Float, 0.1);
    DOUBLES_EQUAL(gps_info.GPS_UBX_longitude_Float, ret_gps_info.GPS_UBX_longitude_Float, 0.1);
    DOUBLES_EQUAL(gps_info.GPSaltitude_mm, ret_gps_info.GPSaltitude_mm, 10000);
    DOUBLES_EQUAL(gps_info.unix_time, ret_gps_info.unix_time, 60);
}

/**
 * @brief Test to see if the encoded and decoded values match
 * 
 */
TEST(test_position_time_encoder, test_encode_decode_7932m_altitude)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = 52.3,
        .GPS_UBX_longitude_Float = 3.53,
        .GPSaltitude_mm = 7932000,
        .unix_time = 1635594095,
    };

    check_encode_decode(gps_info);
}

/**
 * @brief Test to see if the encoded and decoded values match
 * 
 */
TEST(test_position_time_encoder, test_encode_decode_16000m_altitude)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = 52.3,
        .GPS_UBX_longitude_Float = 3.53,
        .GPSaltitude_mm = 16000000,
        .unix_time = 1635594095,
    };

    check_encode_decode(gps_info);
}

/**
 * @brief Test to see if the encoded and decoded values match
 * 
 */
TEST(test_position_time_encoder, test_encode_decode_18000m_altitude)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = 52.3,
        .GPS_UBX_longitude_Float = 3.53,
        .GPSaltitude_mm = 18000000,
        .unix_time = 1635594095,
    };

    check_encode_decode(gps_info);
}

/**
 * @brief Test to see if the encoded and decoded values match
 * 
 */
TEST(test_position_time_encoder, test_encode_decode_179_longitude)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = 52.3,
        .GPS_UBX_longitude_Float = 179,
        .GPSaltitude_mm = 13000000,
        .unix_time = 1635594095,
    };

    check_encode_decode(gps_info);
}

/**
 * @brief Test to see if the encoded and decoded values match
 * 
 */
TEST(test_position_time_encoder, test_encode_decode_minus_179_longitude)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = 52.3,
        .GPS_UBX_longitude_Float = -179,
        .GPSaltitude_mm = 13000000,
        .unix_time = 1635594095,
    };

    check_encode_decode(gps_info);
}

TEST(test_position_time_encoder, test_encode_decode_minus_60_latitude)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = -60,
        .GPS_UBX_longitude_Float = -179,
        .GPSaltitude_mm = 13000000,
        .unix_time = 1635594095,
    };

    check_encode_decode(gps_info);
}

TEST(test_position_time_encoder, test_encode_decode_edges)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = -90,
        .GPS_UBX_longitude_Float = -180,
        .GPSaltitude_mm = 13000000,
        .unix_time = 1635594095,
    };

    check_encode_decode(gps_info);
}

TEST(test_position_time_encoder, test_encode_decode_edges2)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = -90,
        .GPS_UBX_longitude_Float = 180,
        .GPSaltitude_mm = 13000000,
        .unix_time = 1635594095,
    };

    check_encode_decode(gps_info);
}

/**
 * @brief Check if it can encode/decode a timestamp before 2015. It wont be able to do it.
 * So return our epoch value instead.
 * 
 */
TEST(test_position_time_encoder, test_timestamps_before_2015)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = -90,
        .GPS_UBX_longitude_Float = 180,
        .GPSaltitude_mm = 13000000,
        .unix_time = 1319982932, // Sun Oct 30 2011 13:55:32 GMT+0000
    };

    time_pos_fix_t time_pos_fix = encode_time_pos(gps_info);
    gps_info_t ret_gps_info = decode_time_pos(time_pos_fix);

    DOUBLES_EQUAL(1577840461, ret_gps_info.unix_time, 60);
}

/**
 * @brief Check if it can encode/decode a timestamp after 2022
 * 
 */
TEST(test_position_time_encoder, test_timestamps_after_2022)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = -90,
        .GPS_UBX_longitude_Float = 180,
        .GPSaltitude_mm = 13000000,
        .unix_time = 1698674132, // Mon Oct 30 2023 13:55:32 GMT+0000
    };

    check_encode_decode(gps_info);
}

/**
 * @brief Check if it can encode/decode a timestamp after 2025
 * 
 */
TEST(test_position_time_encoder, test_timestamps_after_2025)
{

    gps_info_t gps_info = {
        .GPS_UBX_latitude_Float = -90,
        .GPS_UBX_longitude_Float = 180,
        .GPSaltitude_mm = 13000000,
        .unix_time = 1764510932, // Sun Nov 30 2025 13:55:32 GMT+0000
    };

    check_encode_decode(gps_info);
}
