/*
 / _____)			 _				| |
( (____	_____ ____ _| |_ _____	____| |__
 \____ \| ___ |	(_	_) ___ |/ ___)	_ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
	(C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <stdint.h>

#include "stm32l1xx.h"
#include "stm32l1xx_hal.h"

#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "mma8451.h"
#include "mpl3115.h"
#include "gps.h"
#include "gps-board.h"
#include "rtc-board.h"
#include "sx1276-board.h"
#include "uart-board.h"
#include "uart-board.h"
#include "eeprom-board.h"

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS		1
#endif

#ifndef FAIL
#define FAIL		0
#endif

/*!
 * Board MCU pins definitions
 */
#define RADIO_RESET				PB_13

#define RADIO_MOSI				PA_7
#define RADIO_MISO				PA_6
#define RADIO_SCLK				PA_5
#define RADIO_NSS				PB_0

#define RADIO_DIO_0				PA_11
#define RADIO_DIO_1				PB_1
#define RADIO_DIO_2				PA_3
#define RADIO_DIO_3				PH_0
#define RADIO_DIO_4				PC_13

#define RADIO_ANT_SWITCH_TX_PA	PA_4
#define RADIO_ANT_SWITCH_RX		PB_6
#define RADIO_TCXO_EN			PH_1
#define RADIO_RF_HF				PB_7

#define OSC_LSE_IN				PC_14
#define OSC_LSE_OUT				PC_15

#define SWDIO					PA_13
#define SWCLK					PA_14

#ifdef BOARD_RAK811_TRACKER

	#define I2C_SCL				P?_?
	#define I2C_SDA				P?_?

	#define BAT_LEVEL_PIN		PB_9
	#define BAT_LEVEL_CHANNEL	ADC_CHANNEL_3

	#define IRQ_1_MMA8451		P?_?

	#define IRQ_MPL3115			P?_?
	#define IRQ_MAG3110			P?_?
	#define GPS_POWER_ON		P?_?
	#define LED_1				P?_?

#else
	#define I2C_SCL				NC
	#define I2C_SDA				NC
#endif

#define UART_TX					PA_9
#define UART_RX					PA_10

/*
--------------------------------

--------------------------------
*/
//#define RAK811_PIN1			GND
#define RAK811_PIN2				PB_12
#define RAK811_PIN3				PB_14
#define RAK811_PIN4				PB_15
#define RAK811_PIN5				PA_8
#define RAK811_PIN6				PA_9
#define RAK811_PIN7				PA_10
#define RAK811_PIN8				PA_12
#define RAK811_PIN9				PB_4
#define RAK811_PIN10			PA_13	// SWD
//#define RAK811_PIN11			VCC
//#define RAK811_PIN12			GND
#define RAK811_PIN13			PA_14	// SWC
#define RAK811_PIN14			PA_15
#define RAK811_PIN15			PB_3
#define RAK811_PIN16			PB_5
//#define RAK811_PIN17			BOOT0
#define RAK811_PIN18			PB_8
#define RAK811_PIN19			PB_9
#define RAK811_PIN20			PA_2
//#define RAK811_PIN21			GND
#define RAK811_PIN22			PA_1
#define RAK811_PIN23			PA_0
#define RAK811_PIN25			PB_10
#define RAK811_PIN26			PB_11
#define RAK811_PIN27			PB_2
//#define RAK811_PIN28			GND

/*!
 * Low battery threshold definition.
 */
#define LOW_BAT_THRESHOLD                           3450 // mV


/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;

/*!
 * MCU objects
 */
extern Adc_t Adc;
extern I2c_t I2c;
extern Uart_t Uart1;
extern Uart_t Uart2;

/*!
 * Possible power sources
 */
enum BoardPowerSources
{
	USB_POWER = 0,
	BATTERY_POWER,
};

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
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *		consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Battery voltage
 *
 * \retval value	battery voltage in volts
 */
uint32_t BoardGetBatteryVoltage( void );

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
 * \brief Get the board power source
 *
 * \retval value	power source [0: USB_POWER, 1: BATTERY_POWER]
 */
uint8_t GetBoardPowerSource( void );

#endif // __BOARD_H__
