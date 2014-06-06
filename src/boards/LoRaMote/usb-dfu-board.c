/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: DFU bootloader USB initialisation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "board.h"
#include "usb_pwr.h"

#include "usb-dfu-board.h"

#include "dfu_mal.h"

EXTI_InitTypeDef EXTI_InitStructure;

static void IntToUnicode( uint32_t value, uint8_t *pbuf, uint8_t len );

void UsbMcuInit( void )
{
    /* Enable the SYSCFG module clock */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );

    FLASH_Unlock( );
 
    /* Init the media interface */
    MAL_Init( );
    USB_Cable_Config( ENABLE );

    /* Configure the EXTI line 18 connected internally to the USB IP */
    EXTI_ClearITPendingBit( EXTI_Line18 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line18;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init( &EXTI_InitStructure ); 

    /* Enable USB clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USB, ENABLE );
    
    UsbMcuInterruptsConfig ( );
    
    USB_Init( );
}

void UsbMcuInterruptsConfig( void )
{
    NVIC_InitTypeDef NVIC_InitStructure; 

    /* 2 bit for pre-emption priority, 2 bits for subpriority */
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2 );

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    /* Enable the USB Wake-up interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USB_FS_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );
}

void UsbMcuEnterLowPowerMode( void )
{
    /* Set the device state to suspend */
    bDeviceState = SUSPENDED;
}

void UsbMcuLeaveLowPowerMode( void )
{
    DEVICE_INFO *pInfo = &Device_Info;

    /* Set the device state to the correct state */
    if( pInfo->Current_Configuration != 0 )
    {
        /* Device configured */
        bDeviceState = CONFIGURED;
    }
    else
    {
        bDeviceState = ATTACHED;
    }
    /*Enable SystemCoreClock*/
    SystemInit( );
}

void UsbMcuCableConfig( FunctionalState newState )
{
    if( newState != DISABLE )
    {
        SYSCFG_USBPuCmd( ENABLE );
    }
    else
    {
        SYSCFG_USBPuCmd( DISABLE );
    }  
}

void UsbMcuResetDevice( void )
{
    UsbMcuCableConfig( DISABLE );
    NVIC_SystemReset( );
}

void UsbMcuGetSerialNum( void )
{
    uint32_t deviceSerial0, deviceSerial1, deviceSerial2;

    deviceSerial0 = *( uint32_t* )ID1;
    deviceSerial1 = *( uint32_t* )ID2;
    deviceSerial2 = *( uint32_t* )ID3;

    deviceSerial0 += deviceSerial2;

    if( deviceSerial0 != 0 )
    {
        IntToUnicode( deviceSerial0, &DFU_StringSerial[2] , 8 );
        IntToUnicode( deviceSerial1, &DFU_StringSerial[18], 4 );
    }
}

static void IntToUnicode( uint32_t value , uint8_t *pbuf , uint8_t len )
{
    uint8_t idx = 0;

    for( idx = 0; idx < len; idx++ )
    {
        if( ( ( value >> 28 ) ) < 0xA )
        {
            pbuf[2 * idx] = ( value >> 28 ) + '0';
        }
        else
        {
            pbuf[2 * idx] = ( value >> 28 ) + 'A' - 10; 
        }

        value = value << 4;

        pbuf[2 * idx + 1] = 0;
    }
}

bool UsbMcuIsDeviceConfigured( void )
{
    return bDeviceState == CONFIGURED;
}

void USB_LP_IRQHandler(void)
{
    USB_Istr( );
}

void USB_FS_WKUP_IRQHandler(void)
{
    EXTI_ClearITPendingBit( EXTI_Line18 );
}
