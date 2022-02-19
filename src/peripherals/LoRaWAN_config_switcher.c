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
#include "config.h"
#include "geofence.h"

int8_t datarate_calculator(LoRaMacRegion_t LoRaMacRegion);
network_keys_t get_next_network_keys_in_region(void);

uint32_t counter = 0;

picotracker_lorawan_settings_t get_lorawan_setting(LoRaMacRegion_t current_region)
{

    /* select data rate depending on region of the world. */
    int8_t datarate = datarate_calculator(current_region);

    /* Init loramac stack */
    picotracker_lorawan_settings_t settings =
        {
            .datarate = datarate,
        };

    return settings;
}

/**
* Use datarate of DR_5 for China, EU, ASia but DR_4 over rest of the world
*/
int8_t datarate_calculator(LoRaMacRegion_t LoRaMacRegion)
{
    int8_t dr = 0;

    switch (LoRaMacRegion)
    {
    case LORAMAC_REGION_EU868:
        dr = DR_4;
        break;

    case LORAMAC_REGION_US915:
        dr = DR_4; // IN the US, this is the highest datarate
        break;

    case LORAMAC_REGION_CN470:
        dr = DR_4;
        break;

    case LORAMAC_REGION_AS923:
        dr = DR_4;
        break;

    default:
        dr = DR_4;
        break;
    }

    return dr;
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
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = COMMON_AS923_KEYS_EU1,

        .AppSKey = {0x67, 0x27, 0x87, 0x20, 0x50, 0xA8, 0xD4, 0x0D, 0x3A, 0xA0, 0xC2, 0x75, 0x09, 0x03, 0xE8, 0x13},
        .DevAddr = (uint32_t)0x260BD61D,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,
    },
    {
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = COMMON_AS923_KEYS_AU1,
        .AppSKey = {0x34, 0x3C, 0xCF, 0x72, 0xC1, 0xE2, 0xC3, 0xD7, 0xF0, 0x92, 0x06, 0x1A, 0x02, 0xFF, 0x80, 0xAE},
        .DevAddr = (uint32_t)0x260D24A9,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = COMMON_AU915_KEYS,
        .AppSKey = {0x49, 0xE5, 0xE3, 0x6B, 0xD9, 0xCC, 0x3A, 0x14, 0x0D, 0x4C, 0xC0, 0xD0, 0xB9, 0x76, 0xE7, 0x52},
        .DevAddr = (uint32_t)0x260DE191,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = COMMON_CN470_KEYS,
        .AppSKey = {0xA7, 0x61, 0x92, 0x2C, 0xF7, 0x65, 0x9E, 0x7B, 0x24, 0x74, 0x2F, 0x98, 0xDD, 0xE1, 0x3D, 0x5E},
        .DevAddr = (uint32_t)0x260B74CE,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        // icspace26_eu1_eu_863_870_device_1
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x97, 0x8B, 0x56, 0x62, 0xC4, 0x77, 0x2C, 0x2A, 0x60, 0x7D, 0xAA, 0x41, 0xB6, 0x08, 0x50, 0xDC},
        .AppSKey = {0x6C, 0xFD, 0x8D, 0xB0, 0xDE, 0x17, 0xF5, 0x46, 0x26, 0x41, 0x71, 0xEC, 0xD5, 0x07, 0x38, 0xE2},
        .DevAddr = (uint32_t)0x260BCC58,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        // icspace26_eu1_eu_863_870_device_2
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xD8, 0xCE, 0xC5, 0x64, 0x15, 0x0F, 0x6E, 0xFD, 0x07, 0xC8, 0x7B, 0xDC, 0x68, 0xEC, 0x2B, 0xCE},
        .AppSKey = {0x0C, 0x30, 0xE4, 0x80, 0x4E, 0x2C, 0xFC, 0x72, 0xB3, 0x67, 0x65, 0xD2, 0x93, 0x33, 0x1C, 0x3C},
        .DevAddr = (uint32_t)0x260B3592,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        // icspace26_eu1_eu_863_870_device_3
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xE7, 0xF2, 0xA6, 0x62, 0x20, 0x97, 0x58, 0xC1, 0x3D, 0x13, 0x31, 0x1F, 0x2F, 0x2A, 0x40, 0xAD},
        .AppSKey = {0x71, 0x0D, 0x9E, 0x92, 0x0B, 0xA0, 0x28, 0xE4, 0x79, 0x39, 0x67, 0xC9, 0xAB, 0x8E, 0xFB, 0xE4},
        .DevAddr = (uint32_t)0x260B1428,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        // icspace26_eu1_eu_863_870_device_4
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x70, 0x66, 0x98, 0xCA, 0xCE, 0x4A, 0xD0, 0xD1, 0x5D, 0x34, 0xD3, 0x3B, 0xB9, 0x95, 0x77, 0x59},
        .AppSKey = {0x28, 0xAB, 0xDE, 0xCC, 0x96, 0x1F, 0xA9, 0x5D, 0x9F, 0x42, 0x5E, 0x35, 0x2E, 0x29, 0x84, 0xC7},
        .DevAddr = (uint32_t)0x260B5D1F,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        // icspace26_eu1_eu_863_870_device_5
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xA7, 0xC8, 0x0F, 0x2F, 0xAF, 0x9A, 0x2B, 0x59, 0x66, 0xA5, 0xD0, 0xF4, 0x69, 0xA2, 0xC9, 0xB1},
        .AppSKey = {0xDF, 0x1C, 0x07, 0x95, 0xB6, 0xA5, 0xE1, 0xF5, 0x46, 0xA6, 0x31, 0xFD, 0xF9, 0x80, 0x0A, 0xB6},
        .DevAddr = (uint32_t)0x260B349D,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = COMMON_KR920_KEYS,
        .AppSKey = {0xC9, 0x47, 0x9E, 0xF9, 0xB2, 0xF5, 0x09, 0x8C, 0x37, 0xD9, 0xAD, 0x07, 0xB2, 0x57, 0xD3, 0x33},
        .DevAddr = (uint32_t)0x260B0C3C,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        // icspace26_au1_kr_920_923_ttn
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x34, 0x7F, 0x7F, 0x62, 0x34, 0xEE, 0x13, 0x94, 0xBC, 0x7E, 0xE5, 0x6C, 0x63, 0x50, 0x85, 0x97},
        .AppSKey = {0xC9, 0x40, 0xE7, 0x86, 0x69, 0x27, 0x92, 0xB9, 0xA5, 0x8E, 0xB9, 0xFF, 0x46, 0x3D, 0x60, 0x80},
        .DevAddr = (uint32_t)0x260DD5C5,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = COMMON_IN865_KEYS,
        .AppSKey = {0xB9, 0xA6, 0xF2, 0x26, 0x4B, 0x48, 0x05, 0x5B, 0x79, 0x53, 0xDE, 0x55, 0x9C, 0x2A, 0x77, 0x08},
        .DevAddr = (uint32_t)0x260BE033,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_us1_us915_device_1
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x8B, 0x3C, 0x1D, 0xB3, 0x95, 0x45, 0x17, 0xDB, 0xF7, 0x8D, 0x6A, 0xB7, 0x65, 0xD1, 0x1B, 0x80},
        .AppSKey = {0x8B, 0xC7, 0x51, 0x22, 0xAC, 0x83, 0x49, 0xB5, 0x99, 0xBD, 0x35, 0x30, 0x95, 0x7A, 0x0D, 0xA0},
        .DevAddr = (uint32_t)0x260C24A9,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_us1_us915_device_2
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x47, 0x18, 0xA9, 0x3A, 0x91, 0x7E, 0xB6, 0x1A, 0xFD, 0xB3, 0x78, 0x6E, 0xA0, 0x4E, 0xC3, 0xEE},
        .AppSKey = {0xD3, 0xAB, 0xC3, 0x3C, 0x12, 0xD9, 0x75, 0xF2, 0x78, 0x5F, 0xFA, 0x46, 0xAF, 0x75, 0x95, 0xE2},
        .DevAddr = (uint32_t)0x260C4E99,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_us1_us915_device_3
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x5C, 0xB4, 0xED, 0x34, 0x36, 0xFD, 0x09, 0x41, 0x87, 0x9D, 0x03, 0x19, 0xE5, 0x31, 0x90, 0x6E},
        .AppSKey = {0x58, 0x72, 0xAF, 0xD4, 0x22, 0x8E, 0x05, 0xEF, 0x4E, 0x99, 0xA1, 0x25, 0x5D, 0x9A, 0x25, 0x34},
        .DevAddr = (uint32_t)0x260CFA2F,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_us1_us915_device_4
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xF6, 0xF9, 0x73, 0x90, 0xBE, 0x97, 0xA9, 0x43, 0x61, 0xFE, 0xD0, 0xB2, 0xC3, 0x48, 0x39, 0xD3},
        .AppSKey = {0x97, 0x54, 0xA6, 0xF1, 0xE5, 0x7A, 0x1B, 0x17, 0x84, 0x11, 0x97, 0x3B, 0x5F, 0x59, 0x58, 0xA2},
        .DevAddr = (uint32_t)0x260C936B,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_us1_us915_device_5
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xD8, 0xAB, 0x27, 0x90, 0xBA, 0x18, 0xE1, 0x1B, 0x57, 0x9B, 0x57, 0xD9, 0x2C, 0xAD, 0x42, 0x0E},
        .AppSKey = {0x4C, 0x38, 0xAF, 0xF9, 0x2D, 0x8B, 0xAF, 0xEE, 0xD6, 0x99, 0xAE, 0xAD, 0x2D, 0x59, 0xCA, 0xDD},
        .DevAddr = (uint32_t)0x260C44FA,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = COMMON_RU864_KEYS,
        .AppSKey = {0xD3, 0xBF, 0xD6, 0xC4, 0x7E, 0xE5, 0x9C, 0x2E, 0xC7, 0x8D, 0x35, 0x95, 0x4E, 0x47, 0x36, 0x97},
        .DevAddr = (uint32_t)0x260B790D,
        .frame_count = TTN_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_1
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xBB, 0x4C, 0xCA, 0x88, 0x4F, 0x44, 0x94, 0x6B, 0x65, 0xF2, 0x7F, 0x95, 0x80, 0xBF, 0xF5, 0xA1},
        .AppSKey = {0xCC, 0x4E, 0x5E, 0xA2, 0x56, 0x27, 0x00, 0xC5, 0x29, 0x33, 0x99, 0x33, 0x63, 0xA4, 0x1E, 0x61},
        .DevAddr = (uint32_t)0x48000003,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_2
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xE7, 0xE0, 0x1A, 0x46, 0x21, 0xDB, 0x0F, 0xB2, 0x7F, 0x9B, 0x28, 0xE9, 0x44, 0xFF, 0xC7, 0x43},
        .AppSKey = {0x9E, 0x16, 0xD8, 0x69, 0x8B, 0x65, 0xD9, 0x32, 0x48, 0xFF, 0x1A, 0x0A, 0xE1, 0x66, 0x48, 0xA8},
        .DevAddr = (uint32_t)0x48000004,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_3
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x08, 0x48, 0x42, 0x42, 0xC6, 0x9B, 0x83, 0x32, 0xBB, 0xF0, 0x92, 0x86, 0x4A, 0xCD, 0xC3, 0x3B},
        .AppSKey = {0x13, 0x87, 0x1B, 0x39, 0x57, 0x65, 0x2B, 0xFF, 0x62, 0x40, 0x1F, 0x78, 0xC6, 0x3A, 0xF3, 0x8A},
        .DevAddr = (uint32_t)0x48000005,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_4
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x9B, 0xBC, 0x25, 0x03, 0x67, 0xA4, 0x73, 0xC1, 0x85, 0xE5, 0xF8, 0x64, 0x19, 0x0D, 0xD5, 0xB5},
        .AppSKey = {0xF5, 0x35, 0x0C, 0xBA, 0x9E, 0x8A, 0xD9, 0xD0, 0x0C, 0xD0, 0xF7, 0x67, 0x50, 0x08, 0x1F, 0x2E},
        .DevAddr = (uint32_t)0x48000006,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_5
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x76, 0xDE, 0x6A, 0x82, 0x29, 0xD4, 0x5A, 0x36, 0x9E, 0xBD, 0x78, 0xEC, 0x73, 0x8B, 0xA7, 0xA0},
        .AppSKey = {0x91, 0xD0, 0x7A, 0x85, 0xCC, 0x32, 0x33, 0xAC, 0x5D, 0x82, 0x80, 0x5D, 0xD4, 0x67, 0xF1, 0x29},
        .DevAddr = (uint32_t)0x48000007,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_6
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x8B, 0xDB, 0x77, 0xCC, 0x61, 0x55, 0xBE, 0xC9, 0x1D, 0x11, 0x5A, 0x7E, 0xDC, 0xE4, 0xB4, 0xA3},
        .AppSKey = {0x19, 0x3E, 0x1B, 0x2B, 0x73, 0x98, 0xE2, 0xDA, 0xC4, 0x65, 0x3C, 0x4B, 0xB2, 0x3D, 0x33, 0xE0},
        .DevAddr = (uint32_t)0x48000008,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_7
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x36, 0x81, 0xA2, 0xE4, 0xC0, 0x48, 0xCC, 0x0E, 0xB9, 0xCC, 0x78, 0x63, 0x32, 0xC4, 0x1F, 0x3E},
        .AppSKey = {0xE5, 0x2A, 0x6D, 0xD7, 0x2A, 0x2B, 0xF6, 0xCA, 0x4E, 0xCA, 0xA4, 0x5D, 0x21, 0x66, 0x30, 0x74},
        .DevAddr = (uint32_t)0x4800000a,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_8
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xD4, 0x53, 0x98, 0x02, 0xDA, 0x5B, 0x9D, 0x6A, 0xEE, 0x33, 0x3E, 0x1B, 0xDC, 0x5E, 0xC1, 0x67},
        .AppSKey = {0x1D, 0x6E, 0x3F, 0x78, 0xB7, 0x07, 0x1D, 0x12, 0xEB, 0xCD, 0x50, 0x25, 0xCA, 0xE5, 0x8E, 0x5A},
        .DevAddr = (uint32_t)0x4800000c,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_9
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xF7, 0xD4, 0x0D, 0x8C, 0x64, 0xF2, 0x6E, 0x17, 0x79, 0x6D, 0xFD, 0x86, 0xBF, 0x25, 0x2B, 0x6C},
        .AppSKey = {0xC2, 0xEC, 0xB4, 0xA0, 0xEC, 0x22, 0x9E, 0xE9, 0x87, 0xF3, 0x44, 0xC4, 0x88, 0x8B, 0x9E, 0xFA},
        .DevAddr = (uint32_t)0x4800000d,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_10
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x68, 0xC6, 0x37, 0xC7, 0x74, 0x6F, 0xA1, 0x26, 0x25, 0x11, 0x0C, 0x02, 0x1E, 0x63, 0xB2, 0xB8},
        .AppSKey = {0x5D, 0x99, 0x7C, 0x0F, 0xDD, 0xD4, 0xE7, 0x83, 0xDB, 0x90, 0x48, 0x98, 0x2E, 0x37, 0xF2, 0xD1},
        .DevAddr = (uint32_t)0x4800000f,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },

    {
        //icspace26_helium_11
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xAD, 0xA1, 0xA9, 0x0D, 0xCC, 0xD7, 0x17, 0xB5, 0xC7, 0xEF, 0xB9, 0x2B, 0x5A, 0x7E, 0x71, 0x06},
        .AppSKey = {0x83, 0x3F, 0x7D, 0xFE, 0x11, 0x57, 0x71, 0xB1, 0xDA, 0xD9, 0xDA, 0x2A, 0xB8, 0xB2, 0x0D, 0x8E},
        .DevAddr = (uint32_t)0x48000012,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_12
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xB5, 0x6C, 0x49, 0x7F, 0xD4, 0xF1, 0x59, 0x5F, 0x1D, 0x01, 0xAE, 0x04, 0xBA, 0xA2, 0xE4, 0x43},
        .AppSKey = {0x35, 0x57, 0xAF, 0xBE, 0xBB, 0xCB, 0xD0, 0xD5, 0x69, 0x4B, 0x89, 0x5B, 0x4C, 0x31, 0x59, 0xD4},
        .DevAddr = (uint32_t)0x48000013,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_13
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x94, 0x1E, 0x29, 0xB7, 0x8E, 0x28, 0x03, 0x4F, 0xE7, 0x03, 0xD5, 0x12, 0x74, 0x52, 0x4F, 0xCA},
        .AppSKey = {0x54, 0x46, 0xB2, 0xFE, 0xBC, 0x29, 0xBB, 0xDC, 0xAD, 0x67, 0x52, 0x8D, 0xC0, 0xA6, 0x3C, 0x67},
        .DevAddr = (uint32_t)0x48000015,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_14
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xA8, 0x3E, 0x11, 0xDF, 0x15, 0xC5, 0xD6, 0x22, 0xDA, 0xBD, 0x8F, 0xD7, 0x1C, 0x98, 0xD4, 0x82},
        .AppSKey = {0x3D, 0x62, 0x7D, 0x58, 0x9C, 0xAF, 0x73, 0x0B, 0x3A, 0xA9, 0x13, 0x11, 0x49, 0xE7, 0xE5, 0xD6},
        .DevAddr = (uint32_t)0x48000017,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_15
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xC3, 0x60, 0xD4, 0x25, 0xD4, 0xBD, 0x46, 0xCB, 0x6D, 0x15, 0xBD, 0xBA, 0x14, 0xD4, 0x26, 0xBB},
        .AppSKey = {0x62, 0x48, 0x44, 0x33, 0x76, 0x0B, 0xDB, 0xCC, 0x31, 0xAE, 0x42, 0x75, 0x78, 0xD7, 0x4F, 0x80},
        .DevAddr = (uint32_t)0x48000018,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_16
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xC1, 0xAA, 0xEF, 0xE9, 0x9B, 0x54, 0x66, 0xD2, 0xF4, 0xE3, 0xDF, 0xA0, 0x06, 0x6E, 0x35, 0x15},
        .AppSKey = {0x7F, 0xEE, 0x86, 0xAE, 0xB4, 0x13, 0x28, 0x1D, 0xCA, 0x30, 0x99, 0x4B, 0xAC, 0xB2, 0x21, 0x68},
        .DevAddr = (uint32_t)0x4800001a,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_17
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xD0, 0xB7, 0xDC, 0x9B, 0x5A, 0xAC, 0x3B, 0x9F, 0xB5, 0x77, 0xD5, 0x55, 0xD3, 0x47, 0x70, 0x25},
        .AppSKey = {0x08, 0xD2, 0x06, 0xFE, 0x7E, 0x43, 0xF9, 0xBF, 0xA5, 0xFA, 0x55, 0xB7, 0xE0, 0x02, 0x64, 0x0E},
        .DevAddr = (uint32_t)0x4800001b,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_18
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0x54, 0x10, 0x60, 0x00, 0x8B, 0x44, 0x55, 0x5A, 0xED, 0x41, 0xDC, 0xF3, 0xAF, 0x7A, 0x73, 0x27},
        .AppSKey = {0x78, 0x55, 0x49, 0xFB, 0xD0, 0x50, 0xF0, 0xEA, 0xB6, 0x74, 0xB0, 0x40, 0xDF, 0x04, 0xBE, 0x48},
        .DevAddr = (uint32_t)0x4800001c,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_19
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xA0, 0xE0, 0x49, 0xDD, 0x35, 0x46, 0x2F, 0x5C, 0x22, 0xBC, 0xFE, 0x85, 0xD8, 0x89, 0x2F, 0x45},
        .AppSKey = {0xF8, 0x76, 0xCE, 0xA8, 0x59, 0x88, 0x35, 0x7D, 0x48, 0x3C, 0x7F, 0x3E, 0xB9, 0xE4, 0x22, 0x18},
        .DevAddr = (uint32_t)0x4800001e,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },
    {
        //icspace26_helium_20
        .FNwkSIntKey_SNwkSIntKey_NwkSEncKey = {0xEA, 0x99, 0xB4, 0x84, 0x0F, 0xE8, 0x8C, 0x8D, 0x10, 0x00, 0xF2, 0xAA, 0x4A, 0xF6, 0x89, 0x1E},
        .AppSKey = {0xF3, 0x0E, 0xE0, 0xEA, 0xF9, 0xA6, 0x0A, 0x1E, 0x85, 0xC1, 0x36, 0xEA, 0x1F, 0x2F, 0x6E, 0x35},
        .DevAddr = (uint32_t)0x4800001f,
        .frame_count = HELIUM_FRAME_COUNT_START,
        .ReceiveDelay1 = 1000,
        .ReceiveDelay2 = 2000,

    },

};

size_t network_key_list_size = sizeof(network_key_list);

network_keys_t get_current_network_keys()
{
    return get_next_network_keys_in_region();
}

/**
 * @brief Implements a function to get next network in list of networks for that country
 * A bug means that the first registered device in the registered_devices_t is SKIPPED, and starts from
 * the second one. But when its done with the whole list it will return back to transmitting on the first
 * registered device and carry on.
 */

void init_LoRaWAN_config_switcher()
{
    counter = randr(0, NUMBER_OF_REGISTERED_DEVICES + 1);
}

const registered_devices_t eu868_region_keys[] = {
    icspace26_eu1_eu_863_870_device_1,
    icspace26_helium_1,

    icspace26_eu1_eu_863_870_device_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,

    icspace26_eu1_eu_863_870_device_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,

    icspace26_eu1_eu_863_870_device_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,

    icspace26_eu1_eu_863_870_device_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,

    icspace26_eu1_eu_863_870_device_1,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,

    icspace26_eu1_eu_863_870_device_2,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,

    icspace26_eu1_eu_863_870_device_3,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,

    icspace26_eu1_eu_863_870_device_4,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,

    icspace26_eu1_eu_863_870_device_5,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,

    icspace26_eu1_eu_863_870_device_1,
    icspace26_helium_11,

    icspace26_eu1_eu_863_870_device_2,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,

    icspace26_eu1_eu_863_870_device_3,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,

    icspace26_eu1_eu_863_870_device_4,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,

    icspace26_eu1_eu_863_870_device_5,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,

    icspace26_eu1_eu_863_870_device_1,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,

    icspace26_eu1_eu_863_870_device_2,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,

    icspace26_eu1_eu_863_870_device_3,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,

    icspace26_eu1_eu_863_870_device_4,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,

    icspace26_eu1_eu_863_870_device_5,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,

};

const registered_devices_t us915_region_keys[] = {
    icspace26_us1_us915_device_1,
    icspace26_helium_1,

    icspace26_us1_us915_device_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,

    icspace26_us1_us915_device_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,

    icspace26_us1_us915_device_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,

    icspace26_us1_us915_device_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,

    icspace26_us1_us915_device_1,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,

    icspace26_us1_us915_device_2,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,

    icspace26_us1_us915_device_3,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,

    icspace26_us1_us915_device_4,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,

    icspace26_us1_us915_device_5,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,

    icspace26_us1_us915_device_1,
    icspace26_helium_11,

    icspace26_us1_us915_device_2,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,

    icspace26_us1_us915_device_3,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,

    icspace26_us1_us915_device_4,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,

    icspace26_us1_us915_device_5,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,

    icspace26_us1_us915_device_1,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,

    icspace26_us1_us915_device_2,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,

    icspace26_us1_us915_device_3,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,

    icspace26_us1_us915_device_4,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,

    icspace26_us1_us915_device_5,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
};

const registered_devices_t as923_region_keys[] = {
    AS923_KEYS_AU1,

    icspace26_helium_1,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,

    icspace26_helium_2,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,

    icspace26_helium_3,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,

    icspace26_helium_4,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,

    icspace26_helium_5,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,

    icspace26_helium_6,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,

    icspace26_helium_7,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,

    icspace26_helium_8,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,

    icspace26_helium_9,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,

    icspace26_helium_10,
    AS923_KEYS_EU1,
    AS923_KEYS_AU1,

}; // In japan, start with helium network first, then go through ttn end node registered on EU cluster then AU cluster

const registered_devices_t ru864_region_keys[] = {
    RU864_KEYS_EU1,
    icspace26_helium_1,

    RU864_KEYS_EU1,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,

    RU864_KEYS_EU1,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,

    RU864_KEYS_EU1,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,

    RU864_KEYS_EU1,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,

    RU864_KEYS_EU1,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,

    RU864_KEYS_EU1,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,

    RU864_KEYS_EU1,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,

    RU864_KEYS_EU1,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,

    RU864_KEYS_EU1,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,

    RU864_KEYS_EU1,
    icspace26_helium_11,

    RU864_KEYS_EU1,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,

    RU864_KEYS_EU1,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,

    RU864_KEYS_EU1,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,

    RU864_KEYS_EU1,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,

    RU864_KEYS_EU1,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,

    RU864_KEYS_EU1,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,

    RU864_KEYS_EU1,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,

    RU864_KEYS_EU1,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,

    RU864_KEYS_EU1,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
};

const registered_devices_t au915_region_keys[] = {
    AU915_KEYS_AU1,
    icspace26_helium_1,

    AU915_KEYS_AU1,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,

    AU915_KEYS_AU1,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,

    AU915_KEYS_AU1,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,

    AU915_KEYS_AU1,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,

    AU915_KEYS_AU1,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,

    AU915_KEYS_AU1,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,

    AU915_KEYS_AU1,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,

    AU915_KEYS_AU1,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,

    AU915_KEYS_AU1,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,

    AU915_KEYS_AU1,
    icspace26_helium_11,

    AU915_KEYS_AU1,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,

    AU915_KEYS_AU1,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,

    AU915_KEYS_AU1,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,

    AU915_KEYS_AU1,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,

    AU915_KEYS_AU1,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,

    AU915_KEYS_AU1,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,

    AU915_KEYS_AU1,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,

    AU915_KEYS_AU1,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,

    AU915_KEYS_AU1,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,

};

const registered_devices_t kr920_region_keys[] = {
    KR920_KEYS_EU1,

    icspace26_helium_1,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,

    icspace26_helium_2,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,

    icspace26_helium_3,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,

    icspace26_helium_4,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,

    icspace26_helium_5,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,

    icspace26_helium_6,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,

    icspace26_helium_7,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,

    icspace26_helium_8,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,

    icspace26_helium_9,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,

    icspace26_helium_10,
    icspace26_au1_kr_920_923_ttn,
    KR920_KEYS_EU1,

};

const registered_devices_t in865_region_keys[] = {
    IN865_KEYS_EU1,
    icspace26_helium_1,

    IN865_KEYS_EU1,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,

    IN865_KEYS_EU1,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,

    IN865_KEYS_EU1,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,

    IN865_KEYS_EU1,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,

    IN865_KEYS_EU1,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,

    IN865_KEYS_EU1,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,

    IN865_KEYS_EU1,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,

    IN865_KEYS_EU1,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,

    IN865_KEYS_EU1,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,

    IN865_KEYS_EU1,
    icspace26_helium_11,

    IN865_KEYS_EU1,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,

    IN865_KEYS_EU1,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,

    IN865_KEYS_EU1,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,

    IN865_KEYS_EU1,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,

    IN865_KEYS_EU1,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,

    IN865_KEYS_EU1,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,

    IN865_KEYS_EU1,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,

    IN865_KEYS_EU1,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,

    IN865_KEYS_EU1,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,

};

const registered_devices_t cn470_region_keys[] = {
    CN470_KEYS_EU1,
    icspace26_helium_1,

    CN470_KEYS_EU1,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,
    icspace26_helium_2,

    CN470_KEYS_EU1,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,
    icspace26_helium_3,

    CN470_KEYS_EU1,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,
    icspace26_helium_4,

    CN470_KEYS_EU1,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,
    icspace26_helium_5,

    CN470_KEYS_EU1,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,
    icspace26_helium_6,

    CN470_KEYS_EU1,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,
    icspace26_helium_7,

    CN470_KEYS_EU1,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,
    icspace26_helium_8,

    CN470_KEYS_EU1,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,
    icspace26_helium_9,

    CN470_KEYS_EU1,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,
    icspace26_helium_10,

    CN470_KEYS_EU1,
    icspace26_helium_11,

    CN470_KEYS_EU1,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,
    icspace26_helium_12,

    CN470_KEYS_EU1,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,
    icspace26_helium_13,

    CN470_KEYS_EU1,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,
    icspace26_helium_14,

    CN470_KEYS_EU1,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,
    icspace26_helium_15,

    CN470_KEYS_EU1,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,
    icspace26_helium_16,

    CN470_KEYS_EU1,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,
    icspace26_helium_17,

    CN470_KEYS_EU1,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,
    icspace26_helium_18,

    CN470_KEYS_EU1,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,
    icspace26_helium_19,

    CN470_KEYS_EU1,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,
    icspace26_helium_20,

};

void switch_to_next_registered_credentials()
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
        current_network = eu868_region_keys[counter % (sizeof(eu868_region_keys) / sizeof(eu868_region_keys[0]))]; // Use EU credentials if all fails
        break;
    }

    return current_network;
}

network_keys_t get_next_network_keys_in_region()
{

    registered_devices_t device = get_current_network();

    return network_key_list[device];
}