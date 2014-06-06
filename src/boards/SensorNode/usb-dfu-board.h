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
#ifndef __BOARD_USB_H__
#define __BOARD_USB_H__

#include <stdbool.h>
#include "stm32l1xx.h"

#define ApplicationAddress                          0x08003000

#define Get_SerialNum                               UsbMcuGetSerialNum
#define USB_Cable_Config                            UsbMcuCableConfig
#define Leave_LowPowerMode                          UsbMcuLeaveLowPowerMode
#define Enter_LowPowerMode                          UsbMcuEnterLowPowerMode
#define USB_Interrupts_Config                       UsbMcuInterruptsConfig
#define Reset_Device                                UsbMcuResetDevice

void SMI_FLASH_Init( void );
void SMI_FLASH_SectorErase( uint32_t Address );
void SMI_FLASH_WordWrite( uint32_t Address, uint32_t Data );
void SMI_FLASH_PageWrite( uint32_t Address, uint32_t* wBuffer );

void UsbMcuInit( void );
void UsbMcuInterruptsConfig( void );
void UsbMcuEnterLowPowerMode( void );
void UsbMcuLeaveLowPowerMode( void );
void UsbMcuCableConfig( FunctionalState newState );
void UsbMcuResetDevice( void );
void UsbMcuGetSerialNum( void );
bool UsbMcuIsDeviceConfigured( void );
uint32_t UsbMcuCdcTxData( uint8_t *buffer, uint8_t length );
uint32_t UsbMcuCdcRxData( void );


#endif // __BOARD_USB_H__
