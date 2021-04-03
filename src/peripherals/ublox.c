/*
 * ublox.c 
 * Author: Medad Newman 23/12/19
 * Imperial College Space Society ( Medad Newman, Richard Ibbotson)
 *
 * modified from TT7's ublox code: 
 * https://github.com/TomasTT7/TT7F-Float-Tracker/blob/99133b762c971b24040d007fff3f1a348941d788/Software/ARM_UBLOX.c
 * Author: Tomy2
 *
 */

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */

#include "ublox.h"
#include <stdbool.h>
#include "utilities.h"
#include "i2c.h"
#include <board.h>
#include <systime.h>
#include "delay.h"


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "SparkFun_Ublox_Arduino_Library.h"

#include "config.h"

extern I2c_t I2c;
extern Gpio_t Led1;
extern Gpio_t Gps_int;

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */
#define SATS 4 // number of satellites required for positional solution

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */

#ifdef DUMMY_GPS_COORDS

uint16_t dummy_coord_counter = 0;

/*dummy Coords ARRAYS (longitude, latitude) */
static float dummy_coords_array[] = {
	126.9833, 37.5500,	   // South Korea
	13.4000, 52.5167,	   // Germany
	2.3333, 48.8667,	   // France
	13.4000, 52.5167,	   // Germany
	-4.4833, 54.1500,	   // Isle of Man
	13.4000, 52.5167,	   // Germany
	2.3333, 48.8667,	   // France
	24.7167, 59.4333,	   // Estonia
	25.9000, -24.6333,	   // Botswana
	3.0500, 36.7500,	   // Algeria
	35.2333, 31.7667,	   // Palestine
	19.8167, 41.3167,	   // Albania
	-14.411667, -7.928611, // Georgetown, Ascension Island over the sea
	149.1333, -35.2667,	   // Australia
	-88.7667, 17.2500,	   // Belize
	89.6333, 27.4667,	   // Bhutan
	104.9167, 11.5500,	   // Cambodia
	15.2833, -4.2500,	   // Republic of Congo
	-5.2667, 6.8167,	   // Cote d'Ivoire
	-16.5667, 13.4500,	   // The Gambia
	44.8333, 41.6833,	   // Georgia
	-51.7500, 64.1833,	   // Greenland
	-61.7500, 12.0500,	   // Grenada
	-4.4833, 54.1500,	   // Isle of Man
	35.2333, 31.7667,	   // Israel
	12.4833, 41.9000,	   // Italy
	126.9833, 37.5500,	   // South Korea
	13.1667, 32.8833,	   // Libya
	101.7000, 3.1667,	   // Malaysia
	7.4167, 43.7333,	   // Monaco
};

const unsigned short dummy_coord_n = sizeof(dummy_coords_array) / (sizeof(float) * 2);

#endif

uint16_t load_solar_voltage = 0;
gps_info_t gps_info = {.unix_time = UINT16_MAX, .latest_gps_status = GPS_FAILURE};

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */

gps_status_t get_location_fix(uint32_t timeout);
gps_status_t setup_GPS(void);
gps_status_t get_latest_gps_status(void);
gps_status_t Backup_GPS(void);

void make_dummy_coordinates(void);
static gps_status_t display_still_searching(void);
static gps_status_t display_fix_found(void);
static gps_status_t reinit_gps(void);
static gps_status_t init_for_fix(void);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */

gps_status_t get_latest_gps_status(void)
{
	return gps_info.latest_gps_status;
}

/* Get solar voltage when under load from GPS */
uint16_t get_load_solar_voltage()
{
	return load_solar_voltage;
}

/* 
 * GPS backup. 
 */
gps_status_t Backup_GPS()
{
	if (put_in_power_save_mode(defaultMaxWait) == false)
	{
		printf("***!!! Warning: put_in_power_save_mode failed !!!***\n");
	}
	else
	{
		printf("put_in_power_save_mode carried out successfully!\n");
	}
	//HAL_GPIO_WritePin(GPS_INT_GPIO_Port, GPS_INT_Pin, GPIO_PIN_RESET);    // force GPS backup mode by pulling GPS extint pin low
	return GPS_SUCCESS;
}

/* 
 * sets up gps by putting in airbourne mode, setting to use GPS satellites only, turning off NMEA
 * Needs TO BE REFACTORED TO TIME OUT OR EXIT IF NO MESSAGED IS ReCEIVED BACK!
 */
gps_status_t setup_GPS()
{

	// wake up gps in case it is in Lower Power mode
	GpioWrite(&Gps_int, 1); // pull GPS extint0 pin high to wake gps

	factoryReset();
	DelayMs(GPS_WAKEUP_TIMEOUT); // Wait for things to be setup

	/* Set the I2C port to output UBX only (turn off NMEA noise) */
	if (setI2COutput(COM_TYPE_UBX, defaultMaxWait) == false) //Set the I2C port to output UBX only (turn off NMEA noise)
	{
		printf("***!!! Warning: setI2COutput failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("set setI2COutput carried out successfully!\n");
	}

	/* For running the self test part of the program */
	if (isConnected(defaultMaxWait) == false) //Connect to the Ublox module using Wire port
	{
		printf("SELFTEST: GPS did not respond. GPS error...\r\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("SELFTEST: GPS responds. GPS OK...\r\n");
	}

	if (setGPS_constellation_only(defaultMaxWait) == false) // Set the constellation to use only GPS
	{
		printf("***!!! Warning: setGPS_constellation_only failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("set GPS constellation only carried out successfully!\n");
	}

	if (setDynamicModel(DYN_MODEL_AIRBORNE1g, defaultMaxWait) == false) // set to airbourne mode
	{
		printf("***!!! Warning: setDynamicModel failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("Dynamic platform model changed successfully!\n");
	}

	if (set_powersave_config(defaultMaxWait) == false) // Save powersave config to ram. can be activated later.
	{
		printf("***!!! Warning: set_powersave_config failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("set_powersave_config carried out successfully!\n");
	}

	if (saveConfiguration(defaultMaxWait) == false) // saveConfiguration config to BBR ram.
	{
		printf("***!!! Warning: saveConfiguration failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("saveConfiguration carried out successfully!\n");
	}

	return GPS_SUCCESS;
}

/* Get the location fix */
gps_status_t get_location_fix(uint32_t timeout)
{

	// HAL_IWDG_Refresh(&hiwdg);

	init_for_fix();

	// HAL_IWDG_Refresh(&hiwdg);

	/* Set all the GPS info values to zeros */
	memset(&gps_info, 0, sizeof(gps_info_t));

	/* poll UBX-NAV-PVT until the module has fix */
	uint32_t startTime = HAL_GetTick();
	while (HAL_GetTick() - startTime < timeout)
	{
		// HAL_IWDG_Refresh(&hiwdg);

		display_still_searching();

		uint8_t temp_GPSfix_type = getFixType(defaultMaxWait);
		uint8_t temp_GPSsats = getSIV(defaultMaxWait);
		uint8_t temp_GPShour = getHour(defaultMaxWait);
		uint8_t temp_GPSminute = getMinute(defaultMaxWait);
		uint8_t temp_GPSsecond = getSecond(defaultMaxWait);
		uint8_t temp_GPSmillisecond = getMillisecond(defaultMaxWait);
		uint8_t temp_GPSnanosecond = getNanosecond(defaultMaxWait);

		uint16_t temp_GPSyear = getYear(defaultMaxWait);
		uint8_t temp_GPSmonth = getMonth(defaultMaxWait);
		uint8_t temp_GPSday = getDay(defaultMaxWait);
		uint8_t temp_GPSfix_OK = getgnssFixOK(defaultMaxWait);


		SysTime_t stime = SysTimeGetMcuTime();
		printf("%3ds%03dms: ",stime.Seconds, stime.SubSeconds);
		
		uint32_t current_time = HAL_GetTick() - startTime;
		float current_time_F = (float)current_time / 1000;

		printf("Fixtype: ");
		if (temp_GPSfix_type == 0)
			printf("No fix ");
		else if (temp_GPSfix_type == 1)
			printf("Dead reckoning ");
		else if (temp_GPSfix_type == 2)
			printf("2D ");
		else if (temp_GPSfix_type == 3)
			printf("3D ");
		else if (temp_GPSfix_type == 4)
			printf("GNSS+Dead reckoning ");

		printf(" Sats:%d ", temp_GPSsats);
		printf(" GPSfix_OK:%d ", temp_GPSfix_OK);
		printf(" GPS time: %02d/%02d/%04d, %02d:%02d:%02d.%04d ", temp_GPSday, temp_GPSmonth, temp_GPSyear, temp_GPShour, temp_GPSminute, temp_GPSsecond, temp_GPSmillisecond);
		printf(" GPS Search time: %.3f seconds \r\n", current_time_F);

		struct tm t;
		time_t t_of_day;

		t.tm_year = temp_GPSyear - 1900; // Year - 1900
		t.tm_mon = temp_GPSmonth - 1;	 // Month, where 0 = jan
		t.tm_mday = temp_GPSday;		 // Day of the month
		t.tm_hour = temp_GPShour;
		t.tm_min = temp_GPSminute;
		t.tm_sec = temp_GPSsecond;
		t.tm_isdst = 0; // Is DST on? 1 = yes, 0 = no, -1 = unknown
		t_of_day = mktime(&t);

		printf("seconds since the Epoch: %ld\n", (uint32_t)t_of_day);

		load_solar_voltage = BoardGetBatteryVoltage();

		if (temp_GPSfix_type == 3 && temp_GPSsats >= SATS && temp_GPSfix_OK == 1) // check if we have a good fix
		{
			display_fix_found();

			gps_info.GPSyear = temp_GPSyear;
			gps_info.GPSmonth = temp_GPSmonth;
			gps_info.GPSday = temp_GPSday;
			gps_info.GPShour = temp_GPShour;
			gps_info.GPSminute = temp_GPSminute;
			gps_info.GPSsecond = temp_GPSsecond;
			gps_info.GPSsats = temp_GPSsats;
			gps_info.GPSfix_type = temp_GPSfix_type;
			gps_info.GPSfix_OK = temp_GPSfix_OK;
			gps_info.GPS_UBX_latitude = getLatitude(defaultMaxWait);
			gps_info.GPS_UBX_longitude = getLongitude(defaultMaxWait);
			gps_info.GPS_UBX_latitude_Float = (float)gps_info.GPS_UBX_latitude / 10000000;
			gps_info.GPS_UBX_longitude_Float = (float)gps_info.GPS_UBX_longitude / 10000000;
			gps_info.GPSaltitude = getAltitude(defaultMaxWait);
			gps_info.unix_time = (uint32_t)t_of_day;

			Backup_GPS();
			gps_info.latest_gps_status = GPS_SUCCESS;
			return GPS_SUCCESS;
		}
		DelayMs(1000);
	}

	/* If fix taking too long, reset and re-initialize GPS module. 
	* It does a forced hardware reset and recovers from a cold start
	* Reset only after timeout.
	*/
	reinit_gps();

	Backup_GPS();
	gps_info.latest_gps_status = GPS_FAILURE;
	return GPS_FAILURE;
}

/* wakeup gps  */
static gps_status_t init_for_fix()
{

	// HAL_IWDG_Refresh(&hiwdg);

	/* pull GPS extint0 pin high to wake gps */
	GpioWrite(&Gps_int, 1);

	DelayMs(GPS_WAKEUP_TIMEOUT);

	// HAL_IWDG_Refresh(&hiwdg);

	if (put_in_continueous_mode(defaultMaxWait) == false) // Set the constellation to use only GPS
	{
		printf("***!!! Warning: put_in_continueous_mode failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("put_in_continueous_mode carried out successfully!\n");
	}

	// HAL_IWDG_Refresh(&hiwdg);

	/* Set the I2C port to output UBX only (turn off NMEA noise) */
	if (setI2COutput(COM_TYPE_UBX, defaultMaxWait) == false) //Set the I2C port to output UBX only (turn off NMEA noise)
	{
		printf("***!!! Warning: setI2COutput failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("set setI2COutput carried out successfully!\n");
	}

	// HAL_IWDG_Refresh(&hiwdg);

	/* Check if we are in airbourne mode. check if dynamic mode is correct */
	uint8_t newDynamicModel = getDynamicModel(defaultMaxWait);
	if (newDynamicModel == 255)
	{
		printf("***!!! Warning: getDynamicModel failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else if (newDynamicModel != DYN_MODEL_AIRBORNE1g)
	{
		printf("The current dynamic model is INCORRECT. The current dynamic model is: %d\n", newDynamicModel);

		if (setDynamicModel(DYN_MODEL_AIRBORNE1g, defaultMaxWait) == false) // Set the dynamic model to PORTABLE
		{
			printf("***!!! Warning: setDynamicModel failed !!!***\n");
			I2cResetBus(&I2c);
		}
		else
		{
			printf("Dynamic platform model changed successfully!\n");
		}
	}
	else if (newDynamicModel == DYN_MODEL_AIRBORNE1g)
	{
		printf("The current dynamic model correct and is: %d\n", newDynamicModel);
	}
	else
	{
		printf("dynamic model setting error\n");
	}

	// HAL_IWDG_Refresh(&hiwdg);

	return GPS_SUCCESS;
}

static gps_status_t reinit_gps()
{
	// HAL_IWDG_Refresh(&hiwdg);

	// configure gps module again
	factoryReset();
	DelayMs(GPS_WAKEUP_TIMEOUT); // wait for GPS module to be ready

	// HAL_IWDG_Refresh(&hiwdg);

	if (setI2COutput(COM_TYPE_UBX, defaultMaxWait) == false) //Set the I2C port to output UBX only (turn off NMEA noise)
	{
		printf("***!!! Warning: setI2COutput failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("set setI2COutput carried out successfully!\n");
	}

	// HAL_IWDG_Refresh(&hiwdg);

	if (setI2COutput(COM_TYPE_UBX, defaultMaxWait) == false) //Set the I2C port to output UBX only (turn off NMEA noise)
	{
		printf("***!!! Warning: setI2COutput failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("set setI2COutput carried out successfully!\n");
	}

	// HAL_IWDG_Refresh(&hiwdg);

	if (setGPS_constellation_only(defaultMaxWait) == false) // Set the constellation to use only GPS
	{
		printf("***!!! Warning: setGPS_constellation_only failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("set GPS constellation only carried out successfully!\n");
	}

	// HAL_IWDG_Refresh(&hiwdg);

	if (setDynamicModel(DYN_MODEL_AIRBORNE1g, defaultMaxWait) == false) // set to airbourne mode
	{
		printf("***!!! Warning: setDynamicModel failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("Dynamic platform model changed successfully!\n");
	}

	// HAL_IWDG_Refresh(&hiwdg);

	if (set_powersave_config(defaultMaxWait) == false) // Save powersave config to ram. can be activated later.
	{
		printf("***!!! Warning: set_powersave_config failed !!!***\n");
		I2cResetBus(&I2c);
	}
	else
	{
		printf("set_powersave_config carried out successfully!\n");
	}

	// HAL_IWDG_Refresh(&hiwdg);

	return GPS_SUCCESS;
}

static gps_status_t display_still_searching()
{
	// Indicator led to indicate that still searching
	GpioWrite(&Led1, 1);
	DelayMs(100);
	GpioWrite(&Led1, 0);

	return GPS_SUCCESS;
}

/* indicate that fix has been found */
static gps_status_t display_fix_found()
{
	for (uint8_t i = 0; i < 20; i++)
	{
		GpioWrite(&Led1, 1);
		DelayMs(50);
		GpioWrite(&Led1, 0);
		DelayMs(50);
	}

	return GPS_SUCCESS;
}
