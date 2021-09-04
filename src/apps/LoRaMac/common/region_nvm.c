/**
 * @file region_nvm.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "region_nvm.h"

uint16_t get_eeprom_location_for_region(LoRaMacRegion_t region)
{
    switch (region)
    {

    case LORAMAC_REGION_EU868:
        return 0;
        break;
    case LORAMAC_REGION_US915:
        return 1200;
        break;
    case LORAMAC_REGION_CN470:
        return 2400;
        break;
    default:
        return 0; // should we dare to overwrite EU868?
        break;
    }
}