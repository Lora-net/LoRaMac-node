/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : config.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CONFIG
#define CONFIG

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "stdint.h"
#include "LoRaMacTypes.h"

  /* Exported functions prototypes ---------------------------------------------*/
  void Error_Handler(void);

// PRELAUNCH IMPORTANT!
// comment out these defines to disable sensor, Radio, GPS or LED
#define SENSOR_ENABLED 0 /* Enable ms5607 sensor. Init the sensor as well. Allowed values: 0 disabled , 1(default) enabled */
#define GPS_ENABLED 1    /* Enable Ublox GPS. Init the GPS as well. Allowed values: 0 disabled , 1(default) enabled */
#define USE_LED 1        /* Enable LED blinky. Allowed values: 0 disabled , 1(default) enabled */

  extern bool USE_NVM_STORED_LORAWAN_REGION; /* Use LoRaWAN region stored in EEPROm. Allowed values: 0 disabled , 1(default) enabled. If not using EEPROM location, \
                                         * use EU868                                                                                                           \
                                         */

  extern uint32_t APP_TX_DUTYCYCLE; /* Set the interval between each transmission(in milliseconds) */

  /* GPS RELATED DEFINES */
  /* ----------------------------------------------------------------------------------- */

#define GPS_LOCATION_FIX_TIMEOUT 180000
#define GPS_WAKEUP_TIMEOUT 1000

/* GEOFENCE RELATED DEFINES */
/* ----------------------------------------------------------------------------------- */
/* For geofence testing */
//#define DUMMY_GPS_COORDS        /* Bypass the GPS chip to return dummy GPS values . Allowed values: 0(default) disabled , 1 enabled */

/* LORAWAN RELATED DEFINES */
/* ----------------------------------------------------------------------------------- */
/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE 242
#define DOWNLINK_CONFIG_PORT 18
#define CHANGE_KEYS_PORT 19

/* PLAYBACK RELATED DEFINES */
/* ----------------------------------------------------------------------------------- */
#define HOW_OFTEN_TO_SAVE_POS_TIM_TO_EEPROM 60U /* Save to eeprom every 60 minutes */
#define DEFAULT_N_POSITIONS_TO_SEND 13U         /* Number of past positions to send in each transmission */
#define MAX_N_POSITIONS_TO_SEND DEFAULT_N_POSITIONS_TO_SEND
#define DEFAULT_N_POSITIONS_OFFSET 0U
#define PLAYBACK_DAYS 30U /* How many days ago to send playback positions(default). Max 45 days*/

/* WATCHDOG RELATED DEFINES */
/* ----------------------------------------------------------------------------------- */
#define USE_WATCHDOG 1                /* Use watchdog. Allowed values: 0 disabled , 1(default) enabled */
#define WATCH_DOG_KICK_INTERVAL 15000 /* How often to wakeup the MCU from deepsleep to kick the watchdog */
/**
 * @brief Choose whether to printout debug info for EEPROM
 * 
 */
#define PRINT_EEPROM_DEBUG 0

/**
 * @brief Choose whether to go into Deep sleep or not
 * 
 */
#define DEEP_SLEEP_ENABLE true

/*!
 * LoRaWAN default end-device class
 */
#define LORAWAN_DEFAULT_CLASS CLASS_A

/**
 * @brief Number of past positions saved in EEPROM to printout for debugging
 * 
 */
#define N_POSITIONS_TO_PRINTOUT 30

/**
 * @brief Select number of transmission to do without a new fix on a single network
 * 
 */
#define NO_FIX_N_TRANSMISSIONS 1
#define YES_FIX_N_TRANSMISSIONS 2

#ifdef __cplusplus
}
#endif

#endif /* CONFIG */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
