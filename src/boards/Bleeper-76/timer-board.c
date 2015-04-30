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
volatile TimerTime_t TimerTickCounter = 1;

/*!
 * Saved value of the Tick counter at the start of the next event
 */
static TimerTime_t TimerTickCounterContext = 0;

/*!
 * Value trigging the IRQ
 */
volatile TimerTime_t TimeoutCntValue = 0;

/*!
 * Increment the Hardware Timer tick counter
 */
void TimerIncrementTickCounter( void );

/*!
 * Counter used for the Delay operations
 */
volatile uint32_t TimerDelayCounter = 0;

/*!
 * Retunr the value of the counter used for a Delay
 */
uint32_t TimerHwGetDelayValue( void );

/*!
 * Increment the value of TimerDelayCounter
 */
void TimerIncrementDelayCounter( void );


void TimerHwInit( void )
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* TIM2 clock enable */ 
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );

    /* --------------------------NVIC Configuration -------------------------------*/
    /* Enable the TIM2 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init( &NVIC_InitStructure );

    TimeoutCntValue = 0;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 3199;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );

    TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );

    /* TIM2 disable counter */
    TIM_Cmd( TIM2, ENABLE );
        
        /* TIM3 clock enable */ 
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );

    /* --------------------------NVIC Configuration -------------------------------*/
    /* Enable the TIM3 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init( &NVIC_InitStructure );

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 3199;
    TIM_TimeBaseStructure.TIM_Prescaler = 10;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit( TIM3, &TIM_TimeBaseStructure );

    TIM_ITConfig( TIM3, TIM_IT_Update, DISABLE );

    /* TIM3 disable counter */
    TIM_Cmd( TIM3, DISABLE );


    TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE );
    TIM_Cmd( TIM3, ENABLE );
    
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
    uint32_t delayValue = 0;

    delayValue = delay;

    TimerDelayCounter = 0;

    TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE );
    TIM_Cmd( TIM3, ENABLE );

    while( TimerHwGetDelayValue( ) < delayValue )
    {
    }

    TIM_ITConfig( TIM3, TIM_IT_Update, DISABLE );
    TIM_Cmd( TIM3, DISABLE );
}

TimerTime_t TimerHwGetElapsedTime( void )
{
     return( ( ( TimerHwGetTimerValue( ) - TimerTickCounterContext ) + 1 )  * HW_TIMER_TIME_BASE );
}

TimerTime_t TimerHwGetTimerValue( void )
{
    TimerTime_t val = 0;

    __disable_irq( );

    val = TimerTickCounter;

    __enable_irq( );

    return( val );
}

TimerTime_t TimerHwGetTime( void )
{

    return TimerHwGetTimerValue( ) * HW_TIMER_TIME_BASE;
}

uint32_t TimerHwGetDelayValue( void )
{
    uint32_t val = 0;

    __disable_irq( );

    val = TimerDelayCounter;

    __enable_irq( );

    return( val );
}

void TimerIncrementTickCounter( void )
{
    __disable_irq( );

    TimerTickCounter++;

    __enable_irq( );
}

void TimerIncrementDelayCounter( void )
{
    __disable_irq( );

    TimerDelayCounter++;

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
        TIM_ClearITPendingBit( TIM2, TIM_IT_Update );
    
        if( TimerTickCounter == TimeoutCntValue )
        {
            TimerIrqHandler( );
        }
    }
}

/*!
 * Timer IRQ handler
 */
void TIM3_IRQHandler( void )
{
    if( TIM_GetITStatus( TIM3, TIM_IT_Update ) != RESET )
    {
        TimerIncrementDelayCounter( );
        TIM_ClearITPendingBit( TIM3, TIM_IT_Update );
    }
}

void TimerHwEnterLowPowerStopMode( void )
{
#ifndef USE_DEBUGGER
    __WFI( );
#endif
}
