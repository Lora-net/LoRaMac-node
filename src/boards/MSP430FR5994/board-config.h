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

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#if defined( SX1262MBXDAS )
#define BOARD_TCXO_WAKEUP_TIME                      5
#else
#define BOARD_TCXO_WAKEUP_TIME                      0
#endif

/*!
 * Board MCU pins definitions
   PA_x stands for Port 1, Pin x with 
   PA = Port 1
   PB = Port 2
   C = 3
   D = 4
   E = 5
   F = 6
   G = 7
   H = 8
 */
#define RADIO_RESET                                 PD_4 //4.4

#define RADIO_MOSI                                  PE_0 //5.0
#define RADIO_MISO                                  PE_1 //5.1
#define RADIO_SCLK                                  PE_2 //5.2

#define RADIO_NSS                                   PE_3 //5.3

#define RADIO_DIO_0                                 PC_0 //P3_0
#define RADIO_DIO_1                                 PC_1 //P3_1
#define RADIO_DIO_2                                 PC_2 //P3_2
#define RADIO_DIO_3                                 PC_3 //P3_3

#define RADIO_ANT_SWITCH                            PD_7 //P4_7

#define LED_1                                       PA_0 //P1_0
#define LED_2                                       PA_1 //P1_1

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            PA_4 //P1_4
#define RADIO_DBG_PIN_RX                            PA_5 //P1_5

#define I2C_SCL                                     PG_1 //P7_1
#define I2C_SDA                                     PG_0 //P7_0

#define UART_TX                                     PB_0 //P2_0
#define UART_RX                                     PB_1 //P2_1

#ifdef __cplusplus
}
#endif

#endif // __BOARD_CONFIG_H__
