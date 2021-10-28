/**
 * @file message_sender_ut.cpp
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "message_sender.h"
#include "ublox.h"
#include "bsp.h"
#include "main.h"
#include "geofence.h"
}

LmHandlerAppData_t AppData;

TEST_GROUP(message_sender){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

gps_info_t gps_info_mock_message = {
    .GPS_UBX_latitude_Float = 53.23,
    .GPS_UBX_longitude_Float = 0.02,
    .unix_time = 1627938039,
    .latest_gps_status = GPS_SUCCESS,

};

void setup_test()
{
    mock().expectNCalls(12, "get_latest_gps_info").andReturnValue(&gps_info_mock_message);
    BSP_sensor_Init();
    do_n_transmissions(2);
}
/**
 * @brief Test if it transmits when loramac region correctly set
 * 
 */
TEST(message_sender, test_loramac_region_correct)
{
    setup_test();

    bool ret = sensor_read_and_send(&AppData, LORAMAC_REGION_EU868);

    CHECK_TRUE(ret);
}

/**
 * @brief Test if it does not transmit when loramac region incorrectly set
 * 
 */
TEST(message_sender, test_loramac_region_incorrect)
{
    setup_test();

    /* Assume the tracker was last over China but now its over Europe. 
     * It should not tx
     */
    bool ret = sensor_read_and_send(&AppData, LORAMAC_REGION_CN470);
    CHECK_FALSE(ret);

    /**
     * LoRaWAN stack has not yet been updated to the new region
     * 
     */
    ret = sensor_read_and_send(&AppData, LORAMAC_REGION_CN470);
    CHECK_FALSE(ret);

    /* The next time we check, it should have been set correctly */
    ret = sensor_read_and_send(&AppData, LORAMAC_REGION_EU868);
    CHECK_TRUE(ret);
}
