#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "LoRaMac.h"
#include "LoRaWAN_config_switcher.h"
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
    network_keys_t eu_network_keys = get_current_network_keys();

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

TEST(lorawan_credentials, test_cycling_through_eu_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_EU868;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_eu1_dev_addr = 0x260BD67C;

    for (int i = 1; i < 5; ++i)
    {
        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(ttn_eu1_dev_addr, network_keys.DevAddr);

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);
    }
}

TEST(lorawan_credentials, test_cycling_through_us_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_US915;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_nam1_dev_addr = 0x260CB928;
    for (int i = 1; i < 5; ++i)
    {
        network_keys = get_current_network_keys();
        CHECK_EQUAL(ttn_nam1_dev_addr, network_keys.DevAddr);

        uint8_t expected[] = {0xE4, 0x70, 0xDC, 0x81, 0xE1, 0x43, 0x8D, 0x99, 0x14, 0x22, 0x84, 0x83, 0xD9, 0xA3, 0x6B, 0xC7};
        MEMCMP_EQUAL(expected, network_keys.AppSKey, 16);

        switch_to_next_region();

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);
        uint8_t expected_helium_appskey[] = {249, 136, 1, 246, 88, 194, 178, 131, 121, 45, 60, 213, 92, 8, 58, 121};
        MEMCMP_EQUAL(expected_helium_appskey, network_keys.AppSKey, 16);
    }
}

TEST(lorawan_credentials, test_cycling_through_japan_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_AS923;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_eu1_dev_addr = 0x260BD61D;
    uint32_t ttn_au1_dev_addr = 0x260D24A9;

    for (int i = 1; i < 5; ++i)
    {
        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(ttn_eu1_dev_addr, network_keys.DevAddr);

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(ttn_au1_dev_addr, network_keys.DevAddr);
    }
}

TEST(lorawan_credentials, test_cycling_through_russia_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_RU864;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_eu1_dev_addr = 0x260B790D;

    for (int i = 1; i < 5; ++i)
    {
        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(ttn_eu1_dev_addr, network_keys.DevAddr);

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);
    }
}

TEST(lorawan_credentials, test_cycling_through_australia_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_AU915;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_au1_dev_addr = 0x260DE191;

    for (int i = 1; i < 5; ++i)
    {
        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(ttn_au1_dev_addr, network_keys.DevAddr);

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);
    }
}

TEST(lorawan_credentials, test_cycling_through_china_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_CN470;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_eu1_dev_addr = 0x260B74CE;

    for (int i = 1; i < 5; ++i)
    {

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(ttn_eu1_dev_addr, network_keys.DevAddr);

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);
    }
}

TEST(lorawan_credentials, test_cycling_through_india_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_IN865;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_eu1_dev_addr = 0x260BE033;

    for (int i = 1; i < 5; ++i)
    {

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(ttn_eu1_dev_addr, network_keys.DevAddr);

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);
    }
}

TEST(lorawan_credentials, test_cycling_through_korea_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_KR920;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_eu1_dev_addr = 0x260B0C3C;

    for (int i = 1; i < 5; ++i)
    {

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(ttn_eu1_dev_addr, network_keys.DevAddr);

        network_keys = get_current_network_keys();
        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);
    }
}
