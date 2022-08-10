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

void print_registered_device(registered_devices_t registered_device)
{
    printf("Registered device: ");

    switch (registered_device)
    {
    case AS923_KEYS_EU1:
        printf("AS923_KEYS_EU1");
        break;
    case AS923_KEYS_AU1:
        printf("AS923_KEYS_AU1");
        break;
    case AU915_KEYS_AU1:
        printf("AU915_KEYS_AU1");
        break;
    case CN470_KEYS_EU1:
        printf("CN470_KEYS_EU1");
        break;
    case icspace26_eu1_eu_863_870_device_1:
        printf("icspace26_eu1_eu_863_870_device_1");
        break;
    case icspace26_eu1_eu_863_870_device_2:
        printf("icspace26_eu1_eu_863_870_device_2");
        break;
    case icspace26_eu1_eu_863_870_device_3:
        printf("icspace26_eu1_eu_863_870_device_3");
        break;
    case icspace26_eu1_eu_863_870_device_4:
        printf("icspace26_eu1_eu_863_870_device_4");
        break;
    case icspace26_eu1_eu_863_870_device_5:
        printf("icspace26_eu1_eu_863_870_device_5");
        break;
    case KR920_KEYS_EU1:
        printf("KR920_KEYS_EU1");
        break;
    case icspace26_au1_kr_920_923_ttn:
        printf("icspace26_au1_kr_920_923_ttn");
        break;
    case IN865_KEYS_EU1:
        printf("IN865_KEYS_EU1");
        break;
    case icspace26_us1_us915_device_1:
        printf("icspace26_us1_us915_device_1");
        break;
    case icspace26_us1_us915_device_2:
        printf("icspace26_us1_us915_device_2");
        break;
    case icspace26_us1_us915_device_3:
        printf("icspace26_us1_us915_device_3");
        break;
    case icspace26_us1_us915_device_4:
        printf("icspace26_us1_us915_device_4");
        break;
    case icspace26_us1_us915_device_5:
        printf("icspace26_us1_us915_device_5");
        break;
    case RU864_KEYS_EU1:
        printf("RU864_KEYS_EU1");
        break;
    case icspace26_helium_1:
        printf("icspace26_helium_1");
        break;
    case icspace26_helium_2:
        printf("icspace26_helium_2");
        break;
    case icspace26_helium_3:
        printf("icspace26_helium_3");
        break;
    case icspace26_helium_4:
        printf("icspace26_helium_4");
        break;
    case icspace26_helium_5:
        printf("icspace26_helium_5");
        break;
    case icspace26_helium_6:
        printf("icspace26_helium_6");
        break;
    case icspace26_helium_7:
        printf("icspace26_helium_7");
        break;
    case icspace26_helium_8:
        printf("icspace26_helium_8");
        break;
    case icspace26_helium_9:
        printf("icspace26_helium_9");
        break;
    case icspace26_helium_10:
        printf("icspace26_helium_10");
        break;
    case icspace26_helium_11:
        printf("icspace26_helium_11");
        break;
    case icspace26_helium_12:
        printf("icspace26_helium_12");
        break;
    case icspace26_helium_13:
        printf("icspace26_helium_13");
        break;
    case icspace26_helium_14:
        printf("icspace26_helium_14");
        break;
    case icspace26_helium_15:
        printf("icspace26_helium_15");
        break;
    case icspace26_helium_16:
        printf("icspace26_helium_16");
        break;
    case icspace26_helium_17:
        printf("icspace26_helium_17");
        break;
    case icspace26_helium_18:
        printf("icspace26_helium_18");
        break;
    case icspace26_helium_19:
        printf("icspace26_helium_19");
        break;
    case icspace26_helium_20:
        printf("icspace26_helium_20");
        break;
    default:

        printf("unknown device registration");
        break;
    }
}
