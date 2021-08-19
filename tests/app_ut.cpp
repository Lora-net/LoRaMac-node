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

#include <list>

void prepare_n_position_mocks(int number_of_readings, int degrees_moved_per_shift);

TEST_GROUP(app){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

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

TEST(app, run_app_through_3_geofence_regions_5degrees_shift_per_fix)
{

    prepare_n_position_mocks(10000, 5);

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

TEST(app, run_app_through_3_geofence_regions_2degrees_shift_per_fix)
{

    prepare_n_position_mocks(10000, 2);

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

    CHECK_EQUAL(LORAMAC_REGION_EU868, current_geofence_status.current_loramac_region);
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

    ret = setup_board();
    CHECK_EQUAL(EXIT_SUCCESS, ret);
    ret = init_loramac_stack_and_tx_scheduling();
    CHECK_EQUAL(EXIT_SUCCESS, ret);

    /* Get pointer to frame counter */
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    /* Check if framee incrementation has happened in first 100 ms after initialisation */
    number_of_milliseconds_to_run = 100;
    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }

    CHECK_EQUAL(10, nvm->Crypto.FCntList.FCntUp);

    /* Run it further 40 seconds */
    number_of_milliseconds_to_run = 40100;

    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }

    CHECK_EQUAL(10, nvm->Crypto.FCntList.FCntUp);

    /* New run it another minute */
    number_of_milliseconds_to_run = 60000;

    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }

    CHECK_EQUAL(12, nvm->Crypto.FCntList.FCntUp);

    /* New run it another minute */
    number_of_milliseconds_to_run = 60000;

    while (number_of_milliseconds_to_run--)
    {
        run_loop_once();
    }

    CHECK_EQUAL(14, nvm->Crypto.FCntList.FCntUp);
};

/**
 * @brief Ensure transmission happens immediately after boot. 
 * 
 */
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

/**
 * @brief Check if NVM read is happening.
 * 
 */
TEST(app, read_and_check_nvm_values)
{
    /* Setup environment params */
    LoRaMacNvmData_t nvm_inited;
    LoRaMacNvmData_t *nvm = &nvm_inited;

    printf("LoRaMacNvmData_t size: %d\n", sizeof(LoRaMacNvmData_t));

    NvmmRead((uint8_t *)nvm, sizeof(LoRaMacNvmData_t), 0);

    CHECK_EQUAL(4, sizeof(float));
    CHECK_EQUAL(1, sizeof(bool));

    CHECK_EQUAL(52, sizeof(LoRaMacCryptoNvmData_t));
    CHECK_EQUAL(24, sizeof(LoRaMacNvmDataGroup1_t));

    /* Loramac LoRaMacNvmDataGroup2_t constituents */
    CHECK_EQUAL(1, sizeof(LoRaMacRegion_t));
    CHECK_EQUAL(60, sizeof(LoRaMacParams_t));
    CHECK_EQUAL(4, sizeof(uint32_t *));
    CHECK_EQUAL(4, sizeof(uint8_t *));
    CHECK_EQUAL(44, sizeof(MulticastCtx_t));
    CHECK_EQUAL(1, sizeof(DeviceClass_t));
    CHECK_EQUAL(1, sizeof(bool));
    CHECK_EQUAL(8, sizeof(SysTime_t));
    CHECK_EQUAL(4, sizeof(Version_t));
    CHECK_EQUAL(1, sizeof(ActivationType_t));

    CHECK_EQUAL(340, sizeof(LoRaMacNvmDataGroup2_t));

    CHECK_EQUAL(416, sizeof(SecureElementNvmData_t));
    CHECK_EQUAL(1180, sizeof(RegionNvmDataGroup2_t));

    CHECK_EQUAL(2200, sizeof(LoRaMacNvmData_t));

    CHECK_EQUAL(9, nvm->Crypto.FCntList.FCntUp);

    uint8_t expected[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x82, 0x4D};

    for (unsigned int a = 0; a < sizeof(expected) / sizeof(expected[0]); a++)
    {
        CHECK_EQUAL(expected[a], nvm->SecureElement.JoinEui[a]);
    }

    CHECK_EQUAL(LORAMAC_REGION_EU868, nvm->MacGroup2.Region);
};

/**
 * @brief Check if NVM read is happening.
 * 
 */
TEST(app, check_crc_for_all_parameters)
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
    mock().expectNCalls(1, "get_latest_gps_info").andReturnValue(&world_trip_mock);

    /* Now setup the main program */
    int ret;
    int number_of_milliseconds_to_run;

    ret = setup_board();
    CHECK_EQUAL(EXIT_SUCCESS, ret);
    ret = init_loramac_stack_and_tx_scheduling();
    CHECK_EQUAL(EXIT_SUCCESS, ret);

    /* Setup environment params */
    LoRaMacNvmData_t nvm_inited;
    LoRaMacNvmData_t *nvm = &nvm_inited;

    printf("LoRaMacNvmData_t size: %d\n", sizeof(LoRaMacNvmData_t));

    NvmmRead((uint8_t *)nvm, sizeof(LoRaMacNvmData_t), 0);

    uint16_t offset = 0;

    // Crypto
    CHECK_FALSE(NvmmCrc32Check(sizeof(LoRaMacCryptoNvmData_t), offset) == false);
    offset += sizeof(LoRaMacCryptoNvmData_t);

    // Mac Group 1
    CHECK_FALSE(NvmmCrc32Check(sizeof(LoRaMacNvmDataGroup1_t), offset) == false);
    offset += sizeof(LoRaMacNvmDataGroup1_t);

    // Mac Group 2
    CHECK_FALSE(NvmmCrc32Check(sizeof(LoRaMacNvmDataGroup2_t), offset) == false);
    offset += sizeof(LoRaMacNvmDataGroup2_t);

    // Secure element
    CHECK_FALSE(NvmmCrc32Check(sizeof(SecureElementNvmData_t), offset) == false);
    offset += sizeof(SecureElementNvmData_t);

    // Region group 1
    CHECK_FALSE(NvmmCrc32Check(sizeof(RegionNvmDataGroup1_t), offset) == false);

    offset += sizeof(RegionNvmDataGroup1_t);

    // Region group 2
    CHECK_FALSE(NvmmCrc32Check(sizeof(RegionNvmDataGroup2_t), offset) == false);

    offset += sizeof(RegionNvmDataGroup2_t);

    // Class b
    CHECK_FALSE(NvmmCrc32Check(sizeof(LoRaMacClassBNvmData_t), offset) == false);
};

/**
 * @brief Ensure the device alternates between abp and otaa.
 * 
 */
TEST(app, alternate_abp_otaa)
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
    mock().expectNCalls(6000, "get_latest_gps_info").andReturnValue(&world_trip_mock); /* Expect a lot of calls */

    /* Now setup the main program */
    setup_board();

    int number_of_tx = 4;
    while (number_of_tx--)
    {
        run_country_loop();
    }
};