/**
 * @file LoRaWAN_config_switcher.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef REGION_SETTING_H
#define REGION_SETTING_H

#include "LoRaMac.h"
#include "stdint.h"

#define SIZE_OF_NETWORK_KEYS_T 52

    typedef struct
    {
        LoRaMacRegion_t region;
        int8_t datarate;
        bool is_over_the_air_activation;
        uint32_t tx_interval;
    } picotracker_lorawan_settings_t;

    typedef struct
    {
        /**
		 * ABP Credentials
		 */
        uint8_t FNwkSIntKey_SNwkSIntKey_NwkSEncKey[16];
        uint8_t AppSKey[16];
        uint32_t DevAddr;
        uint32_t frame_count;
        uint32_t ReceiveDelay1;
        uint32_t ReceiveDelay2;
        uint32_t Crc32;
    } network_keys_t;

    typedef enum
    {
        AS923_KEYS_EU1,
        AS923_KEYS_AU1,
        AU915_KEYS_AU1,
        CN470_KEYS_EU1,
        icspace26_eu1_eu_863_870_device_1,
        icspace26_eu1_eu_863_870_device_2,
        icspace26_eu1_eu_863_870_device_3,
        icspace26_eu1_eu_863_870_device_4,
        icspace26_eu1_eu_863_870_device_5,
        KR920_KEYS_EU1,
        icspace26_au1_kr_920_923_ttn,
        IN865_KEYS_EU1,
        icspace26_us1_us915_device_1,
        icspace26_us1_us915_device_2,
        icspace26_us1_us915_device_3,
        icspace26_us1_us915_device_4,
        icspace26_us1_us915_device_5,
        RU864_KEYS_EU1, //18 devices on TTN
        icspace26_helium_1,
        icspace26_helium_2,
        icspace26_helium_3,
        icspace26_helium_4,
        icspace26_helium_5,
        icspace26_helium_6,
        icspace26_helium_7,
        icspace26_helium_8,
        icspace26_helium_9,
        icspace26_helium_10,
        NUMBER_OF_REGISTERED_DEVICES, // this should be the last in the enum

    } registered_devices_t;

    registered_devices_t get_current_network();
    network_keys_t get_current_network_keys();
    picotracker_lorawan_settings_t get_lorawan_setting(LoRaMacRegion_t current_region);
    void switch_to_next_registered_credentials();

#endif
#ifdef __cplusplus
}
#endif