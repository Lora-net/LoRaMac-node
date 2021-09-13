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

/*!
 * MAC status strings
 */
const char *RegionStrings[] =
    {
        "LORAMAC_REGION_AS923",
        "LORAMAC_REGION_AU915",
        "LORAMAC_REGION_CN470",
        "LORAMAC_REGION_CN779",
        "LORAMAC_REGION_EU433",
        "LORAMAC_REGION_EU868",
        "LORAMAC_REGION_KR920",
        "LORAMAC_REGION_IN865",
        "LORAMAC_REGION_US915",
        "LORAMAC_REGION_RU864",
};

const char *get_lorawan_region_string(LoRaMacRegion_t region)
{
    return RegionStrings[region];
}