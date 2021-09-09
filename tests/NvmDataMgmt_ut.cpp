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
 * @brief Verify if CRC test passes when indeed it should.
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

/**
 * @brief Just verify that the memset function works
 * 
 */
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

    CHECK_EQUAL(34, nvm->MacGroup2.MaxDCycle);
}

/**
 * @brief Test to see if there is garbage data in the EEPROM, it will fail
 * the CRC check. It should.
 * 
 */
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

/**
 * @brief IF the EEPROM is filled with garbage, it ought to restore no bytes
 * 
 */
TEST(NvmDataMgmt, test_if_nvm_restore_works_correctly_when_nvm_incorrect)
{
    /* Initilalise the mac layer */
    int ret = init_loramac_stack_and_tx_scheduling();

    uint16_t ret1 = NvmDataMgmtRestore();
    CHECK_EQUAL(0, ret1);
}

/**
 * @brief Tests whether eeprom read write works
 * 
 */
TEST(NvmDataMgmt, test_eeprom_read_write)
{
    int res = eeprom_read_write_test();
    CHECK_EQUAL(0, res);
}

/**
 * @brief Checks to see if the eeprom location to store NVM data is
 * correct for each region.
 * 
 */
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

/**
 * @brief Verify that struct sizes are the same between the ARM compiler
 * that compiles for the target and the test machine(e.g. x86). They have to
 * match. This includes pointer sizes and enum sizes.
 * 
 */
TEST(NvmDataMgmt, check_struct_sizes)
{
    CHECK_EQUAL(4, sizeof(float));
    CHECK_EQUAL(1, sizeof(bool));

    CHECK_EQUAL(52, sizeof(LoRaMacCryptoNvmData_t));
    CHECK_EQUAL(24, sizeof(LoRaMacNvmDataGroup1_t));

    /* Loramac LoRaMacNvmDataGroup2_t constituents */
    CHECK_EQUAL(1, sizeof(LoRaMacRegion_t));
    CHECK_EQUAL(60, sizeof(LoRaMacParams_t));
    CHECK_EQUAL(4, sizeof(uint32_t *));
    CHECK_EQUAL(4, sizeof(uint8_t *));
    CHECK_EQUAL(44, sizeof(MulticastCtx_t));
    CHECK_EQUAL(1, sizeof(DeviceClass_t));
    CHECK_EQUAL(1, sizeof(bool));
    CHECK_EQUAL(8, sizeof(SysTime_t));
    CHECK_EQUAL(4, sizeof(Version_t));
    CHECK_EQUAL(1, sizeof(ActivationType_t));

    CHECK_EQUAL(340, sizeof(LoRaMacNvmDataGroup2_t));

    CHECK_EQUAL(416, sizeof(SecureElementNvmData_t));
    CHECK_EQUAL(1180, sizeof(RegionNvmDataGroup2_t));

    CHECK_EQUAL(2200, sizeof(LoRaMacNvmData_t));
}

/**
 * @brief Reads and checks if the dummy eeprom returns
 * correct data.
 * 
 */
TEST(NvmDataMgmt, read_and_check_nvm_values)
{
    /* Setup environment params */
    LoRaMacNvmData_t nvm_inited;
    LoRaMacNvmData_t *nvm = &nvm_inited;

    printf("LoRaMacNvmData_t size: %d\n", sizeof(LoRaMacNvmData_t));

    memcpy((uint8_t *)nvm, new_nvm_struct, sizeof(LoRaMacNvmData_t));

    CHECK_EQUAL(18, nvm->Crypto.FCntList.FCntUp);

    uint8_t expected[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x82, 0x4D};

    for (unsigned int a = 0; a < sizeof(expected) / sizeof(expected[0]); a++)
    {
        CHECK_EQUAL(expected[a], nvm->SecureElement.JoinEui[a]);
    }

    CHECK_EQUAL(LORAMAC_REGION_EU868, nvm->MacGroup2.Region);
};

/**
 * @brief Check if NVM read is happening.
 * 
 */
TEST(NvmDataMgmt, check_crc_for_all_parameters)
{

    /* Setup environment params */
    LoRaMacNvmData_t nvm_inited;
    LoRaMacNvmData_t *nvm = &nvm_inited;

    printf("LoRaMacNvmData_t size: %d\n", sizeof(LoRaMacNvmData_t));

    NvmmRead((uint8_t *)nvm, sizeof(LoRaMacNvmData_t), 0);

    uint16_t offset = 0;

    // Crypto
    CHECK_FALSE(NvmmCrc32Check(sizeof(LoRaMacCryptoNvmData_t), offset) == false);
    offset += sizeof(LoRaMacCryptoNvmData_t);

    // Mac Group 1
    CHECK_FALSE(NvmmCrc32Check(sizeof(LoRaMacNvmDataGroup1_t), offset) == false);
    offset += sizeof(LoRaMacNvmDataGroup1_t);

    // Mac Group 2
    CHECK_FALSE(NvmmCrc32Check(sizeof(LoRaMacNvmDataGroup2_t), offset) == false);
    offset += sizeof(LoRaMacNvmDataGroup2_t);

    // Secure element
    CHECK_FALSE(NvmmCrc32Check(sizeof(SecureElementNvmData_t), offset) == false);
    offset += sizeof(SecureElementNvmData_t);

    // Region group 1
    CHECK_FALSE(NvmmCrc32Check(sizeof(RegionNvmDataGroup1_t), offset) == false);

    offset += sizeof(RegionNvmDataGroup1_t);

    // Region group 2
    CHECK_FALSE(NvmmCrc32Check(sizeof(RegionNvmDataGroup2_t), offset) == false);

    offset += sizeof(RegionNvmDataGroup2_t);

    // Class b
    CHECK_FALSE(NvmmCrc32Check(sizeof(LoRaMacClassBNvmData_t), offset) == false);
};
