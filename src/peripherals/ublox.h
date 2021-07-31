/**
  ******************************************************************************
  * @file           : ublox.h
  * @brief          : Driver header file for ublox.c
  ******************************************************************************
  * Imperial College Space Society
	* Medad Newman, Richard Ibbotson
  *
  *
  ******************************************************************************
  */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef UBLOX_H
#define UBLOX_H


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */

#include <stdint.h>

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */

typedef enum
{
	GPS_FAILURE = 0,
	GPS_SUCCESS
} gps_status_t;


typedef struct{
	float GPS_UBX_latitude_Float;  // YY.YYYYYYY, in +/- DEGREES, 
	float GPS_UBX_longitude_Float;  // XXX.XXXXXXX, in +/- DEGREES,
	int32_t GPSaltitude;
	uint8_t GPSsats;

	int32_t GPS_UBX_latitude;	// YYYYYYYYY, +/-
	int32_t GPS_UBX_longitude;	// XXXXXXXXXX, +/-
					

	uint8_t GPShour;
	uint8_t GPSminute;
	uint8_t GPSsecond;
	uint8_t GPSday;
	uint8_t GPSmonth;
	uint16_t GPSyear;

	uint8_t GPSfix_type;
	uint8_t GPSfix_OK;
	uint8_t GPSvalidity;

	uint8_t GPSnavigation;
	
	uint32_t week;
	uint32_t TOW;
	uint32_t unix_time;
	
	gps_status_t latest_gps_status;

} gps_info_t;

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* Definition of public (external) data types go here */



/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/* Function prototypes for public (external) functions go here */

extern gps_status_t get_location_fix(uint32_t timeout);
extern gps_status_t setup_GPS(void);
extern gps_status_t get_latest_gps_status(void);
extern uint16_t get_load_solar_voltage(void);

gps_info_t *get_gps_info_ptr(void);


#endif
#ifdef __cplusplus
}
#endif







