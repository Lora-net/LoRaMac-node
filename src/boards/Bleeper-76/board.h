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
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "mpl3115.h"
#include "rtc-board.h"
#include "timer-board.h"
#include "sx1276-board.h"
#include "selector.h"

/*!
 * NULL definition
 */
#ifndef NULL
    #define NULL                                    ( ( void * )0 )
#endif

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
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80064 )

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED                                   ( ( *( uint32_t* )ID1 ) ^ \
                                                      ( *( uint32_t* )ID2 ) ^ \
                                                      ( *( uint32_t* )ID3 ) )

/*!
 * Board MCU pins definitions
 */
#define LED_1                                       PB_9
#define LED_2                                       PB_8
#define LED_3                                       PB_5

#define RADIO_RESET                                 PC_2

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5
#define RADIO_NSS                                   PA_4

#define RADIO_DIO_0                                 PA_0
#define RADIO_DIO_1                                 PA_1
#define RADIO_DIO_2                                 PA_2
#define RADIO_DIO_3                                 PA_3
#define RADIO_DIO_4                                 PC_4
#define RADIO_DIO_5                                 PC_5

#define RADIO_ANT_SWITCH_LF                         PC_0
#define RADIO_ANT_SWITCH_HF                         PC_1

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define NC_1                                        PC_13
#define NC_2                                        PC_3
#define NC_3                                        PB_1
#define NC_4                                        PA_10

#define J1_1                                        PB_12
#define J1_2                                        PB_15
#define J1_3                                        PB_14
#define J1_4                                        PB_13

#define J2_2                                        PB_7
#define J2_3                                        PB_6

#define SEL_1                                       PA_8
#define SEL_2                                       PA_9
#define SEL_3                                       PC_6
#define SEL_4                                       PC_7

#define USB_DM                                      PA_11
#define USB_DP                                      PA_12

#define BOOT_1                                      PB_2

#define JTAG_TMS                                    PA_13
#define JTAG_TCK                                    PA_14
#define JTAG_TDI                                    PA_15
#define JTAG_TDO                                    PB_3
#define JTAG_NRST                                   PB_4

#define NC_7                                        PB_0
#define NC_8                                        PC_8
#define NC_9                                        PC_9
#define NC_10                                       PC_10
#define NC_11                                       PC_11
#define NC_12                                       PC_12
#define NC_13                                       PD_2

#define I2C_SCL                                     PB_10
#define I2C_SDA                                     PB_11

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;
extern Gpio_t Led3;

/*!
 * Hex coder selector GPIO pins objects
 */
extern Gpio_t Sel1;
extern Gpio_t Sel2;
extern Gpio_t Sel3;
extern Gpio_t Sel4;

/*!
 * Debug GPIO pins objects
 */
#if defined( USE_DEBUG_PINS )
extern Gpio_t DbgPin1;
extern Gpio_t DbgPin2;
extern Gpio_t DbgPin3;
extern Gpio_t DbgPin4;
#endif

/*!
 * MCU objects
 */
extern I2c_t I2c;

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
 * \brief Measure the Battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardMeasureBatterieLevel( void );

/*!
 * \brief Gets the board 64 bits unique ID 
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

#endif // __BOARD_H__
