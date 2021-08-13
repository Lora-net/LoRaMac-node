#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "config.h"
#include "ublox.h"
#include "main.h"
#include <math.h> /* fmod */
#include "geofence.h"
}

#include <list>

void prepare_n_position_mocks(int number_of_readings);

TEST_GROUP(app){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

TEST(app, init_sequence)
{

    float latitude = 53.23;
    int longitude = 23 % 360 - 180;

    gps_info_t world_trip_mock = {
        .GPS_UBX_latitude_Float = latitude,
        .GPS_UBX_longitude_Float = (float)longitude,
        .GPSaltitude = 12342000,
        .GPS_UBX_latitude = latitude * 1e7,
        .GPS_UBX_longitude = longitude * 1e7,
        .unix_time = 1627938039 + 60 * 60 * 2, /* travel one degree longitude every day */
        .latest_gps_status = GPS_SUCCESS,

    };
    mock().expectOneCall("get_latest_gps_info").andReturnValue(&world_trip_mock);

    int ret;

    ret = setup_board();
    CHECK_EQUAL(EXIT_SUCCESS, ret);

    ret = init_loramac_stack_and_tx_scheduling();
    CHECK_EQUAL(EXIT_SUCCESS, ret);
}

extern geofence_status_t current_geofence_status;

std::list<gps_info_t> position_list;

TEST(app, run_app_fixed_loops)
{

    prepare_n_position_mocks(100);

    int ret;

    ret = setup_board();
    CHECK_EQUAL(EXIT_SUCCESS, ret);

    int region_switches = 3;

    while (region_switches--)
    {
        ret = init_loramac_stack_and_tx_scheduling();
        CHECK_EQUAL(EXIT_SUCCESS, ret);
        /* Start loop */
        while (run_loop_once() == true)
        {
        }
    }
    position_list.clear();

    CHECK_EQUAL(LORAMAC_REGION_CN470, current_geofence_status.current_loramac_region);
};

void prepare_n_position_mocks(int n)
{

    gps_info_t world_trip_mock;

    /* Fill list with positions to simulate */
    n = 10000;
    for (int i = 0; i <= n; i++)
    {

        float latitude = 53.23;
        float longitude = fmod(0.02 + i * 10, 360) - 180;

        world_trip_mock.GPS_UBX_latitude_Float = latitude;
        world_trip_mock.GPS_UBX_longitude_Float = (float)longitude;
        world_trip_mock.GPSaltitude = 12342000;
        world_trip_mock.GPS_UBX_latitude = latitude * 1e7;
        world_trip_mock.GPS_UBX_longitude = longitude * 1e7;
        world_trip_mock.unix_time = 1627938039 + 60 * 60 * i; /* travel one degree longitude every day */
        world_trip_mock.latest_gps_status = GPS_SUCCESS;

        position_list.push_back(world_trip_mock);
    }

    /* Now mock those positions */
    for (auto &position : position_list)
    {
        mock().expectOneCall("get_latest_gps_info").andReturnValue(&position);
    }
}