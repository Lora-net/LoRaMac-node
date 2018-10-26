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
#define BOARD_TCXO_WAKEUP_TIME                      5

/*!
 * Board MCU pins definitions
 */
#define RADIO_RESET                                 PC_0

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PB_3

#define RADIO_NSS                                   PA_15

#define RADIO_DIO_0                                 PB_4
#define RADIO_DIO_1                                 PB_1
#define RADIO_DIO_2                                 PB_0
#define RADIO_DIO_3                                 PC_13
#define RADIO_DIO_4                                 PA_5
#define RADIO_DIO_5                                 PA_4

#define RADIO_TCXO_POWER                            PA_12

#define RADIO_ANT_SWITCH_RX                         PA_1
#define RADIO_ANT_SWITCH_TX_BOOST                   PC_1
#define RADIO_ANT_SWITCH_TX_RFO                     PC_2

#define LED_1                                       PB_5
#define LED_2                                       PA_5
#define LED_3                                       PB_6
#define LED_4                                       PB_7

#define LED_GREEN                                   LED_1
#define LED_RED1                                    LED_2
#define LED_BLUE                                    LED_3
#define LED_RED2                                    LED_4

#define BTN_1                                       PB_2

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define SWCLK                                       PA_14
#define SWDAT                                       PA_13

#define I2C_SCL                                     PB_8
#define I2C_SDA                                     PB_9

#define UART_TX                                     PA_2
#define UART_RX                                     PA_3

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            PB_13
#define RADIO_DBG_PIN_RX                            PB_14

#endif // __BOARD_CONFIG_H__
