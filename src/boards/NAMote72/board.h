/*
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
#include "sx1272/sx1272.h"
#include "mma8451.h"
#include "mpl3115.h"
#include "sx9500.h"
#include "gps.h"
#include "gps-board.h"
#include "rtc-board.h"
#include "sx1272-board.h"
#include "uart-board.h"

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
 * Board MCU pins definitions
 */
#define RADIO_RESET                                 PC_2

#define RADIO_MOSI                                  PB_15
#define RADIO_MISO                                  PB_14
#define RADIO_SCLK                                  PB_13
#define RADIO_NSS                                   PB_12

#define RADIO_DIO_0                                 PC_6
#define RADIO_DIO_1                                 PC_10
#define RADIO_DIO_2                                 PC_11
#define RADIO_DIO_3                                 PC_8
#define RADIO_DIO_4                                 PC_9
#define RADIO_DIO_5                                 PC_12

#define RADIO_SWITCH_CTRL1                          PC_4
#define RADIO_SWITCH_CTRL2                          PC_13
#define RADIO_PWRAMP_CTRL                           PD_2

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define USB_DM                                      PA_11
#define USB_DP                                      PA_12

#define JTAG_TMS                                    PA_13
#define JTAG_TCK                                    PA_14
#define JTAG_TDI                                    PA_15
#define JTAG_TDO                                    PB_3
#define JTAG_NRST                                   PB_4

#define I2C_SCL                                     PB_8
#define I2C_SDA                                     PB_9

#define BAT_LEVEL_PIN_PA0                           PA_0
#define BAT_LEVEL_CHANNEL_PA0                       ADC_CHANNEL_0

#define BAT_LEVEL_PIN_PA1                           PA_1
#define BAT_LEVEL_CHANNEL_PA1                       ADC_CHANNEL_1

#define BOOT_1                                      PB_2

#define GPS_POWER_ON                                PB_11
#define GPS_PPS                                     PC_5
#define GPS_UART_TX                                 PB_6
#define GPS_UART_RX                                 PB_7

#define UART_TX                                     PA_2
#define UART_RX                                     PA_3

#define TX_EN_SX9500                                PA_9
#define N_IRQ_SX9500                                PA_10

#define IRQ_1_MMA8451                               PB_4
#define IRQ_2_MMA8451                               PB_4

#define IRQ_MPL3115                                 PB_4

#define LED_RED                                     PB_1  // Active low
#define LED_YELLOW                                  PB_10 // Active low
#define LED_GREEN                                   PC_3  // Active low
#define LED_USR                                     PA_5  // Active high

#define BOARD_VERSION_PC1                           PC_1
#define BOARD_VERSION_PC7                           PC_7

/*!
 * Low battery threshold definition.
 */
#define LOW_BAT_THRESHOLD                           3450 // mV

/*!
 * LED GPIO pins objects
 */
extern Gpio_t LedRed;
extern Gpio_t LedYellow;
extern Gpio_t LedGreen;
extern Gpio_t LedUsr;
#define Led1                                        LedRed
#define Led2                                        LedYellow
#define Led3                                        LedUsr

/*!
 * PushButton GPIO pin object
 */
extern Gpio_t PushButton;

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
 * Board versions
 */
typedef enum
{
    BOARD_VERSION_NONE = 0,
    BOARD_VERSION_2,
    BOARD_VERSION_3,
}BoardVersion_t;

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

/*!
 * \brief Get the board version
 *
 * \retval value  Version [0: MOTE_VERSION_NONE,
 *                         1: MOTE_VERSION_2,
 *                         2: MOTE_VERSION_3]
 */
BoardVersion_t BoardGetVersion( void );

/*!
 * \brief Generates Lower 32 bits of DEVEUI using 96 bits unique device ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetDevEUI( uint8_t *id );

#endif // __BOARD_H__
