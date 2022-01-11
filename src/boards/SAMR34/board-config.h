/*!
 * \file  board-config.h
 *
 * \brief Board configuration
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright Stackforce 2021. All rights reserved.
 * Copyright TWTG on behalf of Microchip/Atmel (c)2021. All rights reserved.
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

#include <hal_gpio.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#define BOARD_TCXO_WAKEUP_TIME                      1

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 GPIO( GPIO_PORTB, 15 )
#define RADIO_TCXO_PWR_PIN                          GPIO( GPIO_PORTA, 9 )
#define RF_SWITCH_PIN                               GPIO( GPIO_PORTA, 13 )

#define RADIO_MOSI                                  GPIO( GPIO_PORTB, 30 )
#define RADIO_MISO                                  GPIO( GPIO_PORTC, 19 )
#define RADIO_SCLK                                  GPIO( GPIO_PORTC, 18 )
#define RADIO_NSS                                   GPIO( GPIO_PORTB, 31 )

#define RADIO_DIO_0                                 GPIO( GPIO_PORTB, 16 )
#define RADIO_DIO_1                                 GPIO( GPIO_PORTA, 11 )
#define RADIO_DIO_2                                 GPIO( GPIO_PORTA, 12 )
#define RADIO_DIO_3                                 GPIO( GPIO_PORTB, 17 )

#define LED_1                                       GPIO( GPIO_PORTA, 19 )

#define UART_TX                                     GPIO( GPIO_PORTA, 4 )
#define UART_RX                                     GPIO( GPIO_PORTA, 5 )
#define I2C_SDA                                     GPIO( GPIO_PORTA, 16 )
#define I2C_SCL                                     GPIO( GPIO_PORTA, 17 )

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            NC
#define RADIO_DBG_PIN_RX                            NC

#ifdef __cplusplus
}
#endif

#endif // __BOARD_CONFIG_H__
