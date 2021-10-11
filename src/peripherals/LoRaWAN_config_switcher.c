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
uint32_t tx_interval_calculator(LoRaMacRegion_t LoRaMacRegion);
network_keys_t get_next_network_keys_in_region(LoRaMacRegion_t region);

bool is_over_the_air_activation = false;

picotracker_lorawan_settings_t get_lorawan_setting(LoRaMacRegion_t current_region)
{

    /* select data rate depending on region of the world. */
    int8_t datarate = datarate_calculator(current_region);
    uint32_t tx_interval = tx_interval_calculator(current_region);

    /* Init loramac stack */
    picotracker_lorawan_settings_t settings =
        {
            .datarate = datarate,
            .region = current_region,
            .is_over_the_air_activation = is_over_the_air_activation,
            .tx_interval = tx_interval,
        };

    return settings;
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

    case LORAMAC_REGION_US915:
        dr = DR_4; // IN the US, this is the highest datarate
        break;

    default:
        dr = DR_4;
        break;
    }

    return dr;
}

/**
 * @brief select which TX interval to use for each region
 * 
 * @param LoRaMacRegion Input region
 * @return uint32_t Interval period in milliseconds 
 */
uint32_t tx_interval_calculator(LoRaMacRegion_t LoRaMacRegion)
{
    // TODO: update for more regions
    uint32_t interval = 0;

    switch (LoRaMacRegion)
    {
    case LORAMAC_REGION_EU868:
        interval = 60000;
        break;
    case LORAMAC_REGION_US915:
        interval = 60000;
        break;
    default:
        interval = 60000;
        break;
    }

    return interval;
}

// Device icspace26-hab-as-923
#define COMMON_AS923_KEYS_EU1                                                                          \
    {                                                                                                  \
        0xAE, 0x8F, 0xC7, 0xCD, 0x28, 0x79, 0xDA, 0x24, 0x8D, 0xF5, 0x2A, 0x09, 0x8A, 0x60, 0x6E, 0x33 \
    }

// Device au1-as-923
#define COMMON_AS923_KEYS_AU1                                                                          \
    {                                                                                                  \
        0x56, 0x2B, 0xD9, 0x9C, 0x9C, 0x1C, 0x09, 0x8A, 0x30, 0x1A, 0x5A, 0x30, 0xBB, 0xDC, 0xCB, 0x7B \
    }

// Device: icspace26-au1-au-915-928-fsb-2
#define COMMON_AU915_KEYS                                                                              \
    {                                                                                                  \
        0xE3, 0xD6, 0x5F, 0x9A, 0xAE, 0x16, 0x67, 0x85, 0xE1, 0x6E, 0x8B, 0x1F, 0x79, 0xCC, 0x65, 0x36 \
    }

// Device: icspace26-hab-cn-470-510-fsb-11
#define COMMON_CN470_KEYS                                                                              \
    {                                                                                                  \
        0x82, 0xF8, 0x47, 0x2D, 0x18, 0x83, 0x3E, 0x9B, 0xE0, 0x5E, 0x76, 0x9E, 0x2A, 0x28, 0x94, 0xB2 \
    }

// Device: icspace26-hab-eu-863-870
#define COMMON_EU868_KEYS                                                                              \
    {                                                                                                  \
        0x54, 0x58, 0xC6, 0xF6, 0xF8, 0x65, 0x87, 0x1F, 0x56, 0xAF, 0xC8, 0xF4, 0xC1, 0x2A, 0xEA, 0xE6 \
    }

// Helium network device
#define COMMON_HELIUM_KEYS                                                      \
    {                                                                           \
        106, 12, 38, 81, 243, 117, 115, 64, 14, 141, 204, 230, 39, 254, 51, 141 \
    }

// Device: icspace26-hab-kr-920-923-ttn
#define COMMON_KR920_KEYS                                                                              \
    {                                                                                                  \
        0xCD, 0x5F, 0x49, 0x98, 0xA1, 0x15, 0x1D, 0x3F, 0xF1, 0xED, 0xB4, 0x47, 0xD4, 0xDF, 0x66, 0x01 \
    }

#define COMMON_IN865_KEYS                                                                              \
    {                                                                                                  \
        0x64, 0xB4, 0x78, 0x96, 0xD9, 0x24, 0x6F, 0x99, 0xA5, 0xA6, 0x56, 0x08, 0x80, 0x04, 0x70, 0x9A \
    }

/**
 * @brief Device nam1-us-902-928-fsb-2. Its registered on nam1.cloud.thethings.network
 * 
 */
#define COMMON_US915_KEYS                                                                              \
    {                                                                                                  \
        0xAC, 0x7E, 0xF6, 0x61, 0x48, 0x02, 0x71, 0x9C, 0x03, 0xD0, 0xF5, 0x3D, 0xDE, 0xE4, 0xEE, 0x07 \
    }
// Device icspace26-hab-ru-864-870-ttn
#define COMMON_RU864_KEYS                                                                              \
    {                                                                                                  \
        0xB4, 0xDF, 0xCD, 0x17, 0x6E, 0x52, 0xE9, 0x3C, 0x27, 0x8C, 0x10, 0xFF, 0xE1, 0x0D, 0x45, 0xC6 \
    }

const network_keys_t network_key_list[] = {
    {
        .FNwkSIntKey = COMMON_AS923_KEYS_EU1,
        .SNwkSIntKey = COMMON_AS923_KEYS_EU1,
        .NwkSEncKey = COMMON_AS923_KEYS_EU1,
        .AppSKey = {0x67, 0x27, 0x87, 0x20, 0x50, 0xA8, 0xD4, 0x0D, 0x3A, 0xA0, 0xC2, 0x75, 0x09, 0x03, 0xE8, 0x13},
        .DevAddr = (uint32_t)0x260BD61D,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,
    },
    {
        .FNwkSIntKey = COMMON_AS923_KEYS_AU1,
        .SNwkSIntKey = COMMON_AS923_KEYS_AU1,
        .NwkSEncKey = COMMON_AS923_KEYS_AU1,
        .AppSKey = {0x34, 0x3C, 0xCF, 0x72, 0xC1, 0xE2, 0xC3, 0xD7, 0xF0, 0x92, 0x06, 0x1A, 0x02, 0xFF, 0x80, 0xAE},
        .DevAddr = (uint32_t)0x260D24A9,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey = COMMON_AU915_KEYS,
        .SNwkSIntKey = COMMON_AU915_KEYS,
        .NwkSEncKey = COMMON_AU915_KEYS,
        .AppSKey = {0x49, 0xE5, 0xE3, 0x6B, 0xD9, 0xCC, 0x3A, 0x14, 0x0D, 0x4C, 0xC0, 0xD0, 0xB9, 0x76, 0xE7, 0x52},
        .DevAddr = (uint32_t)0x260DE191,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey = COMMON_CN470_KEYS,
        .SNwkSIntKey = COMMON_CN470_KEYS,
        .NwkSEncKey = COMMON_CN470_KEYS,
        .AppSKey = {0xA7, 0x61, 0x92, 0x2C, 0xF7, 0x65, 0x9E, 0x7B, 0x24, 0x74, 0x2F, 0x98, 0xDD, 0xE1, 0x3D, 0x5E},
        .DevAddr = (uint32_t)0x260B74CE,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey = COMMON_EU868_KEYS,
        .SNwkSIntKey = COMMON_EU868_KEYS,
        .NwkSEncKey = COMMON_EU868_KEYS,
        .AppSKey = {0xD2, 0x02, 0x95, 0x6B, 0xF5, 0x36, 0xFF, 0x15, 0x29, 0xA0, 0x83, 0x58, 0xAC, 0x3E, 0xE8, 0x88},
        .DevAddr = (uint32_t)0x260BD67C,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey = COMMON_HELIUM_KEYS,
        .SNwkSIntKey = COMMON_HELIUM_KEYS,
        .NwkSEncKey = COMMON_HELIUM_KEYS,
        .AppSKey = {249, 136, 1, 246, 88, 194, 178, 131, 121, 45, 60, 213, 92, 8, 58, 121},
        .DevAddr = (uint32_t)0x4800002d, // Note that the Helium console and virtual lorawan device will display this in the wrong endian(i.e. 0x2d000048)
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey = COMMON_KR920_KEYS,
        .SNwkSIntKey = COMMON_KR920_KEYS,
        .NwkSEncKey = COMMON_KR920_KEYS,
        .AppSKey = {0xC9, 0x47, 0x9E, 0xF9, 0xB2, 0xF5, 0x09, 0x8C, 0x37, 0xD9, 0xAD, 0x07, 0xB2, 0x57, 0xD3, 0x33},
        .DevAddr = (uint32_t)0x260B0C3C,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey = COMMON_IN865_KEYS,
        .SNwkSIntKey = COMMON_IN865_KEYS,
        .NwkSEncKey = COMMON_IN865_KEYS,
        .AppSKey = {0xB9, 0xA6, 0xF2, 0x26, 0x4B, 0x48, 0x05, 0x5B, 0x79, 0x53, 0xDE, 0x55, 0x9C, 0x2A, 0x77, 0x08},
        .DevAddr = (uint32_t)0x260BE033,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey = COMMON_US915_KEYS,
        .SNwkSIntKey = COMMON_US915_KEYS,
        .NwkSEncKey = COMMON_US915_KEYS,
        .AppSKey = {0xE4, 0x70, 0xDC, 0x81, 0xE1, 0x43, 0x8D, 0x99, 0x14, 0x22, 0x84, 0x83, 0xD9, 0xA3, 0x6B, 0xC7},
        .DevAddr = (uint32_t)0x260CB928,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey = COMMON_RU864_KEYS,
        .SNwkSIntKey = COMMON_RU864_KEYS,
        .NwkSEncKey = COMMON_RU864_KEYS,
        .AppSKey = {0xD3, 0xBF, 0xD6, 0xC4, 0x7E, 0xE5, 0x9C, 0x2E, 0xC7, 0x8D, 0x35, 0x95, 0x4E, 0x47, 0x36, 0x97},
        .DevAddr = (uint32_t)0x260B790D,
        .frame_count = 0,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    }

};

network_keys_t get_current_network_keys()
{

    LoRaMacRegion_t current_loramac_region = get_current_loramac_region();
    return get_next_network_keys_in_region(current_loramac_region);
}

/**
 * @brief Implements a function to get next network in list of networks for that country
 * returns
 */

uint32_t counter = 0;

const registered_devices_t eu868_region_keys[] = {EU868_KEYS_EU1, HELIUM_KEYS};
const registered_devices_t us915_region_keys[] = {US915_KEYS_US1, HELIUM_KEYS};
const registered_devices_t as923_region_keys[] = {HELIUM_KEYS, AS923_KEYS_EU1, AS923_KEYS_AU1}; // In japan, start with helium network first, then go through ttn end node registered on EU cluster then AU cluster
const registered_devices_t ru864_region_keys[] = {RU864_KEYS_EU1, HELIUM_KEYS};
const registered_devices_t au915_region_keys[] = {AU915_KEYS_AU1, HELIUM_KEYS};
const registered_devices_t kr920_region_keys[] = {KR920_KEYS_EU1, HELIUM_KEYS}; // update keys, possibly to AU1
const registered_devices_t in865_region_keys[] = {IN865_KEYS_EU1, HELIUM_KEYS};
const registered_devices_t cn470_region_keys[] = {CN470_KEYS_EU1, HELIUM_KEYS};

void switch_to_next_region()
{
    counter++;
}

registered_devices_t get_current_network()
{

    LoRaMacRegion_t region = get_current_loramac_region();
    registered_devices_t current_network;

    switch (region)
    {
    case LORAMAC_REGION_AS923:
        current_network = as923_region_keys[counter % (sizeof(as923_region_keys) / sizeof(as923_region_keys[0]))];
        break;

    case LORAMAC_REGION_AU915:
        current_network = au915_region_keys[counter % (sizeof(au915_region_keys) / sizeof(au915_region_keys[0]))];
        break;

    case LORAMAC_REGION_CN470:
        current_network = cn470_region_keys[counter % (sizeof(cn470_region_keys) / sizeof(cn470_region_keys[0]))];
        break;

    case LORAMAC_REGION_EU868:
        current_network = eu868_region_keys[counter % (sizeof(eu868_region_keys) / sizeof(eu868_region_keys[0]))];
        break;

    case LORAMAC_REGION_KR920:
        current_network = kr920_region_keys[counter % (sizeof(kr920_region_keys) / sizeof(kr920_region_keys[0]))];
        break;

    case LORAMAC_REGION_IN865:
        current_network = in865_region_keys[counter % (sizeof(in865_region_keys) / sizeof(in865_region_keys[0]))];
        break;

    case LORAMAC_REGION_US915:
        current_network = us915_region_keys[counter % (sizeof(us915_region_keys) / sizeof(us915_region_keys[0]))];
        break;

    case LORAMAC_REGION_RU864:
        current_network = ru864_region_keys[counter % (sizeof(ru864_region_keys) / sizeof(ru864_region_keys[0]))];
        break;

    default:
        current_network = eu868_region_keys[counter % (sizeof(eu868_region_keys) / sizeof(eu868_region_keys[0]))]; // Use EU frequencies if all fails
        break;
    }

    return current_network;
}

network_keys_t get_next_network_keys_in_region(LoRaMacRegion_t region)
{

    registered_devices_t device = get_current_network(region);

    return network_key_list[device];
}
