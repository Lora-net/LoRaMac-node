/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: contains all hardware driver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "config.h"


/* Exported types ------------------------------------------------------------*/

typedef struct{
  uint16_t pressure;    /* in mbar */  
  int8_t temperature; /* in °C   */
  int8_t humidity;    /* in %    */
  uint16_t latitude;
  uint16_t longitude ;
  uint16_t altitudeGps;       /* in m */
  uint8_t no_load_solar_voltage;
  uint8_t load_solar_voltage;
  uint8_t sats;                 /* satellites in fix */
  uint16_t reset_count;          /* Count number of resets */
  uint8_t data_received;
  uint8_t days_of_playback;     /* How many days of playback we have */
} sensor_t;

typedef struct
{
  uint32_t minutes_since_epoch; // minutes since Epoch
  uint16_t latitude;        // Latitude
  uint16_t longitude;       // Longitude
  uint16_t altitude;     // Altitude

}time_pos_fix_t;



#define RESITOR_DIVIDER_PROD                    ((100000.0+100000.0)/100000.0)
#define RESITOR_DIVIDER                         (float)(RESITOR_DIVIDER_PROD)



// EEPROM related defines

#define NVM_PlAYBACK_EEPROM_ADDR_START             1500 /* Amount of space allocated for loramac NVM storage */

#define FRAME_COUNTER_EEPROM_ADDRESS               (0 + NVM_PlAYBACK_EEPROM_ADDR_START)
#define FRAME_COUNTER_EEPROM_LEN                   (4)

#define LORAMAC_REGION_EEPROM_ADDR                 (4 + NVM_PlAYBACK_EEPROM_ADDR_START)
#define LORAMAC_REGION_EEPROM_LEN                  (4)


#define RESET_COUNTER_ADDR                         (8 + NVM_PlAYBACK_EEPROM_ADDR_START)
#define RESET_COUNTER_LEN                          (2)


#define CURRENT_PLAYBACK_INDEX_IN_EEPROM_ADDR      (10 + NVM_PlAYBACK_EEPROM_ADDR_START)
#define CURRENT_PLAYBACK_INDEX_IN_EEPROM_LEN       (2)

#define N_PLAYBACK_POSITIONS_SAVED_IN_EEPROM_ADDR  (12 + NVM_PlAYBACK_EEPROM_ADDR_START)
#define N_PLAYBACK_POSITIONS_SAVED_IN_EEPROM_LEN   (2)

#define PLAYBACK_EEPROM_ADDR_START                 (14 + NVM_PlAYBACK_EEPROM_ADDR_START)
#define PLAYBACK_EEPROM_PACKET_SIZE                (9)
#define MAX_PLAYBACK_POSITIONS_SAVED_IN_EEPROM     (400)
#define PLAYBACK_EEPROM_SIZE                       (MAX_PLAYBACK_POSITIONS_SAVED_IN_EEPROM * PLAYBACK_EEPROM_PACKET_SIZE)

/* WARNING! Ensure this value is less than DATA_EEPROM_BANK2_END. Or else, it will overflow EEPROM */
#define EEPROM_ADDR_END                            (NVM_PlAYBACK_EEPROM_ADDR_START + PLAYBACK_EEPROM_SIZE)

#define EEPROM_SIZE 0x17FFUL /* 6143 bytes */
#if ( EEPROM_ADDR_END > EEPROM_SIZE )
#error Space allocated to playback exceeds available EEPROM
#endif
																												


/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
/**
 * @brief  initialises the sensor
 *
 * @note
 * @retval None
 */
void  BSP_sensor_Init( void  );

/**
 * @brief  sensor  read. 
 *
 * @note none
 * @retval sensor_data
 */
void BSP_sensor_Read( void );






uint16_t get_time_pos_index_older_than(uint32_t minutes_from_epoch);
void manage_incoming_instruction(uint8_t *instructions);
  void print_buffer(uint8_t *buff, uint16_t size);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
