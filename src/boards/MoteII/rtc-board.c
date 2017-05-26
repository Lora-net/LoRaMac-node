/*
  ______                              _
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
#include "rtc-board.h"

/*!
 * RTCCLK is 32768 Hz from LSE, or 37000 Hz from LSI
 *
 * It should be divided twice to get a 1 Hz clock:
 *
 * CK_SPRE = RTCCLK / ( PREDIV_A + 1 ) * ( PREDIV_S + 1 )
 *
 * LSE Example:
 * 1 = 32768 / ( 16 * 2048 )
 *
 * LSI Example:
 * 1 = 37000 / ( 37 * 1000 )
 *
 * PREDIV_A can be 1,2,3,..., or 127  (7-bit)
 * PREDIV_S can be 0,1,2,..., or 8191 (13-bit)
 *
 * The PREDIV_S + 1 value also becomes the number of subseconds in a second,
 * also note that subseconds are being counted from up to down
 */

/* Asynchronous prediv */
#define PREDIV_A                  ( 16 - 1 )

/* Synchronous prediv */
#define PREDIV_S                  ( 2048 - 1 )

/* RTC Time base in us */
#define USEC_NUMBER               1000000
#define MSEC_NUMBER               1000

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
 * Number of seconds in a leap year
 */
static const uint32_t SecondsInLeapYear = 31622400;

/*!
 * Number of seconds in a year
 */
static const uint32_t SecondsInYear = 31536000;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Flag used to indicates a the MCU has waken-up from an external IRQ
 */
volatile bool NonScheduledWakeUp = false;

/*!
 * RTC timer context
 */
typedef struct RtcCalendar_s
{
    RTC_DateTypeDef CalendarDate; //! Reference time in calendar format
    RTC_TimeTypeDef CalendarTime; //! Reference date in calendar format
} RtcCalendar_t;

/*!
 * Current RTC timer context
 */
RtcCalendar_t RtcCalendarContext;

/*!
 * \brief Flag to indicate if the timestamp until the next event is long enough
 * to set the MCU into low power mode
 */
static bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the low power mode even if the timestamp until the
 * next event is long enough to allow low power mode
 */
static bool LowPowerDisableDuringTask = false;

/*!
 * \brief RTC Handler
 */
RTC_HandleTypeDef RtcHandle = { 0 };

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitialized = false;

/*!
 * \brief Indicates if the RTC Wake Up Time is calibrated or not
 */
static bool WakeUpTimeInitialized = false;

/*!
 * \brief Hold the Wake-up time duration in ms
 */
volatile MsTime_t McuWakeUpTime = 0;

/*!
 * \brief RTC wakeup time computation
 */
static void RtcComputeWakeUpTime( void );

/*!
 * \brief Start the RTC Alarm (timeoutValue is in ms)
 */
static void RtcStartWakeUpAlarm( TimerTime_t timeoutValue );

/*!
 * \brief Converts a TimerTime_t value into RtcCalendar_t value
 *
 * \param[IN] timeCounter Value to convert to RTC calendar
 * \retval rtcCalendar New RTC calendar value
 */
//
// REMARK: Removed function static attribute in order to suppress
//         "#177-D function was declared but never referenced" warning.
// static RtcCalendar_t RtcConvertTimerTimeToCalendarTick( TimerTime_t timeCounter )
//
RtcCalendar_t RtcConvertTimerTimeToCalendarTick( TimerTime_t timeCounter );

/*!
 * \brief Converts a RtcCalendar_t value into TimerTime_t value
 *
 * \param[IN/OUT] calendar Calendar value to be converted
 *                         [NULL: compute from "now",
 *                          Others: compute from given calendar value]
 * \retval timerTime New TimerTime_t value
 */
static TimerTime_t RtcConvertCalendarTickToTimerTime( RtcCalendar_t *calendar );

static TimerTime_t RtcConvertMsToTick( MsTime_t timeoutValue );

static MsTime_t RtcConvertTickToMs( TimerTime_t timeoutValue );

/*!
 * \brief Converts a TimerTime_t value into a value for the RTC Alarm
 *
 * \param[IN] timeCounter Value in ms to convert into a calendar alarm date
 * \param[IN] now Current RTC calendar context
 * \retval rtcCalendar Value for the RTC Alarm
 */
static RtcCalendar_t RtcComputeTimerTimeToAlarmTick( TimerTime_t timeCounter, RtcCalendar_t now );

/*!
 * \brief Returns the internal RTC Calendar and check for RTC overflow
 *
 * \retval calendar RTC calendar
 */
static RtcCalendar_t RtcGetCalendar( void );

void RtcInit( void )
{
    RtcCalendar_t rtcInit;

    if( RtcInitialized == false )
    {
        __HAL_RCC_RTC_ENABLE( );

        RtcHandle.Instance = RTC;
        RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
        RtcHandle.Init.AsynchPrediv = PREDIV_A; // RTC_ASYNCH_PREDIV;
        RtcHandle.Init.SynchPrediv = PREDIV_S;  // RTC_SYNCH_PREDIV;

        RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
        RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
        if( HAL_RTC_Init( &RtcHandle ) != HAL_OK )
        {
            assert_param(FAIL);
        }

        if( HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0) != 0x32F2 )
        {
            // Set Date: Friday 1st of January 2000
            rtcInit.CalendarDate.Year = 0;
            rtcInit.CalendarDate.Month = RTC_MONTH_JANUARY;
            rtcInit.CalendarDate.Date = 1;
            rtcInit.CalendarDate.WeekDay = RTC_WEEKDAY_SATURDAY;
            if( HAL_RTC_SetDate( &RtcHandle, &rtcInit.CalendarDate, RTC_FORMAT_BIN ) != HAL_OK )
            {
                assert_param(FAIL);
            }

            // Set Time: 00:00:00
            rtcInit.CalendarTime.Hours = 0;
            rtcInit.CalendarTime.Minutes = 0;
            rtcInit.CalendarTime.Seconds = 0;
            rtcInit.CalendarTime.SecondFraction = 0;
            rtcInit.CalendarTime.TimeFormat = RTC_HOURFORMAT12_AM;
            rtcInit.CalendarTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
            rtcInit.CalendarTime.StoreOperation = RTC_STOREOPERATION_RESET;
            if( HAL_RTC_SetTime( &RtcHandle, &rtcInit.CalendarTime, RTC_FORMAT_BIN ) != HAL_OK )
            {
                assert_param(FAIL);
            }

            HAL_RTCEx_BKUPWrite( &RtcHandle, RTC_BKP_DR0, 0x32F2 );
        }

        // Enable Direct Read of the calendar registers (not through Shadow registers)
        HAL_RTCEx_EnableBypassShadow( &RtcHandle );

        // Enable the interrupt
        HAL_NVIC_SetPriority( RTC_IRQn, 1, 0 );
        HAL_NVIC_EnableIRQ( RTC_IRQn );

        RtcInitialized = true;
    }
}

void RtcSetTimeout( MsTime_t timeout )
{
    RtcStartWakeUpAlarm( RtcConvertMsToTick( timeout ) );
}

MsTime_t RtcGetAdjustedTimeoutValue( MsTime_t timeout )
{
    if( timeout > McuWakeUpTime )
    {   // we have waken up from a GPIO and we have lost "McuWakeUpTime" that we need to compensate on next event
        if( NonScheduledWakeUp == true )
        {
            NonScheduledWakeUp = false;
            timeout -= McuWakeUpTime;
        }
    }

    if( timeout > McuWakeUpTime )
    {   // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
        if( timeout < 50 ) // 50 ms
        {
            RtcTimerEventAllowsLowPower = false;
        }
        else
        {
            RtcTimerEventAllowsLowPower = true;
            timeout -= McuWakeUpTime;
        }
    }
    return  timeout;
}

MsTime_t RtcGetTimerValue( void )
{
    TimerTime_t retVal;

    retVal = RtcConvertCalendarTickToTimerTime( NULL );

    return( RtcConvertTickToMs( retVal ) );
}

MsTime_t RtcGetElapsedAlarmTime( void )
{
    TimerTime_t retVal;
    TimerTime_t currentTime;
    TimerTime_t contextTime;

    currentTime = RtcConvertCalendarTickToTimerTime( NULL );
    contextTime = RtcConvertCalendarTickToTimerTime( &RtcCalendarContext );

    if( currentTime < contextTime )
    {
        retVal = ( currentTime + ( TIMERTIME_MAX - contextTime ) );
    }
    else
    {
        retVal = ( currentTime - contextTime );
    }

    return( RtcConvertTickToMs( retVal ) );
}

MsTime_t RtcComputeFutureEventTime( MsTime_t futureEventInTime )
{
    return( RtcGetTimerValue( ) + futureEventInTime );
}

MsTime_t RtcComputeElapsedTime( MsTime_t eventInTime )
{
    MsTime_t elapsedTime;

    // Needed at boot, cannot compute with 0 or elapsed time will be equal to current time
    if( eventInTime == 0 )
    {
        return 0;
    }

    elapsedTime = RtcConvertTickToMs( RtcConvertCalendarTickToTimerTime( NULL ) );

    if( elapsedTime < eventInTime )
    { // roll over of the counter
        return( elapsedTime + ( MSTIME_MAX - eventInTime ) );
    }
    else
    {
        return( elapsedTime - eventInTime );
    }
}

void BlockLowPowerDuringTask ( bool status )
{
    if( status == true )
    {
        RtcRecoverMcuStatus( );
    }
    LowPowerDisableDuringTask = status;
}

void RtcEnterLowPowerStopMode( void )
{
    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        BoardDeInitMcu( );

        // Disable the Power Voltage Detector
        HAL_PWR_DisablePVD( );

        SET_BIT( PWR->CR, PWR_CR_CWUF );

        // Enable Ultra low power mode
        HAL_PWREx_EnableUltraLowPower( );

        // Enable the fast wake up from Ultra low power mode
        HAL_PWREx_EnableFastWakeUp( );

        // SysTick interrupt would wake up the MCU for every tick
        HAL_SuspendTick();

        // Enter Stop Mode        
#if defined(DEBUG)
        HAL_PWR_EnterSTOPMode( PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI );
#else
        HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
#endif

        // Resume the SysTick interrupt
        HAL_ResumeTick( );
    }
}

void RtcRecoverMcuStatus( void )
{
    // PWR_FLAG_WU indicates the Alarm has waken-up the MCU
    if( __HAL_PWR_GET_FLAG( PWR_FLAG_WU ) != RESET )
    {
        __HAL_PWR_CLEAR_FLAG( PWR_FLAG_WU );
    }
    else
    {
        NonScheduledWakeUp = true;
    }
    // check the clk source and set to full speed if we are coming from sleep mode
    if( ( __HAL_RCC_GET_SYSCLK_SOURCE( ) == RCC_SYSCLKSOURCE_STATUS_HSI ) ||
        ( __HAL_RCC_GET_SYSCLK_SOURCE( ) == RCC_SYSCLKSOURCE_STATUS_MSI ) )
    {
        BoardInitMcu( );
    }
}

static void RtcComputeWakeUpTime( void )
{
    TimerTime_t start;
    TimerTime_t stop;
    RTC_AlarmTypeDef alarmRtc;
    RtcCalendar_t now;

    if( WakeUpTimeInitialized == false )
    {
        now = RtcGetCalendar( );
        HAL_RTC_GetAlarm( &RtcHandle, &alarmRtc, RTC_ALARM_A, RTC_FORMAT_BIN );

        start = alarmRtc.AlarmTime.Hours    * 3600 * (PREDIV_S + 1);
        start += alarmRtc.AlarmTime.Minutes * 60   * (PREDIV_S + 1);
        start += alarmRtc.AlarmTime.Seconds * 1    * (PREDIV_S + 1);
        start += PREDIV_S - alarmRtc.AlarmTime.SubSeconds;

        stop = now.CalendarTime.Hours * 3600 * (PREDIV_S + 1);
        stop += now.CalendarTime.Minutes * 60 * (PREDIV_S + 1);
        stop += now.CalendarTime.Seconds * (PREDIV_S + 1);
        stop += PREDIV_S - now.CalendarTime.SubSeconds;

        McuWakeUpTime = RtcConvertTickToMs( stop - start );

        WakeUpTimeInitialized = true;
    }
}

static void RtcStartWakeUpAlarm( TimerTime_t timeoutValue )
{
    RtcCalendar_t now;
    RtcCalendar_t alarmTimer;
    RTC_AlarmTypeDef alarmStructure;

    HAL_RTC_DeactivateAlarm( &RtcHandle, RTC_ALARM_A );

    if( timeoutValue <= 3 )
    {
        timeoutValue = 3;
    }

    // Load the RTC calendar
    now = RtcGetCalendar( );

    // Save the calendar into RtcCalendarContext to be able to calculate the elapsed time
    RtcCalendarContext = now;

    // timeoutValue is in subseconds
    alarmTimer = RtcComputeTimerTimeToAlarmTick( timeoutValue, now );

    alarmStructure.Alarm = RTC_ALARM_A;
    alarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    alarmStructure.AlarmMask = RTC_ALARMMASK_NONE;
    alarmStructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    alarmStructure.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;

    alarmStructure.AlarmTime.SubSeconds = alarmTimer.CalendarTime.SubSeconds;
    alarmStructure.AlarmTime.Seconds = alarmTimer.CalendarTime.Seconds;
    alarmStructure.AlarmTime.Minutes = alarmTimer.CalendarTime.Minutes;
    alarmStructure.AlarmTime.Hours = alarmTimer.CalendarTime.Hours;
    alarmStructure.AlarmDateWeekDay = alarmTimer.CalendarDate.Date;

    if( HAL_RTC_SetAlarm_IT( &RtcHandle, &alarmStructure, RTC_FORMAT_BIN ) != HAL_OK )
    {
        assert_param( FAIL );
    }
}

static RtcCalendar_t RtcComputeTimerTimeToAlarmTick( TimerTime_t timeCounter, RtcCalendar_t now )
{
    RtcCalendar_t calendar = now;

    uint32_t subseconds = PREDIV_S - now.CalendarTime.SubSeconds;
    uint16_t seconds = now.CalendarTime.Seconds;
    uint16_t minutes = now.CalendarTime.Minutes;
    uint16_t hours = now.CalendarTime.Hours;
    uint16_t day = now.CalendarDate.Date;
    uint8_t month = now.CalendarDate.Month;
    uint16_t year = now.CalendarDate.Year;

    // Extract the subseconds part from the timeCounter
    subseconds += timeCounter % ( PREDIV_S + 1 );

    // Convert timeout to whole seconds
    timeCounter /= ( PREDIV_S + 1 );

    // Calculate the days
    while( timeCounter >= SecondsInDay )
    {
        timeCounter -= SecondsInDay;
        day++;
    }

    // Calculate the hours
    while( timeCounter >= SecondsInHour )
    {
        timeCounter -= SecondsInHour;
        hours++;
    }

    // Calculate the minutes
    while( timeCounter >= SecondsInMinute )
    {
        timeCounter -= SecondsInMinute;
        minutes++;
    }

    // Calculate the seconds
    seconds += timeCounter;
    while( subseconds >= ( PREDIV_S + 1 ) )
    {
        subseconds -= ( PREDIV_S + 1 );
        seconds++;
    }

    // Convert the calculated seconds and subseconds into a calendar

    while( seconds >= SecondsInMinute )
    {
        seconds -= SecondsInMinute;
        minutes++;
    }

    while( minutes >= 60 )
    {
        minutes -= 60;
        hours++;
    }

    while( hours >= HoursInDay )
    {
        hours -= HoursInDay;
        day++;
    }

    for ( ;; )
    {
        uint8_t cont = 0;

        if( year % 4 == 0 )
        {
            while( day > DaysInMonthLeapYear[month - 1] )
            {
                day -= DaysInMonthLeapYear[month - 1];
                month++;

                if( month > 12 )
                {
                    month -= 12;
                    year++;

                    cont = 1;
                    break;
                }
            }

            if( cont )
            {
                // Year was changed, start from the scratch
                continue;
            }

            // Calculations done
            break;
        }
        else
        {
            while( day > DaysInMonth[month - 1] )
            {
                day -= DaysInMonth[month - 1];
                month++;

                if( month > 12 )
                {
                    month -= 12;
                    year++;

                    cont = 1;
                    break;
                }
            }

            if( cont )
            {
                // Year was changed, start from the scratch
                continue;
            }

            // Calculations done
            break;
        }
    }

    calendar.CalendarTime.SubSeconds = PREDIV_S - subseconds;
    calendar.CalendarTime.Seconds = seconds;
    calendar.CalendarTime.Minutes = minutes;
    calendar.CalendarTime.Hours = hours;
    calendar.CalendarDate.Date = day;
    calendar.CalendarDate.Month = month;
    calendar.CalendarDate.Year = year;

    return calendar;
}

//
// REMARK: Removed function static attribute in order to suppress
//         "#177-D function was declared but never referenced" warning.
// static RtcCalendar_t RtcConvertTimerTimeToCalendarTick( TimerTime_t timeCounter )
//
RtcCalendar_t RtcConvertTimerTimeToCalendarTick( TimerTime_t timeCounter )
{
    RtcCalendar_t calendar = { { 0 }, { 0 } };

    uint32_t subseconds;
    uint16_t seconds = 0;
    uint16_t minutes = 0;
    uint16_t hours = 0;
    uint16_t day = 0;
    uint8_t month = 1; // Start at 1, month 0 does not exist
    uint16_t year = 0;

    // Extract the subseconds part from the timeCounter
    subseconds = timeCounter % ( PREDIV_S + 1 );

    // Convert timeout to whole seconds
    timeCounter /= ( PREDIV_S + 1 );

    // Calculate the day
    while( timeCounter >= SecondsInDay )
    {
        timeCounter -= SecondsInDay;
        day++;
    }

    // Calculate the hours
    while( timeCounter >= SecondsInHour )
    {
        timeCounter -= SecondsInHour;
        hours++;
    }

    // Calculate the minutes
    while( timeCounter >= SecondsInMinute )
    {
        timeCounter -= SecondsInMinute;
        minutes++;
    }

    // Calculate the seconds
    seconds += timeCounter;
    while( subseconds >= ( PREDIV_S + 1 ) )
    {
        subseconds -= ( PREDIV_S + 1 );
        seconds++;
    }

    // Convert the calculated seconds and subseconds into a calendar

    while( seconds >= SecondsInMinute )
    {
        seconds -= SecondsInMinute;
        minutes++;
    }

    while( minutes >= 60 )
    {
        minutes -= 60;
        hours++;
    }

    while( hours >= HoursInDay )
    {
        hours -= HoursInDay;
        day++;
    }

    while( hours >= HoursInDay )
    {
        hours -= HoursInDay;
        day++;
    }

    for ( ;; )
    {
        uint8_t cont = 0;

        if( year % 4 == 0 )
        {
            while( day > DaysInMonthLeapYear[month - 1] )
            {
                day -= DaysInMonthLeapYear[month - 1];
                month++;

                if( month > 12 )
                {
                    month -= 12;
                    year++;

                    cont = 1;
                    break;
                }
            }

            if( cont )
            {
                // Year was changed, start from the scratch
                continue;
            }

            // Calculations done
            break;
        }
        else
        {
            while( day > DaysInMonth[month - 1] )
            {
                day -= DaysInMonth[month - 1];
                month++;

                if( month > 12 )
                {
                    month -= 12;
                    year++;

                    cont = 1;
                    break;
                }
            }

            if( cont )
            {
                // Year was changed, start from the scratch
                continue;
            }

            // Calculations done
            break;
        }
    }

    calendar.CalendarTime.SubSeconds = PREDIV_S - subseconds;
    calendar.CalendarTime.Seconds = seconds;
    calendar.CalendarTime.Minutes = minutes;
    calendar.CalendarTime.Hours = hours;
    calendar.CalendarDate.Date = day;
    calendar.CalendarDate.Month = month;
    calendar.CalendarDate.Year = year;

    return calendar;
}

static TimerTime_t RtcConvertCalendarTickToTimerTime( RtcCalendar_t *calendar )
{
    TimerTime_t timeCounter = 0;
    RtcCalendar_t now;

    // Passing a NULL pointer will compute from "now" else,
    // compute from the given calendar value
    if( calendar == NULL )
    {
        now = RtcGetCalendar( );
    }
    else
    {
        now = *calendar;
    }

    // Years (calculation valid up to year 2099)
    for( uint8_t i = 0; i < now.CalendarDate.Year ; i++ )
    {
        if( i % 4 == 0 )
        {
            timeCounter += SecondsInLeapYear;
        }
        else
        {
            timeCounter += SecondsInYear;
        }
    }

    // Months (calculation valid up to year 2099)*/
    if( now.CalendarDate.Year % 4 == 0 )
    {
        for( uint8_t i = 0; i < ( now.CalendarDate.Month - 1 ); i++ )
        {
            timeCounter += ( DaysInMonthLeapYear[i] * SecondsInDay );
        }
    }
    else
    {
        for( uint8_t i = 0;  i < ( now.CalendarDate.Month - 1 ); i++ )
        {
            timeCounter += ( DaysInMonth[i] * SecondsInDay );
        }
    }

    timeCounter += ( now.CalendarTime.Seconds +
                     ( now.CalendarTime.Minutes * SecondsInMinute ) +
                     ( now.CalendarTime.Hours * SecondsInHour ) +
                     ( now.CalendarDate.Date * SecondsInDay ) );

    timeCounter *= (PREDIV_S + 1);

    timeCounter += PREDIV_S - now.CalendarTime.SubSeconds;

    return ( timeCounter );
}

TimerTime_t RtcConvertMsToTick( MsTime_t timeoutValue )
{
#if (PREDIV_S + 1 == MSEC_NUMBER)
    return ( TimerTime_t )timeoutValue;
#else
    double retVal = round( ( double )( timeoutValue * ( PREDIV_S + 1 ) ) / MSEC_NUMBER );
    return( ( TimerTime_t )retVal );
#endif
}

MsTime_t RtcConvertTickToMs( TimerTime_t timeoutValue )
{
#if (PREDIV_S + 1 == MSEC_NUMBER)
    return ( MsTime_t )timeoutValue;
#else
    double retVal = round( ( double ) ( timeoutValue * MSEC_NUMBER ) / ( PREDIV_S + 1 ) );
    return( ( MsTime_t )retVal );
#endif
}

static RtcCalendar_t RtcGetCalendar( void )
{
    uint32_t first_read = 0;
    uint32_t second_read = 0;
    RtcCalendar_t now;

    // Get Time and Date
    HAL_RTC_GetTime( &RtcHandle, &now.CalendarTime, RTC_FORMAT_BIN );
    first_read = now.CalendarTime.SubSeconds;
    HAL_RTC_GetTime( &RtcHandle, &now.CalendarTime, RTC_FORMAT_BIN );
    second_read = now.CalendarTime.SubSeconds;

    // make sure it is correct due to asynchronous nature of RTC
    while( first_read != second_read )
    {
        first_read = second_read;
        HAL_RTC_GetTime( &RtcHandle, &now.CalendarTime, RTC_FORMAT_BIN );
        second_read = now.CalendarTime.SubSeconds;
    }
    HAL_RTC_GetDate( &RtcHandle, &now.CalendarDate, RTC_FORMAT_BIN );
    return( now );
}

/*!
 * \brief RTC IRQ Handler of the RTC Alarm
 */
void RTC_IRQHandler( void )
{
    HAL_RTC_AlarmIRQHandler( &RtcHandle );
    HAL_RTC_DeactivateAlarm( &RtcHandle, RTC_ALARM_A );
    RtcRecoverMcuStatus( );
    RtcComputeWakeUpTime( );
    BlockLowPowerDuringTask( false );
    TimerIrqHandler( );
}
