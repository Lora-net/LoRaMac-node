/**
 * @file LoRaWAN_config_switcher.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "LoRaWAN_config_switcher.h"
#include "stdio.h"
#include "geofence.h"

int8_t datarate_calculator(LoRaMacRegion_t LoRaMacRegion);

bool is_over_the_air_activation = true;
extern bool context_management_enabled;

picotracker_lorawan_settings_t get_otaa_abp_setting(LoRaMacRegion_t current_region)
{
    /**
     * @brief Alternate between OTAA and ABP(Helium network and TTN)
     * only when over EU countries.
     * Not sure if it works in other countries.
     * 
     */
    switch (current_region)
    {
    case LORAMAC_REGION_EU868:
        is_over_the_air_activation = !is_over_the_air_activation;
        break;
    case LORAMAC_REGION_US915:
        is_over_the_air_activation = !is_over_the_air_activation;
        break;
    case LORAMAC_REGION_CN470:
        is_over_the_air_activation = !is_over_the_air_activation;
        break;
    default:
        is_over_the_air_activation = false;
        break;
    }

    /**
     * @brief Enable NVM context management only when using
     * OTAA. We need to store the fcount, keys when using OTAA
     * so that we can avoid a join, which requires a downlink(hard to 
     * aquire.)
     * 
     */
    if (is_over_the_air_activation)
    {
        context_management_enabled = true;
        printf("Intialising with OTAA\n");
    }
    else
    {
        context_management_enabled = false;
        printf("Intialising with ABP\n");
    }

    /* select data rate depending on region of the world. */
    int8_t datarate = datarate_calculator(current_region);

    /* Init loramac stack */
    picotracker_lorawan_settings_t settings =
        {
            .datarate = datarate,
            .region = current_region,
            .is_over_the_air_activation = is_over_the_air_activation,
        };

    return settings;
}

void on_tx_complete()
{
}


/**
* Use datarate of DR_5 over the EU but DR_4 over rest of the world
*/
int8_t datarate_calculator(LoRaMacRegion_t LoRaMacRegion)
{
	int8_t dr = 0;

	switch (LoRaMacRegion)
	{
	case LORAMAC_REGION_EU868:
		dr = DR_5;
		break;

	default:
		dr = DR_4;
		break;
	}

	return dr;
}
