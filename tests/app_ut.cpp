#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "config.h"
#include "ublox.h"
#include "main.h"
#include <math.h> /* fmod */
#include "geofence.h"
#include "LoRaMac.h"
#include "nvmm.h"
}

#include "nvm_images.hpp"
#include "eeprom-board-mock.hpp"

#include <list>
#include <string.h>

void prepare_n_position_mocks(int number_of_readings, int degrees_moved_per_shift);

TEST_GROUP(app){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

;
TEST(app, test_init_sequence)
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

TEST(app, test_successful_setup)
{
    prepare_n_position_mocks(10, 2);

    CHECK_EQUAL(EXIT_SUCCESS, setup_board());
    position_list.clear();
}



TEST(app, run_app_through_3_geofence_regions_5degrees_shift_per_fix)
{

    prepare_n_position_mocks(10000, 5);
    setup_board();

    int region_switches = 4;

    polygon_t current_polygon;
    while (region_switches--)
    {
        current_polygon= current_geofence_status.curr_poly_region;

        run_country_loop();
        /* Ensure that region switches happen ONLY when polygon change is detected */
        CHECK_TRUE(current_polygon != current_geofence_status.curr_poly_region);
    }
    position_list.clear();

    CHECK_EQUAL(LORAMAC_REGION_EU868, current_geofence_status.current_loramac_region);
};


extern bool is_over_the_air_activation;
extern bool tx_done;

TEST(app, ensure_its_abp_always_after_initing_for_region)
{

    APP_TX_DUTYCYCLE = 40000; /* 40 second interval between transmissions */

    prepare_n_position_mocks(10000, 2);

    setup_board();

    int region_switches = 1;

    while (region_switches--)
    {
        run_country_loop();
    }

    init_loramac_stack_and_tx_scheduling();
    tx_done = false;
    int n_loops = 3;

    while (n_loops--)
    {
        run_loop_once();
    }

    CHECK_EQUAL(false, is_over_the_air_activation);

    position_list.clear();
};

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

/**
 * @brief Ensure transmission happens immediately after boot. 
 * 
 */
extern bool is_over_the_air_activation;

TEST(app, ensure_tx_happens_immediately_after_boot)
{
    /* Setup environment params */

    USE_NVM_STORED_LORAWAN_REGION = false;
    APP_TX_DUTYCYCLE = 40000; /* 40 second interval between transmissions */

    /* Setup mocks */
    float latitude = 53.23;
    float longitude = 0;

    gps_info_t world_trip_mock = {
        .GPS_UBX_latitude_Float = latitude,
        .GPS_UBX_longitude_Float = (float)longitude,
        .GPSaltitude = 12342000,
        .GPS_UBX_latitude = latitude * 1e7,
        .GPS_UBX_longitude = longitude * 1e7,
        .unix_time = 1627938039 + 60 * 60 * 2, /* travel one degree longitude every day */
        .latest_gps_status = GPS_SUCCESS,

    };
    mock().expectNCalls(100, "get_latest_gps_info").andReturnValue(&world_trip_mock);

    mock().expectOneCall("get_LoRaMacNvmData").andReturnValue(&first_transmission_eeprom_data);

    /* Now setup the main program */
    int ret;
    int number_of_milliseconds_to_run;

    is_over_the_air_activation = false;

    /**
     * @brief Fill the EEPROM with actual values. TODO: limit scope by not 
     * reading from EEROM, set the nvm seperately.
     * 
     */
    fake_eeprom_set();

    ret = setup_board();
    CHECK_EQUAL(EXIT_SUCCESS, ret);
    ret = init_loramac_stack_and_tx_scheduling();
    CHECK_EQUAL(EXIT_SUCCESS, ret);

    /* Get pointer to frame counter */
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    int expected_fcount = 9;
    CHECK_EQUAL(expected_fcount, nvm->Crypto.FCntList.FCntUp);

    /* Check if framee incrementation has happened in first 100 ms after initialisation */
    number_of_milliseconds_to_run = 100;
    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }

    expected_fcount += 1;
    CHECK_EQUAL(expected_fcount, nvm->Crypto.FCntList.FCntUp);

    /* Run it further 40 seconds */
    number_of_milliseconds_to_run = 40100;

    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }

    CHECK_EQUAL(expected_fcount, nvm->Crypto.FCntList.FCntUp);

    /* New run it another minute */
    number_of_milliseconds_to_run = 60000;

    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }
    expected_fcount += 2;
    CHECK_EQUAL(expected_fcount, nvm->Crypto.FCntList.FCntUp);

    /* New run it another minute */
    number_of_milliseconds_to_run = 60000;

    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }

    expected_fcount += 2;
    CHECK_EQUAL(expected_fcount, nvm->Crypto.FCntList.FCntUp);
};

/**
 * @brief Ensure transmission happens immediately after boot. 
 * 
 */

extern bool is_over_the_air_activation;

TEST(app, ensure_region_is_set_according_to_nvm)
{
    /* Setup environment params */

    USE_NVM_STORED_LORAWAN_REGION = false;
    APP_TX_DUTYCYCLE = 40000; /* 40 second interval between transmissions */

    /* Setup mocks */
    float latitude = 53.23;
    float longitude = 0;

    gps_info_t world_trip_mock = {
        .GPS_UBX_latitude_Float = latitude,
        .GPS_UBX_longitude_Float = (float)longitude,
        .GPSaltitude = 12342000,
        .GPS_UBX_latitude = latitude * 1e7,
        .GPS_UBX_longitude = longitude * 1e7,
        .unix_time = 1627938039 + 60 * 60 * 2, /* travel one degree longitude every day */
        .latest_gps_status = GPS_SUCCESS,

    };
    mock().expectNCalls(3, "get_latest_gps_info").andReturnValue(&world_trip_mock);

    /* Now setup the main program */
    int ret;
    int number_of_milliseconds_to_run;
    is_over_the_air_activation = false;
    fake_eeprom_set();

    ret = setup_board();
    CHECK_EQUAL(EXIT_SUCCESS, ret);
    ret = init_loramac_stack_and_tx_scheduling();
    CHECK_EQUAL(EXIT_SUCCESS, ret);

    /* Get pointer to NVM context */
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    number_of_milliseconds_to_run = 3;
    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }

    mock().checkExpectations();
    CHECK_EQUAL(10, nvm->Crypto.FCntList.FCntUp);
    CHECK_EQUAL(LORAMAC_REGION_EU868, nvm->MacGroup2.Region);
};

