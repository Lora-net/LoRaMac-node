/*!
 * \file      gpio-board.c
 *
 * \brief     Target board GPIO driver implementation
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
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */
#include <hal_gpio.h>
#include <hal_ext_irq.h>
#include "gpio-board.h"

void GpioMcuInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value )
{
    obj->pin = pin;

    if( pin == NC )
    {
        return;
    }

    //obj->port = ( void* )GPIO_PORT( obj->pin );

    if( mode == PIN_INPUT )
    {
        gpio_set_pin_direction( obj->pin, GPIO_DIRECTION_IN );
        gpio_set_pin_pull_mode( obj->pin, type ); // the pull up/down need to be set after the direction for correct working
    }
    else // mode output
    {
        gpio_set_pin_direction( obj->pin, GPIO_DIRECTION_OUT );
    }


    // Sets initial output value
    if( mode == PIN_OUTPUT )
    {
        GpioMcuWrite( obj, value );
    }
}

void GpioMcuSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
    //ext_irq_register( obj->pin, irqHandler );
}

void GpioMcuRemoveInterrupt( Gpio_t *obj )
{
    //ext_irq_register( obj->pin, NULL );
}

void GpioMcuWrite( Gpio_t *obj, uint32_t value )
{

    if( ( obj == NULL ) /*|| ( obj->port == NULL )*/ )
    {
        //assert_param( FAIL );
        while( 1 );
    }
    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return;
    }
    gpio_set_pin_level( obj->pin, value );
}

void GpioMcuToggle( Gpio_t *obj )
{
    if( ( obj == NULL ) /*|| ( obj->port == NULL )*/ )
    {
        //assert_param( FAIL );
        while( 1 );
    }

    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return;
    }
    gpio_toggle_pin_level( obj->pin );
}

uint32_t GpioMcuRead( Gpio_t *obj )
{
    if( obj == NULL )
    {
        //assert_param( FAIL );
        while( 1 );
    }
    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return 0;
    }
    return ( uint32_t )gpio_get_pin_level( obj->pin );
}
