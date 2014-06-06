/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SensorNode board USB DFU bootloader

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "dfu_mal.h"

#include "usb-dfu-board.h"

typedef  void ( *pFunction )( void );

uint8_t DeviceState;
uint8_t DeviceStatus[6];
pFunction Jump_To_Application;
uint32_t JumpAddress;

/*
 * Board peripherals objects
 */
Gpio_t RadioPushButton;

Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;
Gpio_t Led4;

Gpio_t UsbDetect;
Gpio_t DcDcEnable;

I2c_t I2c;

static void DelayLoop( __IO uint32_t nCount )
{
    __IO uint32_t index = 0; 
    for( index = ( 5000 * nCount ); index != 0; index-- )
    {
    }
}

int main(void)
{
    I2cInit( &I2c, I2C_SCL, I2C_SDA );

    GpioInit( &UsbDetect, USB_ON, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DcDcEnable, DC_DC_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    GpioInit( &RadioPushButton, RADIO_PUSH_BUTTON, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    
    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led4, LED_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    if( GpioRead( &RadioPushButton ) == 0 )
    { /* Test if user code is programmed starting from address 0x8003000 */
        if( ( ( *( __IO uint32_t* )ApplicationAddress ) & 0x2FFE0000 ) == 0x20000000 )
        { /* Jump to user application */

            JumpAddress = *( __IO uint32_t* )( ApplicationAddress + 4 );
            Jump_To_Application = ( pFunction ) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __set_MSP( *( __IO uint32_t* ) ApplicationAddress );
            Jump_To_Application( );
        }
    } /* Otherwise enters DFU mode to allow user to program his application */

    /* Enter DFU mode */
    DeviceState = STATE_dfuERROR;
    DeviceStatus[0] = STATUS_ERRFIRMWARE;
    DeviceStatus[4] = DeviceState;

    UsbMcuInit( );

    /* Main loop */
    while (1)
    {
        GpioWrite( &Led1, 0 );
        GpioWrite( &Led2, 0 );
        GpioWrite( &Led3, 0 );
        GpioWrite( &Led4, 0 );
        DelayLoop( 500 );
        GpioWrite( &Led1, 1 );
        GpioWrite( &Led2, 1 );
        GpioWrite( &Led3, 1 );
        GpioWrite( &Led4, 1 );
        DelayLoop( 500 );
    }
}
