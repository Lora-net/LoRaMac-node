/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Bleeper board I2C driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __UART_MCU_H__
#define __UART_MCU_H__

/*!
 * Operation Mode for the I2C
 */
typedef enum
{
    TX_ONLY = 0,
    RX_ONLY,
    RX_TX
}UartMode;

/*!
 * Operation Mode for the I2C
 */
typedef enum
{
    UART_8_BIT = 0,
    UART_9_BIT
}WordLength;

/*!
 * I2C signal duty cycle
 */
typedef enum
{
    UART_1_STOP_BIT = 0,
    UART_0_5_STOP_BIT,
    UART_2_STOP_BIT,
    UART_1_5_STOP_BIT
}StopBits;

/*!
 * I2C select if the acknowledge in after the 7th or 10th bit
 */
typedef enum
{
    NO_PARITY = 0,
    EVEN_PARITY,
    ODD_PARITY
}Parity;


typedef enum
{
    NO_FLOW_CTRL = 0,
    RTS_FLOW_CTRL,
    CTS_FLOW_CTRL,
    RTS_CTS_FLOW_CTRL
}FlowCtrl;

/*!
 * \brief Initializes the UART object and MCU peripheral
 *
 * \param [IN] obj  UART object
 * \param [IN] scl  UART Tx pin name to be used
 * \param [IN] sda  UART Rx pin name to be used
 */
void UartMcuInit( Uart_t *obj, PinNames tx, PinNames rx );

/*!
 * \brief Initializes the UART object and MCU peripheral
 *
 * \param [IN] obj          UART object
 * \param [IN] mode         Mode of operation for the UART
 * \param [IN] baudrate     UART baudrate
 * \param [IN] wordLength   packet length
 * \param [IN] stopBits     stop bits setup
 * \param [IN] parity       packet parity
 * \param [IN] flowCtrl     UART flow control
 */
void UartMcuFormat( Uart_t *obj, UartMode mode, uint32_t baudrate, WordLength wordLength, StopBits stopBits, Parity parity, FlowCtrl flowCtrl );

/*!
 * \brief DeInitializes the UART object and MCU peripheral
 *
 * \param [IN] obj  UART object
 */
void UartMcuDeInit( Uart_t *obj );

/*!
 * \brief Initialize the IRQ on the UART
 *
 * \param [IN] obj  UART object
 */
void UartMcuIrqInit( Uart_t *obj );

#endif // __UART_MCU_H__
