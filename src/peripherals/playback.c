/*
 * playback.c 
 * Author: Medad Newman 2/11/2020
 * Imperial College Space Society ( Medad Newman, Richard Ibbotson)
 *
 * Manages the playback of coordinates over time.
 *
 */

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "playback.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "bsp.h"
#include "utilities.h"
#include <math.h>
#include <string.h>

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */

#define CORPUT_BASE 2U

/*!
 * \brief Returns the minimum value between a and b
 *
 * \param [IN] a 1st value
 * \param [IN] b 2nd value
 * \retval minValue Minimum value
 */

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */

/* Dummy values for testing */
time_pos_fix_t current_pos =
	{
		.minutes_since_epoch = 0x0007342E, /*472110 minutes */
		.latitude = 0x17CA /*399121314 == 399121314*/,
		.longitude = 0xD312 /*3541187191 == -753780105 */,
		.altitude = 0x00F2 /*0x0000F221 >>2 */
};

sensor_t current_sensor_data =
	{
		.no_load_solar_voltage = 33, /* 18 - 43 (min 25 values)(5 bits) */
		.load_solar_voltage = 43,	 /* 18 - 43 (min 25 values)(5 bits) */
		.temperature = -23,			 /* -64 to 64 in increments of 2 degrees celcius (min 40 values)(6 bits) */
		.pressure = 400,			 /* 130 - 1030 (min 128 values, 10mbar per increment)(7 bits) */
		.data_received = 1,			 /* 0 or 1. indicates that message was received(1 bit) */
		.sats = 12,					 /* 0 - 32. Number of sats. (4 bits) */
		.reset_count = 7,			 /* 0-7. Number of resets in (3 bits) */
		.days_of_playback = 63		 /* 0-64. Number of days of playback available (6 bits) */

};

playback_key_info_t current_playback_key_info =
	{
		.n_positions_to_send = DEFAULT_N_POSITIONS_TO_SEND, /* Number of positions to send down in single transmission*/
		.position_pool_size_to_select_from = 0,				/* Define size of pool of positions to select from */
		.n_positions_saved_since_boot = 0,					/* Define size of pool of positions to select from */
		.request_from_gnd = false,
		.playback_error = false,
		.requested_pos_index_lower = 0,
		.requested_pos_index_upper = 0

};

static uint8_t tx_str_buffer[LORAWAN_APP_DATA_BUFF_SIZE];
static uint16_t tx_str_buffer_len = 0;
time_pos_fix_t subset_positions[MAX_N_POSITIONS_TO_SEND];
int corput_n = 0;

sensor_t *current_sensor_data_ptr;
time_pos_fix_t *current_pos_ptr;

struct LGC_params
{
	int stop;
	int start;
	int maximum;
	int value;
	int offset;
	int step;
	int multiplier;
	int modulus;
	int found;
} LGC_current_params;

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */

typedef int (*select_low_discrepancy_T)(int low, int high);

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */

int generate_random(int l, int r);
void fill_subset_positions_buffer(uint16_t subset_size);
void fill_tx_buffer_with_location(uint16_t start_point, uint8_t *buffer, uint16_t latitude, uint16_t longitude, uint16_t altitude);
void fill_tx_buffer_with_location_and_time(uint16_t start_point, uint8_t *buffer,
										   uint16_t latitude, uint16_t longitude,
										   uint16_t altitude, uint32_t minutes_since_epoch);

int mapping(int i, int start, int step);
void init_LGC(int start, int stop, int step);
int next_LCG(void);
int LCG(int lower_val, int upper_val);

/* Initlise pointer to retrieve eeprom time pos */
retrieve_eeprom_time_pos_ptr_T Retrieve_eeprom_time_pos_ptr;
select_low_discrepancy_T select_low_discrepancy_ptr = LCG;

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */

/**
 * \brief Return pointer to current_playback_key_info
 * 
 * \param 
 * 
 * \return playback_key_info_t *
 */
playback_key_info_t *get_playback_key_info_ptr(void)
{
	return &current_playback_key_info;
}

/**
 * \brief Fill buffer of random subset of positions to be sent.
 *  TODO: find out how to select a subset without repeats
 *  TODO: find a better solution of selecting a subset to give us the most info
 *  This buffer is not even needed. The random selection funtion can be used
 *  to select the data points just as the tx string is being built
 *  redundent.
 * \param 
 * 
 * \return void
 */
void fill_positions_to_send_buffer(void)
{

	int upper_val;
	int lower_val;

	if (current_playback_key_info.request_from_gnd == true)
	{
		/* if a valid request for positions were requested, then limit the range of values */
		upper_val = current_playback_key_info.requested_pos_index_upper;
		lower_val = current_playback_key_info.requested_pos_index_lower;
	}
	else
	{
		/* Randomly select from positions in the last 10 days */
		upper_val = current_playback_key_info.position_pool_size_to_select_from;
		lower_val = 0;
	}

	for (int i = 0; i < current_playback_key_info.n_positions_to_send; i++)
	{

		int rand_time_pos_index = select_low_discrepancy_ptr(lower_val, upper_val);

		/* Compensate for new positions added in. Ensure no repeats */
		if (current_playback_key_info.request_from_gnd == false)
		{
			rand_time_pos_index += current_playback_key_info.n_positions_saved_since_boot;
		}

		time_pos_fix_t random_time_pos = Retrieve_eeprom_time_pos_ptr(rand_time_pos_index);

		subset_positions[i].altitude = random_time_pos.altitude;
		subset_positions[i].latitude = random_time_pos.latitude;
		subset_positions[i].longitude = random_time_pos.longitude;
		subset_positions[i].minutes_since_epoch = random_time_pos.minutes_since_epoch;
	}

	/* we have serviced the request. set to false now */
	current_playback_key_info.request_from_gnd = false;
}

int mapping(int i, int start, int step)
{
	return (i * step) + start;
}

/**
 * \brief Initialise the LGC function
 * 
 * \param start
 * \param stop
 * \param step
 * 
 * \return void
 */
void init_LGC(int start, int stop, int step)
{
	if (start >= stop)
	{
		start = 0;
		stop = 1;
	}

	LGC_current_params.found = 0;
	LGC_current_params.stop = stop;
	LGC_current_params.start = start;
	LGC_current_params.step = step;

	LGC_current_params.maximum = (int)floor((LGC_current_params.stop - LGC_current_params.start) / LGC_current_params.step);
	LGC_current_params.value = generate_random(0, LGC_current_params.maximum);

	LGC_current_params.offset = generate_random(0, LGC_current_params.maximum) * 2 + 1;
	LGC_current_params.multiplier = 4 * (int)floor(LGC_current_params.maximum / 4) + 1;
	LGC_current_params.modulus = (int)pow(2, ceil(log2(LGC_current_params.maximum)));
}

/**
 * \brief Adapted from https://stackoverflow.com/a/53551417
 * 
 * \param start
 * \param stop
 * \param step
 * 
 * \return int
 */
int next_LCG()
{
	bool done = false;
	int res;
	while (1)
	{
		// If this is a valid value, yield it in generator fashion.
		if (LGC_current_params.value < LGC_current_params.maximum)
		{
			res = mapping(LGC_current_params.value, LGC_current_params.start, LGC_current_params.step);
			done = true;
		}
		// Calculate the next value in the sequence.
		LGC_current_params.value = (LGC_current_params.value * LGC_current_params.multiplier + LGC_current_params.offset) % LGC_current_params.modulus;

		if (done == true)
		{
			return res;
		}
	}
}

/**
 * \brief Return a value, within the bounds of lower_val(inclusive) and upper_val(not inclusive)
 * 
 * \param lower_val
 * \param upper_val
 * 
 * \return int
 */
int LCG(int lower_val, int upper_val)
{
	if ((LGC_current_params.start != lower_val) || (LGC_current_params.stop != upper_val))
	{
		init_LGC(lower_val, upper_val, 1);
	}

	return next_LCG();
}

/**
 * \brief This will generate random number in range l and r, inclusive of both
 * 
 * \param l: lower bound
 * \param r: upper bound
 * 
 * \return int random number
 */
int generate_random(int l, int r)
{
	int rand_num = (rand() % (r - l + 1)) + l;

	return rand_num;
}

/**
 * \brief  Fill the tx buffer with location info only
 * 
 * \param start_point : index of start point in buffer to start filling
 * \param buffer : pointer to buffer
 * \param latitude 
 * \param longitude
 * \param altitude
 * 
 * \return void
 */
void fill_tx_buffer_with_location(uint16_t start_point, uint8_t *buffer, uint16_t latitude, uint16_t longitude, uint16_t altitude)
{

	/* Send current position. Use the most significant numbers. Truncate to 16 bits.*/
	/* latitude(16 bits) -90 to 90*/
	tx_str_buffer[start_point + 0] = (latitude >> 0) & 0xff;
	tx_str_buffer[start_point + 1] = (latitude >> 8) & 0xff;
	/* longitude(16 bits) -180 to 180 */
	tx_str_buffer[start_point + 2] = (longitude >> 0) & 0xff;
	tx_str_buffer[start_point + 3] = (longitude >> 8) & 0xff;
	/* altitude(16 bits) */
	tx_str_buffer[start_point + 4] = (altitude >> 0) & 0xff;
	tx_str_buffer[start_point + 5] = (altitude >> 8) & 0xff;
}

/**
 * \brief  Fill the tx buffer with location and time information
 * 
 * \param start_point : index of start point in buffer to start filling
 * \param buffer : pointer to buffer
 * \param latitude
 * \param longitude
 * \param altitude
 * \param minutes_since_epoch
 * 
 * \return void
 */
void fill_tx_buffer_with_location_and_time(uint16_t start_point, uint8_t *buffer, uint16_t latitude, uint16_t longitude, uint16_t altitude, uint32_t minutes_since_epoch)
{
	/* Fill the location info */
	fill_tx_buffer_with_location(start_point, buffer, latitude, longitude, altitude);
	/* Send minutes since epoch */
	tx_str_buffer[start_point + POSITION_BYTES_LEN + 0] = (minutes_since_epoch >> 0) & 0xff;
	tx_str_buffer[start_point + POSITION_BYTES_LEN + 1] = (minutes_since_epoch >> 8) & 0xff;
}

void fill_tx_buffer_with_sensor_debug_data(uint16_t start_point, uint8_t *buffer)
{
	/* byte 0: no load voltage(5 bits) and load voltage(3 bits) */
	tx_str_buffer[0] = ((current_sensor_data_ptr->no_load_solar_voltage - 18) & 0x1F) << 3;
	tx_str_buffer[0] |= ((current_sensor_data_ptr->load_solar_voltage - 18) & 0x1C) >> 2;

	/* byte1: load voltage(remaining 2 bits) and days of playback available (6 bits) */
	tx_str_buffer[1] = ((current_sensor_data_ptr->load_solar_voltage - 18) & 0x03) << 6;
	tx_str_buffer[1] |= (current_sensor_data_ptr->days_of_playback & 0x3F);

	/* byte2: pressure(7 bits) and data received flag(1 bit)*/
	tx_str_buffer[2] = ((current_sensor_data_ptr->pressure / 10) & 0x7F) << 1;
	tx_str_buffer[2] |= (current_playback_key_info.playback_error & 0x01);

	/* byte3: Sats(5 bits) and reset count(3 bits)*/
	tx_str_buffer[3] = (current_sensor_data_ptr->sats & 0x1F) << 3;
	tx_str_buffer[3] |= (current_sensor_data_ptr->reset_count & 0x07);

	/* byte4: Temperature (8 bits)*/
	tx_str_buffer[4] = (uint8_t)(current_sensor_data_ptr->temperature);
}

void fill_tx_buffer_with_past_data(uint16_t start_point, uint8_t *buffer)
{

	for (int i = 0; i < current_playback_key_info.n_positions_to_send; i++)
	{
		time_pos_fix_t temp_pos = subset_positions[i];

		/* calculate time delta between current time and the past time. Up to 2 bytes in length. ~45 days */
		uint16_t delta_time = (uint16_t)(current_pos_ptr->minutes_since_epoch - temp_pos.minutes_since_epoch);

		fill_tx_buffer_with_location_and_time(start_point + i * (POSITION_BYTES_LEN + MINUTES_SINCE_EPOCH_DELTA_BYTES_LEN),
											  buffer,
											  temp_pos.latitude,
											  temp_pos.longitude,
											  temp_pos.altitude,
											  delta_time);
	}
}

/**
 * \brief Prepare Tx string, by filling the AppData.Buff[] with the 
 * data string to be sent down to gateways on the ground. 
 * 
 * \return returns buffer and length in a struct
 */
PicoTrackerAppData_t prepare_tx_buffer()
{
	fill_tx_buffer_with_sensor_debug_data(0, tx_str_buffer);

	fill_tx_buffer_with_location(SENSOR_DEBUG_BYTES_LEN, tx_str_buffer, current_pos_ptr->latitude, current_pos_ptr->longitude, current_pos_ptr->altitude);

	fill_tx_buffer_with_past_data(SENSOR_DEBUG_BYTES_LEN + POSITION_BYTES_LEN, tx_str_buffer);

	tx_str_buffer_len = SENSOR_DEBUG_BYTES_LEN + POSITION_BYTES_LEN + (POSITION_BYTES_LEN + MINUTES_SINCE_EPOCH_DELTA_BYTES_LEN) * current_playback_key_info.n_positions_to_send;

	current_playback_key_info.playback_error = false;

	PicoTrackerAppData_t data = {.Buffer = tx_str_buffer, .BufferSize = tx_str_buffer_len};

	return data;
}

/**
 * \brief Intialise the pointers with pointers to actual buffer locations
 *  Initialises this module
 * \param sensor_data
 * \param current_pos
 * \param retrieve_eeprom_time_pos_ptr
 * 
 * \return void
 */
void init_playback(sensor_t *sensor_data, time_pos_fix_t *current_pos,
				   retrieve_eeprom_time_pos_ptr_T retrieve_eeprom_time_pos_ptr,
				   uint16_t n_positions_to_select_from)
{

	memset(&current_playback_key_info, 0, sizeof(playback_key_info_t));
	current_playback_key_info.n_positions_to_send = DEFAULT_N_POSITIONS_TO_SEND;
	current_playback_key_info.position_pool_size_to_select_from = n_positions_to_select_from;

	current_sensor_data_ptr = sensor_data;
	current_pos_ptr = current_pos;
	Retrieve_eeprom_time_pos_ptr = retrieve_eeprom_time_pos_ptr;

	init_LGC(0, 0, 1);
}

/**
 * \brief Handle request from ground for past position fixes from the specified date
 * range. Take message from ground and parse it, to set the values in current_playback_key_info
 * 
 * \param instructions as a pointer
 * 
 * \return bool: true if error, else false
 */
bool process_playback_instructions(uint16_t recent_timepos_index, uint16_t older_timepos_index)
{
	bool success = true;

	if ((recent_timepos_index > 0) && (older_timepos_index > 0) && !(recent_timepos_index >= older_timepos_index))
	{
		current_playback_key_info.requested_pos_index_lower = recent_timepos_index;
		current_playback_key_info.requested_pos_index_upper = older_timepos_index;
		current_playback_key_info.request_from_gnd = true;

		success = true;
	}
	else
	{
		current_playback_key_info.playback_error = true;

		success = false;
	}

	return success;
}

//Given a spot in the payload array, extract four bytes and build a long
uint32_t extractLong_from_buff(uint8_t spotToStart, uint8_t *buff)
{
	uint32_t val = 0;
	val |= (uint32_t)buff[spotToStart + 0] << 8 * 0;
	val |= (uint32_t)buff[spotToStart + 1] << 8 * 1;
	val |= (uint32_t)buff[spotToStart + 2] << 8 * 2;
	val |= (uint32_t)buff[spotToStart + 3] << 8 * 3;
	return (val);
}
