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
 */
#include "asf.h"

#include "board-config.h"
#include "gpio-board.h"

void GpioMcuInit( Gpio_t* obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value )
{
    struct port_config pin_conf;

    obj->pin = pin;

    if( pin == NC )
    {
        return;
    }

    port_get_config_defaults( &pin_conf );

    if( ( mode == PIN_INPUT ) || ( mode == PIN_ANALOGIC ) )
    {
        pin_conf.direction  = PORT_PIN_DIR_INPUT;
        pin_conf.input_pull = ( enum port_pin_pull ) type;
    }
    else if( mode == PIN_ALTERNATE_FCT )
    {
        // assert_param( LMN_STATUS_ERROR );
        while( 1 )
            ;
    }
    else  // mode output
    {
        pin_conf.direction = PORT_PIN_DIR_OUTPUT;
    }
    port_pin_set_config( obj->pin, &pin_conf );

    // Sets initial output value
    if( mode == PIN_OUTPUT )
    {
        port_pin_set_output_level( obj->pin, ( value != 0 ) ? true : false );
    }
}

void GpioMcuSetContext( Gpio_t* obj, void* context )
{
    obj->Context = context;
}

void GpioMcuSetInterrupt( Gpio_t* obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler* irqHandler )
{
    // assert_param( LMN_STATUS_ERROR );
    while( 1 )
        ;
}

void GpioMcuRemoveInterrupt( Gpio_t* obj )
{
    // assert_param( LMN_STATUS_ERROR );
    while( 1 )
        ;
}

void GpioMcuWrite( Gpio_t* obj, uint32_t value )
{
    if( obj == NULL )
    {
        // assert_param( LMN_STATUS_ERROR );
        while( 1 )
            ;
    }
    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return;
    }
    port_pin_set_output_level( obj->pin, ( value != 0 ) ? true : false );
}

void GpioMcuToggle( Gpio_t* obj )
{
    if( obj == NULL )
    {
        // assert_param( LMN_STATUS_ERROR );
        while( 1 )
            ;
    }

    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return;
    }
    port_pin_toggle_output_level( obj->pin );
}

uint32_t GpioMcuRead( Gpio_t* obj )
{
    if( obj == NULL )
    {
        // assert_param( LMN_STATUS_ERROR );
        while( 1 )
            ;
    }
    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return 0;
    }
    return ( port_pin_get_output_level( obj->pin ) == true ) ? 1 : 0;
}
