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
        uint8_t FNwkSIntKey[16];
        uint8_t SNwkSIntKey[16];
        uint8_t NwkSEncKey[16];
        uint8_t AppSKey[16];
        uint32_t DevAddr;
        uint32_t frame_count;
        uint32_t Crc32;
    } network_keys_t;

    typedef enum
    {
        AS923_KEYS_EU1,
        AS923_KEYS_AU1,
        AU915_KEYS_AU1,
        CN470_KEYS_EU1,
        EU868_KEYS_EU1,
        HELIUM_KEYS,
        KR920_KEYS_EU1,
        IN865_KEYS_EU1,
        US915_KEYS_US1,
        RU864_KEYS_EU1,
        NUMBER_OF_REGISTERED_DEVICES,

    } registered_devices_t;

    registered_devices_t get_current_network();
    network_keys_t get_current_network_keys();
    picotracker_lorawan_settings_t get_lorawan_setting(LoRaMacRegion_t current_region);
    void switch_to_next_region();

#endif
#ifdef __cplusplus
}
#endif