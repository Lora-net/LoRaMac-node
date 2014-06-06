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
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"

#include "board.h"

#include "usb_istr.h"
#include "usb_pwr.h"

#include "uart-usb-board.h"

/*!
 * FIFO buffers size
 */
#define FIFO_TX_SIZE                                64
#define FIFO_RX_SIZE                                64

uint8_t CdcTxBuffer[FIFO_TX_SIZE];
uint8_t CdcRxBuffer[FIFO_RX_SIZE];

__IO uint32_t UsbPacketTx = 1;
__IO  uint8_t UsbTxBuffer[64];
__IO uint32_t UsbTxLength;

__IO  uint8_t UsbRxBuffer[64];
__IO uint32_t UsbRxLength;

void UartUsbInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;
}

void UartUsbConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    FifoInit( &obj->FifoTx, CdcTxBuffer, FIFO_RX_SIZE );
    FifoInit( &obj->FifoRx, CdcRxBuffer, FIFO_RX_SIZE );
}

void UartUsbDeInit( Uart_t *obj )
{

}

uint8_t UartUsbPutChar( Uart_t *obj, uint8_t data )
{
    if( UsbMcuIsDeviceConfigured( ) == false )
    {
        return 2;
    }

    if( IsFifoFull( &obj->FifoTx ) == false )
    {
        __disable_irq( );
        FifoPush( &obj->FifoTx, data );
        __enable_irq( );

        if( UsbPacketTx == 1 )
        {
            /*Sent flag*/
            UsbPacketTx = 0;
            SetEPTxCount( ENDP1, 0 );
            SetEPTxValid( ENDP1 );
        }
        return 0; // OK
    }
    return 1; // Busy
//    if( UsbPacketTx == 1 )
//    {
//        /*Sent flag*/
//        UsbPacketTx = 0;
//        /* send  packet to PMA*/
//        UserToPMABufferCopy( ( unsigned char* )&data, ENDP1_TXADDR, 1 );
//        SetEPTxCount( ENDP1, 1 );
//        SetEPTxValid( ENDP1 );
//        return 0; // OK
//    }
//    else
//    {
//        if( IsFifoFull( &obj->FifoTx ) == false )
//        {
//            __disable_irq( );
//            FifoPush( &obj->FifoTx, data );
//            __enable_irq( );
//            return 0; // OK
//        }
//    }
//    return 1; // Busy
}

uint8_t UartUsbGetChar( Uart_t *obj, uint8_t *data )
{
    if( UsbMcuIsDeviceConfigured( ) == false )
    {
        return 2;
    }
    SetEPRxValid( ENDP3 );
    if( IsFifoEmpty( &obj->FifoRx ) == false )
    {
        __disable_irq( );
        *data = FifoPop( &obj->FifoRx );
        __enable_irq( );
        return 0;
    }
    return 1;
}

void EP1_IN_Callback (void)
{
    UsbPacketTx = 1;
    
    UsbTxLength = 0;
    
    while( IsFifoEmpty( &UartUsb.FifoTx ) == false )
    {
        UsbTxBuffer[UsbTxLength] = FifoPop( &UartUsb.FifoTx );
        UsbTxLength++;
    }

    if( UsbTxLength > 0 )
    {
        UsbPacketTx = 0;
        UserToPMABufferCopy( ( unsigned char* )UsbTxBuffer, ENDP1_TXADDR, UsbTxLength );
        SetEPTxCount( ENDP1, UsbTxLength );
        SetEPTxValid( ENDP1 );

    }
}

void EP3_OUT_Callback(void)
{
    uint8_t i;
    
    UsbRxLength = GetEPRxCount( ENDP3 );
    PMAToUserBufferCopy( ( unsigned char* )UsbRxBuffer, ENDP3_RXADDR, UsbRxLength );
    
    for( i = 0; i < UsbRxLength; i++ )
    {
        if( IsFifoFull( &UartUsb.FifoRx ) == false )
        {
            // Read one byte from the receive data register
            FifoPush( &UartUsb.FifoRx, UsbRxBuffer[i] );
        }
    }

    if( UartUsb.IrqNotify != NULL )
    {
        UartUsb.IrqNotify( UART_NOTIFY_RX );
    }
}
