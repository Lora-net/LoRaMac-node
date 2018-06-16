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
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 PB_9

#define RADIO_MOSI                                  PB_5
#define RADIO_MISO                                  PB_4
#define RADIO_SCLK                                  PB_3
#define RADIO_NSS                                   PA_15

#define RADIO_DIO_0                                 PC_13
#define RADIO_DIO_1                                 PB_0
#define RADIO_DIO_2                                 PB_1
#define RADIO_DIO_3                                 PB_6
#define RADIO_DIO_4                                 PB_7
#define RADIO_DIO_5                                 PB_8

#define RF_RXTX                                     PA_8

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define LED_1                                       PA_2
#define LED_2                                       PA_3
#define LED_3                                       PA_7
#define LED_4                                       PA_6

#define RADIO_PUSH_BUTTON                           PB_2

#define LIGHT_SENSOR                                PA_4

#define USB_DM                                      PA_11
#define USB_DP                                      PA_12

#define UART_TX                                     PA_9
#define UART_RX                                     PA_10

#define BAT_LEVEL_PIN                               PA_3
#define BAT_LEVEL_CHANNEL                           ADC_CHANNEL_6

#define WKUP1                                       PA_8

#define SWDIO                                       PA_13
#define SWCLK                                       PA_14

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            NC
#define RADIO_DBG_PIN_RX                            NC

#endif // __BOARD_CONFIG_H__
