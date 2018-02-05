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
 * Board MCU pins definitions
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

#define RADIO_ANT_SWITCH_TX                         PA_2
#define RADIO_ANT_SWITCH_RX                         PA_4

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

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            NC
#define RADIO_DBG_PIN_RX                            NC

#endif // __BOARD_CONFIG_H__
