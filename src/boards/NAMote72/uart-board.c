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

typedef struct
{
    UART_HandleTypeDef UartHandle;
    uint8_t RxData;
    uint8_t TxData;
}UartContext_t;

UartContext_t UartContext[2];

void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

    if( obj->UartId == UART_1 )
    {
        __HAL_RCC_USART1_FORCE_RESET( );
        __HAL_RCC_USART1_RELEASE_RESET( );
        __HAL_RCC_USART1_CLK_ENABLE( );

        GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF7_USART1 );
        GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF7_USART1 );
    }
    else if( obj->UartId == UART_2 )
    {
        __HAL_RCC_USART2_FORCE_RESET( );
        __HAL_RCC_USART2_RELEASE_RESET( );
        __HAL_RCC_USART2_CLK_ENABLE( );

        GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF7_USART2 );
        GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF7_USART2 );
    }
}

void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    if( obj->UartId == UART_1 )
    {
        UartContext[obj->UartId].UartHandle.Instance = USART1;
    }
    else if( obj->UartId == UART_2 )
    {
        UartContext[obj->UartId].UartHandle.Instance = USART2;
    }
    UartContext[obj->UartId].UartHandle.Init.BaudRate = baudrate;

    if( mode == TX_ONLY )
    {
        if( obj->FifoTx.Data == NULL )
        {
            assert_param( FAIL );
        }
        UartContext[obj->UartId].UartHandle.Init.Mode = UART_MODE_TX;
    }
    else if( mode == RX_ONLY )
    {
        if( obj->FifoRx.Data == NULL )
        {
            assert_param( FAIL );
        }
        UartContext[obj->UartId].UartHandle.Init.Mode = UART_MODE_RX;
    }
    else if( mode == RX_TX )
    {
        if( ( obj->FifoTx.Data == NULL ) || ( obj->FifoRx.Data == NULL ) )
        {
            assert_param( FAIL );
        }
        UartContext[obj->UartId].UartHandle.Init.Mode = UART_MODE_TX_RX;
    }
    else
    {
       assert_param( FAIL );
    }

    if( wordLength == UART_8_BIT )
    {
        UartContext[obj->UartId].UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    }
    else if( wordLength == UART_9_BIT )
    {
        UartContext[obj->UartId].UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
    }

    switch( stopBits )
    {
    case UART_2_STOP_BIT:
        UartContext[obj->UartId].UartHandle.Init.StopBits = UART_STOPBITS_2;
        break;
    case UART_1_STOP_BIT:
    default:
        UartContext[obj->UartId].UartHandle.Init.StopBits = UART_STOPBITS_1;
        break;
    }

    if( parity == NO_PARITY )
    {
        UartContext[obj->UartId].UartHandle.Init.Parity = UART_PARITY_NONE;
    }
    else if( parity == EVEN_PARITY )
    {
        UartContext[obj->UartId].UartHandle.Init.Parity = UART_PARITY_EVEN;
    }
    else
    {
        UartContext[obj->UartId].UartHandle.Init.Parity = UART_PARITY_ODD;
    }

    if( flowCtrl == NO_FLOW_CTRL )
    {
        UartContext[obj->UartId].UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    }
    else if( flowCtrl == RTS_FLOW_CTRL )
    {
        UartContext[obj->UartId].UartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS;
    }
    else if( flowCtrl == CTS_FLOW_CTRL )
    {
        UartContext[obj->UartId].UartHandle.Init.HwFlowCtl = UART_HWCONTROL_CTS;
    }
    else if( flowCtrl == RTS_CTS_FLOW_CTRL )
    {
        UartContext[obj->UartId].UartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
    }

    UartContext[obj->UartId].UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if( HAL_UART_Init( &UartContext[obj->UartId].UartHandle ) != HAL_OK )
    {
        assert_param( FAIL );
    }

    if( obj->UartId == UART_1 )
    {
        HAL_NVIC_SetPriority( USART1_IRQn, 1, 0 );
        HAL_NVIC_EnableIRQ( USART1_IRQn );
    }
    else if( obj->UartId == UART_2 )
    {
        HAL_NVIC_SetPriority( USART2_IRQn, 1, 0 );
        HAL_NVIC_EnableIRQ( USART2_IRQn );
    }

    /* Enable the UART Data Register not empty Interrupt */
    HAL_UART_Receive_IT( &UartContext[obj->UartId].UartHandle, &UartContext[obj->UartId].RxData, 1 );
}

void UartMcuDeInit( Uart_t *obj )
{
    if( obj->UartId == UART_1 )
    {
        __HAL_RCC_USART1_FORCE_RESET( );
        __HAL_RCC_USART1_RELEASE_RESET( );
        __HAL_RCC_USART1_CLK_DISABLE( );
    }
    else if( obj->UartId == UART_2 )
    {
        __HAL_RCC_USART2_FORCE_RESET( );
        __HAL_RCC_USART2_RELEASE_RESET( );
        __HAL_RCC_USART2_CLK_DISABLE( );
    }

    GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
    BoardDisableIrq( );
    UartContext[obj->UartId].TxData = data;

    if( IsFifoFull( &obj->FifoTx ) == false )
    {
        FifoPush( &obj->FifoTx, UartContext[obj->UartId].TxData );

        // Trig UART Tx interrupt to start sending the FIFO contents.
        __HAL_UART_ENABLE_IT( &UartContext[obj->UartId].UartHandle, UART_IT_TC );

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
    Uart_t *uart = &Uart1;
    UartId_t uartId = UART_1;

    if( handle == &UartContext[UART_1].UartHandle )
    {
        uart = &Uart1;
        uartId = UART_1;
    }
    else if( handle == &UartContext[UART_2].UartHandle )
    {
        uart = &Uart2;
        uartId = UART_2;
    }
    else
    {
        // Unknown UART peripheral skip processing
        return;
    }
    if( IsFifoEmpty( &uart->FifoTx ) == false )
    {
        UartContext[uartId].TxData = FifoPop( &uart->FifoTx );
        //  Write one byte to the transmit data register
        HAL_UART_Transmit_IT( &UartContext[uartId].UartHandle, &UartContext[uartId].TxData, 1 );
    }

    if( uart->IrqNotify != NULL )
    {
        uart->IrqNotify( UART_NOTIFY_TX );
    }
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *handle )
{
    Uart_t *uart = &Uart1;
    UartId_t uartId = UART_1;

    if( handle == &UartContext[UART_1].UartHandle )
    {
        uart = &Uart1;
        uartId = UART_1;
    }
    else if( handle == &UartContext[UART_2].UartHandle )
    {
        uart = &Uart2;
        uartId = UART_2;
    }
    else
    {
        // Unknown UART peripheral skip processing
        return;
    }
    if( IsFifoFull( &uart->FifoRx ) == false )
    {
        // Read one byte from the receive data register
        FifoPush( &uart->FifoRx, UartContext[uartId].RxData );
    }

    if( uart->IrqNotify != NULL )
    {
        uart->IrqNotify( UART_NOTIFY_RX );
    }

    HAL_UART_Receive_IT( &UartContext[uartId].UartHandle, &UartContext[uartId].RxData, 1 );
}

void HAL_UART_ErrorCallback( UART_HandleTypeDef *handle )
{
    UartId_t uartId = UART_1;

    if( handle == &UartContext[UART_1].UartHandle )
    {
        uartId = UART_1;
    }
    else if( handle == &UartContext[UART_2].UartHandle )
    {
        uartId = UART_2;
    }
    else
    {
        // Unknown UART peripheral skip processing
        return;
    }
    HAL_UART_Receive_IT( &UartContext[uartId].UartHandle, &UartContext[uartId].RxData, 1 );
}

void USART1_IRQHandler( void )
{
    // [BEGIN] Workaround to solve an issue with the HAL drivers not managin the uart state correctly.
    uint32_t tmpFlag = 0, tmpItSource = 0;

    tmpFlag = __HAL_UART_GET_FLAG( &UartContext[UART_1].UartHandle, UART_FLAG_TC );
    tmpItSource = __HAL_UART_GET_IT_SOURCE( &UartContext[UART_1].UartHandle, UART_IT_TC );
    // UART in mode Transmitter end
    if( ( tmpFlag != RESET ) && ( tmpItSource != RESET ) )
    {
        if( ( UartContext[UART_1].UartHandle.State == HAL_UART_STATE_BUSY_RX ) || UartContext[UART_1].UartHandle.State == HAL_UART_STATE_BUSY_TX_RX )
        {
            UartContext[UART_1].UartHandle.State = HAL_UART_STATE_BUSY_TX_RX;
        }
    }
    // [BEGIN] Workaround to solve an issue with the HAL drivers not managin the uart state correctly.

    HAL_UART_IRQHandler( &UartContext[UART_1].UartHandle );
}

void USART2_IRQHandler( void )
{
    // [BEGIN] Workaround to solve an issue with the HAL drivers not managin the uart state correctly.
    uint32_t tmpFlag = 0, tmpItSource = 0;

    tmpFlag = __HAL_UART_GET_FLAG( &UartContext[UART_2].UartHandle, UART_FLAG_TC );
    tmpItSource = __HAL_UART_GET_IT_SOURCE( &UartContext[UART_2].UartHandle, UART_IT_TC );
    // UART in mode Transmitter end
    if( ( tmpFlag != RESET ) && ( tmpItSource != RESET ) )
    {
        if( ( UartContext[UART_2].UartHandle.State == HAL_UART_STATE_BUSY_RX ) || UartContext[UART_2].UartHandle.State == HAL_UART_STATE_BUSY_TX_RX )
        {
            UartContext[UART_2].UartHandle.State = HAL_UART_STATE_BUSY_TX_RX;
        }
    }
    // [END] Workaround to solve an issue with the HAL drivers not managin the uart state correctly.

    HAL_UART_IRQHandler( &UartContext[UART_2].UartHandle );
}

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
int __io_putchar( int c )
#else /* __GNUC__ */
int fputc( int c, FILE *stream )
#endif
{
    while( UartMcuPutChar( &Uart2, c ) != 0 );
    return c;
}
