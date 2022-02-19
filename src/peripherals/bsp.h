/**
  ******************************************************************************
  * @file    bsp.h
  * @author  MCD Application Team
  * @brief   contains all hardware driver
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

#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "config.h"
#include "LoRaWAN_config_switcher.h"

  typedef struct
  {
    uint8_t status_bitfields; /* flexible bitfields for sending status. */
    int8_t temperature;       /* in °C   */
    uint8_t no_load_solar_voltage;
    uint8_t load_solar_voltage;
    uint8_t sats;         /* satellites in fix */
    uint16_t reset_count; /* Count number of resets */
    uint8_t data_received;
    uint8_t days_of_playback; /* How many days of playback we have */
  } sensor_t;

  typedef struct
  {
    uint32_t minutes_since_epoch; // minutes since Epoch
    uint16_t latitude_encoded;    // Latitude * 65536
    uint16_t longitude_encoded;   // Longitude * 65536
    uint16_t altitude_encoded;    // Altitude in meters / 256

  } time_pos_fix_t;

  typedef struct
  {
    uint16_t current_EEPROM_index;
    uint16_t n_playback_positions_saved;
    uint32_t Crc32;
  } eeprom_playback_stats_t;

  typedef struct
  {
    uint32_t tx_interval;
    uint32_t Crc32;
  } tx_interval_eeprom_t;

#define RESITOR_DIVIDER_PROD ((100000.0 + 100000.0) / 100000.0)
#define RESITOR_DIVIDER (float)(RESITOR_DIVIDER_PROD)

// EEPROM related defines
#define NVM_PlAYBACK_EEPROM_ADDR_START (SIZE_OF_NETWORK_KEYS_T * NUMBER_OF_REGISTERED_DEVICES + 5) /* Amount of space allocated for saving keys. Add 5 bytes leeway */

#define TX_INTERVAL_EEPROM_ADDRESS (NVM_PlAYBACK_EEPROM_ADDR_START)
#define TX_INTERVAL_EEPROM_LEN (4 + 4)

#define LORAMAC_REGION_EEPROM_ADDR (TX_INTERVAL_EEPROM_ADDRESS + TX_INTERVAL_EEPROM_LEN)
#define LORAMAC_REGION_EEPROM_LEN (4)

#define RESET_COUNTER_ADDR (LORAMAC_REGION_EEPROM_ADDR + LORAMAC_REGION_EEPROM_LEN)
#define RESET_COUNTER_LEN (2)

#define CURRENT_PLAYBACK_INDEX_IN_EEPROM_ADDR (RESET_COUNTER_ADDR + RESET_COUNTER_LEN)
#define CURRENT_PLAYBACK_INDEX_IN_EEPROM_LEN (2 + 2 + 4)

#define PLAYBACK_EEPROM_ADDR_START (CURRENT_PLAYBACK_INDEX_IN_EEPROM_ADDR + CURRENT_PLAYBACK_INDEX_IN_EEPROM_LEN)
#define PLAYBACK_EEPROM_PACKET_SIZE (9)
#define MAX_PLAYBACK_POSITIONS_SAVED_IN_EEPROM (450)
#define PLAYBACK_EEPROM_SIZE (MAX_PLAYBACK_POSITIONS_SAVED_IN_EEPROM * PLAYBACK_EEPROM_PACKET_SIZE) // ensure this is at least one position fix less than full. Because it does this wierd overflow

#define EEPROM_SIZE 0x17FFUL /* 6143 bytes */

  /**
 * @brief  initialises the sensor
 *
 * @note
 * @retval None
 */
  void BSP_sensor_Init(void);

  /**
 * @brief  sensor  read. 
 *
 * @note none
 * @retval sensor_data
 */
  void BSP_sensor_Read(void);

  uint16_t get_time_pos_index_older_than(uint32_t minutes_from_epoch);
  bool manage_incoming_instruction(uint8_t *instructions);

  void retrieve_eeprom_stored_lorawan_region(void);
  void set_eeprom_stored_lorwan_region(void);

  typedef enum
  {
    PLAYBACK_NAK,
    EEPROM_CHANGED_BITS,
    PLAYBACK_ACK,
    TX_INTERVAL_CHANGED,
    TX_INTERVAL_NOT_CHANGED,
    EEPROM_WIPED,
  } bit_location_t;

  void set_bits(bit_location_t bit_location);
  void clear_bits(void);
  sensor_t get_current_sensor_data(void);
  bool update_device_tx_interval_in_eeprom(uint32_t interval_ms);
  uint32_t read_tx_interval_in_eeprom(void);
  void read_playback_stats_from_eeprom(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */
