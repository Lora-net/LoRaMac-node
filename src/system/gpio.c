/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Generic GPIO driver implementation

Comment: Relies on the specific board GPIO implementation as well as on
         IO expander driver implementation if one is available on the target
         board.

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

#include "gpio-board.h"
#include "gpio-ioe.h"

void GpioInit( Gpio_t *obj, PinNames pin, PinModes mode,  PinConfigs config, PinTypes type, uint32_t value )
{
    if( ( uint32_t )( pin >> 4 ) <= 6 ) 
    {
        GpioMcuInit( obj, pin, mode, config, type, value );
    }
    else
    {
        // IOExt Pin
        GpioIoeInit( obj, pin, mode, config, type, value );
    }
}

void GpioSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
    if( ( uint32_t )( obj->pin >> 4 ) <= 6 ) 
    {
        GpioMcuSetInterrupt( obj, irqMode, irqPriority, irqHandler );
    }
    else
    {
        // IOExt Pin
        GpioIoeSetInterrupt( obj, irqMode, irqPriority, irqHandler );
    }
}

void GpioRemoveInterrupt( Gpio_t *obj )
{
    if( ( uint32_t )( obj->pin >> 4 ) <= 6 ) 
    {
        //GpioMcuRemoveInterrupt( obj );
    }
    else
    {
        // IOExt Pin
 //       GpioIoeRemoveInterrupt( obj );
    }
}

void GpioWrite( Gpio_t *obj, uint32_t value )
{
    if( ( uint32_t )( obj->pin >> 4 ) <= 6 ) 
    {
        GpioMcuWrite( obj, value );
    }
    else
    {
        // IOExt Pin
        GpioIoeWrite( obj, value );
    }
}

uint32_t GpioRead( Gpio_t *obj )
{
    if( ( uint32_t )( obj->pin >> 4 ) <= 6 ) 
    {
        return GpioMcuRead( obj );
    }
    else
    {
        // IOExt Pin
        return GpioIoeRead( obj );
    }
}
