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
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "utilities.h"
#include "board.h"
#include "uart-board.h"
#include "msp430.h"
#include "stdio.h"
#include "eusci_a_uart.h"

/*!
 * Number of times the UartPutBuffer will try to send the buffer before
 * returning ERROR
 */
#define TX_BUFFER_RETRY_COUNT                       2

uint8_t RxData = 0;
uint8_t TxData = 0;

extern Uart_t Uart2;

void UartMcuInit( Uart_t *obj, UartId_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

    if( uartId == UART_USB_CDC )
    {
#if defined( USE_USB_CDC )
        UartUsbInit( obj, uartId, NC, NC );
#endif
    }
    else
    {
        GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 0);
        GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1);
    }
}

void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
      // Configure UART
    //TODO: make use of parameters (see below) --> currently only static implementation with baudrate =
    EUSCI_A_UART_initParam param = {//8MHz clock
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,
        52,                                                                         // clockPrescalar
        1,                                                                          // firstModReg
        73,                                                                         // secondModReg
        EUSCI_A_UART_NO_PARITY,
        EUSCI_A_UART_LSB_FIRST,
        EUSCI_A_UART_ONE_STOP_BIT,
        EUSCI_A_UART_MODE,
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
        };

    if( ( obj->FifoTx.Data == NULL ) || ( obj->FifoRx.Data == NULL ) )
    {
       //assert( LMN_STATUS_ERROR );
    }

    if(STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param)) 
    {
       //assert( 0 );
    }
       

    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
                                EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
                                    EUSCI_A_UART_RECEIVE_INTERRUPT); // Enable interrupt

    // Enable globale interrupt
    __enable_interrupt();

//TODO: distinguish RX_ONLY/TX_ONLY mode?


    // if( wordLength == UART_8_BIT )
    // {
    //     UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    // }
    // else if( wordLength == UART_9_BIT )
    // {
    //     UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
    // }

    // switch( stopBits )
    // {
    // case UART_2_STOP_BIT:
    //     UartHandle.Init.StopBits = UART_STOPBITS_2;
    //     break;
    // case UART_1_STOP_BIT:
    // default:
    //     UartHandle.Init.StopBits = UART_STOPBITS_1;
    //     break;
    // }

    // if( parity == NO_PARITY )
    // {
    //     UartHandle.Init.Parity = UART_PARITY_NONE;
    // }
    // else if( parity == EVEN_PARITY )
    // {
    //     UartHandle.Init.Parity = UART_PARITY_EVEN;
    // }
    // else
    // {
    //     UartHandle.Init.Parity = UART_PARITY_ODD;
    // }

    // if( flowCtrl == NO_FLOW_CTRL )
    // {
    //     UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    // }
    // else if( flowCtrl == RTS_FLOW_CTRL )
    // {
    //     UartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS;
    // }
    // else if( flowCtrl == CTS_FLOW_CTRL )
    // {
    //     UartHandle.Init.HwFlowCtl = UART_HWCONTROL_CTS;
    // }
    // else if( flowCtrl == RTS_CTS_FLOW_CTRL )
    // {
    //     UartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
    // }

    // UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    // if( HAL_UART_Init( &UartHandle ) != HAL_OK )
    // {
    //     assert_param( LMN_STATUS_ERROR );
    // }

    // HAL_NVIC_SetPriority( USART2_IRQn, 1, 0 );
    // HAL_NVIC_EnableIRQ( USART2_IRQn );

    // /* Enable the UART Data Register not empty Interrupt */
    // HAL_UART_Receive_IT( &UartHandle, &RxData, 1 );
    
}

void UartMcuDeInit( Uart_t *obj )
{
        GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    
}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
    //TODO: allow use of multiple UARTS (make store EUSCI_A0_BASE in Uart_t obj)
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, data);
    return 0;
}

uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data )
{
    CRITICAL_SECTION_BEGIN( );

    if( IsFifoEmpty( &obj->FifoRx ) == false )
    {
        *data = FifoPop( &obj->FifoRx );
        CRITICAL_SECTION_END( );
        return 0;
    }
    CRITICAL_SECTION_END( );
    return 1;
}

uint8_t UartMcuPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size )
{
    uint8_t retryCount;
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
                return 1; // Error
            }
        }
    }
    return 0; // OK
}

uint8_t UartMcuGetBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size, uint16_t *nbReadBytes )
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
        return 1; // Empty
    }
    return 0; // OK
}

//Not used on MSP430
//void HAL_UART_TxCpltCallback( UART_HandleTypeDef *handle )
// void HAL_UART_TxCpltCallback()
// {
//     if( IsFifoEmpty( &Uart2.FifoTx ) == false )
//     {
//         TxData = FifoPop( &Uart2.FifoTx );
//         //  Write one byte to the transmit data register
//        // HAL_UART_Transmit_IT( &UartHandle, &TxData, 1 );
//     }

//     if( Uart2.IrqNotify != NULL )
//     {
//         Uart2.IrqNotify( UART_NOTIFY_TX );
//     }
// }

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        if( IsFifoFull( &Uart2.FifoRx ) == false )
        {
            RxData = UCA0RXBUF;    // Read one byte from the receive data register
            FifoPush( &Uart2.FifoRx, RxData );  
        }
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}


//void HAL_UART_ErrorCallback( UART_HandleTypeDef *handle )
void HAL_UART_ErrorCallback( )
{
   // HAL_UART_Receive_IT( &UartHandle, &RxData, 1 );
}

void USART2_IRQHandler( void )
{
    // [BEGIN] Workaround to solve an issue with the HAL drivers not managing the uart state correctly.
    // uint32_t tmpFlag = 0, tmpItSource = 0;

    // tmpFlag = __HAL_UART_GET_FLAG( &UartHandle, UART_FLAG_TC );
    // tmpItSource = __HAL_UART_GET_IT_SOURCE( &UartHandle, UART_IT_TC );
    // // UART in mode Transmitter end
    // if( ( tmpFlag != RESET ) && ( tmpItSource != RESET ) )
    // {
    //     if( ( UartHandle.State == HAL_UART_STATE_BUSY_RX ) || UartHandle.State == HAL_UART_STATE_BUSY_TX_RX )
    //     {
    //         UartHandle.State = HAL_UART_STATE_BUSY_TX_RX;
    //     }
    // }
    // // [END] Workaround to solve an issue with the HAL drivers not managing the uart state correctly.

    // HAL_UART_IRQHandler( &UartHandle );
}
