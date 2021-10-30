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
#include "bsp.h"
#include "geofence.h"
#include "eeprom-board.h"
#include "LoRaWAN_config_switcher.h"
}
#include <string.h> // For memcmp()
#include "nvm_images.hpp"

void set_correct_notify_flags();

TEST_GROUP(NvmDataMgmt){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

/**
 * @brief Test to see if there is garbage data in the EEPROM, it will fail
 * the CRC check. It should.
 * 
 */
TEST(NvmDataMgmt, test_crc_check_fail)
{
    /* Initilalise the mac layer */
    //int ret = init_loramac_stack_and_tx_scheduling(true);

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
 * @brief Tests whether eeprom read write works
 * 
 */
TEST(NvmDataMgmt, test_eeprom_read_write)
{
    int res = eeprom_read_write_test();
    CHECK_EQUAL(0, res);
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
};

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

    memcpy((uint8_t *)nvm, new_nvm_struct, sizeof(LoRaMacNvmData_t));

    CHECK_EQUAL(18, nvm->Crypto.FCntList.FCntUp);

    uint8_t expected[8] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x82, 0x4D};

    MEMCMP_EQUAL(&expected, &nvm->SecureElement.JoinEui, 8);

    CHECK_EQUAL(LORAMAC_REGION_EU868, nvm->MacGroup2.Region);
};

void set_correct_notify_flags()
{
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
}

TEST(NvmDataMgmt, test_storing_of_fcount_and_keys)
{
    LoRaMacRegion_t Loramac_region = LORAMAC_REGION_US915;
    set_current_loramac_region(Loramac_region);

    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    // Use the first device setting

    // init_loramac_stack_and_tx_scheduling(true);

    // now store as if a few transmissions have been done
    nvm->Crypto.FCntList.FCntUp += 1;
    nvm->MacGroup2.MacParams.ReceiveDelay1 = 5000;
    nvm->MacGroup2.MacParams.ReceiveDelay2 = 6000;
    set_correct_notify_flags();
    NvmDataMgmtStore();

    nvm->Crypto.FCntList.FCntUp += 1;
    set_correct_notify_flags();
    NvmDataMgmtStore();

    // now verify if the eeprom is the way it must be.
    uint32_t read_bytes = sizeof(network_keys_t);
    uint8_t current_eeprom[read_bytes];

    // IT should be the second device credential for the US, because after running init_loramac_stack_and_tx_scheduling(), the s
    // stack moves on to the second one.
    uint8_t expected_eeprom_2nd_device[read_bytes] =
        {
            0x0D, 0x2A, 0xDD, 0xA4, 0xFC, 0x9D, 0xD2, 0x17, 0xE8, 0xE3, 0xB3, 0x8E, 0xDB, 0x48, 0x90, 0x20, // FNwkSIntKey_SNwkSIntKey_NwkSEncKey
            0xAA, 0x9E, 0x20, 0xC4, 0x18, 0xEE, 0x2F, 0x09, 0x4F, 0x3A, 0xBE, 0x6C, 0xEF, 0x88, 0x52, 0x67, // AppSKey
            0xCE, 0x03, 0x00, 0x48,                                                                         // DevAddr for Helium network
            0x43, 0x9C, 0x00, 0x00,                                                                         // frame_count of 40003
            0x88, 0x13, 0x00, 0x00,                                                                         // ReceiveDelay1
            0x70, 0x17, 0x00, 0x00,                                                                         // ReceiveDelay2
            0x4E, 0x37, 0x8A, 0x29,                                                                         // Crc32
        };

    registered_devices_t registered_device = get_current_network();
    uint16_t eeprom_location = registered_device * sizeof(network_keys_t);
    EepromMcuReadBuffer(eeprom_location, current_eeprom, sizeof(network_keys_t));

    MEMCMP_EQUAL(expected_eeprom_2nd_device, current_eeprom, read_bytes);
    CHECK_EQUAL(icspace26_helium_1, registered_device);
    CHECK_EQUAL(936, eeprom_location);

    // Now move on to the next device credentials

    // Startup as usual
    // init_loramac_stack_and_tx_scheduling(true);

    // now store as if a few transmissions have been done
    nvm->Crypto.FCntList.FCntUp += 1;
    set_correct_notify_flags();
    NvmDataMgmtStore();

    nvm->Crypto.FCntList.FCntUp += 1;
    set_correct_notify_flags();
    NvmDataMgmtStore();

    // IT should be the second device credential for the US, because after running init_loramac_stack_and_tx_scheduling(), the s
    // stack moves on to the second one.
    uint8_t expected_eeprom_1st_device[read_bytes] =
        {
            0x47, 0x18, 0xA9, 0x3A, 0x91, 0x7E, 0xB6, 0x1A, 0xFD, 0xB3, 0x78, 0x6E, 0xA0, 0x4E, 0xC3, 0xEE, // FNwkSIntKey_SNwkSIntKey_NwkSEncKey
            0xD3, 0xAB, 0xC3, 0x3C, 0x12, 0xD9, 0x75, 0xF2, 0x78, 0x5F, 0xFA, 0x46, 0xAF, 0x75, 0x95, 0xE2, // AppSKey
            0x99, 0x4E, 0x0C, 0x26,                                                                         // DevAddr for Things Network
            0x03, 0x00, 0x00, 0x00,                                                                         // frame_count of 3
            0xE8, 0x03, 0x00, 0x00,                                                                         // ReceiveDelay1
            0xD0, 0x07, 0x00, 0x00,                                                                         // ReceiveDelay2
            0xD4, 0x09, 0x3E, 0x2F,                                                                         // Crc32
        };

    registered_device = get_current_network();
    eeprom_location = registered_device * sizeof(network_keys_t);
    EepromMcuReadBuffer(eeprom_location, current_eeprom, sizeof(network_keys_t));

    CHECK_EQUAL(icspace26_us1_us915_device_2, registered_device);
    CHECK_EQUAL(676, eeprom_location);
    MEMCMP_EQUAL(expected_eeprom_1st_device, current_eeprom, read_bytes);

    // Finally, lets move to the next device setting, which should be the first one.
    // It should have persisted the RX1/2 delay value

    // Startup as usual
    // init_loramac_stack_and_tx_scheduling(true);

    uint8_t expected_eeprom_1st_device_second_time[read_bytes] =
        {
            0xA9, 0xB6, 0xFF, 0xF8, 0x99, 0x0C, 0x16, 0x70, 0x3A, 0xB2, 0xF7, 0x87, 0xB3, 0x6C, 0xAE, 0x4D, // FNwkSIntKey_SNwkSIntKey_NwkSEncKey
            0xCB, 0xDD, 0x41, 0x1F, 0x0E, 0xEF, 0x20, 0x98, 0x17, 0x3B, 0xF3, 0x0F, 0xE3, 0x90, 0xA1, 0x35, // AppSKey
            0x9F, 0x02, 0x00, 0x48,                                                                         // DevAddr for Helium network
            0x41, 0x9C, 0x00, 0x00,                                                                         // frame_count of 40001
            0xE8, 0x03, 0x00, 0x00,                                                                         // ReceiveDelay1
            0xD0, 0x07, 0x00, 0x00,                                                                         // ReceiveDelay2
            0x44, 0xE0, 0xC3, 0xBB,                                                                         // Crc32
        };

    registered_device = get_current_network();
    eeprom_location = registered_device * sizeof(network_keys_t);
    EepromMcuReadBuffer(eeprom_location, current_eeprom, sizeof(network_keys_t));

    MEMCMP_EQUAL(expected_eeprom_1st_device_second_time, current_eeprom, read_bytes);
    CHECK_EQUAL(icspace26_helium_2, registered_device);
    CHECK_EQUAL(988, eeprom_location);
}

/**
 * @brief test the function update_device_credentials_to_eeprom() to see if
 * it correctly changes the value in EEPROM
 * 
 */
TEST(NvmDataMgmt, test_behaviour_of_update_device_credentials_to_eeprom)
{
    network_keys_t keys = {
        //icspace26_us1_us915_device_2
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x47, 0x18, 0xA9, 0x3A, 0x91, 0x7E, 0xB6, 0x1A, 0xFD, 0xB3, 0x78, 0x6E, 0xA0, 0x4E, 0xC3, 0xEE},
        .AppSKey = {0xD3, 0xAB, 0xC3, 0x3C, 0x12, 0xD9, 0x75, 0xF2, 0x78, 0x5F, 0xFA, 0x46, 0xAF, 0x75, 0x95, 0xE2},
        .DevAddr = (uint32_t)0x260C4E99,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    };
    registered_devices_t registered_device = icspace26_us1_us915_device_2;

    // do update credentials
    update_device_credentials_to_eeprom(keys, registered_device);

    network_keys_t current_eeprom;

    // now check the flash data
    EepromMcuReadBuffer(icspace26_us1_us915_device_2 * sizeof(network_keys_t), (uint8_t *)&current_eeprom, sizeof(network_keys_t));

    MEMCMP_EQUAL(&keys, &current_eeprom, sizeof(network_keys_t));
}

/**
 * @brief test the function update_device_credentials_to_eeprom() returns correct values
 * 
 */
TEST(NvmDataMgmt, test_return_value_of_update_device_credentials_to_eeprom)
{
    network_keys_t keys = {
        //icspace26_us1_us915_device_2
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x47, 0x18, 0xA9, 0x3A, 0x91, 0x7E, 0xB6, 0x1A, 0xFD, 0xB3, 0x78, 0x6E, 0xA0, 0x4E, 0xC3, 0xEE},
        .AppSKey = {0xD3, 0xAB, 0xC3, 0x3C, 0x12, 0xD9, 0x75, 0xF2, 0x78, 0x5F, 0xFA, 0x46, 0xAF, 0x75, 0x95, 0xE2},
        .DevAddr = (uint32_t)0x260C4E99,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    };

    registered_devices_t registered_device = icspace26_us1_us915_device_2;

    // do update credentials. It should update from a EEPROm state of all zeros
    bool ret = update_device_credentials_to_eeprom(keys, registered_device);

    CHECK_TRUE(ret);

    // do update credentials. It should in theory not change any bytes since its already been set
    ret = update_device_credentials_to_eeprom(keys, registered_device);

    CHECK_FALSE(ret);

}
