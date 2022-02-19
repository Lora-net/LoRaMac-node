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

#define USE_NVM_STORED_LORAWAN_REGION 1 /* Use LoRaWAN region stored in EEPROm. Allowed values: 0 disabled , 1(default) enabled. If not using EEPROM location, \
                                         * use EU868                                                                                                           \
                                         */

  /* GPS RELATED DEFINES */
  /* ----------------------------------------------------------------------------------- */

#define GPS_SEARCH_TIMEOUT 180000
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
#define CHANGE_TX_INTERVAL_PORT 20
#define CHANGE_GPS_SEARCH_TIME_PORT 21
#define WIPE_EEPROM_PORT 62

/* PLAYBACK RELATED DEFINES */
/* ----------------------------------------------------------------------------------- */
#define HOW_OFTEN_TO_SAVE_POS_TIM_TO_EEPROM 60U /* Save to eeprom every 60 minutes */
#define DEFAULT_N_POSITIONS_TO_SEND 4U          /* Number of past positions to send in each transmission */
#define MAX_N_POSITIONS_TO_SEND DEFAULT_N_POSITIONS_TO_SEND
#define DEFAULT_N_POSITIONS_OFFSET 0U
#define PLAYBACK_DAYS 30U /* How many days ago to send playback positions(default). Max 45 days*/

/* WATCHDOG RELATED DEFINES */
/* ----------------------------------------------------------------------------------- */
#define USE_WATCHDOG 1                /* Use watchdog. Allowed values: 0 disabled , 1(default) enabled */
#define WATCH_DOG_KICK_INTERVAL 15000 /* How often to wakeup the MCU from deepsleep to kick the watchdog */

/* TTN, Helium frame count settings. Set the frame count to start from for either network. */
#define HELIUM_FRAME_COUNT_START 130
#define TTN_FRAME_COUNT_START 0

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

/**
 * @brief Choose weather to disable serial output to reduce peak currents
 * 
 */
#define DISABLE_SERIAL_OUTPUT true

/**
 * @brief Number of past positions saved in EEPROM to printout for debugging
 * 
 */
#define N_POSITIONS_TO_PRINTOUT 30

/**
 * @brief Number of transmissions to do per network
 * 
 */
#define N_TRANMISSIONS_PER_NETWORK 2

/**
 * @brief Define how quickly to return to searching for a GPS fix after transmitting
 * 
 */
#define TX_INTERVAL_GPS_FIX_OK 2500 /* When fix was aquired, then sleep for this period (in milliseconds) before searching again */

  /**
   * @brief Lorawan defaults, normally will not be changed 
   * 
   */

  /*!
 * LoRaWAN default end-device class
 */
#define LORAWAN_DEFAULT_CLASS CLASS_A

#define ACTIVE_REGION LORAMAC_REGION_EU868

/*!
 * LoRaWAN application port
 * @remark The allowed port range is from 1 up to 223. Other values are reserved.
 */
#define LORAWAN_APP_PORT 99

/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND 1000

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE LORAMAC_HANDLER_ADR_OFF

/*!
 * Default datarate
 *
 * \remark Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
 */
#define LORAWAN_DEFAULT_DATARATE DR_4

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE LORAMAC_HANDLER_UNCONFIRMED_MSG

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON true

#ifdef __cplusplus
}
#endif

#endif /* CONFIG */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
