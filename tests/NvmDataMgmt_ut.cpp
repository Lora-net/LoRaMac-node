/**
 * @file NvmDataMgmt_ut.cpp
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-08-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "NvmDataMgmt.h"
#include "LoRaMac.h"
}

#include "nvm_images.hpp"

TEST_GROUP(NvmDataMgmt){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

TEST(NvmDataMgmt, check_saved_length)
{

    uint16_t notifyFlags = LORAMAC_NVM_NOTIFY_FLAG_NONE;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_CRYPTO;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP1;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP2;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_SECURE_ELEMENT;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP1;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP2;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_CLASS_B;

    NvmDataMgmtEvent(notifyFlags);

    mock().expectOneCall("get_LoRaMacNvmData").andReturnValue(&first_transmission_eeprom_data);

    uint16_t res = NvmDataMgmtStore();
    CHECK_EQUAL(712, res);
}
