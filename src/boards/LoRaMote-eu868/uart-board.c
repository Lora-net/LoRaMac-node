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
#include "board.h"
#include "uart-board.h"
#include "up501.h"

#define RXBUFFERSIZE    80

uint8_t RxBuffer[RXBUFFERSIZE];
uint8_t NbrOfDataToRead = RXBUFFERSIZE;
volatile uint16_t RxCounter = 0; 
volatile uint8_t nmeaDetection = 0;

/*!
 * MCU UART peripherals enumeration
 */
typedef enum 
{
    UART_1 = ( uint32_t )USART1_BASE,
    UART_2 = ( uint32_t )USART2_BASE,
}UartName;


void UartMcuInit( Uart_t *obj, PinNames tx, PinNames rx )
{
    obj->Uart = ( USART_TypeDef *)USART1_BASE;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );

    USART_DeInit( obj->Uart );

    GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );

    GPIO_PinAFConfig( obj->Tx.port, obj->Tx.pin, GPIO_AF_USART1 );
    GPIO_PinAFConfig( obj->Rx.port, obj->Rx.pin, GPIO_AF_USART1 ); 
}

void UartMcuFormat( Uart_t *obj, UartMode mode, uint32_t baudrate, WordLength wordLength, StopBits stopBits, Parity parity, FlowCtrl flowCtrl )
{
    USART_InitTypeDef USART_InitStructure;

    if( mode == TX_ONLY )
    {
        USART_InitStructure.USART_Mode = 0x0008;
    }
    else if( mode == RX_ONLY )
    {
        USART_InitStructure.USART_Mode = 0x0004;
    }
    else
    {
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

    USART_Init( obj->Uart, &USART_InitStructure );

    USART_Cmd( USART1, ENABLE ); 
}

void UartMcuIrqInit( Uart_t *obj )
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );
}

void UartMcuDeInit( Uart_t *obj )
{
    USART_DeInit( USART1 );  

    GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void USART1_IRQHandler( void )
{
    char dataIn;

    if( USART_GetITStatus( USART1, USART_IT_RXNE ) != RESET )
    {    
        dataIn = ( USART_ReceiveData(USART1) & 0x7F ); 
    
        if( ( dataIn == '$' ) && ( nmeaDetection == 0 ) )  
        {
            nmeaDetection = 1;
            RxCounter = 0;
        }
        
        if( ( nmeaDetection == 1 ) && ( RxCounter <= NbrOfDataToRead ) )
        {          
            /* Read one byte from the receive data register */
            RxBuffer[RxCounter++] = dataIn;
        }
       
        if( ( dataIn == '*' ) && ( nmeaDetection == 1 ) )  
        {
            if( ( up501ParseGpsData( (char *)RxBuffer ) ) == SUCCESS )
            {                
                USART_ClearITPendingBit( USART1, USART_IT_RXNE );
                USART_ITConfig( USART1, USART_IT_RXNE, DISABLE );
            }
            else
            {    
                USART_ClearITPendingBit( USART1, USART_IT_RXNE );
                UartInit( &Uart, UART_TX, UART_RX );
                USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );
            }
            RxCounter = 0;
            nmeaDetection = 0;
            BlockLowPowerDuringTask ( false );
        }
    
        if( RxCounter > NbrOfDataToRead )
        {
            RxCounter = 0;
            nmeaDetection = 0;
            USART_ClearITPendingBit( USART1, USART_IT_RXNE );
            USART_ITConfig( USART1, USART_IT_RXNE, DISABLE );
        }
    }
}
