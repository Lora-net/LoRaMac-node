#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "config.h"
#include "ublox.h"
#include "main.h"
#include <math.h> /* fmod */
}

TEST_GROUP(app){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

gps_info_t gps_info_mock1 = {
    .GPS_UBX_latitude_Float = 35.652832,
    .GPS_UBX_longitude_Float = 139.839478,
    .unix_time = 1627938039,
    .latest_gps_status = GPS_SUCCESS,

};

TEST(app, run_app)
{
    int number_of_readings = 100;
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
    }

    run_app();
}
