/**
  ******************************************************************************
  * @file           : geofence.h
  * @brief          : Algorithms for geofencing
  ******************************************************************************
  * Imperial College Space Society
	* Medad Newman, Richard Ibbotson
	* Modified from TT7's code 
	* https://github.com/TomasTT7/TT7F-Float-Tracker
  * The following coordinates are autogenerated from a kml file containing all the polygons.
  *
  ******************************************************************************
  */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef GEOFENCE_H
#define GEOFENCE_H

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

	/* ==================================================================== */
	/* ========================== public data ============================= */
	/* ==================================================================== */

	/* Definition of public (external) data types go here */

	/*!
 * Polygon region enumeration
 */
	typedef enum polygon_t
	{
		EU863870_AFRICA_polygon,
		EU863870_PHILIPPINES_polygon,
		US902928_NAMERICA_polygon,
		AS923925_BRUNEI_polygon,
		AS923925_TAIWAN_polygon,
		AS923925_INDONESIA_polygon,
		AS923925_THAILAND_polygon,
		US902928_ARGENTINA_polygon,
		AU915928_BRAZIL_polygon,
		AU915928_CHILE_polygon,
		CN779787_CHINA_polygon,
		IN865867_INDIA_polygon,
		AS920923_JAPAN_polygon,
		KR920923_SKOREA_polygon,
		AS920923_MALAYSIASG_polygon,
		AU915928_AUSTRALIA_polygon,
		RU864870_RUSSIA_polygon,
		EU863870_EUROPE_polygon,
		OUTSIDE_polygon
	} Polygon_t;

	typedef enum
	{
		TX_OK = 0,
		TX_NOT_OK
	} tx_permission_t;

	typedef struct
	{
		bool reinit_loramac_stack_pending;
		LoRaMacRegion_t current_loramac_region;
		Polygon_t curr_poly_region;
		tx_permission_t tx_permission;

	} geofence_status_t;

	// Externally accessible variables
	extern geofence_status_t current_geofence_status;

	/* ==================================================================== */
	/* ======================= public functions =========================== */
	/* ==================================================================== */

	/* Function prototypes for public (external) functions go here */

	void update_geofence_position(float latitude, float longitude);
	void geofence_init(void);
	Polygon_t get_polygon(float latitude, float longitude);
	int8_t datarate_calculator(LoRaMacRegion_t LoRaMacRegion);
	LoRaMacRegion_t get_current_loramac_region();


#endif
#ifdef __cplusplus
}
#endif
