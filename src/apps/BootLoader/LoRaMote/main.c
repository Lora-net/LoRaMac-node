/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRaMote board USB DFU bootloader

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
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;

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
    uint8_t regValue = 0;
    uint8_t status = 0;
    uint16_t offset = 0;
    
    I2cInit( &I2c, I2C_SCL, I2C_SDA );
    
    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    
    // Init SAR
    SX9500Init( );
    
    SX9500Write( SX9500_REG_IRQMSK, 0x10 );
    SX9500Write( SX9500_REG_IRQSRC, 0x10 );
    
    do
    {
        SX9500Read( SX9500_REG_IRQSRC, &status );
    }while( ( status & 0x10 ) == 0x00 ); // While compensation for CS0 is pending
    
    // Read 1st sensor offset
    SX9500Read( SX9500_REG_OFFSETMSB, ( uint8_t* )&regValue );
    offset = regValue << 8;
    SX9500Read( SX9500_REG_OFFSETLSB, ( uint8_t* )&regValue );
    offset |= regValue;
    
    if( offset < 2000 )
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
        DelayLoop( 500 );
        GpioWrite( &Led1, 1 );
        GpioWrite( &Led2, 1 );
        GpioWrite( &Led3, 1 );
        DelayLoop( 500 );
    }
}
