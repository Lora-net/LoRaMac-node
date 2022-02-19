#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "LoRaMac.h"
#include "geofence.h"
#include "RegionAS923.h"
}

TEST_GROUP(test_get_current_lorawan_region){
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
TEST(test_get_current_lorawan_region, UK_test)
{
    update_geofence_position(52, 0);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Ascension_island)
{
    update_geofence_position(-7.96666, -14.4);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Saint_Helena)
{
    update_geofence_position(-15.9500, -5.7167);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Tristan_da_Cunha)
{
    update_geofence_position(-37.1052, -12.2777);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, mauritius)
{
    update_geofence_position(-20.165111, 57.499266);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Chagos_islands)
{
    update_geofence_position(-6.166666, 72.0);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, azores)
{
    update_geofence_position(37.7412, -25.6756);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Heard_and_McDonald_Islands)
{
    update_geofence_position(-53.0818, 73.5042);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, antartica)
{
    update_geofence_position(-80.96000880094358, 35.106277888713585);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, McMurdo_Station_antartica)
{
    update_geofence_position(-77.51, 166.40);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Au_test)
{
    update_geofence_position(-33.8696, 151.20695);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AU915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, US_test)
{
    update_geofence_position(47.7511, -120.7401);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Singapore_test)
{
    update_geofence_position(1.3521, 103.8198);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AS923, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
    CHECK_EQUAL(CHANNEL_PLAN_GROUP_AS923_1, get_as923_subband());
}

TEST(test_get_current_lorawan_region, Russia_test)
{
    update_geofence_position(61.5240, 105.3188);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_RU864, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Iran_test)
{
    update_geofence_position(32.6539, 51.6660);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, China_test)
{
    update_geofence_position(35.8617, 104.1954);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_CN470, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Hawaii)
{
    update_geofence_position(19.38629551, -155.69824219);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Germany)
{
    update_geofence_position(51.1657, 10.4515);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Japan)
{
    update_geofence_position(35.6762, 139.6503);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_AS923, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
    CHECK_EQUAL(CHANNEL_PLAN_GROUP_AS923_1_JP, get_as923_subband());
}

TEST(test_get_current_lorawan_region, India)
{
    update_geofence_position(13.0827, 80.2707);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_IN865, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, China)
{
    update_geofence_position(28.087486, 116.255585);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_CN470, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Sudan)
{
    update_geofence_position(17.409349, 30.866612);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Kazakstan)
{
    update_geofence_position(46.631330, 68.461685);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Korea)
{
    update_geofence_position(34.972397, 127.850327);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_KR920, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Russia)
{
    update_geofence_position(56.032768, 158.066928);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_RU864, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Tasmaina)
{
    update_geofence_position(-42.475341, 145.066763);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AU915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Hokkaido)
{
    update_geofence_position(41.849920, 141.553205);
    LoRaMacRegion_t region = get_current_loramac_region();

    CHECK_EQUAL(LORAMAC_REGION_AS923, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
    CHECK_EQUAL(CHANNEL_PLAN_GROUP_AS923_1_JP, get_as923_subband());
}

TEST(test_get_current_lorawan_region, Honduras)
{
    update_geofence_position(14.366573, -87.882151);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Mexico)
{
    update_geofence_position(25.097538, -104.062500);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Honolulu)
{
    update_geofence_position(21.31240491, -157.89276123);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Israel)
{
    update_geofence_position(32.896193, 35.374242);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AS923, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
    CHECK_EQUAL(CHANNEL_PLAN_GROUP_AS923_4, get_as923_subband());
}

TEST(test_get_current_lorawan_region, Norway)
{
    update_geofence_position(62.629729, 10.526518);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Sweden)
{
    update_geofence_position(64.544663, 18.256346);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Manila)
{
    update_geofence_position(14.426168, 120.662100);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AS923, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
    CHECK_EQUAL(CHANNEL_PLAN_GROUP_AS923_3, get_as923_subband());
}

TEST(test_get_current_lorawan_region, New_Zealand)
{
    update_geofence_position(-40.9006, 174.8860);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AU915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Taiwan)
{
    update_geofence_position(23.883828, 121.194817);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AS923, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
    CHECK_EQUAL(CHANNEL_PLAN_GROUP_AS923_1, get_as923_subband());
}

TEST(test_get_current_lorawan_region, Fukuda)
{
    update_geofence_position(33.227660, 130.136745);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AS923, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
    CHECK_EQUAL(CHANNEL_PLAN_GROUP_AS923_1_JP, get_as923_subband());
}

TEST(test_get_current_lorawan_region, Azores)
{
    update_geofence_position(38.815743, -27.794621);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Iceland)
{
    update_geofence_position(64.990508, -16.913441);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Greenland)
{
    update_geofence_position(68.904679, -37.814359);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Norge)
{
    update_geofence_position(79.134947, 15.767405);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

/**
 * @brief South America
 * 
 */

TEST(test_get_current_lorawan_region, Chile)
{
    update_geofence_position(-33.447487, -70.673676);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AU915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Argentina)
{
    update_geofence_position(-41.032962, -67.575228);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Falklands)
{
    update_geofence_position(-51.639841, -58.97460938);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Brazil)
{
    update_geofence_position(-13.49647277, -47.54882813);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AU915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Paraguay)
{
    update_geofence_position(-22.18740499, -59.72167969);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Ecuador)
{
    update_geofence_position(-1.40610884, -78.09082031);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Bolivia)
{
    update_geofence_position(-17.484816, -64.6875);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, calama_chile)
{
    update_geofence_position(-22.899707, -68.87878418);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_AU915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

/**
 * @brief North America
 * 
 */

TEST(test_get_current_lorawan_region, Arizona)
{
    update_geofence_position(34.884128, -112.21435547);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Albuquerque)
{
    update_geofence_position(34.811999, -106.67724609);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Seattle)
{
    update_geofence_position(47.79101618, -121.94824219);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Guam)
{
    update_geofence_position(13.444304, 144.793732);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

TEST(test_get_current_lorawan_region, Maldives)
{
    update_geofence_position(3.2028, 73.2207);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_US915, region);
    CHECK_EQUAL(TX_OK, get_current_tx_permission());
}

/**
 * @brief Regions where its not meant to TX
 * 
 */

TEST(test_get_current_lorawan_region, North_Korea)
{
    update_geofence_position(40.3399, 127.5101);
    LoRaMacRegion_t region = get_current_loramac_region();
    CHECK_EQUAL(LORAMAC_REGION_EU868, region);
    CHECK_EQUAL(TX_NOT_OK, get_current_tx_permission());
}