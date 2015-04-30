/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Bleeper board UART driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

#include "uart-board.h"

void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );

    USART_DeInit( USART1 );

    GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );

    GPIO_PinAFConfig( obj->Tx.port, ( obj->Tx.pin & 0x0F ), GPIO_AF_USART1 );
    GPIO_PinAFConfig( obj->Rx.port, ( obj->Rx.pin & 0x0F ), GPIO_AF_USART1 );
}

void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    if( mode == TX_ONLY )
    {
        if( obj->FifoTx.Data == NULL )
        {
            while( 1 );
        }

        USART_InitStructure.USART_Mode = 0x0008;
    }
    else if( mode == RX_ONLY )
    {
        if( obj->FifoRx.Data == NULL )
        {
            while( 1 );
        }

        USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );
        USART_InitStructure.USART_Mode = 0x0004;
    }
    else
    {
        if( ( obj->FifoTx.Data == NULL ) || ( obj->FifoRx.Data == NULL ) )
        {
            while( 1 );
        }

        USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );
        USART_InitStructure.USART_Mode = 0x000C;
    }

    USART_InitStructure.USART_BaudRate = baudrate;

    if( wordLength == UART_8_BIT )
    {
        USART_InitStructure.USART_WordLength = 0x0000;
    }
    else 
    {
        USART_InitStructure.USART_WordLength = 0x1000;
    }

    if( stopBits == UART_1_STOP_BIT )
    {
        USART_InitStructure.USART_StopBits = 0x0000;
    }
    else if( stopBits == UART_0_5_STOP_BIT )
    {
        USART_InitStructure.USART_StopBits = 0x1000;
    }
    else if( stopBits == UART_2_STOP_BIT )
    {
        USART_InitStructure.USART_StopBits = 0x2000;
    }
    else if( stopBits == UART_1_5_STOP_BIT )
    {
        USART_InitStructure.USART_StopBits = 0x3000;
    }

    if( parity == NO_PARITY )
    {
        USART_InitStructure.USART_Parity = 0x0000;
    }
    else if( parity == EVEN_PARITY )
    {
        USART_InitStructure.USART_Parity = 0x0400;
    }
    else
    {
        USART_InitStructure.USART_Parity = 0x0600;
    }

    if( flowCtrl == NO_FLOW_CTRL )
    {
        USART_InitStructure.USART_HardwareFlowControl = 0x0000;
    }
    else if( flowCtrl == RTS_FLOW_CTRL ) 
    {
        USART_InitStructure.USART_HardwareFlowControl = 0x0100;
    }
    else if( flowCtrl == CTS_FLOW_CTRL ) 
    {
        USART_InitStructure.USART_HardwareFlowControl = 0x0200;
    }
    else if( flowCtrl == RTS_CTS_FLOW_CTRL ) 
    {
        USART_InitStructure.USART_HardwareFlowControl = 0x0300;
    }

    USART_Init( USART1, &USART_InitStructure );

    USART_Cmd( USART1, ENABLE );
}

void UartMcuDeInit( Uart_t *obj )
{
    USART_DeInit( USART1 );

    GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
    if( IsFifoFull( &obj->FifoTx ) == false )
    {
        __disable_irq( );
        FifoPush( &obj->FifoTx, data );
        __enable_irq( );
        // Enable the USART Transmit interrupt
        USART_ITConfig( USART1, USART_IT_TXE, ENABLE );
        return 0; // OK
    }
    return 1; // Busy
}

uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data )
{
    if( IsFifoEmpty( &obj->FifoRx ) == false )
    {
        __disable_irq( );
        *data = FifoPop( &obj->FifoRx );
        __enable_irq( );
        return 0;
    }
    return 1;
}

void USART1_IRQHandler( void )
{
    uint8_t data;

    if( USART_GetITStatus( USART1, USART_IT_TXE ) != RESET )
    {    
        if( IsFifoEmpty( &Uart1.FifoTx ) == false )
        {
            data = FifoPop( &Uart1.FifoTx );
            //  Write one byte to the transmit data register 
            USART_SendData( USART1, data );
        }
        else
        {
            // Disable the USART Transmit interrupt
            USART_ITConfig( USART1, USART_IT_TXE, DISABLE );
        }
        if( Uart1.IrqNotify != NULL )
        {
            Uart1.IrqNotify( UART_NOTIFY_TX );
        }
    }

    if( USART_GetITStatus( USART1, USART_IT_ORE_RX ) != RESET )
    {
        USART_ReceiveData( USART1 );
    }

    if( USART_GetITStatus( USART1, USART_IT_RXNE ) != RESET )
    {    
        data = USART_ReceiveData( USART1 );
        if( IsFifoFull( &Uart1.FifoRx ) == false )
        {
            // Read one byte from the receive data register
            FifoPush( &Uart1.FifoRx, data );
        }
        if( Uart1.IrqNotify != NULL )
        {
            Uart1.IrqNotify( UART_NOTIFY_RX );
        }
    }
}
