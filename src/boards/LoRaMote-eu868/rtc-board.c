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
#include "rtc-board.h"

/*!
 * RTC Time base in us
 */
#define RTC_ALARM_TIME_BASE                             122.07

/*!
 * MCU Wake Up Time
 */
#define MCU_WAKE_UP_TIME                                3400

/*!
 * \brief Configure the Rtc hardware
 */
static void RtcSetConfig( void );

/*!
 * \brief Configure the Rtc Alarm
 */
static void RtcSetAlarmConfig( void );

/*!
 * \brief Start the Rtc Alarm (time base 1s)
 */
static void RtcStartWakeUpAlarm( uint32_t timeoutValue );

/*!
 * \brief Clear the RTC flags and Stop all IRQs
 */
static void RtcClearStatus( void );

/*!
 * \brief Indicates if the RTC is already Initalized or not
 */
static bool RtcInitalized = false;

/*!
 * \brief Indicates if the RTC is used for Time measure or for 
 */
static bool LowPowerEnable = false;

/*!
 * Value of the RTC registers saved when the RTC is stopped
 */
static uint32_t timerDateValue = 0;
static uint32_t timerTimeValue = 0;


void RtcInit( void )
{
    if( RtcInitalized == false )
    {
        RtcSetConfig( );
        RtcSetAlarmConfig( );
        RtcInitalized = true;
    }
}

static void RtcSetConfig( void )
{
    RTC_InitTypeDef RTC_InitStructure;

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE );

    /* Allow access to RTC */
    PWR_RTCAccessCmd( ENABLE );

    /* Reset RTC Domain */
    RCC_RTCResetCmd( ENABLE );
    RCC_RTCResetCmd( DISABLE );

    /* Enable the LSE OSC */
    RCC_LSEConfig( RCC_LSE_ON );

    /* Wait till LSE is ready */  
    while( RCC_GetFlagStatus( RCC_FLAG_LSERDY ) == RESET )
    {}

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig( RCC_RTCCLKSource_LSE );

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd( ENABLE );
    
    /* Configure the RTC data register and RTC prescaler */
    RTC_InitStructure.RTC_AsynchPrediv = 0x01;
    RTC_InitStructure.RTC_SynchPrediv  = 0x01;
    RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
    RTC_Init( &RTC_InitStructure );
    
    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro( );
}

static void RtcSetAlarmConfig( void )
{
    EXTI_InitTypeDef EXTI_InitStructure;
    RTC_AlarmTypeDef RTC_AlarmStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* EXTI configuration */
    EXTI_ClearITPendingBit( EXTI_Line17 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init( &EXTI_InitStructure );

    /* Enable the RTC Alarm Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    /* Set the alarmA Masks */
    RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
    RTC_SetAlarm( RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure );

    /* Enable AlarmA interrupt */
    RTC_ITConfig( RTC_IT_ALRA, DISABLE );

    /* Enable the alarmA */
    RTC_AlarmCmd( RTC_Alarm_A, DISABLE );
}


void RtcStopTimer( void )
{
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

    // Check if the Alarm A is Enable or not
    if( ( RTC->CR & RTC_Alarm_A ) == RTC_Alarm_A )
    {
        RTC_WaitForSynchro( );

        RTC_GetTime( RTC_Format_BIN, &RTC_TimeStruct ); 
        RTC_GetDate( RTC_Format_BIN, &RTC_DateStruct ); 

        timerTimeValue = ( RTC_TimeStruct.RTC_Hours * 3600 ) + ( RTC_TimeStruct.RTC_Minutes * 60 ) +
                        RTC_TimeStruct.RTC_Seconds;

        timerDateValue = ( RTC_DateStruct.RTC_Date - 1 ) * 86400; // number of second in 1 day

    }

    RtcClearStatus( );

    RCC_RTCCLKCmd( DISABLE ); 
}

void RtcSetTimeout( uint32_t timeout )
{
    uint32_t timeoutValue = 0;

    if( timeout < 50000 ) 
    {
        // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
        LowPowerEnable = false;
        timeoutValue = timeout; 
    }
    else
    {
        LowPowerEnable = true;
        timeoutValue = timeout - MCU_WAKE_UP_TIME;  
    }
    RtcStartWakeUpAlarm( timeoutValue ); 
}

uint32_t RtcGetTimerValue( void )
{
    uint32_t timerValue = 0;

    timerValue = ( timerDateValue + timerTimeValue ) * RTC_ALARM_TIME_BASE;
    
    return( timerValue );
}

static void RtcClearStatus( void )
{
    PWR_RTCAccessCmd( ENABLE );

    /* Clear RTC Alarm Flag */
    RTC_ClearFlag(RTC_FLAG_ALRAF);

    /* Enable RTC Alarm A Interrupt */
    RTC_ITConfig( RTC_IT_ALRA, DISABLE );
  
    /* Enable the Alarm A */
    RTC_AlarmCmd( RTC_Alarm_A, DISABLE );  
}

static void RtcStartWakeUpAlarm( uint32_t timeoutValue )
{
    uint8_t rtcSeconds = 0;
    uint8_t rtcMinutes = 0;
    uint8_t rtcHours = 0;
    uint8_t rtcDays = 0;

    RTC_AlarmTypeDef RTC_AlarmStructure;
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

    RtcStopTimer( );
    
    timeoutValue = timeoutValue / RTC_ALARM_TIME_BASE;

    if( timeoutValue >= 86400 )
    {
        rtcDays = ( 1 + ( timeoutValue / 86400 ) ) % 31;
        rtcHours = ( timeoutValue / 3600 ) % 24;
        rtcMinutes = ( timeoutValue / 60 ) % 60;
        rtcSeconds = timeoutValue % 60;
    }
    else if( timeoutValue >= 3600 )
    {
        rtcDays = 1;
        rtcHours = ( timeoutValue / 3600 ) % 24;
        rtcMinutes = ( timeoutValue / 60 ) % 60;
        rtcSeconds = timeoutValue % 60;
    }
    else if( timeoutValue >= 60 )
    {
        rtcDays = 1;
        rtcHours = 0;
        rtcMinutes = ( timeoutValue / 60 ) % 60;
        rtcSeconds = timeoutValue % 60;
    }
    else
    {
        rtcDays = 1;
        rtcHours = 0;
        rtcMinutes = 0;
        rtcSeconds = timeoutValue;
    }

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd( ENABLE );

    /* Set the alarm */
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_TimeStruct.RTC_H12;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = rtcHours;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = rtcMinutes;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = rtcSeconds;
    RTC_AlarmStructure.RTC_AlarmDateWeekDay      = rtcDays;
    RTC_AlarmStructure.RTC_AlarmDateWeekDaySel   = RTC_AlarmDateWeekDaySel_Date;
    RTC_AlarmStructure.RTC_AlarmMask             = RTC_AlarmMask_None;
    RTC_SetAlarm( RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure );
  
    /* Set the time to Monday, January 01 xx00 */
    RTC_DateStructInit( &RTC_DateStruct );
    RTC_SetDate( RTC_Format_BIN, &RTC_DateStruct );

    /* Set the time to 00h 00mn 00s AM */
    RTC_TimeStructInit( &RTC_TimeStruct );
    RTC_SetTime( RTC_Format_BIN, &RTC_TimeStruct );

    /* Enable RTC Alarm A Interrupt */
    RTC_ITConfig( RTC_IT_ALRA, ENABLE );
  
    /* Enable the Alarm A */
    RTC_AlarmCmd( RTC_Alarm_A, ENABLE );
}

void RtcEnterLowPowerStopMode( void )
{   
    if( LowPowerEnable == true )
    {
        BoardDeInitMcu( );

        /* Disable the Power Voltage Detector */
        PWR_PVDCmd( DISABLE );

        /* Set MCU in ULP (Ultra Low Power) */
        PWR_UltraLowPowerCmd( ENABLE ); // add up to 3ms wakeup time

        /*Disable fast wakeUp*/
        PWR_FastWakeUpCmd( DISABLE );

        /* Enter Stop Mode */
        PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI );
    }
}

void RtcRecoverMcuStatus( void )
{
#ifdef LOW_POWER_MODE_ENABLE
    if( LowPowerEnable == true )
    {
        // Disable IRQ while the MCU is not running on HSE
        __disable_irq( );

        /* After wake-up from STOP reconfigure the system clock */
        /* Enable HSE */
        RCC_HSEConfig( RCC_HSE_ON );
        
        /* Wait till HSE is ready */
        while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
        {}
        
        /* Enable PLL */
        RCC_PLLCmd( ENABLE );
        
        /* Wait till PLL is ready */
        while( RCC_GetFlagStatus( RCC_FLAG_PLLRDY ) == RESET )
        {}
        
        /* Select PLL as system clock source */
        RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );
        
        /* Wait till PLL is used as system clock source */
        while( RCC_GetSYSCLKSource( ) != 0x0C )
        {}

        /* Set MCU in ULP (Ultra Low Power) */
        PWR_UltraLowPowerCmd( DISABLE ); // add up to 3ms wakeup time
        
        /* Enable the Power Voltage Detector */
        PWR_PVDCmd( ENABLE );
            
        BoardInitMcu( );  
                    
        __enable_irq( );   
    }
#endif
}

/*!
 * \brief RTC IRQ Handler on the RTC Alarm
 */
void RTC_Alarm_IRQHandler( void )
{
    if( RTC_GetITStatus( RTC_IT_ALRA ) != RESET )
    {   
        RtcRecoverMcuStatus( );
    
        TimerIrqHandler( );
    
        RTC_ClearITPendingBit( RTC_IT_ALRA );
        EXTI_ClearITPendingBit( EXTI_Line17 );  
    }
}

void BlockLowPowerDuringTask ( bool Status )
{
    if( Status == true )
    {
        RtcRecoverMcuStatus( );
        LowPowerEnable = false; 
    }
    else
    {
        LowPowerEnable = true;
    }
}
