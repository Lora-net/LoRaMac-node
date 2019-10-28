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
#define BOARD_TCXO_WAKEUP_TIME                      0

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

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            PA_6
#define RADIO_DBG_PIN_RX                            PA_7

/*!
 * Low battery threshold definition.
 */
#define LOW_BAT_THRESHOLD                           3450 // mV

/*!
 * LED GPIO pins objects
 */
#define Led1                                        LedRed
#define Led2                                        LedYellow
#define Led3                                        LedUsr

#ifdef __cplusplus
}
#endif

#endif // __BOARD_CONFIG_H__
