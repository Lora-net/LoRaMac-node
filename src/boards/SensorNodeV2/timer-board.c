/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: MCU RTC timer and low power modes management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <math.h>
#include "board.h"
#include "timer-board.h"

/*!
 * Hardware Time base in us
 */
#define HW_TIMER_TIME_BASE                              100 //us 

/*!
 * Hardware Timer tick counter
 */
volatile uint64_t TimerTickCounter = 1;     

/*!
 * Saved value of the Tick counter at the start of the next event
 */
static uint64_t TimerTickCounterContext = 0;            

/*!
 * Value trigging the IRQ
 */
volatile uint64_t TimeoutCntValue = 0;

void TimerIncrementTickCounter( void );


void TimerHwInit( void )
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* TIM2 clock enable */ 
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );

    /* --------------------------NVIC Configuration -------------------------------*/
    /* Enable the TIM2 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init( &NVIC_InitStructure );

    TimeoutCntValue = 0;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 3199;
    TIM_TimeBaseStructure.TIM_Prescaler = 0; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE );

    /* TIM2 disable counter */
    TIM_Cmd( TIM2, ENABLE ); 
}

void TimerHwDeInit( void )
{
    /* Deinitialize the timer */
    TIM_DeInit( TIM2 );
}

uint32_t TimerHwGetMinimumTimeout( void )
{
    return( ceil( 2 * HW_TIMER_TIME_BASE ) );
}

void TimerHwStart( uint32_t val )
{
    TimerTickCounterContext = TimerHwGetTimerValue( );

    if( val <= HW_TIMER_TIME_BASE + 1 )
    {
        TimeoutCntValue = TimerTickCounterContext + 1;
    }
    else
    {
        TimeoutCntValue = TimerTickCounterContext + ( ( val - 1 ) / HW_TIMER_TIME_BASE );
    }
}

void TimerHwStop( void )
{
    TIM_ITConfig( TIM2, TIM_IT_CC1, DISABLE );
    TIM_Cmd( TIM2, DISABLE );  
}

void TimerHwDelayMs( uint32_t delay )
{
    uint64_t delayValue = 0;
    uint64_t timeout = 0;

    delayValue = delay * 1000;

    timeout = TimerHwGetTimerValue( );

    while( ( ( TimerHwGetTimerValue( ) - timeout  ) * HW_TIMER_TIME_BASE ) < delayValue )
    {
    }
}

uint64_t TimerHwGetElapsedTime( void )
{
     return( ( ( TimerHwGetTimerValue( ) - TimerTickCounterContext ) + 1 )  * HW_TIMER_TIME_BASE );
}

uint64_t TimerHwGetTimerValue( void )
{
    uint64_t val = 0;

    __disable_irq( );

    val = TimerTickCounter;

    __enable_irq( );

    return( val );
}

void TimerIncrementTickCounter( void )
{
    __disable_irq( );

    TimerTickCounter++;

    __enable_irq( );
}

/*!
 * Timer IRQ handler
 */
void TIM2_IRQHandler( void )
{
    if( TIM_GetITStatus( TIM2, TIM_IT_Update ) != RESET )
    {
        TimerIncrementTickCounter( );
    
        if( TimerTickCounter == TimeoutCntValue )
        {
            TimerIrqHandler( );
        }
    
        TIM_ClearITPendingBit( TIM2, TIM_IT_Update );
    }
}

void TimerHwEnterLowPowerStopMode( void )
{
#ifndef USE_DEBUGGER
    __WFI( );
#endif
}
