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

TEST(lorawan_credentials, test_cycling_through_eu_settings)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_EU868;
    current_geofence_status.current_loramac_region = Loramac_region;

    network_keys_t network_keys;
    registered_devices_t registered_device;

    uint32_t helium_dev_addr = 0x4800002d;
    uint32_t ttn_eu1_dev_addr = 0x260BD67C;

    for (int i = 1; i < 5; ++i)
    {
        network_keys = get_current_network_keys();
        registered_device = get_current_network();

        switch_to_next_region();
        CHECK_EQUAL(ttn_eu1_dev_addr, network_keys.DevAddr);
        CHECK_EQUAL(EU868_KEYS_EU1, registered_device);

        network_keys = get_current_network_keys();
        registered_device = get_current_network();

        switch_to_next_region();
        CHECK_EQUAL(helium_dev_addr, network_keys.DevAddr);
        CHECK_EQUAL(HELIUM_KEYS, registered_device);
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
