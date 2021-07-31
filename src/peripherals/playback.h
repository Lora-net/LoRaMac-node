/**
  ******************************************************************************
  * @file           : playback.h
  * @brief          : manages playback of coordinates
  ******************************************************************************
  * Imperial College Space Society
	* Medad Newman, Richard Ibbotson
	* 2/11/2020
  *
  *
  ******************************************************************************
  */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef PLAYBACK_H
#define PLAYBACK_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include <stdint.h>
#include "bsp.h"
#include <stdbool.h>

  /* ==================================================================== */
  /* ============================ constants ============================= */
  /* ==================================================================== */

  /* #define and enum statements go here */

#define LONGITUDE_BYTES_LEN 2U
#define LATITUDE_BYTES_LEN 2U
#define ALTITUDE_BYTES_LEN 2U
#define MINUTES_SINCE_EPOCH_BYTES_LEN 3U
#define MINUTES_SINCE_EPOCH_DELTA_BYTES_LEN 2U
#define POSITION_BYTES_LEN (LONGITUDE_BYTES_LEN + LATITUDE_BYTES_LEN + ALTITUDE_BYTES_LEN)
#define SENSOR_DEBUG_BYTES_LEN 5U

  typedef struct
  {
    uint16_t n_positions_to_send;               /* Number of positions to send down in single transmission*/
    uint16_t position_pool_size_to_select_from; /* Define size of pool of positions to select from */
    uint16_t n_positions_saved_since_boot;      /* Number of posiionts that have been saved since boot */
    bool request_from_gnd;
    bool playback_error;
    uint16_t requested_pos_index_lower;
    uint16_t requested_pos_index_upper;

  } playback_key_info_t;

  /*!
 * Application data structure
 */
  typedef struct PicoTrackerAppData_s
  {
    uint8_t BufferSize;
    uint8_t *Buffer;
  } PicoTrackerAppData_t;

  typedef time_pos_fix_t (*retrieve_eeprom_time_pos_ptr_T)(uint16_t time_pos_index);

  /* ==================================================================== */
  /* ========================== public data ============================= */
  /* ==================================================================== */

  /* Definition of public (external) data types go here */
  PicoTrackerAppData_t prepare_tx_buffer(void);
  playback_key_info_t *get_playback_key_info_ptr(void);

  void init_playback(
      sensor_t *sensor_data,
      time_pos_fix_t *current_pos,
      retrieve_eeprom_time_pos_ptr_T retrieve_eeprom_time_pos_ptr,
      uint16_t n_positions_to_select_from);

  bool process_playback_instructions(uint16_t recent_timepos_index, uint16_t older_timepos_index);

  uint32_t extractLong_from_buff(uint8_t spotToStart, uint8_t *buff);

  void fill_positions_to_send_buffer(void);
  /* ==================================================================== */
  /* ======================= public functions =========================== */
  /* ==================================================================== */

  /* Function prototypes for public (external) functions go here */

#endif
#ifdef __cplusplus
}
#endif
