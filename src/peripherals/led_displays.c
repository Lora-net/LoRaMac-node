/**
 * @file led_displays.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "led_displays.h"
#include "stdint.h"
#include "gpio.h"
#include "delay.h"

extern Gpio_t Led1;

void display_still_searching()
{
    // Indicator led to indicate that still searching
    GpioWrite(&Led1, 1);
    DelayMs(100);
    GpioWrite(&Led1, 0);

}

/* indicate that fix has been found */
void display_fix_found()
{
    for (uint8_t i = 0; i < 20; i++)
    {
        GpioWrite(&Led1, 1);
        DelayMs(50);
        GpioWrite(&Led1, 0);
        DelayMs(50);
    }

}
