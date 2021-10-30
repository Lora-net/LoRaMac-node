/**
 * @file gps_mock_utils.cpp
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "ublox.h"
#include "main.h"
#include <math.h> /* fmod */
}
#include <list>

std::list<gps_info_t> position_list;

void prepare_n_position_mocks(int number_of_readings, int degrees_moved_per_shift)
{

    gps_info_t world_trip_mock;

    /* Fill list with positions to simulate */
    for (int i = 0; i <= number_of_readings; i++)
    {

        float latitude = 53.23;
        float longitude = fmod(0.3 + i * degrees_moved_per_shift, 360) - 180;

        world_trip_mock.GPS_UBX_latitude_Float = latitude;
        world_trip_mock.GPS_UBX_longitude_Float = (float)longitude;
        world_trip_mock.GPSaltitude_mm = 12342000;
        world_trip_mock.unix_time = 1631323786 + 60 * 60 * i; /* travel one degree longitude every hour */
        world_trip_mock.latest_gps_status = GPS_SUCCESS;
        world_trip_mock.GPSsats = 16;

        position_list.push_back(world_trip_mock);
    }

    /* Now mock those positions */
    for (auto &position : position_list)
    {
        mock().expectOneCall("get_latest_gps_info").andReturnValue(&position);
    }
}

void teardown_n_positions_mock()
{
    position_list.clear();
}
