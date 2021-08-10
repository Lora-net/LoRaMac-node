#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "config.h"
#include "ublox.h"
#include "main.h"
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
    mock().expectNCalls(100, "get_latest_gps_info").andReturnValue(&gps_info_mock1);

    run_app();
}
