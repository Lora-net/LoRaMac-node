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
#include "bsp.h"
#include <string.h>
#include "linear_congruential_generator.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */

playback_key_info_t current_playback_key_info =
	{
		.n_positions_to_send = DEFAULT_N_POSITIONS_TO_SEND, /* Number of positions to send down in single transmission*/
		.position_pool_size_to_select_from = 0,				/* Define size of pool of positions to select from */
		.n_positions_saved_since_boot = 0,					/* Define size of pool of positions to select from */
		.request_from_gnd = false,
		.requested_pos_index_lower = 0,
		.requested_pos_index_upper = 0

};

static uint8_t tx_str_buffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];
static uint16_t tx_str_buffer_len = 0;
time_pos_fix_t subset_positions[MAX_N_POSITIONS_TO_SEND];

time_pos_fix_t *current_pos_ptr;

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */

typedef int (*select_low_discrepancy_T)(int low, int high);

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */

void fill_subset_positions_buffer(uint16_t subset_size);
void fill_tx_buffer_with_location(uint16_t start_point, uint8_t *buffer, uint16_t latitude, uint16_t longitude, uint16_t altitude);
void fill_tx_buffer_with_location_and_time(uint16_t start_point, uint8_t *buffer,
										   uint16_t latitude, uint16_t longitude,
										   uint16_t altitude, uint32_t minutes_since_epoch);

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

		subset_positions[i].altitude_encoded = random_time_pos.altitude_encoded;
		subset_positions[i].latitude_encoded = random_time_pos.latitude_encoded;
		subset_positions[i].longitude_encoded = random_time_pos.longitude_encoded;
		subset_positions[i].minutes_since_epoch = random_time_pos.minutes_since_epoch;
	}

	/* we have serviced the request. set to false now */
	current_playback_key_info.request_from_gnd = false;
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
	tx_str_buffer[start_point + POSITION_BYTES_LEN + 2] = (minutes_since_epoch >> 16) & 0xff;
}

void fill_tx_buffer_with_sensor_debug_data(uint16_t start_point, uint8_t *buffer)
{
	sensor_t current_sensor_data = get_current_sensor_data();
	/* byte 0: no load voltage(5 bits) and load voltage(3 bits) */
	tx_str_buffer[0] = ((current_sensor_data.no_load_solar_voltage - 18) & 0x1F) << 3;
	tx_str_buffer[0] |= ((current_sensor_data.load_solar_voltage - 18) & 0x1C) >> 2;

	/* byte1: load voltage(remaining 2 bits) and days of playback available (6 bits) */
	tx_str_buffer[1] = ((current_sensor_data.load_solar_voltage - 18) & 0x03) << 6;
	tx_str_buffer[1] |= (current_sensor_data.days_of_playback & 0x3F);

	/* byte2: status_bitfields(8 bits) */
	tx_str_buffer[2] = current_sensor_data.status_bitfields;

	/* byte3: Sats(5 bits) and reset count(3 bits)*/
	tx_str_buffer[3] = (current_sensor_data.sats & 0x1F) << 3;
	tx_str_buffer[3] |= (current_sensor_data.reset_count & 0x07);

	/* byte4: Temperature (8 bits)*/
	tx_str_buffer[4] = (uint8_t)(current_sensor_data.temperature);
}

void fill_tx_buffer_with_past_data(uint16_t start_point, uint8_t *buffer)
{

	for (int i = 0; i < current_playback_key_info.n_positions_to_send; i++)
	{
		time_pos_fix_t temp_pos = subset_positions[i];

		fill_tx_buffer_with_location_and_time(start_point + i * (POSITION_BYTES_LEN + MINUTES_SINCE_EPOCH_BYTES_LEN),
											  buffer,
											  temp_pos.latitude_encoded,
											  temp_pos.longitude_encoded,
											  temp_pos.altitude_encoded,
											  temp_pos.minutes_since_epoch);
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
	// Fill the bitfields field in the message with a crc8 checksum of all stored EEPROM settings.
	settings_crc_set();

	fill_tx_buffer_with_sensor_debug_data(0, tx_str_buffer);

	fill_tx_buffer_with_location(SENSOR_DEBUG_BYTES_LEN, tx_str_buffer, current_pos_ptr->latitude_encoded, current_pos_ptr->longitude_encoded, current_pos_ptr->altitude_encoded);

	fill_tx_buffer_with_past_data(SENSOR_DEBUG_BYTES_LEN + POSITION_BYTES_LEN, tx_str_buffer);

	tx_str_buffer_len = SENSOR_DEBUG_BYTES_LEN + POSITION_BYTES_LEN + (POSITION_BYTES_LEN + MINUTES_SINCE_EPOCH_BYTES_LEN) * current_playback_key_info.n_positions_to_send;

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
void init_playback(time_pos_fix_t *current_pos,
				   retrieve_eeprom_time_pos_ptr_T retrieve_eeprom_time_pos_ptr,
				   uint16_t n_positions_to_select_from)
{

	memset(&current_playback_key_info, 0, sizeof(playback_key_info_t));
	current_playback_key_info.n_positions_to_send = DEFAULT_N_POSITIONS_TO_SEND;
	current_playback_key_info.position_pool_size_to_select_from = n_positions_to_select_from;

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
		success = false;
	}

	return success;
}
