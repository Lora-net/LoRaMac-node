/*!
 * \file      uart-usb-board.c
 *
 * \brief     Target board UART over USB driver implementation
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
#include "board.h"
#include "uart.h"
#include "uart-board.h"
#include "board-config.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "uart-usb-board.h"

// USB Device Core handle declaration
USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_FS;
extern USBD_DescriptorsTypeDef FS_Desc;

void UartUsbInit( Uart_t *obj, UartId_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

    __HAL_RCC_COMP_CLK_ENABLE( );
    __HAL_RCC_SYSCFG_CLK_ENABLE( );

    CDC_Set_Uart_Obj( obj );

    /* Init Device Library, Add Supported Class and Start the library */
    USBD_Init( &hUsbDeviceFS, &FS_Desc, DEVICE_FS );

    USBD_RegisterClass( &hUsbDeviceFS, &USBD_CDC );

    USBD_CDC_RegisterInterface( &hUsbDeviceFS, &USBD_Interface_fops_FS );

    USBD_Start( &hUsbDeviceFS );
}

void UartUsbConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{

}

void UartUsbDeInit( Uart_t *obj )
{

}

uint8_t UartUsbIsUsbCableConnected( void )
{
    if( hUsbDeviceFS.dev_address == 0 )
    {
        // USB is low Power mode meaning USB cable is not connected
        return 0;
    }
    else
    {
        // USB is in active mode meaning USB cable is connected
        return 1;
    }
}

uint8_t UartUsbPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size )
{
    return CDC_Transmit_FS( buffer, size );
}

uint8_t UartUsbPutChar( Uart_t *obj, uint8_t data )
{
   return UartUsbPutBuffer( obj, &data, 1 );
}

uint8_t UartUsbGetChar( Uart_t *obj, uint8_t *data )
{
    if( IsFifoEmpty( &obj->FifoRx ) == false )
    {
        BoardDisableIrq( );
        *data = FifoPop( &obj->FifoRx );
        BoardEnableIrq( );
        return 0;
    }
    return 1;
}

void USB_LP_IRQHandler( void )
{
    HAL_PCD_IRQHandler( &hpcd_USB_FS );
}
