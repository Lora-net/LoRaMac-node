/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: IO expander implementation (based on the sx1509)

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "gpio-ioe.h"
#include "sx1509.h"

void GpioIoeInit( Gpio_t *obj, PinNames pin, PinModes mode,  PinConfigs config, PinTypes type, uint32_t value )
{
    uint8_t regAdd = 0;
    uint8_t regVal = 0;
    uint8_t tempVal = 0;

    SX1509Init( );

    obj->pin = pin;
    obj->pinIndex = ( 0x01 << pin % 16 );

    if( ( obj->pin % 16 ) > 0x07 )
    {
        regAdd = RegDirB;
        obj->pinIndex = ( obj->pinIndex >> 8 ) & 0x00FF;
    }
    else
    {
        regAdd = RegDirA;
        obj->pinIndex = ( obj->pinIndex ) & 0x00FF;
    }

    SX1509Read( regAdd, &regVal );

    if( mode == PIN_OUTPUT )
    {
        regVal = regVal & ~obj->pinIndex;
    }
    else
    {
        regVal = regVal | obj->pinIndex;
    }
    SX1509Write( regAdd, regVal );


    if( ( obj->pin % 16 ) > 0x07 )
    {
        SX1509Read( RegOpenDrainB, &tempVal );
        if( config == PIN_OPEN_DRAIN )
        {
            SX1509Write( RegOpenDrainB, tempVal | obj->pinIndex );
        }
        else
        {
            SX1509Write( RegOpenDrainB, tempVal & ~obj->pinIndex );
        }
        regAdd = RegDataB;
    }
    else
    {
        SX1509Read( RegOpenDrainA, &tempVal );
        if( config == PIN_OPEN_DRAIN )
        {
            SX1509Write( RegOpenDrainA, tempVal | obj->pinIndex );
        }
        else
        {
            SX1509Write( RegOpenDrainA, tempVal & ~obj->pinIndex );
        }
        regAdd = RegDataA;
    }

    SX1509Read( regAdd, &regVal );


    // Sets initial output value
    if( value == 0 )
    {
        regVal = regVal & ~obj->pinIndex;
    }
    else
    {
        regVal = regVal | obj->pinIndex;
    }
    SX1509Write( regAdd, regVal );

}

void GpioIoeSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
// to be implemented
}

void GpioIoeWrite( Gpio_t *obj, uint32_t value )
{
    uint8_t regAdd = 0;
    uint8_t regVal = 0;

    if( ( obj->pin % 16 ) > 0x07 )
    {
        regAdd = RegDataB;
    }
    else
    {
        regAdd = RegDataA;
    }

    SX1509Read( regAdd, &regVal );

    // Sets initial output value
    if( value == 0 )
    {
        regVal = regVal & ~obj->pinIndex;
    }
    else
    {
        regVal = regVal | obj->pinIndex;
    }
    SX1509Write( regAdd, regVal );
}

uint32_t GpioIoeRead( Gpio_t *obj )
{
    uint8_t regAdd = 0;
    uint8_t regVal = 0;

    if( ( obj->pin % 16 ) > 0x07 )
    {
        regAdd = RegDataB;
    }
    else
    {
        regAdd = RegDataA;
    }

    SX1509Read( regAdd, &regVal );

    if( ( regVal & obj->pinIndex ) == 0x00 )
    {
        return 0;
    }
    else
    {
        return 1;
    } 
}

