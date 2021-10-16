/**
  ******************************************************************************
  * @file    bsp.c
  * @author  MCD Application Team
  * @brief   manages the sensors on the application
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "bsp.h"
#include "ublox.h"
#include "playback.h"
#include "nvmm.h"
#include "iwdg.h"
#include "board.h"
#include "stdio.h"
#include "config.h"
#include <time.h>
#include "geofence.h"
#include "print_utils.h"
#include "struct.h"
#include "gpio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_5

#define MINUTES_IN_DAY 1440UL
#define MINUTES_AGO_TO_SELECT_FROM (MINUTES_IN_DAY * PLAYBACK_DAYS)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

eeprom_playback_stats_t eeprom_playback_stats = {0};

/* Dummy values for testing */

time_pos_fix_t current_position =
	{
		.minutes_since_epoch = 0x0007342E, /*472110 minutes */
		.latitude = 0x17CA /*399121314 == 399121314*/,
		.longitude = 0xD312 /*3541187191 == -753780105 */,
		.altitude = 0x00F2 /*0x0000F221 >>2 */
};

sensor_t sensor_data;
playback_key_info_t *playback_key_info_ptr;
gps_info_t gps_info_latest;

extern Gpio_t Load_enable;

double TEMPERATURE_Value;
double PRESSURE_Value;

/* Private function prototypes -----------------------------------------------*/
void save_current_position_info_to_EEPROM(time_pos_fix_t *currrent_position);
uint32_t unix_time_to_minutes_since_epoch(uint32_t unix_time);
int mod(int a, int b);
void print_stored_coordinates(void);
time_pos_fix_t get_oldest_pos_time(void);
time_pos_fix_t retrieve_eeprom_time_pos(uint16_t time_pos_index);
void increment_eeprom_index_counters(void);
void playback_hw_init(void);
void update_reset_counts_in_ram_nvm(void);
void pretty_print_sensor_values(double *TEMPERATURE_Value, double *PRESSURE_Value, gps_info_t *gps_info, uint16_t *no_load_solar_voltage, uint16_t *load_solar_voltage);
void save_data_to_nvm(void);
void fill_to_send_structs(double *TEMPERATURE_Value, double *PRESSURE_Value, gps_info_t *gps_info, uint16_t *no_load_solar_voltage, uint16_t *load_solar_voltage);
void printDouble(double v, int decimalDigits);
uint32_t minutes_since_epoch_to_unix_time(uint32_t minutes_since_epoch);
void print_time_pos_fix(time_pos_fix_t temp);
void update_geofence_status();

/* Exported functions ---------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

void BSP_sensor_Read(void)
{
	IWDG_reset();

	printf("READING SENSOR AND GPS\n\r");

	/* USER CODE BEGIN 5 */
#if SENSOR_ENABLED
	MS5607_get_temp_pressure();
	IWDG_reset();
#else
	TEMPERATURE_Value = (double)HW_GetTemperatureLevel_int();
#endif

#if GPS_ENABLED
	get_location_fix(GPS_LOCATION_FIX_TIMEOUT);
	gps_info_latest = get_latest_gps_info();
	IWDG_reset();
#endif

	if (get_latest_gps_status() == GPS_SUCCESS)
	{
		update_geofence_status();
	}

	/* read solar voltage under gps and no load */
	uint16_t no_load_solar_voltage = BoardGetBatteryVoltage();
	uint16_t load_solar_voltage = get_load_solar_voltage();

	/* calculate days of playback available */
	time_pos_fix_t oldest_timepos_record = get_oldest_pos_time();
	time_pos_fix_t most_recent_timepos_record = retrieve_eeprom_time_pos(0);

	sensor_data.days_of_playback = (uint8_t)((most_recent_timepos_record.minutes_since_epoch - oldest_timepos_record.minutes_since_epoch) / MINUTES_IN_DAY);

	/* pretty print sensor values for debugging */
	pretty_print_sensor_values(&TEMPERATURE_Value, &PRESSURE_Value, &gps_info_latest, &no_load_solar_voltage, &load_solar_voltage);

	/* Fill up the structs that will be used to make the packet that is sent down over radio */
	fill_to_send_structs(&TEMPERATURE_Value, &PRESSURE_Value, &gps_info_latest, &no_load_solar_voltage, &load_solar_voltage);

	/* fill up the buffer to send down */
	fill_positions_to_send_buffer();

	IWDG_reset();

	/* Save GPS data to non volatile memory */
	save_data_to_nvm();
}

void update_geofence_status()
{
	/* Find out which region of world we are in and update region parm*/

	gps_info_t gps_info = get_latest_gps_info();
	update_geofence_position(gps_info.GPS_UBX_latitude_Float, gps_info.GPS_UBX_longitude_Float);

	/* Save current polygon to eeprom only if gps fix was valid */
	NvmmWrite((void *)&current_geofence_status.current_loramac_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);

	IWDG_reset();
}

/**
 * \brief Fill the structs that will be used for generating the packet that will be sent down over
 * the radio 
 * \param TEMPERATURE_Value
 * \param PRESSURE_Value
 * \param gps_info
 * \param no_load_solar_voltage
 * \param load_solar_voltage
 * 
 * \return void
 */
void fill_to_send_structs(double *TEMPERATURE_Value, double *PRESSURE_Value, gps_info_t *gps_info, uint16_t *no_load_solar_voltage, uint16_t *load_solar_voltage)
{
	// This cast from int32_t to uint16_t could have a roll over if the value was negative.
	// So clip to 0 if below zero altitude.
	if (gps_info->GPSaltitude < 0)
	{
		current_position.altitude = 0;
	}
	else
	{
		current_position.altitude = (gps_info->GPSaltitude >> 8) & 0xffff;
	}

	current_position.latitude = (gps_info->GPS_UBX_latitude >> 16) & 0xffff;
	current_position.longitude = (gps_info->GPS_UBX_longitude >> 16) & 0xffff;
	current_position.minutes_since_epoch = unix_time_to_minutes_since_epoch(gps_info->unix_time) & 0x00ffffff;

	sensor_data.temperature = (int8_t)*TEMPERATURE_Value;
	sensor_data.pressure = (uint16_t)*PRESSURE_Value;
	sensor_data.no_load_solar_voltage = (uint8_t)(*no_load_solar_voltage / 100);
	sensor_data.load_solar_voltage = (uint8_t)(*load_solar_voltage / 100);
	sensor_data.sats = (uint8_t)gps_info->GPSsats;
}

/**
 * \brief Save GPS data to EEPROM
 * 
 * 
 * \return void
 */
void save_data_to_nvm()
{
	/* now save all this data to non volatile memory */
	time_pos_fix_t most_recent = retrieve_eeprom_time_pos(0);

	if (gps_info_latest.latest_gps_status == GPS_SUCCESS)
	{
		/* After the time between saving(HOW_OFTEN_TO_SAVE_POS_TIM_TO_EEPROM) has elapsed, then
		 * increment the counter such that it can save to the next location
		 */
		if ((current_position.minutes_since_epoch - most_recent.minutes_since_epoch > HOW_OFTEN_TO_SAVE_POS_TIM_TO_EEPROM) && (eeprom_playback_stats.n_playback_positions_saved != 0))
		{
			increment_eeprom_index_counters();
		}

		/* Save position to eeprom, overwriting the latest position with every fix. */
		save_current_position_info_to_EEPROM(&current_position);

		if (eeprom_playback_stats.n_playback_positions_saved == 0)
		{
			increment_eeprom_index_counters();
		}

		IWDG_reset();
	}
}
/**
 * @brief Print a double value, with decimal place
 * 
 * @param v 
 * @param decimalDigits 
 */
void printDouble(double v, int decimalDigits)
{
	int i = 1;
	int intPart, fractPart;
	for (; decimalDigits != 0; i *= 10, decimalDigits--)
		;
	intPart = (int)v;
	fractPart = (int)((v - (double)(int)v) * i);
	if (fractPart < 0)
		fractPart *= -1;
	printf("%i.%i", intPart, fractPart);
}

/**
 * \brief Pretty print sensor reading values for debugging
 * 
 * \param TEMPERATURE_Value
 * \param PRESSURE_Value
 * \param gps_info
 * \param no_load_solar_voltage
 * \param load_solar_voltage
 * 
 * \return void
 */

void pretty_print_sensor_values(double *TEMPERATURE_Value, double *PRESSURE_Value, gps_info_t *gps_info, uint16_t *no_load_solar_voltage, uint16_t *load_solar_voltage)
{
	printf("================================================================\r\n");
	printf("SENSOR AND GPS VALUES");
	printf("\r\n");
	printf("================================================================\r\n");

	printf("Temperature degrees C: ");
	printDouble(*TEMPERATURE_Value, 3);
	printf("\r\n");
	printf("Pressure mBar: ");
	printDouble(*PRESSURE_Value, 3);
	printf("\r\n");
	printf("Longitude: ");
	printDouble(gps_info->GPS_UBX_longitude_Float, 6);
	printf(" Latitude: ");
	printDouble(gps_info->GPS_UBX_latitude_Float, 6);
	printf(" altitude: ");
	printf("%ld", gps_info->GPSaltitude / 1000);
	printf("\r\n");
	const char *region_string = get_lorawan_region_string(current_geofence_status.current_loramac_region);
	printf("Loramac region: %s\r\n", region_string);
	printf("GPS time: ");
	printf("%ld", gps_info->unix_time);
	printf("\r\n");
	printf("Solar voltage no load: ");
	printf("%d", *no_load_solar_voltage);
	printf("\r\n");
	printf("Solar voltage with GPS load: ");
	printf("%d", *load_solar_voltage);
	printf("\r\n");
	printf("Reset Count: ");
	printf("%d", sensor_data.reset_count);
	printf("\r\n");
	printf("Data received from ground: ");
	printf("%d", sensor_data.data_received);
	printf("\r\n");
	printf("Days of playback: %d \r\n", sensor_data.days_of_playback);
	printf("================================================================\r\n");
}

bool manage_incoming_instruction(uint8_t *instructions)
{
	uint32_t recent_time_min = extractLong_from_buff(0, instructions);
	uint16_t recent_timepos_index = get_time_pos_index_older_than(recent_time_min);

	printf("Received instruction recent. time(min):%lu timepos index: %d\n", recent_time_min, recent_timepos_index);

	uint32_t older_time_min = extractLong_from_buff(4, instructions);
	uint16_t older_timepos_index = get_time_pos_index_older_than(older_time_min);

	printf("Received instruction older. time(min):%lu timepos index: %d\n", older_time_min, older_timepos_index);

	bool success = process_playback_instructions(recent_timepos_index, older_timepos_index);

	return success;
}

/**
 * \brief Intialise all the sensors and playback
 * 
 * \param 
 * 
 * \return void
 */
void BSP_sensor_Init(void)
{
	IWDG_reset();

	update_reset_counts_in_ram_nvm();

	playback_hw_init();

	IWDG_reset();

	printf("SELFTEST: Initialisng ms5607\n\r");
#if SENSOR_ENABLED
	/* Initialize sensors */
	ms5607_Init();

	IWDG_reset();

#endif

#if GPS_ENABLED
	printf("SELFTEST: Initialising GPS\n\r");

	GpioWrite(&Load_enable, 0); /* Enable power to GPS */
	gps_info_latest = get_latest_gps_info();
	//GPS SETUP
	setup_GPS();

	IWDG_reset();

#endif
}

/**
 * \brief Increment the reset counts in ram and non volatile memory.
 * 
 * 
 * \return void
 */
void update_reset_counts_in_ram_nvm()
{
	/* record number of resets to EEPROM, and also to send down */
	NvmmRead((void *)&sensor_data.reset_count, RESET_COUNTER_LEN, RESET_COUNTER_ADDR);
	sensor_data.reset_count += 1;
	NvmmWrite((void *)&sensor_data.reset_count, RESET_COUNTER_LEN, RESET_COUNTER_ADDR);
}

/**
 * \brief Initialise the hardware aspects of playback i.e. reading from EEPROM.
 * 
 * 
 * \return void
 */
void playback_hw_init()
{
	IWDG_reset();

	NvmmRead((void *)&eeprom_playback_stats, sizeof(eeprom_playback_stats), CURRENT_PLAYBACK_INDEX_IN_EEPROM_ADDR);

	/* Check if CRC is correct */
	if (is_crc_correct(sizeof(eeprom_playback_stats), &eeprom_playback_stats) == false)
	{
		eeprom_playback_stats.current_EEPROM_index = 0;
		eeprom_playback_stats.n_playback_positions_saved = 0;
	}

	/* We want to send positions from the last n days, defined by PLAYBACK_DAYS. Therefore, we need to calculate how 
	 * many saved eeprom position/times we should select from. We take the most recent timepos, then calculate back n days
	 * then calculate the index in eeprom of this timepos index.
	 */
	time_pos_fix_t most_recent_timepos_record = retrieve_eeprom_time_pos(0);
	uint32_t earliest_time_to_send = most_recent_timepos_record.minutes_since_epoch - MINUTES_AGO_TO_SELECT_FROM;

	/* if there is not timepos index older than the calculated earliest time to send, then select from all the 
	 * n_playback_positions_saved
	 */
	uint16_t earliest_timepos_index;
	uint16_t older_index = get_time_pos_index_older_than(earliest_time_to_send);

	if (older_index == 0)
	{
		earliest_timepos_index = eeprom_playback_stats.n_playback_positions_saved;
	}
	else
	{
		earliest_timepos_index = older_index;
	}

	printf("earliest_timepos_index: %d\n", earliest_timepos_index);
	printf("older_index: %d\n", older_index);

	/* Initialise playback */
	init_playback(&sensor_data, &current_position, &retrieve_eeprom_time_pos, earliest_timepos_index);

	IWDG_reset();

	playback_key_info_ptr = get_playback_key_info_ptr();

	/* print out stored time/pos data for debugging */
	print_stored_coordinates();

	IWDG_reset();
}

/**
 * \brief Print out all the stored coordinates
 * 
 * 
 * \return void
 */
void print_stored_coordinates()
{
	/* test stored positoins */

	uint32_t n_positions_to_print = MIN(eeprom_playback_stats.n_playback_positions_saved, N_POSITIONS_TO_PRINTOUT);

	printf("%d playback positions have been saved. Printing %ld most recent positions saved.\n",
		   eeprom_playback_stats.n_playback_positions_saved,
		   n_positions_to_print);

	for (uint16_t i = 0; i < n_positions_to_print; i++)
	{
		time_pos_fix_t temp = retrieve_eeprom_time_pos(i);
		printf("index: %d,", i);
		print_time_pos_fix(temp);
	}
}

void print_time_pos_fix(time_pos_fix_t temp)
{
	uint32_t unix_time = minutes_since_epoch_to_unix_time(temp.minutes_since_epoch);

	time_t now;
	struct tm ts;
	char buf[80];

	now = unix_time;

	// Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
	ts = *localtime(&now);
	strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

	printf("long: %d, lat: %d, alt: %d, ts: %s\n", temp.longitude, temp.latitude, temp.altitude, buf);
}

/**
 * \brief get postime oldest datapoint
 * 
 * 
 * \return time_pos_fix_t
 */
time_pos_fix_t get_oldest_pos_time()
{
	/* test stored positoins */
	printf("Getting oldest position:\n");

	/* The index starts from 0. If 1 position is saved, it will have an index of 0.
	 * Generalising, if there are n positions, the nth position index will be n-1.
	 * The problem is, if there are 0 poitions saved, the index calculated will be -1.
	 * This will break the retrieve_eeprom_time_pos() because it reads only positive numbers.
	 * So if thats the case, then force the index to be 0.
	 * TODO: make it return a null value when n_playback_positions_saved == 0
	 */
	uint16_t index = (eeprom_playback_stats.n_playback_positions_saved == 0) ? 0 : eeprom_playback_stats.n_playback_positions_saved - 1;

	time_pos_fix_t temp = retrieve_eeprom_time_pos(index);

	printf("oldest position/time record: ");
	print_time_pos_fix(temp);

	return temp;
}

/**
 * \brief look through eeprom to find the time/pos entry older than 
 * the given minute_from_epoch
 * 
 * 
 * \return index of time pos. If there is no time/pos older in eeprom, then return 0
 */
uint16_t get_time_pos_index_older_than(uint32_t minutes_from_epoch)
{
	uint16_t res_index = 0;
	for (uint16_t i = 0; i < eeprom_playback_stats.n_playback_positions_saved; i++)
	{
		time_pos_fix_t temp = retrieve_eeprom_time_pos(i);
		if (temp.minutes_since_epoch < minutes_from_epoch)
		{
			return i;
		}
	}

	return res_index;
}

/**
 * \brief Update counters in ram and EEPROM for index in EEPROM where to write
 * and how many positions we have saved so far. 
 * 
 * \return void
 */
void increment_eeprom_index_counters()
{
	/* Now update the index in EEPROM */
	eeprom_playback_stats.current_EEPROM_index = mod(eeprom_playback_stats.current_EEPROM_index + PLAYBACK_EEPROM_PACKET_SIZE, PLAYBACK_EEPROM_SIZE);
	eeprom_playback_stats.n_playback_positions_saved = MIN(eeprom_playback_stats.n_playback_positions_saved + 1, MAX_PLAYBACK_POSITIONS_SAVED_IN_EEPROM);
	playback_key_info_ptr->n_positions_saved_since_boot += 1;

	eeprom_playback_stats.Crc32 = Crc32((uint8_t *)&eeprom_playback_stats, sizeof(eeprom_playback_stats) - sizeof(eeprom_playback_stats.Crc32));
	NvmmWrite((void *)&eeprom_playback_stats, sizeof(eeprom_playback_stats), CURRENT_PLAYBACK_INDEX_IN_EEPROM_ADDR);
}

/**
  * @brief Save all the position data and time to EEPROM
  * @param none
  * @retval none
  */
void save_current_position_info_to_EEPROM(time_pos_fix_t *currrent_position)
{
	/* save Long, Lat, Altitude, minutes since epoch to EEPROM */
	uint16_t location_to_write = PLAYBACK_EEPROM_ADDR_START + eeprom_playback_stats.current_EEPROM_index;
	NvmmWrite((void *)&current_position.altitude, ALTITUDE_BYTES_LEN, location_to_write + 0);
	NvmmWrite((void *)&current_position.latitude, LATITUDE_BYTES_LEN, location_to_write + 2);
	NvmmWrite((void *)&current_position.longitude, LONGITUDE_BYTES_LEN, location_to_write + 4);
	NvmmWrite((void *)&current_position.minutes_since_epoch, MINUTES_SINCE_EPOCH_BYTES_LEN, location_to_write + 6);
}

/**
 * \brief Read the eeprom for a position and time and a certain index.
 * 
 * \param index. index 0 is the most recent timepos stored in eeprom.
 * 
 * \return time_pos_fix_t position time fix
 */
time_pos_fix_t retrieve_eeprom_time_pos(uint16_t time_pos_index)
{
	IWDG_reset();

	time_pos_fix_t time_pos_fix = {0};

	/* read Long, Lat, Altitude, minutes since epoch from EEPROM */
	uint16_t location_to_read = PLAYBACK_EEPROM_ADDR_START + mod(eeprom_playback_stats.current_EEPROM_index - (time_pos_index + 1) * PLAYBACK_EEPROM_PACKET_SIZE, PLAYBACK_EEPROM_SIZE);
	NvmmRead((void *)&time_pos_fix.altitude, ALTITUDE_BYTES_LEN, location_to_read + 0);
	NvmmRead((void *)&time_pos_fix.latitude, LATITUDE_BYTES_LEN, location_to_read + 2);
	NvmmRead((void *)&time_pos_fix.longitude, LONGITUDE_BYTES_LEN, location_to_read + 4);
	NvmmRead((void *)&time_pos_fix.minutes_since_epoch, MINUTES_SINCE_EPOCH_BYTES_LEN, location_to_read + 6);

	IWDG_reset();

	return time_pos_fix;
}

/*
 * \brief Mathematical mod operation. a mod b.
 * 
 * \param a
 * \param b
 * 
 * \return int
 */
int mod(int a, int b)
{
	int r = a % b;
	return r < 0 ? r + b : r;
}

/**
  * @brief Calculate minutes since epoch. Based on GPS time.
	* Epoch is set to 1 Jan 2020 01:01:01H( unix time: 1577840461)
  * @param none
  * @retval none
  */
uint32_t unix_time_to_minutes_since_epoch(uint32_t unix_time)
{
	return (unix_time - 1577840461) / 60;
}

uint32_t minutes_since_epoch_to_unix_time(uint32_t minutes_since_epoch)
{
	return minutes_since_epoch * 60 + 1577840461;
}

void retrieve_eeprom_stored_lorawan_region()
{

	IWDG_reset();

	/* read the eeprom value instead */
	// TODO: must ensure that eeprom is not filled with garbage. i.e. when the eeprom has never been programed
	if (USE_NVM_STORED_LORAWAN_REGION == true)
	{
		NvmmRead((void *)&current_geofence_status.current_loramac_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);
	}
	IWDG_reset();
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
