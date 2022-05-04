/*!
 * \file      main.c
 *
 * \brief     Hello-World
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
 */
#include "board.h"
#include "gpio_sys.h"
#include "msp430.h"
#include <stdio.h>

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led2;
extern Gpio_t Led1;

/**
 * Main application entry point.
 */
int main( void )
{

  uint8_t data[10];

    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );

    printf("Start Program\n");

    while(1)
    {
        GpioToggle(&Led1);
        GpioToggle(&Led2);
        // Delay
        for(long i=100000; i>0; i--);
    }

}