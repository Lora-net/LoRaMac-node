/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Implements the generic UART driver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __UART_H__
#define __UART_H__

#include "fifo.h"

typedef enum
{
    UART_NOTIFY_TX,
    UART_NOTIFY_RX
}UartNotifyId_t;

/*!
 * UART object type definition
 */
typedef struct
{
    USART_TypeDef* Uart;
    Gpio_t Tx;
    Gpio_t Rx;
    tFifo FifoTx;
    tFifo FifoRx;
    /*!
     * IRQ user notification callback prototype.
     */
    void ( *IrqNotify )( UartNotifyId_t id );
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

/*!
 * \brief Sends a character to the UART
 *
 * \param [IN] obj   UART object
 * \param [IN] data  Character to be sent
 * \retval status    [0: OK, 1: Busy]
 */
uint8_t UartPutChar( Uart_t *obj, uint8_t data );

/*!
 * \brief Gets a character from the UART
 *
 * \param [IN] obj   UART object
 * \param [IN] data  Received character
 * \retval status    [0: OK, 1: Busy]
 */
uint8_t UartGetChar( Uart_t *obj, uint8_t *data );

/*!
 * \brief Sends a buffer to the UART
 *
 * \param [IN] obj    UART object
 * \param [IN] buffer Buffer to be sent
 * \param [IN] size   Buffer size
 * \retval status     [0: OK, 1: Busy]
 */
uint8_t UartPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size );

/*!
 * \brief Gets a character from the UART
 *
 * \param [IN] obj          UART object
 * \param [IN] buffer       Buffer to be sent
 * \param [IN] size         Buffer size
 * \param [OUT] nbReadBytes Number of bytes really read
 * \retval status           [0: OK, 1: Busy]
 */
uint8_t UartGetBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size, uint16_t *nbReadBytes );

#endif  // __UART_H__
