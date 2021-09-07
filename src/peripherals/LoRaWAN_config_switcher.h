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

#include "LoRaMac.h"

typedef struct
{
    LoRaMacRegion_t region;
    int8_t datarate;
    bool is_over_the_air_activation;

} picotracker_lorawan_settings_t;

picotracker_lorawan_settings_t get_otaa_abp_setting(LoRaMacRegion_t current_region);
void on_tx_complete();