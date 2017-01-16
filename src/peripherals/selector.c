/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Hex coder selector driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

#include "selector.h"

uint8_t SelectorGetValue( void )
{
    /*!
     * Hex coder selector GPIO pins objects
     */
    Gpio_t Sel1;
    Gpio_t Sel2;
    Gpio_t Sel3;
    Gpio_t Sel4;

    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t k = 255;

    // Active
    GpioInit( &Sel1, SEL_1, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioInit( &Sel2, SEL_2, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioInit( &Sel3, SEL_3, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioInit( &Sel4, SEL_4, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );

    do
    {
        j = i;
        k = j;
        // 1 ms delay between checks
        DelayMs( 1 );
        i  =     !GpioRead( &Sel1 );
        i += 2 * !GpioRead( &Sel2 );
        i += 4 * !GpioRead( &Sel3 );
        i += 8 * !GpioRead( &Sel4 );
    } while( ( i != j ) && ( i != k ) ); // Waits for 3 successive values to be equal

    // Sleep
    GpioInit( &Sel1, SEL_1, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel2, SEL_2, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel3, SEL_3, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel4, SEL_4, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );

    return i;
}
