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
#include "nvmm.h"
#include "main.h"
#include "nvmm.h"
#include "region_nvm.h"
}
#include <string.h> // For memcmp()
#include "nvm_images.hpp"

TEST_GROUP(NvmDataMgmt){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;
/**
 * @brief Check if the data compressed and stored is the same after decompression.
 * 
 */

extern bool is_over_the_air_activation;

TEST(NvmDataMgmt, test_storing_of_data_with_compression)
{
    is_over_the_air_activation = false;
    /* Initilalise the mac layer */
    init_loramac_stack_and_tx_scheduling();

    /*
     * Set LoRaMacNvmData_t nvm as it would have been after being
     * restored from EEPROM. 
     */
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    memcpy((uint8_t *)nvm, new_nvm_struct, sizeof(LoRaMacNvmData_t));

    /* Make a copy for later comparison */
    LoRaMacNvmData_t original_nvm = *nvm;

    /* Set flags so that it does the store function */
    uint16_t notifyFlags = LORAMAC_NVM_NOTIFY_FLAG_NONE;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_CRYPTO;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP1;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP2;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_SECURE_ELEMENT;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP1;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP2;
    notifyFlags |= LORAMAC_NVM_NOTIFY_FLAG_CLASS_B;

    NvmDataMgmtEvent(notifyFlags);

    /* Now run the store/restore function */

    /* Store to eeprom in compressed state */
    uint16_t ret1 = NvmDataMgmtStore();

    /* Now assume restart from boot. LoRaMacNvmData_t Nvm not set yet */
    memset(nvm, 0, sizeof(LoRaMacNvmData_t));

    /* Now restore data from nvm after decompression */
    NvmDataMgmtRestore();

    /* Check if compression and then decompression does work */
    CHECK_EQUAL(0, memcmp(&original_nvm, nvm, sizeof(LoRaMacNvmData_t)));
}

/**
 * @brief Check if the crc checker works
 * 
 */
TEST(NvmDataMgmt, test_crc_check_pass)
{
    /* Initilalise the mac layer */
    int ret = init_loramac_stack_and_tx_scheduling();

    /*
     * Set LoRaMacNvmData_t nvm as it would have been after being
     * restored from EEPROM. 
     */
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    memcpy((uint8_t *)nvm, new_nvm_struct, sizeof(LoRaMacNvmData_t));

    bool crc_status;

    crc_status = is_crc_correct(sizeof(LoRaMacNvmDataGroup1_t), &nvm->MacGroup1);
    CHECK_TRUE(crc_status);

    crc_status = is_crc_correct(sizeof(LoRaMacNvmDataGroup2_t), &nvm->MacGroup2);
    CHECK_TRUE(crc_status);
}

TEST(NvmDataMgmt, memset_test)
{
    /* Initilalise the mac layer */
    int ret = init_loramac_stack_and_tx_scheduling();

    /*
     * Set LoRaMacNvmData_t nvm as it would have been after being
     * restored from EEPROM. 
     */
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    memcpy((uint8_t *)nvm, new_nvm_struct, sizeof(LoRaMacNvmData_t));

    /* Now assume restart from boot. LoRaMacNvmData_t Nvm not set yet */
    memset((void *)&nvm->MacGroup2.MaxDCycle, 34, sizeof(uint8_t));

    /* Check if compression and then decompression does work */
    CHECK_EQUAL(34, nvm->MacGroup2.MaxDCycle);
}

TEST(NvmDataMgmt, test_crc_check_fail)
{
    /* Initilalise the mac layer */
    int ret = init_loramac_stack_and_tx_scheduling();

    /*
     * Set LoRaMacNvmData_t nvm as it would have been after being
     * restored from EEPROM. 
     */
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    memcpy((uint8_t *)nvm, new_nvm_struct, sizeof(LoRaMacNvmData_t));

    nvm->MacGroup1.AdrAckCounter = 34234;
    nvm->MacGroup2.Crc32 = 3890845;

    bool crc_status;

    crc_status = is_crc_correct(sizeof(LoRaMacNvmDataGroup1_t), &nvm->MacGroup1);
    CHECK_FALSE(crc_status);

    crc_status = is_crc_correct(sizeof(LoRaMacNvmDataGroup2_t), &nvm->MacGroup2);
    CHECK_FALSE(crc_status);
}

TEST(NvmDataMgmt, test_if_nvm_restore_works_correctly_when_nvm_incorrect)
{
    /* Initilalise the mac layer */
    int ret = init_loramac_stack_and_tx_scheduling();

    uint16_t ret1 = NvmDataMgmtRestore();
    CHECK_EQUAL(0, ret1);
}

TEST(NvmDataMgmt, test_eeprom_read_write)
{
    int res = eeprom_read_write_test();
    CHECK_EQUAL(0, res);
}

TEST(NvmDataMgmt, check_if_correct_region_eeprom_location_set)
{
    uint16_t EEPROM_location;
    EEPROM_location = get_eeprom_location_for_region(LORAMAC_REGION_EU868);
    CHECK_EQUAL(0, EEPROM_location);

    EEPROM_location = get_eeprom_location_for_region(LORAMAC_REGION_US915);
    CHECK_EQUAL(1200, EEPROM_location);

    EEPROM_location = get_eeprom_location_for_region(LORAMAC_REGION_CN470);
    CHECK_EQUAL(2400, EEPROM_location);

    EEPROM_location = get_eeprom_location_for_region(LORAMAC_REGION_IN865);
    CHECK_EQUAL(0, EEPROM_location);
    EEPROM_location = get_eeprom_location_for_region(LORAMAC_REGION_AS923);
    CHECK_EQUAL(0, EEPROM_location);
    EEPROM_location = get_eeprom_location_for_region(LORAMAC_REGION_CN779);
    CHECK_EQUAL(0, EEPROM_location);
    EEPROM_location = get_eeprom_location_for_region(LORAMAC_REGION_RU864);
    CHECK_EQUAL(0, EEPROM_location);
    EEPROM_location = get_eeprom_location_for_region(LORAMAC_REGION_KR920);
    CHECK_EQUAL(0, EEPROM_location);
}