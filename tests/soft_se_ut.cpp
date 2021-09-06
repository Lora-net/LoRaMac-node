#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "secure-element.h"
#include "LoRaMac.h"
}

#include "nvm_images.hpp"
#include "string.h"

void run_stout_test(void);

TEST_GROUP(secure_element){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

extern SecureElementNvmData_t *SeNvm;

TEST(secure_element, test_setting_soft_se)
{

    LoRaMacNvmData_t nvm1;

    LoRaMacNvmData_t *nvm = &nvm1;

    memcpy((uint8_t *)nvm, new_nvm_struct, sizeof(LoRaMacNvmData_t));
    SecureElementInit(&nvm->SecureElement);

    CHECK_EQUAL(0x00, SeNvm->DevEui[0]);
    CHECK_EQUAL(0xED, SeNvm->DevEui[1]);

    /* Check if Dev eui is correctly set */

    uint8_t LoRaWAN_Device_Eui[8] = {0x00, 0xED, 0xDF, 0x3B, 0x7A, 0xDA, 0x7F, 0x6B};

    CHECK_TRUE(std::equal(std::begin(LoRaWAN_Device_Eui),
                          std::end(LoRaWAN_Device_Eui),
                          std::begin(SeNvm->DevEui)));

    uint8_t LoRaWAN_Join_Eui[8] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x82, 0x4D};

    CHECK_TRUE(std::equal(std::begin(LoRaWAN_Join_Eui),
                          std::end(LoRaWAN_Join_Eui),
                          std::begin(SeNvm->JoinEui)));
}
