/**
 * @file position_time_encoder.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-10-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "position_time_encoder.h"

#define UNIX_TIME_ON_1_JAN_2020 1577840461

uint32_t minutes_since_epoch_to_unix_time(uint32_t minutes_since_epoch);
uint32_t unix_time_to_minutes_since_epoch(uint32_t unix_time);

time_pos_fix_t encode_time_pos(gps_info_t gps_info)
{

    time_pos_fix_t current_position;
    // This cast from int32_t to uint16_t could have a roll over if the value was negative.
    // So clip to 0 if below zero altitude.
    if (gps_info.GPSaltitude_mm < 0)
    {
        current_position.altitude_encoded = 0;
    }
    else
    {
        current_position.altitude_encoded = (gps_info.GPSaltitude_mm >> 8) & 0xffff;
    }

    uint32_t latitude = gps_info.GPS_UBX_latitude_Float * 1e7;
    uint32_t longitude = gps_info.GPS_UBX_longitude_Float * 1e7;

    current_position.latitude_encoded = (latitude >> 16) & 0xffff;
    current_position.longitude_encoded = (longitude >> 16) & 0xffff;

    /**
     * @brief We cannot deal with values before our specific epoch of 1 Jan 2020. 
     * This was done to minimise bytesize of the timestamp value. So if somehow the 
     * GPS returns a value before this date, encode it as 1 Jan 2020. 
     * This could happen if the GPS does not get a time fix, instead returning a default
     * value somewhere in 2015
     */
    uint32_t unix_time_epoch_considered = gps_info.unix_time > UNIX_TIME_ON_1_JAN_2020 ? gps_info.unix_time : UNIX_TIME_ON_1_JAN_2020;

    current_position.minutes_since_epoch = unix_time_to_minutes_since_epoch(unix_time_epoch_considered) & 0x00ffffff;

    return current_position;
}

gps_info_t decode_time_pos(time_pos_fix_t time_pos_fix)
{

    gps_info_t gps_info;

    gps_info.unix_time = minutes_since_epoch_to_unix_time(time_pos_fix.minutes_since_epoch);

    gps_info.GPS_UBX_latitude_Float = (float)((time_pos_fix.latitude_encoded * 0xFFFF) / 1e7);

    gps_info.GPS_UBX_longitude_Float = (float)((time_pos_fix.longitude_encoded * 0xFFFF) / 1e7);

    gps_info.GPSaltitude_mm = time_pos_fix.altitude_encoded << 8;

    return gps_info;
}

/**
 * @brief Calculate minutes since epoch. Based on GPS time.
 * Epoch is set to 1 Jan 2020 01:01:01H( unix time: 1577840461)
 * 
 * @param unix_time unix time in seconds
 * @return uint32_t minutes since our epoch
 */
uint32_t unix_time_to_minutes_since_epoch(uint32_t unix_time)
{
    return (unix_time - UNIX_TIME_ON_1_JAN_2020) / 60;
}

/**
 * @brief Convert minutes since epoch to unix time
 * 
 * @param minutes_since_epoch minutes since epoch(1/1/2020)
 * @return uint32_t unix time in seconds
 */
uint32_t minutes_since_epoch_to_unix_time(uint32_t minutes_since_epoch)
{
    return minutes_since_epoch * 60 + UNIX_TIME_ON_1_JAN_2020;
}
