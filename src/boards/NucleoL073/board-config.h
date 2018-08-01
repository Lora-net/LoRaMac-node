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
#if defined( SX1262DVK1DAS )
#define BOARD_TCXO_WAKEUP_TIME                      5
#else
#define BOARD_TCXO_WAKEUP_TIME                      0
#endif

/*!
 * Board MCU pins definitions
 */
#define RADIO_RESET                                 PA_0

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5

#if defined( SX1261DVK1BAS ) || defined( SX1262DVK1CAS ) || defined( SX1262DVK1DAS )

#define RADIO_NSS                                   PA_8
#define RADIO_BUSY                                  PB_3
#define RADIO_DIO_1                                 PB_4

#define RADIO_ANT_SWITCH_POWER                      PA_9
#define RADIO_FREQ_SEL                              PA_1
#define RADIO_XTAL_SEL                              PB_0
#define RADIO_DEVICE_SEL                            PA_4

#define LED_1                                       PC_1
#define LED_2                                       PC_0

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            PB_6
#define RADIO_DBG_PIN_RX                            PC_7

#elif defined( SX1272MB2DAS) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )

#define RADIO_NSS                                   PB_6

#define RADIO_DIO_0                                 PA_10
#define RADIO_DIO_1                                 PB_3
#define RADIO_DIO_2                                 PB_5
#define RADIO_DIO_3                                 PB_4
#define RADIO_DIO_4                                 PA_9
#define RADIO_DIO_5                                 PC_7

#define RADIO_ANT_SWITCH                            PC_1

#define LED_1                                       NC
#define LED_2                                       NC

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            PB_0
#define RADIO_DBG_PIN_RX                            PA_4

#endif

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

#endif // __BOARD_CONFIG_H__
