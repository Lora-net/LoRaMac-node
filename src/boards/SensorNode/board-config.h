/*!
 * \file      board-config.h
 *
 * \brief     Board configuration
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#define BOARD_TCXO_WAKEUP_TIME                      0

/*!
 * Define indicating if an external IO expander is to be used
 */
#define BOARD_IOE_EXT

/*!
 * Board IO Extender pins definitions
 */
#define IRQ_MPL3115                                 IOE_0
#define IRQ_MAG3110                                 IOE_1
#define GPS_POWER_ON                                IOE_2
#define RADIO_PUSH_BUTTON                           IOE_3
#define BOARD_POWER_DOWN                            IOE_4
#define SPARE_IO_EXT_5                              IOE_5
#define SPARE_IO_EXT_6                              IOE_6
#define SPARE_IO_EXT_7                              IOE_7
#define N_IRQ_SX9500                                IOE_8
#define IRQ_1_MMA8451                               IOE_9
#define IRQ_2_MMA8451                               IOE_10
#define TX_EN_SX9500                                IOE_11
#define LED_1                                       IOE_12
#define LED_2                                       IOE_13
#define LED_3                                       IOE_14
#define LED_4                                       IOE_15

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 PB_10

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5
#define RADIO_NSS                                   PA_4

#define RADIO_DIO_0                                 PB_11
#define RADIO_DIO_1                                 PC_13
#define RADIO_DIO_2                                 PB_9
#define RADIO_DIO_3                                 PB_4
#define RADIO_DIO_4                                 PB_3
#define RADIO_DIO_5                                 PA_15

#define RADIO_ANT_SWITCH_HF                         PA_0
#define RADIO_ANT_SWITCH_LF                         PA_1

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define USB_DM                                      PA_11
#define USB_DP                                      PA_12

#define I2C_SCL                                     PB_6
#define I2C_SDA                                     PB_7

#define BOOT_1                                      PB_2

#define GPS_PPS                                     PB_1
#define UART_TX                                     PA_9
#define UART_RX                                     PA_10

#define DC_DC_EN                                    PB_8
#define BAT_LEVEL_PIN                               PB_0
#define BAT_LEVEL_CHANNEL                           ADC_CHANNEL_8

#define WKUP1                                       PA_8
#define USB_ON                                      PA_2

#define RF_RXTX                                     PA_3

#define SWDIO                                       PA_13
#define SWCLK                                       PA_14

#define TEST_POINT1                                 PB_12
#define TEST_POINT2                                 PB_13
#define TEST_POINT3                                 PB_14
#define TEST_POINT4                                 PB_15

#define PIN_NC                                      PB_5

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            NC
#define RADIO_DBG_PIN_RX                            NC

#endif // __BOARD_CONFIG_H__
