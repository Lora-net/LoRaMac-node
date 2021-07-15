/**
 * @file region_settings.c
 * @author Medad Rufus Newman (mailto@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-05-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "region_setting.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */

#define COMMON_AS923_KEYS                                                                              \
	{                                                                                                  \
		0xAE, 0x8F, 0xC7, 0xCD, 0x28, 0x79, 0xDA, 0x24, 0x8D, 0xF5, 0x2A, 0x09, 0x8A, 0x60, 0x6E, 0x33 \
	}
network_keys_t AS923_network_keys = {
	.FNwkSIntKey = COMMON_AS923_KEYS,
	.SNwkSIntKey = COMMON_AS923_KEYS,
	.NwkSEncKey = COMMON_AS923_KEYS,
	.AppSKey = {0x67, 0x27, 0x87, 0x20, 0x50, 0xA8, 0xD4, 0x0D, 0x3A, 0xA0, 0xC2, 0x75, 0x09, 0x03, 0xE8, 0x13},
	.DevAddr = (uint32_t)0x260BD61D,
};

#define COMMON_AU915_KEYS                                                                              \
	{                                                                                                  \
		0x7F, 0x94, 0xCA, 0xBE, 0x58, 0x50, 0x40, 0x37, 0xA9, 0xDB, 0x1D, 0x2E, 0xCC, 0xD8, 0x8E, 0x52 \
	}
network_keys_t AU915_network_keys = {
	.FNwkSIntKey = COMMON_AU915_KEYS,
	.SNwkSIntKey = COMMON_AU915_KEYS,
	.NwkSEncKey = COMMON_AU915_KEYS,
	.AppSKey = {0xAE, 0x4A, 0xDA, 0xB6, 0x3B, 0x76, 0x20, 0x7E, 0xBA, 0x36, 0x77, 0x66, 0x5C, 0xDF, 0xFD, 0x94},
	.DevAddr = (uint32_t)0x260B5634,
};

#define COMMON_CN470_KEYS                                                                              \
	{                                                                                                  \
		0x82, 0xF8, 0x47, 0x2D, 0x18, 0x83, 0x3E, 0x9B, 0xE0, 0x5E, 0x76, 0x9E, 0x2A, 0x28, 0x94, 0xB2 \
	}
network_keys_t CN470_network_keys = {
	.FNwkSIntKey = COMMON_CN470_KEYS,
	.SNwkSIntKey = COMMON_CN470_KEYS,
	.NwkSEncKey = COMMON_CN470_KEYS,
	.AppSKey = {0xA7, 0x61, 0x92, 0x2C, 0xF7, 0x65, 0x9E, 0x7B, 0x24, 0x74, 0x2F, 0x98, 0xDD, 0xE1, 0x3D, 0x5E},
	.DevAddr = (uint32_t)0x260B74CE,
};

#define COMMON_EU868_KEYS                                                                              \
	{                                                                                                  \
		0x54, 0x58, 0xC6, 0xF6, 0xF8, 0x65, 0x87, 0x1F, 0x56, 0xAF, 0xC8, 0xF4, 0xC1, 0x2A, 0xEA, 0xE6 \
	}
network_keys_t EU868_network_keys = {
	.FNwkSIntKey = COMMON_EU868_KEYS,
	.SNwkSIntKey = COMMON_EU868_KEYS,
	.NwkSEncKey = COMMON_EU868_KEYS,
	.AppSKey = {0xD2, 0x02, 0x95, 0x6B, 0xF5, 0x36, 0xFF, 0x15, 0x29, 0xA0, 0x83, 0x58, 0xAC, 0x3E, 0xE8, 0x88},
	.DevAddr = (uint32_t)0x260BD67C,
};

#define COMMON_KR920_KEYS                                                                              \
	{                                                                                                  \
		0xCD, 0x5F, 0x49, 0x98, 0xA1, 0x15, 0x1D, 0x3F, 0xF1, 0xED, 0xB4, 0x47, 0xD4, 0xDF, 0x66, 0x01 \
	}
network_keys_t KR920_network_keys = {
	.FNwkSIntKey = COMMON_KR920_KEYS,
	.SNwkSIntKey = COMMON_KR920_KEYS,
	.NwkSEncKey = COMMON_KR920_KEYS,
	.AppSKey = {0xC9, 0x47, 0x9E, 0xF9, 0xB2, 0xF5, 0x09, 0x8C, 0x37, 0xD9, 0xAD, 0x07, 0xB2, 0x57, 0xD3, 0x33},
	.DevAddr = (uint32_t)0x260B0C3C,
};

#define COMMON_IN865_KEYS                                                                              \
	{                                                                                                  \
		0x64, 0xB4, 0x78, 0x96, 0xD9, 0x24, 0x6F, 0x99, 0xA5, 0xA6, 0x56, 0x08, 0x80, 0x04, 0x70, 0x9A \
	}
network_keys_t IN865_network_keys = {
	.FNwkSIntKey = COMMON_IN865_KEYS,
	.SNwkSIntKey = COMMON_IN865_KEYS,
	.NwkSEncKey = COMMON_IN865_KEYS,
	.AppSKey = {0xB9, 0xA6, 0xF2, 0x26, 0x4B, 0x48, 0x05, 0x5B, 0x79, 0x53, 0xDE, 0x55, 0x9C, 0x2A, 0x77, 0x08},
	.DevAddr = (uint32_t)0x260BE033,
};

#define COMMON_US915_KEYS                                                                              \
	{                                                                                                  \
		0xFB, 0x72, 0xF1, 0x86, 0x7C, 0xD8, 0x38, 0x3E, 0x61, 0x33, 0xAC, 0xEA, 0xE3, 0x0D, 0x5A, 0x2F \
	}
network_keys_t US915_network_keys = {
	.FNwkSIntKey = COMMON_US915_KEYS,
	.SNwkSIntKey = COMMON_US915_KEYS,
	.NwkSEncKey = COMMON_US915_KEYS,
	.AppSKey = {0x00, 0xC4, 0xA2, 0xC4, 0xBB, 0x2C, 0x4A, 0xE3, 0xC8, 0x05, 0x4A, 0xF3, 0xE5, 0xF8, 0x06, 0xA5},
	.DevAddr = (uint32_t)0x260BE652,
};

#define COMMON_RU864_KEYS                                                                              \
	{                                                                                                  \
		0xB4, 0xDF, 0xCD, 0x17, 0x6E, 0x52, 0xE9, 0x3C, 0x27, 0x8C, 0x10, 0xFF, 0xE1, 0x0D, 0x45, 0xC6 \
	}
network_keys_t RU864_network_keys = {
	.FNwkSIntKey = COMMON_RU864_KEYS,
	.SNwkSIntKey = COMMON_RU864_KEYS,
	.NwkSEncKey = COMMON_RU864_KEYS,
	.AppSKey = {0xD3, 0xBF, 0xD6, 0xC4, 0x7E, 0xE5, 0x9C, 0x2E, 0xC7, 0x8D, 0x35, 0x95, 0x4E, 0x47, 0x36, 0x97},
	.DevAddr = (uint32_t)0x260B790D,
};

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */
network_keys_t get_network_keys(LoRaMacRegion_t Loramac_region);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */

network_keys_t get_network_keys(LoRaMacRegion_t Loramac_region)
{
	network_keys_t network_keys;

	/**
	 * @brief return network keys for each region. Don't bother with
	 * LORAMAC_REGION_CN779 or LORAMAC_REGION_EU433 because we
	 * don't use them
	 */
	switch (Loramac_region)
	{
	case LORAMAC_REGION_AS923:
		network_keys = AS923_network_keys;
		break;

	case LORAMAC_REGION_AU915:
		network_keys = AU915_network_keys;
		break;

	case LORAMAC_REGION_CN470:
		network_keys = CN470_network_keys;
		break;

	case LORAMAC_REGION_EU868:
		network_keys = EU868_network_keys;
		break;

	case LORAMAC_REGION_KR920:
		network_keys = KR920_network_keys;
		break;

	case LORAMAC_REGION_IN865:
		network_keys = IN865_network_keys;
		break;

	case LORAMAC_REGION_US915:
		network_keys = US915_network_keys;
		break;

	case LORAMAC_REGION_RU864:
		network_keys = RU864_network_keys;
		break;

	default:
		network_keys = EU868_network_keys;
		break;
	}

	return network_keys;
}
