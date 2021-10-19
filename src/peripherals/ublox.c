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
#include "config.h"
#include "utilities.h"
#include "i2c.h"
#include <board.h>
#include <systime.h>
#include <string.h>
#include "iwdg.h"
#include "deep_sleep_delay.h"

#include "SparkFun_Ublox_Arduino_Library.h"

#include "i2c_middleware.h"
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

uint32_t systimeMS_get(void);
static gps_status_t display_still_searching(void);
static gps_status_t display_fix_found(void);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */

uint32_t systimeMS_get()
{
	return SysTimeToMs(SysTimeGet());
}

gps_status_t get_latest_gps_status(void)
{
	return gps_info.latest_gps_status;
}

gps_info_t get_latest_gps_info(void)
{
	return gps_info;
}

/* Get solar voltage when under load from GPS */
uint16_t get_load_solar_voltage()
{
	return load_solar_voltage;
}

/* 
 * Sets up gps by putting in airbourne mode, setting to use GPS satellites only, turning off NMEA
 * 
 */
gps_status_t setup_GPS()
{
	GpioWrite(&Gps_int, 0); /* Create Edge on GPS extint0 pin to wake gps incase it was in backup state */
	GpioWrite(&Gps_int, 1); /* Force GPS to stay in active mode by setting extint0 pin high */

	DeepSleepDelayMs(GPS_WAKEUP_TIMEOUT); // Wait for things to be setup

	/* Set the I2C port to output UBX only (turn off NMEA noise) */
	if (setI2COutput(COM_TYPE_UBX, defaultMaxWait) == false) //Set the I2C port to output UBX only (turn off NMEA noise)
	{
		printf("***!!! Warning: setI2COutput failed !!!***\n");
		reinit_i2c();
	}
	else
	{
		printf("set setI2COutput carried out successfully!\n");
	}

	if (setDynamicModel(DYN_MODEL_AIRBORNE1g, defaultMaxWait) == false) // Set the dynamic model to DYN_MODEL_AIRBORNE1g
	{
		printf("***!!! Warning: setDynamicModel failed !!!***\n");
		reinit_i2c();
	}
	else
	{
		printf("Dynamic platform model changed successfully!\n");
	}

	if (set_powersave_config(defaultMaxWait) == false)
	{
		printf("***!!! Warning: set_powersave_config failed !!!***\n");
		reinit_i2c();
	}
	else
	{
		printf("set_powersave_config carried out successfully!\n");
	}

	return GPS_SUCCESS;
}

/* Get the location fix */
gps_status_t get_location_fix(uint32_t timeout)
{

	IWDG_reset();

	setup_GPS();

	IWDG_reset();

	/* Set all the GPS info values to zeros */
	memset(&gps_info, 0, sizeof(gps_info_t));

	/* poll UBX-NAV-PVT until the module has fix */
	uint32_t startTime = systimeMS_get();
	while (systimeMS_get() - startTime < timeout)
	{
		IWDG_reset();

		display_still_searching();

		uint8_t temp_GPSfix_type = getFixType(defaultMaxWait);
		uint8_t temp_GPSsats = getSIV(defaultMaxWait);
		uint8_t temp_GPShour = getHour(defaultMaxWait);
		uint8_t temp_GPSminute = getMinute(defaultMaxWait);
		uint8_t temp_GPSsecond = getSecond(defaultMaxWait);
		uint8_t temp_GPSmillisecond = getMillisecond(defaultMaxWait);

		uint16_t temp_GPSyear = getYear(defaultMaxWait);
		uint8_t temp_GPSmonth = getMonth(defaultMaxWait);
		uint8_t temp_GPSday = getDay(defaultMaxWait);
		uint8_t temp_GPSfix_OK = getgnssFixOK(defaultMaxWait);

		SysTime_t stime = SysTimeGetMcuTime();
		printf("%3lds%03dms: ", stime.Seconds, stime.SubSeconds);

		uint32_t current_time = (systimeMS_get() - startTime) / 1000;

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
		printf(" GPS Search time[s]: %ld ", current_time);

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

		printf("Epoch[s]: %ld\n", (uint32_t)t_of_day);

		load_solar_voltage = BoardGetBatteryVoltage();

		if (temp_GPSsats >= SATS) // As long as we have 4 sats in solution, the fix is good enough. First time to fix is much more important than accuracy.
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
			GpioWrite(&Gps_int, 0); /* force it to sleep*/

			gps_info.latest_gps_status = GPS_SUCCESS;
			return GPS_SUCCESS;
		}
		DeepSleepDelayMs(2000);
	}

	/* If fix taking too long,resend all the settings,
	 * and put it to sleep.
	 */
	setup_GPS();
	GpioWrite(&Gps_int, 0); /* force it to sleep*/

	gps_info.latest_gps_status = GPS_FAILURE;
	return GPS_FAILURE;
}

/* Indicator led to indicate that still searching */
static gps_status_t display_still_searching()
{
	GpioWrite(&Led1, 1);
	DeepSleepDelayMs(100);
	GpioWrite(&Led1, 0);

	return GPS_SUCCESS;
}

/* Indicate that fix has been found */
static gps_status_t display_fix_found()
{
	for (uint8_t i = 0; i < 20; i++)
	{
		GpioWrite(&Led1, 1);
		DeepSleepDelayMs(50);
		GpioWrite(&Led1, 0);
		DeepSleepDelayMs(50);
	}

	return GPS_SUCCESS;
}
