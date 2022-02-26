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

/**
 * @brief Test cycling through network keys over the US
 * 
 */
TEST(lorawan_credentials, test_cycling_through_us_settings)
{
    /**
     * @brief Set the current loramac region(US)
     * 
     */
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_US915;
    set_current_loramac_region(Loramac_region);


    uint32_t ttn_nam1_dev_addr = 0x260C24A9;
    network_keys_t network_keys;

    /**
     * @brief Now get the current network keys and check correctness
     * 
     */
    network_keys = get_current_network_keys();
    CHECK_EQUAL(ttn_nam1_dev_addr, network_keys.DevAddr);


    /**
     * @brief Cycle through all the keys
     * 
     */
    switch_to_next_registered_credentials();
    switch_to_next_registered_credentials();
    switch_to_next_registered_credentials();
    switch_to_next_registered_credentials();
    switch_to_next_registered_credentials();
    switch_to_next_registered_credentials();

    /**
     * @brief Check if we are back to the first network key
     * 
     */
    network_keys = get_current_network_keys();
    CHECK_EQUAL(ttn_nam1_dev_addr, network_keys.DevAddr);
}

extern size_t network_key_list_size;

TEST(lorawan_credentials, basic_sanity_check)
{
    CHECK_EQUAL(NUMBER_OF_REGISTERED_DEVICES, network_key_list_size / sizeof(network_keys_t));
}
