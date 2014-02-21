/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Bleeper board Delay functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "delay-board.h"

/*!
 * Systick counter
 */
volatile uint32_t WaitDelay = 0;

static bool DelayInitalized = false;

void DelayMcuInit( void )
{
    if( DelayInitalized == true )
    {
        return;
    }
    DelayInitalized = true;

    // Based on the System core clock
    // Desired Time base (s) = SysTick Counter Clock (Hz) / Reload Value
    if( SysTick_Config( SystemCoreClock / 1000 ) )
    { 
        /* Capture error */ 
        while( 1 );
    }
    DelayMcuEnable( );
}

void DelayMcuEnable( void )
{
    SysTick->CTRL  |= SysTick_CTRL_TICKINT_Msk;     // Systick IRQ on           
}

void DelayMcuDisable( void )
{
    DelayInitalized = false;
    SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;    // Systick IRQ off 
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;            // Clear SysTick Exception pending flag
}

uint32_t DelayMcuRead( void )
{
    return WaitDelay;
}

void DelayMcuWrite( uint32_t timeout )
{
    if( DelayInitalized == false )
    {
        DelayMcuInit( );
    }
    DelayInitalized = true;
    WaitDelay = timeout;
}

void DelayMcuDecrement( void )
{
    if( WaitDelay != 0x00 )
    { 
        WaitDelay--;
    }
}

/*!
 * Systick IRQ handler
 */
void SysTick_Handler( void )
{
    DelayMcuDecrement( );
}

