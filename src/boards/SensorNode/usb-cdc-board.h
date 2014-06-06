#ifndef __BOARD_USB_H__
#define __BOARD_USB_H__

#define Get_SerialNum                               UsbMcuGetSerialNum
#define USB_Cable_Config                            UsbMcuCableConfig
#define Leave_LowPowerMode                          UsbMcuLeaveLowPowerMode
#define Enter_LowPowerMode                          UsbMcuEnterLowPowerMode

void UsbMcuInit( void );
void UsbMcuEnterLowPowerMode( void );
void UsbMcuLeaveLowPowerMode( void );
void UsbMcuCableConfig( FunctionalState newState );
void UsbMcuGetSerialNum( void );
bool UsbMcuIsDeviceConfigured( void );
uint32_t UsbMcuCdcTxData( uint8_t *buffer, uint8_t length );
uint32_t UsbMcuCdcRxData( void );


#endif // __BOARD_USB_H__
