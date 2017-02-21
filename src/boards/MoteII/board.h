/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
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
#include <stdint.h>
#include "stm32l0xx.h"
#include "stm32l0xx_hal.h"
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "gpio-ioe.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "radio.h"
#include "sx1272/sx1272.h"
#include "mma8451.h"
#include "mpl3115.h"
#include "sx9500.h"
#include "gps.h"
#include "gps-board.h"
#include "pam7q.h"
#include "rtc-board.h"
#include "sx1272-board.h"
#include "uart-board.h"
#include "display-board.h"

/*!
 * Define indicating if an external IO expander is to be used
 */
#define BOARD_IOE_EXT

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0
#endif

/*!
 * Board IO Extender pins definitions
 */
#define BLE_RST                                     IOE_0
#define BLE_DFU                                     IOE_1
#define BLE_IRQ                                     IOE_2
#define BLE_CS                                      IOE_3
#define SPARE_IO_EXT_4                              IOE_4
#define SPARE_IO_EXT_5                              IOE_5
#define SPARE_IO_EXT_6                              IOE_6
#define GPS_PPS                                     IOE_7
#define SPARE_IO_EXT_8                              IOE_8
#define OLED_EN                                     IOE_9
#define BLE_EN                                      IOE_10
#define OLED_RST                                    IOE_11
#define IRQ_1_MPL3115                               IOE_12
#define IRQ_2_MPL3115                               IOE_13
#define IRQ_1_MMA8451                               IOE_14
#define IRQ_2_MMA8451                               IOE_15

/*!
 * Board MCU pins definition
 */

#define RADIO_RESET                                 PA_5

#define RADIO_MOSI                                  PB_15
#define RADIO_MISO                                  PB_14
#define RADIO_SCLK                                  PB_13
#define RADIO_NSS                                   PB_12

#define RADIO_DIO_0                                 PA_6
#define RADIO_DIO_1                                 PA_7
#define RADIO_DIO_2                                 PB_0
#define RADIO_DIO_3                                 PB_2
#define RADIO_DIO_4                                 PB_1
#define RADIO_DIO_5                                 PB_10

#define RADIO_ANT_SWITCH_RX                         PA_2
#define RADIO_ANT_SWITCH_TX                         PA_4

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define SWCLK                                       PA_14
#define SWDAT                                       PA_13

#define I2C_SCL                                     PB_6
#define I2C_SDA                                     PB_7

#define UART_TX                                     PA_9
#define UART_RX                                     PA_10

#define BAT_LEVEL_PIN                               PA_0
#define BAT_LEVEL_CHANNEL                           ADC_CHANNEL_0

#define LED_1                                       PA_8
#define LED_2                                       PA_11
#define LED_3                                       PB_9

#define NSWITCH_1                                   PB_11
#define NSWITCH_2                                   PA_12
#define NSWITCH_3                                   PA_1

#define BLE_OLED_MISO                               PB_4
#define BLE_OLED_MOSI                               PB_5
#define BLE_OLED_SCK                                PB_3

#define OLED_NSS                                    PA_15
#define OLED_DC                                     PA_3

#define IO_EXP_NINT                                 PC_13

#define GPS_EN                                      PB_8


/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;
extern Gpio_t Led3;

/*!
 * MCU objects
 */
extern I2c_t I2c;
extern Uart_t Uart1;

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
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Battery voltage
 *
 * \retval value  battery voltage in volts
 */
uint32_t BoardGetBatteryVoltage( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level [  0: USB,
 *                                 1: Min level,
 *                                 x: level
 *                               254: fully charged,
 *                               255: Error]
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
 * \retval value  power source [0: USB_POWER, 1: BATTERY_POWER]
 */
uint8_t GetBoardPowerSource( void );

#endif // __BOARD_H__
