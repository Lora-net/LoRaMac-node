#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "LoRaWAN_config_switcher.h"
#include "LoRaMac.h"
}

#include "stdint.h"

extern bool context_management_enabled;

void run_stout_test(void);

TEST_GROUP(config_switcher_tests){
    void setup(){

    }

    void teardown(){
        mock().clear();
}
}
;

TEST(config_switcher_tests, test_eu868_4_tx)
{
    LoRaMacRegion_t target_region = LORAMAC_REGION_EU868;
    int8_t expected_dr = DR_5;

    picotracker_lorawan_settings_t setting;

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);
}

TEST(config_switcher_tests, test_us915_4_tx)
{
    LoRaMacRegion_t target_region = LORAMAC_REGION_US915;
    int8_t expected_dr = DR_4;

    picotracker_lorawan_settings_t setting;

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);
}

TEST(config_switcher_tests, test_as923_4_tx)
{
    LoRaMacRegion_t target_region = LORAMAC_REGION_AS923;
    int8_t expected_dr = DR_4;

    picotracker_lorawan_settings_t setting;

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);

    setting = get_lorawan_setting(target_region);
    CHECK_EQUAL(expected_dr, setting.datarate);
    CHECK_EQUAL(true, context_management_enabled);
}
