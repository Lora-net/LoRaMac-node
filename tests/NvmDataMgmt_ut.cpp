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
 * @brief Verify if CRC test passes when indeed it should.
 * 
 */
TEST(NvmDataMgmt, test_crc_check_pass)
{
    /* Initilalise the mac layer */
    int ret = init_loramac_stack_and_tx_scheduling(true);

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
    int ret = init_loramac_stack_and_tx_scheduling(true);

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
    int ret = init_loramac_stack_and_tx_scheduling(true);

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
    current_geofence_status.current_loramac_region = Loramac_region;

    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibReq);
    LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

    // Use the first device setting

    init_loramac_stack_and_tx_scheduling(true);

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
            0x6A, 0x0C, 0x26, 0x51, 0xF3, 0x75, 0x73, 0x40, 0x0E, 0x8D, 0xCC, 0xE6, 0x27, 0xFE, 0x33, 0x8D, // FNwkSIntKey
            0x6A, 0x0C, 0x26, 0x51, 0xF3, 0x75, 0x73, 0x40, 0x0E, 0x8D, 0xCC, 0xE6, 0x27, 0xFE, 0x33, 0x8D, // SNwkSIntKey
            0x6A, 0x0C, 0x26, 0x51, 0xF3, 0x75, 0x73, 0x40, 0x0E, 0x8D, 0xCC, 0xE6, 0x27, 0xFE, 0x33, 0x8D, // NwkSEncKey
            0xF9, 0x88, 0x01, 0xF6, 0x58, 0xC2, 0xB2, 0x83, 0x79, 0x2D, 0x3C, 0xD5, 0x5C, 0x08, 0x3A, 0x79, // AppSKey
            0x2D, 0x00, 0x00, 0x48,                                                                         // DevAddr for Helium network
            0x03, 0x00, 0x00, 0x00,                                                                         // frame_count of 3
            0x88, 0x13, 0x00, 0x00,                                                                         // ReceiveDelay1
            0x70, 0x17, 0x00, 0x00,                                                                         // ReceiveDelay2
            0xB8, 0x5A, 0xB7, 0x1B,                                                                         // Crc32
        };

    registered_devices_t registered_device = get_current_network();
    uint16_t eeprom_location = registered_device * sizeof(network_keys_t);
    EepromMcuReadBuffer(eeprom_location, current_eeprom, sizeof(network_keys_t));

    MEMCMP_EQUAL(expected_eeprom_2nd_device, current_eeprom, read_bytes);
    CHECK_EQUAL(icspace26_helium_1, registered_device);
    CHECK_EQUAL(420, eeprom_location);

    // Now move on to the next device credentials

    // Startup as usual
    init_loramac_stack_and_tx_scheduling(true);

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
            0xAC, 0x7E, 0xF6, 0x61, 0x48, 0x02, 0x71, 0x9C, 0x03, 0xD0, 0xF5, 0x3D, 0xDE, 0xE4, 0xEE, 0x07, // FNwkSIntKey
            0xAC, 0x7E, 0xF6, 0x61, 0x48, 0x02, 0x71, 0x9C, 0x03, 0xD0, 0xF5, 0x3D, 0xDE, 0xE4, 0xEE, 0x07, // SNwkSIntKey
            0xAC, 0x7E, 0xF6, 0x61, 0x48, 0x02, 0x71, 0x9C, 0x03, 0xD0, 0xF5, 0x3D, 0xDE, 0xE4, 0xEE, 0x07, // NwkSEncKey
            0xE4, 0x70, 0xDC, 0x81, 0xE1, 0x43, 0x8D, 0x99, 0x14, 0x22, 0x84, 0x83, 0xD9, 0xA3, 0x6B, 0xC7, // AppSKey
            0x28, 0xB9, 0x0C, 0x26,                                                                         // DevAddr for Helium network
            0x03, 0x00, 0x00, 0x00,                                                                         // frame_count of 3
            0xE8, 0x03, 0x00, 0x00,                                                                         // ReceiveDelay1
            0xD0, 0x07, 0x00, 0x00,                                                                         // ReceiveDelay2
            0x2E, 0x24, 0x43, 0x82,                                                                         // Crc32
        };

    registered_device = get_current_network();
    eeprom_location = registered_device * sizeof(network_keys_t);
    EepromMcuReadBuffer(eeprom_location, current_eeprom, sizeof(network_keys_t));

    CHECK_EQUAL(icspace26_us1_us915_device_1, registered_device);
    CHECK_EQUAL(672, eeprom_location);
    MEMCMP_EQUAL(expected_eeprom_1st_device, current_eeprom, read_bytes);

    // Finally, lets move to the next device setting, which should be the first one.
    // It should have persisted the RX1/2 delay value

    // Startup as usual
    init_loramac_stack_and_tx_scheduling(true);

    uint8_t expected_eeprom_1st_device_second_time[read_bytes] =
        {
            0x6A, 0x0C, 0x26, 0x51, 0xF3, 0x75, 0x73, 0x40, 0x0E, 0x8D, 0xCC, 0xE6, 0x27, 0xFE, 0x33, 0x8D, // FNwkSIntKey
            0x6A, 0x0C, 0x26, 0x51, 0xF3, 0x75, 0x73, 0x40, 0x0E, 0x8D, 0xCC, 0xE6, 0x27, 0xFE, 0x33, 0x8D, // SNwkSIntKey
            0x6A, 0x0C, 0x26, 0x51, 0xF3, 0x75, 0x73, 0x40, 0x0E, 0x8D, 0xCC, 0xE6, 0x27, 0xFE, 0x33, 0x8D, // NwkSEncKey
            0xF9, 0x88, 0x01, 0xF6, 0x58, 0xC2, 0xB2, 0x83, 0x79, 0x2D, 0x3C, 0xD5, 0x5C, 0x08, 0x3A, 0x79, // AppSKey
            0x2D, 0x00, 0x00, 0x48,                                                                         // DevAddr for Helium network
            0x04, 0x00, 0x00, 0x00,                                                                         // frame_count of 4
            0x88, 0x13, 0x00, 0x00,                                                                         // ReceiveDelay1
            0x70, 0x17, 0x00, 0x00,                                                                         // ReceiveDelay2
            0x37, 0xB3, 0x2F, 0x6F,                                                                         // Crc32
        };

    registered_device = get_current_network();
    eeprom_location = registered_device * sizeof(network_keys_t);
    EepromMcuReadBuffer(eeprom_location, current_eeprom, sizeof(network_keys_t));

    MEMCMP_EQUAL(expected_eeprom_1st_device_second_time, current_eeprom, read_bytes);
    CHECK_EQUAL(icspace26_helium_1, registered_device);
    CHECK_EQUAL(420, eeprom_location);
}
