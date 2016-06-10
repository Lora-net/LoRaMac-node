/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic GPIO driver implementation

Comment: Relies on the specific board GPIO implementation as well as on
         IO expander driver implementation if one is available on the target
         board.

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

#include "gpio-board.h"

#if defined( BOARD_IOE_EXT )
#include "gpio-ioe.h"
#endif

void GpioInit( Gpio_t *obj, PinNames pin, PinModes mode,  PinConfigs config, PinTypes type, uint32_t value )
{
    if( ( uint32_t )( pin >> 4 ) <= 6 )
    {
        GpioMcuInit( obj, pin, mode, config, type, value );
    }
    else
    {
#if defined( BOARD_IOE_EXT )
        // IOExt Pin
        GpioIoeInit( obj, pin, mode, config, type, value );
#endif
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
#if defined( BOARD_IOE_EXT )
        // IOExt Pin
        GpioIoeSetInterrupt( obj, irqMode, irqPriority, irqHandler );
#endif
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
#if defined( BOARD_IOE_EXT )
        // IOExt Pin
        //GpioIoeRemoveInterrupt( obj );
#endif
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
#if defined( BOARD_IOE_EXT )
        // IOExt Pin
        GpioIoeWrite( obj, value );
#endif
    }
}

void GpioToggle( Gpio_t *obj )
{
    if( ( uint32_t )( obj->pin >> 4 ) <= 6 )
    {
        GpioMcuToggle( obj );
    }
    else
    {
#if defined( BOARD_IOE_EXT )
        // IOExt Pin
        GpioIoeWrite( obj, GpioIoeRead( obj ) ^ 1 );
#endif
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
#if defined( BOARD_IOE_EXT )
        // IOExt Pin
        return GpioIoeRead( obj );
#else
        return 0;
#endif
    }
}
