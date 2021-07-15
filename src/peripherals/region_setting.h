/**
 * @file region_setting.h
 * @author Medad Rufus Newman (mailto@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-05-08
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

	/* ==================================================================== */
	/* ========================== include files =========================== */
	/* ==================================================================== */

	/* Inclusion of system and local header files goes here */

#include "stdint.h"
#include "LoRaMac.h"

	/* ==================================================================== */
	/* ============================ constants ============================= */
	/* ==================================================================== */

	/* #define and enum statements go here */
	typedef struct
	{
		uint8_t FNwkSIntKey[16];
		uint8_t SNwkSIntKey[16];
		uint8_t NwkSEncKey[16];
		uint8_t AppSKey[16];
		uint32_t DevAddr;
	} network_keys_t;
	/* ==================================================================== */
	/* ========================== public data ============================= */
	/* ==================================================================== */

	/* Definition of public (external) data types go here */

	/* ==================================================================== */
	/* ======================= public functions =========================== */
	/* ==================================================================== */

	/* Function prototypes for public (external) functions go here */

	network_keys_t get_network_keys(LoRaMacRegion_t Loramac_region);
#endif
#ifdef __cplusplus
}
#endif
