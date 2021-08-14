#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "LoRaMac.h"
#include "region_setting.h"
#include "geofence.h"
#include "secure-element.h"
#include "LmHandlerTypes.h"
#include "main.h"
#include "config.h"
#include "ublox.h"
}

TEST_GROUP(test_abp_credentials){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

TEST_GROUP(lorawan_credentials){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

;
/**
 * @brief Test network key validity
 * 
 */
TEST(test_abp_credentials, EU_network_key_test)
{
    network_keys_t eu_network_keys = get_network_keys(LORAMAC_REGION_EU868);

    CHECK_EQUAL(0xD2, eu_network_keys.AppSKey[0]);
}

extern CommissioningParams_t CommissioningParams;

TEST(lorawan_credentials, ensure_lorawan_credentials_are_set_correctly_eu868) /* TODO: do the tests for other countries; check dynamic changes */
{
    SecureElementNvmData_t SecureElement;

    // Initialize the Secure Element driver
    SecureElementStatus_t status = SecureElementInit(&SecureElement);
    CHECK_EQUAL(SECURE_ELEMENT_SUCCESS, status);

    /* Check if AppSessionKey is correctly set */
    uint8_t APP_S_KEY_EU868[] = {0xD2, 0x02, 0x95, 0x6B, 0xF5, 0x36, 0xFF, 0x15, 0x29, 0xA0, 0x83, 0x58, 0xAC, 0x3E, 0xE8, 0x88};
    CHECK_TRUE(std::equal(std::begin(APP_S_KEY_EU868), std::end(APP_S_KEY_EU868), std::begin(SecureElement.KeyList[7].KeyValue))); /* 7th in the keylist is appskey */
}

/**
 * @brief ensure_eu868_dev_address_is_set_correctly
 * 
 */
TEST(lorawan_credentials, ensure_eu868_dev_address_is_set_correctly)
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

    ret = setup_board();
    CHECK_EQUAL(EXIT_SUCCESS, ret);
    ret = init_loramac_stack_and_tx_scheduling();
    CHECK_EQUAL(EXIT_SUCCESS, ret);

    /* Check LORAWAN_DEVICE_ADDRESS */
    CHECK_EQUAL(0x260BD67C, CommissioningParams.DevAddr);

    /**
     * @brief Verify that GPS check was called once
     * 
     */
    mock().checkExpectations();
};
