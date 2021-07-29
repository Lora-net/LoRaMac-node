#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "LoRaMac.h"
#include "geofence.h"
}

TEST_GROUP(Geofence_Polygon){
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

TEST(Geofence_Polygon, China_test)
{
    Polygon_t poly = get_polygon(35.8617, 104.1954);
    CHECK_EQUAL(CN779787_CHINA_polygon, poly);
}

TEST(Geofence_Polygon, Hawaii)
{
    Polygon_t poly = get_polygon(19.38629551, -155.69824219);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, Germany)
{
    Polygon_t poly = get_polygon(51.1657, 10.4515);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

TEST(Geofence_Polygon, Japan)
{
    Polygon_t poly = get_polygon(35.6762, 139.6503);
    CHECK_EQUAL(AS920923_JAPAN_polygon, poly);
}

TEST(Geofence_Polygon, India)
{
    Polygon_t poly = get_polygon(13.0827, 80.2707);
    CHECK_EQUAL(IN865867_INDIA_polygon, poly);
}

TEST(Geofence_Polygon, China)
{
    Polygon_t poly = get_polygon(28.087486, 116.255585);
    CHECK_EQUAL(CN779787_CHINA_polygon, poly);
}

TEST(Geofence_Polygon, Sudan)
{
    Polygon_t poly = get_polygon(17.409349, 30.866612);
    CHECK_EQUAL(EU863870_AFRICA_polygon, poly);
}

TEST(Geofence_Polygon, Kazakstan)
{
    Polygon_t poly = get_polygon(46.631330, 68.461685);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

TEST(Geofence_Polygon, Korea)
{
    Polygon_t poly = get_polygon(34.972397, 127.850327);
    CHECK_EQUAL(KR920923_SKOREA_polygon, poly);
}

TEST(Geofence_Polygon, Russia)
{
    Polygon_t poly = get_polygon(56.032768, 158.066928);
    CHECK_EQUAL(RU864870_RUSSIA_polygon, poly);
}

TEST(Geofence_Polygon, Tasmaina)
{
    Polygon_t poly = get_polygon(-42.475341, 145.066763);
    CHECK_EQUAL(AU915928_AUSTRALIA_polygon, poly);
}

TEST(Geofence_Polygon, Hokkaido)
{
    Polygon_t poly = get_polygon(41.849920, 141.553205);
    CHECK_EQUAL(AS920923_JAPAN_polygon, poly);
}

TEST(Geofence_Polygon, Honduras)
{
    Polygon_t poly = get_polygon(14.366573, -87.882151);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, Mexico)
{
    Polygon_t poly = get_polygon(25.097538, -104.062500);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, Honolulu)
{
    Polygon_t poly = get_polygon(21.31240491, -157.89276123);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, Israel)
{
    Polygon_t poly = get_polygon(32.896193, 35.374242);
    CHECK_EQUAL(EU863870_AFRICA_polygon, poly);
}

TEST(Geofence_Polygon, Norway)
{
    Polygon_t poly = get_polygon(62.629729, 10.526518);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

TEST(Geofence_Polygon, Sweden)
{
    Polygon_t poly = get_polygon(64.544663, 18.256346);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

TEST(Geofence_Polygon, Manila)
{
    Polygon_t poly = get_polygon(14.426168, 120.662100);
    CHECK_EQUAL(EU863870_PHILIPPINES_polygon, poly);
}

TEST(Geofence_Polygon, Taiwan)
{
    Polygon_t poly = get_polygon(23.883828, 121.194817);
    CHECK_EQUAL(AS923925_TAIWAN_polygon, poly);
}

TEST(Geofence_Polygon, Fukuda)
{
    Polygon_t poly = get_polygon(33.227660, 130.136745);
    CHECK_EQUAL(AS920923_JAPAN_polygon, poly);
}

TEST(Geofence_Polygon, Azores)
{
    Polygon_t poly = get_polygon(38.815743, -27.794621);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

TEST(Geofence_Polygon, Iceland)
{
    Polygon_t poly = get_polygon(64.990508, -16.913441);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

TEST(Geofence_Polygon, Greenland)
{
    Polygon_t poly = get_polygon(68.904679, -37.814359);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

TEST(Geofence_Polygon, Norge)
{
    Polygon_t poly = get_polygon(79.134947, 15.767405);
    CHECK_EQUAL(EU863870_EUROPE_polygon, poly);
}

/**
 * @brief South America
 * 
 */

TEST(Geofence_Polygon, Chile)
{
    Polygon_t poly = get_polygon(-33.447487, -70.673676);
    CHECK_EQUAL(AU915928_CHILE_polygon, poly);
}

TEST(Geofence_Polygon, Argentina)
{
    Polygon_t poly = get_polygon(-41.032962, -67.575228);
    CHECK_EQUAL(US902928_ARGENTINA_polygon, poly);
}

TEST(Geofence_Polygon, Falklands)
{
    Polygon_t poly = get_polygon(-51.639841, -58.97460938);
    CHECK_EQUAL(OUTSIDE_polygon, poly);
}

TEST(Geofence_Polygon, Brazil)
{
    Polygon_t poly = get_polygon(-13.49647277, -47.54882813);
    CHECK_EQUAL(AU915928_BRAZIL_polygon, poly);
}

TEST(Geofence_Polygon, Paraguay)
{
    Polygon_t poly = get_polygon(-22.18740499, -59.72167969);
    CHECK_EQUAL(US902928_ARGENTINA_polygon, poly);
}

TEST(Geofence_Polygon, Ecuador)
{
    Polygon_t poly = get_polygon(-1.40610884, -78.09082031);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, Bolivia)
{
    Polygon_t poly = get_polygon(-17.484816, -64.6875);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, calama_chile)
{
    Polygon_t poly = get_polygon(-22.899707, -68.87878418);
    CHECK_EQUAL(AU915928_CHILE_polygon, poly);
}

/**
 * @brief North America
 * 
 */

TEST(Geofence_Polygon, Arizona)
{
    Polygon_t poly = get_polygon(34.884128, -112.21435547);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, Albuquerque)
{
    Polygon_t poly = get_polygon(34.811999, -106.67724609);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}

TEST(Geofence_Polygon, Seattle)
{
    Polygon_t poly = get_polygon(47.79101618, -121.94824219);
    CHECK_EQUAL(US902928_NAMERICA_polygon, poly);
}
