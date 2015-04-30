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
#include <time.h>
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
 * \brief Read the MCU internal Calendar value
 *
 * \retval Calendar value
 */
static TimerTime_t RtcGetCalendarValue( void );

/*!
 * \brief Clear the RTC flags and Stop all IRQs
 */
static void RtcClearStatus( void );

/*!
 * \brief Indicates if the RTC is already Initalized or not
 */
static bool RtcInitalized = false;

/*!
 * \brief Flag to indicate if the timestamps until the next event is long enough 
 * to set the MCU into low power mode
 */
static bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the LowPower Mode even if the timestamps until the
 * next event is long enough to allow Low Power mode 
 */
static bool LowPowerDisableDuringTask = false;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 */
static TimerTime_t RtcTimerContext = 0;

/*!
 * Number of seconds in a minute
 */
static const uint8_t SecondsInMinute = 60;

/*!
 * Number of seconds in an hour
 */
static const uint16_t SecondsInHour = 3600;

/*!
 * Number of seconds in a day
 */
static const uint32_t SecondsInDay = 86400;

/*!
 * Number of hours in a day
 */
static const uint8_t HoursInDay = 24;

/*!
 * Number of days in a standard year
 */
static const uint16_t DaysInYear = 365;

/*!
 * Number of days in a leap year
 */
static const uint16_t DaysInLeapYear = 366;

/*!
 * Number of days in a century
 */
static const double DaysInCentury = 36524.219;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Hold the previous year value to detect the turn of a century
 */
static uint8_t PreviousYear = 0;

/*!
 * Century counter
 */
static uint8_t Century = 0;

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
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

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
   
    RTC_TimeStructInit( &RTC_TimeStruct );
    RTC_DateStructInit( &RTC_DateStruct );
    
    RTC_SetDate( RTC_Format_BIN, &RTC_DateStruct );
    RTC_SetTime( RTC_Format_BIN, &RTC_TimeStruct );
    
    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro( );
    
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
    RtcClearStatus( );
}

uint32_t RtcGetMinimumTimeout( void )
{
    return( ceil( 3 * RTC_ALARM_TIME_BASE ) );
}

void RtcSetTimeout( uint32_t timeout )
{
    uint32_t timeoutValue = 0;

    timeoutValue = timeout;

    if( timeoutValue < ( 3 * RTC_ALARM_TIME_BASE ) )
    {
        timeoutValue = 3 * RTC_ALARM_TIME_BASE;
    }
    
    if( timeoutValue < 55000 ) 
    {
        // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
        RtcTimerEventAllowsLowPower = false;
    }
    else
    {
        RtcTimerEventAllowsLowPower = true;
    }

    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        timeoutValue = timeoutValue - MCU_WAKE_UP_TIME;
    }

    RtcStartWakeUpAlarm( timeoutValue );
}


uint32_t RtcGetTimerElapsedTime( void )
{
    TimerTime_t CalendarValue = 0;

    CalendarValue = RtcGetCalendarValue( );

    return( ( uint32_t )( ceil ( ( ( CalendarValue - RtcTimerContext ) + 2 ) * RTC_ALARM_TIME_BASE ) ) );
}

TimerTime_t RtcGetTimerValue( void )
{
    TimerTime_t CalendarValue = 0;

    CalendarValue = RtcGetCalendarValue( );

    return( ( CalendarValue + 2 ) * RTC_ALARM_TIME_BASE );
}

static void RtcClearStatus( void )
{
    /* Clear RTC Alarm Flag */
    RTC_ClearFlag(RTC_FLAG_ALRAF);

    /* Enable RTC Alarm A Interrupt */
    RTC_ITConfig( RTC_IT_ALRA, DISABLE );
  
    /* Enable the Alarm A */
    RTC_AlarmCmd( RTC_Alarm_A, DISABLE );
}

static void RtcStartWakeUpAlarm( uint32_t timeoutValue )
{
    uint16_t rtcSeconds = 0;
    uint16_t rtcMinutes = 0;
    uint16_t rtcHours = 0;
    uint16_t rtcDays = 0;

    uint8_t rtcAlarmSeconds = 0;
    uint8_t rtcAlarmMinutes = 0;
    uint8_t rtcAlarmHours = 0;
    uint16_t rtcAlarmDays = 0;

    RTC_AlarmTypeDef RTC_AlarmStructure;
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

    RtcClearStatus( );

    RtcTimerContext = RtcGetCalendarValue( );
    RTC_GetTime( RTC_Format_BIN, &RTC_TimeStruct );
    RTC_GetDate( RTC_Format_BIN, &RTC_DateStruct );
       
    timeoutValue = timeoutValue / RTC_ALARM_TIME_BASE;

    if( timeoutValue > 2160000 ) // 25 "days" in tick 
    {                            // drastically reduce the computation time
        rtcAlarmSeconds = RTC_TimeStruct.RTC_Seconds;
        rtcAlarmMinutes = RTC_TimeStruct.RTC_Minutes;
        rtcAlarmHours = RTC_TimeStruct.RTC_Hours;
        rtcAlarmDays = 25 + RTC_DateStruct.RTC_Date;  // simply add 25 days to current date and time

        if( ( RTC_DateStruct.RTC_Year == 0 ) || ( RTC_DateStruct.RTC_Year % 4 == 0 ) )
        {
            if( rtcAlarmDays > DaysInMonthLeapYear[ RTC_DateStruct.RTC_Month - 1 ] )
            {   
                rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_DateStruct.RTC_Month - 1];
            }
        }
        else
        {
            if( rtcAlarmDays > DaysInMonth[ RTC_DateStruct.RTC_Month - 1 ] )
            {   
                rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_DateStruct.RTC_Month - 1];
            }
        }   
    }
    else
    {
        rtcSeconds = ( timeoutValue % SecondsInMinute ) + RTC_TimeStruct.RTC_Seconds;
        rtcMinutes = ( ( timeoutValue / SecondsInMinute ) % SecondsInMinute ) + RTC_TimeStruct.RTC_Minutes;
        rtcHours = ( ( timeoutValue / SecondsInHour ) % HoursInDay ) + RTC_TimeStruct.RTC_Hours;
        rtcDays = ( timeoutValue / SecondsInDay ) + RTC_DateStruct.RTC_Date;

        rtcAlarmSeconds = ( rtcSeconds ) % 60;
        rtcAlarmMinutes = ( ( rtcSeconds / 60 ) + rtcMinutes ) % 60;
        rtcAlarmHours   = ( ( ( ( rtcSeconds / 60 ) + rtcMinutes ) / 60 ) + rtcHours ) % 24;
        rtcAlarmDays    = ( ( ( ( ( rtcSeconds / 60 ) + rtcMinutes ) / 60 ) + rtcHours ) / 24 ) + rtcDays;

        if( ( RTC_DateStruct.RTC_Year == 0 ) || ( RTC_DateStruct.RTC_Year % 4 == 0 ) )
        {
            if( rtcAlarmDays > DaysInMonthLeapYear[ RTC_DateStruct.RTC_Month - 1 ] )            
            {   
                rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_DateStruct.RTC_Month - 1 ];
            }
        }
        else
        {
            if( rtcAlarmDays > DaysInMonth[ RTC_DateStruct.RTC_Month - 1 ] )            
            {   
                rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_DateStruct.RTC_Month - 1 ];
            }
        }
    }

    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = rtcAlarmSeconds;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = rtcAlarmMinutes;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = rtcAlarmHours;
    RTC_AlarmStructure.RTC_AlarmDateWeekDay      = ( uint8_t )rtcAlarmDays;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_TimeStruct.RTC_H12;
    RTC_AlarmStructure.RTC_AlarmDateWeekDaySel   = RTC_AlarmDateWeekDaySel_Date;
    RTC_AlarmStructure.RTC_AlarmMask             = RTC_AlarmMask_None;
    RTC_SetAlarm( RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure );

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro( );
    
    /* Enable RTC Alarm A Interrupt */
    RTC_ITConfig( RTC_IT_ALRA, ENABLE );
  
    /* Enable the Alarm A */
    RTC_AlarmCmd( RTC_Alarm_A, ENABLE );
}

void RtcEnterLowPowerStopMode( void )
{   
    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {   
        // Disable IRQ while the MCU is being deinitialized to prevent race issues
        __disable_irq( );
    
        BoardDeInitMcu( );
    
        __enable_irq( );
    
        /* Disable the Power Voltage Detector */
        PWR_PVDCmd( DISABLE );

        /* Set MCU in ULP (Ultra Low Power) */
        PWR_UltraLowPowerCmd( ENABLE );

        /*Disable fast wakeUp*/
        PWR_FastWakeUpCmd( DISABLE );

        /* Enter Stop Mode */
        PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI );
    }
}

void RtcRecoverMcuStatus( void )
{    
    if( TimerGetLowPowerEnable( ) == true )
    {
        if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
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
    }
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

void BlockLowPowerDuringTask( bool status )
{
    if( status == true )
    {
        RtcRecoverMcuStatus( );
    }
    LowPowerDisableDuringTask = status;
}

void RtcDelayMs( uint32_t delay )
{
    TimerTime_t delayValue = 0;
    TimerTime_t timeout = 0;

    delayValue = ( TimerTime_t )( delay * 1000 );

    // Wait delay ms
    timeout = RtcGetTimerValue( );
    while( ( ( RtcGetTimerValue( ) - timeout ) ) < delayValue )
    {
        __NOP( );
    }
}

TimerTime_t RtcGetCalendarValue( void )
{
    TimerTime_t calendarValue = 0;
    uint8_t i = 0;

    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

    RTC_GetTime( RTC_Format_BIN, &RTC_TimeStruct );
    RTC_GetDate( RTC_Format_BIN, &RTC_DateStruct );

    RTC_WaitForSynchro( );

    if( ( PreviousYear == 99 ) && ( RTC_DateStruct.RTC_Year == 0 ) )
    {
        Century++;
    }
    PreviousYear = RTC_DateStruct.RTC_Year;

    // century
    for( i = 0; i < Century; i++ )
    {
        calendarValue += ( TimerTime_t )( DaysInCentury * SecondsInDay );
    }

    // years
    for( i = 0; i < RTC_DateStruct.RTC_Year; i++ )
    {
        if( ( i == 0 ) || ( i % 4 == 0 ) )
        {
            calendarValue += DaysInLeapYear * SecondsInDay;
        }
        else
        {
            calendarValue += DaysInYear * SecondsInDay;
        }
    }

    // months
    if( ( RTC_DateStruct.RTC_Year == 0 ) || ( RTC_DateStruct.RTC_Year % 4 == 0 ) )
    {
        for( i = 0; i < ( RTC_DateStruct.RTC_Month - 1 ); i++ )
        {
            calendarValue += DaysInMonthLeapYear[i] * SecondsInDay;
        }
    }
    else
    {
        for( i = 0;  i < ( RTC_DateStruct.RTC_Month - 1 ); i++ )
        {
            calendarValue += DaysInMonth[i] * SecondsInDay;
        }
    }       

    // days
    calendarValue += ( ( uint32_t )RTC_TimeStruct.RTC_Seconds + 
                      ( ( uint32_t )RTC_TimeStruct.RTC_Minutes * SecondsInMinute ) +
                      ( ( uint32_t )RTC_TimeStruct.RTC_Hours * SecondsInHour ) + 
                      ( ( uint32_t )( RTC_DateStruct.RTC_Date * SecondsInDay ) ) );

    return( calendarValue );
}
