/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Board UART driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

#include "uart-board.h"

UART_HandleTypeDef UartHandle;
uint8_t RxData = 0;
uint8_t TxData = 0;

void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

    __HAL_RCC_USART1_FORCE_RESET( );
    __HAL_RCC_USART1_RELEASE_RESET( );
    __HAL_RCC_USART1_CLK_ENABLE( );

    GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF7_USART1 );
    GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF7_USART1 );
}

void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    UartHandle.Instance = USART1;
    UartHandle.Init.BaudRate = baudrate;

    if( mode == TX_ONLY )
    {
        if( obj->FifoTx.Data == NULL )
        {
            assert_param( FAIL );
        }
        UartHandle.Init.Mode = UART_MODE_TX;
    }
    else if( mode == RX_ONLY )
    {
        if( obj->FifoRx.Data == NULL )
        {
            assert_param( FAIL );
        }
        UartHandle.Init.Mode = UART_MODE_RX;
    }
    else if( mode == RX_TX )
    {
        if( ( obj->FifoTx.Data == NULL ) || ( obj->FifoRx.Data == NULL ) )
        {
            assert_param( FAIL );
        }
        UartHandle.Init.Mode = UART_MODE_TX_RX;
    }
    else
    {
       assert_param( FAIL );
    }

    if( wordLength == UART_8_BIT )
    {
        UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    }
    else if( wordLength == UART_9_BIT )
    {
        UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
    }

    switch( stopBits )
    {
    case UART_2_STOP_BIT:
        UartHandle.Init.StopBits = UART_STOPBITS_2;
        break;
    case UART_1_STOP_BIT:
    default:
        UartHandle.Init.StopBits = UART_STOPBITS_1;
        break;
    }

    if( parity == NO_PARITY )
    {
        UartHandle.Init.Parity = UART_PARITY_NONE;
    }
    else if( parity == EVEN_PARITY )
    {
        UartHandle.Init.Parity = UART_PARITY_EVEN;
    }
    else
    {
        UartHandle.Init.Parity = UART_PARITY_ODD;
    }

    if( flowCtrl == NO_FLOW_CTRL )
    {
        UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    }
    else if( flowCtrl == RTS_FLOW_CTRL )
    {
        UartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS;
    }
    else if( flowCtrl == CTS_FLOW_CTRL )
    {
        UartHandle.Init.HwFlowCtl = UART_HWCONTROL_CTS;
    }
    else if( flowCtrl == RTS_CTS_FLOW_CTRL )
    {
        UartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
    }

    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if( HAL_UART_Init( &UartHandle ) != HAL_OK )
    {
        assert_param( FAIL );
    }

    HAL_NVIC_SetPriority( USART1_IRQn, 8, 0 );
    HAL_NVIC_EnableIRQ( USART1_IRQn );

    /* Enable the UART Data Register not empty Interrupt */
    HAL_UART_Receive_IT( &UartHandle, &RxData, 1 );
}

void UartMcuDeInit( Uart_t *obj )
{
    __HAL_RCC_USART1_FORCE_RESET( );
    __HAL_RCC_USART1_RELEASE_RESET( );
    __HAL_RCC_USART1_CLK_DISABLE( );

    GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
    BoardDisableIrq( );
    TxData = data;

    if( IsFifoFull( &obj->FifoTx ) == false )
    {
        FifoPush( &obj->FifoTx, TxData );

        // Trig UART Tx interrupt to start sending the FIFO contents.
        __HAL_UART_ENABLE_IT( &UartHandle, UART_IT_TC );

        BoardEnableIrq( );
        return 0; // OK
    }
    BoardEnableIrq( );
    return 1; // Busy
}

uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data )
{
    BoardDisableIrq( );

    if( IsFifoEmpty( &obj->FifoRx ) == false )
    {
        *data = FifoPop( &obj->FifoRx );
        BoardEnableIrq( );
        return 0;
    }
    BoardEnableIrq( );
    return 1;
}

void HAL_UART_TxCpltCallback( UART_HandleTypeDef *handle )
{
    if( IsFifoEmpty( &Uart1.FifoTx ) == false )
    {
        TxData = FifoPop( &Uart1.FifoTx );
        //  Write one byte to the transmit data register
        HAL_UART_Transmit_IT( &UartHandle, &TxData, 1 );
    }

    if( Uart1.IrqNotify != NULL )
    {
        Uart1.IrqNotify( UART_NOTIFY_TX );
    }
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *handle )
{
    if( IsFifoFull( &Uart1.FifoRx ) == false )
    {
        // Read one byte from the receive data register
        FifoPush( &Uart1.FifoRx, RxData );
    }

    if( Uart1.IrqNotify != NULL )
    {
        Uart1.IrqNotify( UART_NOTIFY_RX );
    }

    HAL_UART_Receive_IT( &UartHandle, &RxData, 1 );
}

void HAL_UART_ErrorCallback( UART_HandleTypeDef *handle )
{
    HAL_UART_Receive_IT( &UartHandle, &RxData, 1 );
}

void USART1_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UartHandle );
}

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
int __io_putchar( int c )
#else /* __GNUC__ */
int fputc( int c, FILE *stream )
#endif
{
    while( UartMcuPutChar( &Uart1, c ) != 0 );
    return c;
}
