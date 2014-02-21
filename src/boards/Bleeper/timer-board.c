/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

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
#define HW_TIMER_TIME_BASE                              10 

/*!
 * Hardware timer counter
 */
volatile uint32_t TimeoutCntValue;

void TimerHwInit( void )
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /* TIM2 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* --------------------------NVIC Configuration -------------------------------*/
    /* Enable the TIM2 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    TimeoutCntValue = 0;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 319;
    TIM_TimeBaseStructure.TIM_Prescaler = 0; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /* Output Compare Timing Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    /* TIM IT enable */
    TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

    /* TIM2 disable counter */
    TIM_Cmd(TIM2, DISABLE); 
}

void TimerHwDeInit( void )
{
    /* Deinitialize the timer */
    TIM_DeInit( TIM2 );
}

void TimerHwStart( uint32_t val )
{
    TimeoutCntValue = ( val - 1 ) / HW_TIMER_TIME_BASE;
    TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
    TIM_Cmd( TIM2, ENABLE );  
}

void TimerHwStop( void )
{
    TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
    TIM_Cmd( TIM2, DISABLE );  
}

uint32_t TimerHwGetTimerValue( void )
{
     return ( TimeoutCntValue * HW_TIMER_TIME_BASE );
}

/*!
 * Timer IRQ handler
 */
void TIM2_IRQHandler( void )
{
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
    
        TimeoutCntValue--;
    
        if( TimeoutCntValue == 0 )
        {
            TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
            TIM_Cmd( TIM2, DISABLE );  
            TimerIrqHandler( );
        }
    }
}
