#ifndef _BOARD_H
#define _BOARD_H

#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"
#include "nrf_log.h"

#include "utilities.h"
#include "timer.h"
#include "gpio-board.h"
#include "spi-board.h"
#include "rtc-board.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "sx1276-board.h"

/*!
 * GPS function definitions
 */
#define GpsGetLatestGpsAltitude()	0
#define GpsStart( )
#define GpsStop( )
#define GpsGetLatestGpsPositionBinary(lat, lon)

/*!

 */
 
/*!
 * Pin definitions
 */
#define RADIO_DIO_0		P7
#define RADIO_DIO_1		P8
#define RADIO_DIO_2		P9
#define RADIO_DIO_3		P10

#define RADIO_NSS		P14
#define RADIO_MOSI		P13
#define RADIO_MISO		P12
#define RADIO_SCK		P11

#define RADIO_RESET		P6
#define RADIO_TCXO		P5
#define RADIO_RF_CTX	P23
#define RADIO_RF_CPS	P22

#define ASSERT_ERROR	0xA55EA55E

#define USE_FULL_ASSERT
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
	#define assert_param(expr)	((expr) ? (void)0U : app_error_handler(ASSERT_ERROR, __LINE__, (const uint8_t *)__FILE__))
#else
	#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

/*!
 * Possible power sources
 */
enum BoardPowerSources
{
	USB_POWER = 0,
	BATTERY_POWER,
};

/*!
 * \brief Measure the Battery voltage
 *
 * \retval value	battery voltage in volts
 */
uint16_t BoardGetBatteryVoltage( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value	battery level [	0: USB,
 *								 1: Min level,
 *								 x: level
 *								254: fully charged,
 *								255: Error]
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Disable interrupts
 *
 * \remark IRQ nesting is managed
 */
void BoardDisableIrq( void );

/*!
 * \brief Enable interrupts
 *
 * \remark IRQ nesting is managed
 */
void BoardEnableIrq( void );

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Processing board events
 */
void BoardProcess( void );

#endif
