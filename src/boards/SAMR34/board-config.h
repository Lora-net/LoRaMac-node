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
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
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
#define TCXO_PWR_PIN                                GPIO( GPIO_PORTA, 9 )
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
