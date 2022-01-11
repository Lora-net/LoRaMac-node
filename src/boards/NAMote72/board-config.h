/*!
 * \file  board-config.h
 *
 * \brief Board configuration
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright Stackforce 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
