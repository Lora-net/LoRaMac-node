#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "LoRaMac.h"
#include "geofence.h"
#include "region_setting.h"
}

TEST_GROUP(Geofence_Polygon){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

TEST_GROUP(test_abp_credentials){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

/**
 * @brief Test point in polygon
 * 
 */
TEST(Geofence_Polygon, UK_test)
{
    Polygon_t poly = get_polygon(52, 0);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

TEST(Geofence_Polygon, Au_test)
{
    Polygon_t poly = get_polygon(-33.8696, 151.20695);
    CHECK_EQUAL(AU915928_AUSTRALIA_polygon, poly);
}

TEST(Geofence_Polygon, US_test)
{
    Polygon_t poly = get_polygon(47.7511, -120.7401);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, Singapore_test)
{
    Polygon_t poly = get_polygon(1.3521, 103.8198);
    CHECK_EQUAL(AS920923_MALAYSIASG_polygon, poly);
}

TEST(Geofence_Polygon, Russia_test)
{
    Polygon_t poly = get_polygon(61.5240, 105.3188);
    CHECK_EQUAL(RU864870_RUSSIA_polygon, poly);
}

TEST(Geofence_Polygon, Iran_test)
{
    Polygon_t poly = get_polygon(32.6539, 51.6660);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

/**
 * @brief Test network key validity
 * 
 */
TEST(test_abp_credentials, EU_network_key_test)
{
    network_keys_t eu_network_keys = get_network_keys(LORAMAC_REGION_EU868);

    CHECK_EQUAL(0xD2, eu_network_keys.AppSKey[0]);
}

TEST(test_abp_credentials, test_string_compare)
{

    STRCMP_EQUAL("hello", "hello");
}

TEST(test_abp_credentials, test_array_compare)
{

    uint8_t iar1[] = {1, 2, 3, 4, 5};
    uint8_t iar2[] = {1, 2, 3, 4, 5};
    uint8_t iar3[] = {1, 2, 3, 2, 5};

    CHECK_TRUE(std::equal(std::begin(iar1), std::end(iar1), std::begin(iar2)));
    CHECK_FALSE(std::equal(std::begin(iar1), std::end(iar1), std::begin(iar3)));
}

TEST(test_abp_credentials, test_eu_array_compare)
{

    uint8_t iar1[] = {1, 2, 3, 4, 5};
    uint8_t iar2[] = {1, 2, 3, 4, 5};
    uint8_t iar3[] = {1, 2, 3, 2, 5};
    CHECK_TRUE(std::equal(std::begin(iar1), std::end(iar1), std::begin(iar2)));
    CHECK_FALSE(std::equal(std::begin(iar1), std::end(iar1), std::begin(iar3)));
}