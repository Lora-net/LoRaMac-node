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

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "uart-usb-board.h"

/* USB handler declaration */
/* Handle for USB Full Speed IP */
//USBD_HandleTypeDef  *hUsbDevice_0;

USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_FS;

void UartUsbInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
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

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
int __io_putchar( int c )
#else /* __GNUC__ */
int fputc( int c, FILE *stream )
#endif
{
    while( UartUsbPutChar( &UartUsb, c ) != 0 );
    return c;
}
