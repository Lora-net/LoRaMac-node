#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "print_utils.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include "LoRaMac.h"
}

void run_stout_test(void);

TEST_GROUP(print_utils){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

int x = 123456;
double y = 3.14;

TEST(print_utils, print_test)
{
    print_bytes(&x, sizeof(x));
    print_bytes(&y, sizeof(y));
}

IGNORE_TEST(print_utils, test_print)
{

    struct stat st;
    int bytesWritten = 0;

    // Redirect stdout
    freopen("redir.txt", "w", stdout);

    print_bytes(&x, sizeof(x));

    // assert checking
    stat("redir.txt", &st);
    bytesWritten = st.st_size;
    printf("%d", bytesWritten);

    CHECK_TRUE(bytesWritten > 0);
}

/**
 * @brief Test to see if the printout of region
 * strings are correct
 * 
 */
TEST(print_utils, region_string_return_test)
{

    const char *ret;

    ret = get_lorawan_region_string(LORAMAC_REGION_AS923);
    STRCMP_EQUAL("LORAMAC_REGION_AS923", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_AU915);
    STRCMP_EQUAL("LORAMAC_REGION_AU915", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_CN470);
    STRCMP_EQUAL("LORAMAC_REGION_CN470", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_CN779);
    STRCMP_EQUAL("LORAMAC_REGION_CN779", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_EU433);
    STRCMP_EQUAL("LORAMAC_REGION_EU433", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_EU868);
    STRCMP_EQUAL("LORAMAC_REGION_EU868", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_KR920);
    STRCMP_EQUAL("LORAMAC_REGION_KR920", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_IN865);
    STRCMP_EQUAL("LORAMAC_REGION_IN865", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_US915);
    STRCMP_EQUAL("LORAMAC_REGION_US915", ret);

    ret = get_lorawan_region_string(LORAMAC_REGION_RU864);
    STRCMP_EQUAL("LORAMAC_REGION_RU864", ret);
}
