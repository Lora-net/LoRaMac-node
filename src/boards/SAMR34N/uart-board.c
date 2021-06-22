/*!
 * \file      uart-board.c
 *
 * \brief     Target board UART driver implementation
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
 * \endcode
 */
#include "asf.h"

#include "utilities.h"

#include "board.h"
#include "uart-board.h"

/*!
 * Number of times the UartPutBuffer will try to send the buffer before
 * returning ERROR
 */
#define TX_BUFFER_RETRY_COUNT 10

/*!
 * UART FIFO buffer size
 */
#define UART_FIFO_TX_SIZE 1024
#define UART_FIFO_RX_SIZE 1024

static uint8_t uart_tx_buffer[UART_FIFO_TX_SIZE];
static uint8_t uart_rx_buffer[UART_FIFO_RX_SIZE];

static struct usart_module host_uart_module;
static struct usart_config host_uart_config;
static Uart_t*             uart_obj;

void UartTxIrqHandler( struct usart_module* const module );

void UartRxIrqHandler( struct usart_module* const module );

void UartMcuInit( Uart_t* obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    uart_obj         = obj;
    uart_obj->UartId = uartId;

    FifoInit( &uart_obj->FifoTx, uart_tx_buffer, UART_FIFO_TX_SIZE );
    FifoInit( &uart_obj->FifoRx, uart_rx_buffer, UART_FIFO_RX_SIZE );

    usart_get_config_defaults( &host_uart_config );
    host_uart_config.baudrate    = 115200;
    host_uart_config.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
    host_uart_config.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
    host_uart_config.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
    host_uart_config.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
    host_uart_config.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
    stdio_serial_init( &host_uart_module, EDBG_CDC_MODULE, &host_uart_config );
    usart_enable( &host_uart_module );
    // Enable transceivers
    usart_enable_transceiver( &host_uart_module, USART_TRANSCEIVER_TX );
    usart_enable_transceiver( &host_uart_module, USART_TRANSCEIVER_RX );

    usart_register_callback( &host_uart_module, UartTxIrqHandler, USART_CALLBACK_BUFFER_TRANSMITTED );
    usart_register_callback( &host_uart_module, UartRxIrqHandler, USART_CALLBACK_BUFFER_RECEIVED );
    usart_enable_callback( &host_uart_module, USART_CALLBACK_BUFFER_TRANSMITTED );
    usart_enable_callback( &host_uart_module, USART_CALLBACK_BUFFER_RECEIVED );
}

void UartMcuConfig( Uart_t* obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits,
                    Parity_t parity, FlowCtrl_t flowCtrl )
{
    // configured via UartMcuInit
}

void UartMcuDeInit( Uart_t* obj )
{
    usart_disable( &host_uart_module );

    // Disable transceivers
    usart_disable_transceiver( &host_uart_module, USART_TRANSCEIVER_TX );
    usart_disable_transceiver( &host_uart_module, USART_TRANSCEIVER_RX );
}

uint8_t UartMcuPutChar( Uart_t* obj, uint8_t data )
{
    CRITICAL_SECTION_BEGIN( );

    if( IsFifoEmpty( &obj->FifoTx ) == true )
    {
        CRITICAL_SECTION_END( );
        
        //  Write one byte to the transmit data register
        if( usart_write_buffer_job( &host_uart_module, &data, 1 ) == STATUS_OK )
        {
            CRITICAL_SECTION_END( );
            return 0;
        }

        return 0;  // OK
    }
    if( IsFifoFull( &obj->FifoTx ) == false )
    {
        FifoPush( &obj->FifoTx, data );

        CRITICAL_SECTION_END( );
        return 0;  // OK
    }
    CRITICAL_SECTION_END( );
    return 1;  // Busy
}

uint8_t UartMcuGetChar( Uart_t* obj, uint8_t* data )
{
    CRITICAL_SECTION_BEGIN( );

    if( IsFifoEmpty( &obj->FifoRx ) == false )
    {
        *data = FifoPop( &obj->FifoRx );
        CRITICAL_SECTION_END( );
        return 0;
    }
    else
    {
        if( usart_read_buffer_job( &host_uart_module, data, 1 ) == STATUS_OK )
        {
            CRITICAL_SECTION_END( );
            return 0;
        }
    }
    CRITICAL_SECTION_END( );
    return 1;
}

uint8_t UartMcuPutBuffer( Uart_t* obj, uint8_t* buffer, uint16_t size )
{
    uint8_t  retryCount;
    uint16_t i;

    for( i = 0; i < size; i++ )
    {
        retryCount = 0;
        while( UartPutChar( obj, buffer[i] ) != 0 )
        {
            retryCount++;

            // Exit if something goes terribly wrong
            if( retryCount > TX_BUFFER_RETRY_COUNT )
            {
                return 1;  // Error
            }
        }
    }
    return 0;  // OK
}

uint8_t UartMcuGetBuffer( Uart_t* obj, uint8_t* buffer, uint16_t size, uint16_t* nbReadBytes )
{
    uint16_t localSize = 0;

    while( localSize < size )
    {
        if( UartGetChar( obj, buffer + localSize ) == 0 )
        {
            localSize++;
        }
        else
        {
            break;
        }
    }

    *nbReadBytes = localSize;

    if( localSize == 0 )
    {
        return 1;  // Empty
    }
    return 0;  // OK
}

void UartTxIrqHandler( struct usart_module* const module )
{
    uint8_t temp;
    if( IsFifoEmpty( &uart_obj->FifoTx ) == false )
    {
        temp = FifoPop( &uart_obj->FifoTx );
        //  Write one byte to the transmit data register
        usart_write_buffer_job( &host_uart_module, &temp, 1 );
    }

    if( uart_obj->IrqNotify != NULL )
    {
        uart_obj->IrqNotify( UART_NOTIFY_TX );
    }
}

void UartRxIrqHandler( struct usart_module* const module )
{
    uint8_t temp;

    usart_read_buffer_job( &host_uart_module, &temp, 1 );

    // Introducing critical section to avoid buffer corruption.
    CRITICAL_SECTION_BEGIN( );
    if( IsFifoFull( &uart_obj->FifoRx ) == false )
    {
        // Read one byte from the receive data register
        FifoPush( &uart_obj->FifoRx, temp );
    }

    if( uart_obj->IrqNotify != NULL )
    {
        uart_obj->IrqNotify( UART_NOTIFY_RX );
    }
    CRITICAL_SECTION_END( );
}
