/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Implements the generic UART driver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __UART_H__
#define __UART_H__

/*!
 * UART object type definition
 */
typedef struct
{
    USART_TypeDef* Uart;
    Gpio_t Tx;
    Gpio_t Rx;
}Uart_t;

/*!
 * \brief Initializes the UART object and MCU peripheral
 *
 * \param [IN] obj  UART object
 * \param [IN] scl  UART Tx pin name to be used
 * \param [IN] sda  UART Rx pin name to be used
 */
void UartInit( Uart_t *obj, PinNames tx, PinNames rx );

/*!
 * \brief DeInitializes the UART object and MCU pin
 *
 * \param [IN] obj  UART object
 */
void UartDeInit( Uart_t *obj );

#endif  // __UART_H__
