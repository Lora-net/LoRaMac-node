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

typedef enum
{
    THE_THINGS_NETWORK,
    HELIUM_NETWORK
} network_t;

typedef struct
{
    LoRaMacRegion_t region;
    int8_t datarate;
    bool is_over_the_air_activation;
    uint32_t tx_interval;
    network_t current_network;
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
    /**
		 * OTAA Credentials 
		 */
    uint8_t LoRaWAN_Device_Eui[8];
    uint8_t LoRaWAN_Join_Eui[8];
    uint8_t Network_Key[16];

} network_keys_t;

network_keys_t get_network_keys(LoRaMacRegion_t Loramac_region, network_t network);
picotracker_lorawan_settings_t get_lorawan_setting(LoRaMacRegion_t current_region);
network_t get_current_network();


#endif
#ifdef __cplusplus
}
#endif