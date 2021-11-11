/**
 * @file print_utils.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-08-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "stdio.h"
#include "print_utils.h"
#include "geofence.h"

void print_bytes(void *ptr, int size)
{
    unsigned char *p = ptr;
    int i;
    for (i = 0; i < size; i++)
    {
        // printf("%02hhX ", p[i]);
        printf("%02x ", p[i]);
    }
    printf("\n");
}

const char *get_lorawan_region_string(LoRaMacRegion_t region)
{

    switch (region)
    {
    case LORAMAC_REGION_AS923:
        return "LORAMAC_REGION_AS923";
        break;

    case LORAMAC_REGION_AU915:
        return "LORAMAC_REGION_AU915";
        break;

    case LORAMAC_REGION_CN470:
        return "LORAMAC_REGION_CN470";
        break;

    case LORAMAC_REGION_CN779:
        return "LORAMAC_REGION_CN779";
        break;

    case LORAMAC_REGION_EU433:
        return "LORAMAC_REGION_EU433";
        break;

    case LORAMAC_REGION_EU868:
        return "LORAMAC_REGION_EU868";
        break;

    case LORAMAC_REGION_KR920:
        return "LORAMAC_REGION_KR920";
        break;

    case LORAMAC_REGION_IN865:
        return "LORAMAC_REGION_IN865";
        break;

    case LORAMAC_REGION_US915:
        return "LORAMAC_REGION_US915";
        break;

    case LORAMAC_REGION_RU864:
        return "LORAMAC_REGION_RU864";
        break;
    default:
        return "UNKNOWN_REGION";
        break;
    }
}

void print_current_region()
{
    const char *region_string = get_lorawan_region_string(get_current_loramac_region());
    printf("Initialising Loramac Stack with Loramac region: %s\n", region_string);
}
