/*!
 * \file      rtc-board.c
 *
 * \brief     Target board RTC timer and low power modes management
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
#include <math.h>
#include <time.h>
#include "stm32l0xx.h"
#include "utilities.h"
#include "delay.h"
#include "board.h"
#include "timer.h"
#include "gpio.h"
#include "rtc-board.h"

/*!
 * RTC Time base in ms
 */
#define RTC_ALARM_TICK_PER_MS                       0x7FF           //  2047 > number of sub-second ticks per second

/* sub-second number of bits */
#define N_PREDIV_S                11

/* Synchronous prediv  */
#define PREDIV_S                  ( ( 1 << N_PREDIV_S ) - 1 )

/* Asynchronous prediv   */
#define PREDIV_A                  ( 1 << ( 15 - N_PREDIV_S ) ) - 1

/* RTC Time base in us */
#define USEC_NUMBER               1000000
#define MSEC_NUMBER               ( USEC_NUMBER / 1000 )
#define RTC_ALARM_TIME_BASE       ( USEC_NUMBER >> N_PREDIV_S )

#define COMMON_FACTOR             3
#define CONV_NUMER                ( MSEC_NUMBER >> COMMON_FACTOR )
#define CONV_DENOM                ( 1 << ( N_PREDIV_S - COMMON_FACTOR ) )

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
volatile uint32_t McuWakeUpTime = 0;

/*!
 * \brief RTC wakeup time computation
 */
static void RtcComputeWakeUpTime( void );

/*!
 * \brief Start the RTC Alarm (timeoutValue is in ms)
 */
static void RtcStartWakeUpAlarm( uint32_t timeoutValue );

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

static TimerTime_t RtcConvertMsToTick( TimerTime_t timeoutValue );

static TimerTime_t RtcConvertTickToMs( TimerTime_t timeoutValue );

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

/*
 * ============================================================================
 * BEGIN of mktime and localtime functions implmentation
 *
 * These functions implementation is based on mbed os mbed_mktime.h and
 * mbed_mktime.c files.
 *
 * mbed Microcontroller Library
 * Copyright (c) 2017-2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ============================================================================
 */

/*!
 * Convert a calendar time into time since UNIX epoch as a time_t.
 *
 * This function is a thread safe (partial) replacement for mktime. It is
 * tailored around RTC peripherals needs and is not by any mean a complete
 * replacement of mktime.
 *
 * \param calendar_time The calendar time to convert into a time_t since epoch.
 * The fields from tm used for the computation are:
 *   - tm_sec
 *   - tm_min
 *   - tm_hour
 *   - tm_mday
 *   - tm_mon
 *   - tm_year
 * Other fields are ignored and won't be renormalized by a call to this function.
 * A valid calendar time is comprised between the 1st january of 1970 at
 * 00:00:00 and the 19th of january 2038 at 03:14:07.
 *
 * \return The calendar time as seconds since UNIX epoch if the input is in the
 * valid range. Otherwise ((time_t) -1).
 *
 * \note Leap seconds are not supported.
 * \note Values in output range from 0 to INT_MAX.
 * \note - For use by the HAL only
 */
static time_t _rtc_mktime( const struct tm* time );

/*!
 * Convert a given time in seconds since epoch into calendar time.
 *
 * This function is a thread safe (partial) replacement for localtime. It is
 * tailored around RTC peripherals specification and is not by any means a
 * complete of localtime.
 *
 * \param timestamp The time (in seconds) to convert into calendar time. Valid
 * input are in the range [0 : INT32_MAX].
 * \param calendar_time Pointer to the object which will contain the result of
 * the conversion. The tm fields filled by this function are:
 *   - tm_sec
 *   - tm_min
 *   - tm_hour
 *   - tm_mday
 *   - tm_mon
 *   - tm_year
 *   - tm_wday
 *   - tm_yday
 * The object remains untouched if the time in input is invalid.
 * \return true if the conversion was successful, false otherwise.
 *
 * \note - For use by the HAL only
 */
static bool _rtc_localtime( time_t timestamp, struct tm* time_info );

/*
 * ============================================================================
 * END of mktime and localtime functions implmentation
 * ============================================================================
 */

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
        HAL_RTC_Init( &RtcHandle );

        // Set Date: Friday 1st of January 2000
        rtcInit.CalendarDate.Year = 0;
        rtcInit.CalendarDate.Month = RTC_MONTH_JANUARY;
        rtcInit.CalendarDate.Date = 1;
        rtcInit.CalendarDate.WeekDay = RTC_WEEKDAY_SATURDAY;
        HAL_RTC_SetDate( &RtcHandle, &rtcInit.CalendarDate, RTC_FORMAT_BIN );

        // Set Time: 00:00:00
        rtcInit.CalendarTime.Hours = 0;
        rtcInit.CalendarTime.Minutes = 0;
        rtcInit.CalendarTime.Seconds = 0;
        rtcInit.CalendarTime.SecondFraction = 0;
        rtcInit.CalendarTime.TimeFormat = RTC_HOURFORMAT12_AM;
        rtcInit.CalendarTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        rtcInit.CalendarTime.StoreOperation = RTC_STOREOPERATION_RESET;
        HAL_RTC_SetTime( &RtcHandle, &rtcInit.CalendarTime, RTC_FORMAT_BIN );

        // Enable Direct Read of the calendar registers (not through Shadow registers)
        HAL_RTCEx_EnableBypassShadow( &RtcHandle );

        HAL_NVIC_SetPriority( RTC_IRQn, 1, 0 );
        HAL_NVIC_EnableIRQ( RTC_IRQn );
        RtcInitialized = true;
    }
}

void RtcSetSysTime( uint32_t seconds, uint16_t subSeconds )
{
    struct tm timeinfo;
    RTC_DateTypeDef dateStruct;
    RTC_TimeTypeDef timeStruct;

    // Add 1 to the seconds in order to compensate the synchronization wait function.
    seconds += 1;

    // Convert the time into a tm
    if( _rtc_localtime( ( time_t )seconds, &timeinfo ) == false )
    {
        return;
    }

    // Fill RTC structures
    if( timeinfo.tm_wday == 0 )
    {
        dateStruct.WeekDay    = 7;
    }
    else
    {
        dateStruct.WeekDay    = timeinfo.tm_wday;
    }
    dateStruct.Month          = timeinfo.tm_mon + 1;
    dateStruct.Date           = timeinfo.tm_mday;
    dateStruct.Year           = ( timeinfo.tm_year >= 100 ) ? timeinfo.tm_year - 100 : timeinfo.tm_year;
    timeStruct.Hours          = timeinfo.tm_hour;
    timeStruct.Minutes        = timeinfo.tm_min;
    timeStruct.Seconds        = timeinfo.tm_sec;
    timeStruct.SubSeconds     = 0;
    timeStruct.TimeFormat     = RTC_HOURFORMAT_24;
    timeStruct.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    timeStruct.StoreOperation = RTC_STOREOPERATION_RESET;

    // Wait 1000ms - subSeconds (ms) before syncing the date/time
    uint16_t waitSync = 0;
    waitSync = 1000 - subSeconds;
    DelayMs( waitSync );

    // Update the RTC current date/time
    HAL_RTC_SetDate( &RtcHandle, &dateStruct, FORMAT_BIN );
    HAL_RTC_SetTime( &RtcHandle, &timeStruct, FORMAT_BIN );
}

uint32_t RtcGetSysTime( uint16_t *subSeconds )
{
    uint32_t firstRead;
    struct tm timeinfo;
    RTC_DateTypeDef dateStruct;
    RTC_TimeTypeDef timeStruct;

    // Get Time and Date
    HAL_RTC_GetTime( &RtcHandle, &timeStruct, RTC_FORMAT_BIN );

    // Make sure it is correct due to asynchronous nature of RTC
    do
    {
        firstRead = timeStruct.SubSeconds;
        HAL_RTC_GetDate( &RtcHandle, &dateStruct, RTC_FORMAT_BIN );
        HAL_RTC_GetTime( &RtcHandle, &timeStruct, RTC_FORMAT_BIN );
    }while( firstRead != timeStruct.SubSeconds );

    // Setup a tm structure based on the RTC
    // tm_wday information is ignored by mktime;
    timeinfo.tm_mon  = dateStruct.Month - 1;
    timeinfo.tm_mday = dateStruct.Date;
    timeinfo.tm_year = dateStruct.Year + 100;
    timeinfo.tm_hour = timeStruct.Hours;
    timeinfo.tm_min  = timeStruct.Minutes;
    timeinfo.tm_sec  = timeStruct.Seconds;
    if( subSeconds != NULL )
    {
        *subSeconds = ( timeStruct.SecondFraction * 1000 - timeStruct.SubSeconds * 1000 ) / ( timeStruct.SecondFraction + 1 );
    }
    // Convert to timestamp
    return _rtc_mktime( &timeinfo );
}

void RtcSetTimeout( uint32_t timeout )
{
    RtcStartWakeUpAlarm( RtcConvertMsToTick( timeout ) );
}

TimerTime_t RtcGetAdjustedTimeoutValue( uint32_t timeout )
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
    {   // we don't go in low power mode for delay below 50ms (needed for LEDs)
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

TimerTime_t RtcGetTimerValue( void )
{
    TimerTime_t retVal = 0;
    retVal = RtcConvertCalendarTickToTimerTime( NULL );
    RtcConvertTickToMs( retVal );

    return( RtcConvertTickToMs( retVal ) );
}

TimerTime_t RtcGetElapsedAlarmTime( void )
{
    TimerTime_t retVal = 0;
    TimerTime_t currentTime = 0;
    TimerTime_t contextTime = 0;

    currentTime = RtcConvertCalendarTickToTimerTime( NULL );
    contextTime = RtcConvertCalendarTickToTimerTime( &RtcCalendarContext );

    if( currentTime < contextTime )
    {
        retVal = ( currentTime + ( 0xFFFFFFFF - contextTime ) );
    }
    else
    {
        retVal = ( currentTime - contextTime );
    }
    return( RtcConvertTickToMs( retVal ) );
}

TimerTime_t RtcComputeFutureEventTime( TimerTime_t futureEventInTime )
{
    return( RtcGetTimerValue( ) + futureEventInTime );
}

TimerTime_t RtcComputeElapsedTime( TimerTime_t eventInTime )
{
    TimerTime_t elapsedTime = 0;

    // Needed at boot, cannot compute with 0 or elapsed time will be equal to current time
    if( eventInTime == 0 )
    {
        return 0;
    }
    // first get the current value of the timer in tick
    elapsedTime = RtcConvertCalendarTickToTimerTime( NULL );
    // convert into ms
    elapsedTime = RtcConvertTickToMs( elapsedTime );

    // compare "eventInTime" with "elapsedTime" while watching for roll over due to 32-bit
    if( elapsedTime < eventInTime ) // // roll over of the counter
    {   // due to convertion tick to ms, roll over value is 0x7D000000 (0x7D000000 * 2.048 = 0xFFFFFFFF)
        return( elapsedTime + ( 0x7D000000 - eventInTime ) );
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

        // Enter Stop Mode
        HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
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
    uint32_t start = 0;
    uint32_t stop = 0;
    RTC_AlarmTypeDef  alarmRtc;
    RtcCalendar_t now;

    if( WakeUpTimeInitialized == false )
    {
        now = RtcGetCalendar( );
        HAL_RTC_GetAlarm( &RtcHandle, &alarmRtc, RTC_ALARM_A, RTC_FORMAT_BIN );

        start = PREDIV_S - alarmRtc.AlarmTime.SubSeconds;
        stop = PREDIV_S - now.CalendarTime.SubSeconds;

        McuWakeUpTime = RtcConvertTickToMs( stop - start );

        WakeUpTimeInitialized = true;
    }
}

static void RtcStartWakeUpAlarm( uint32_t timeoutValue )
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

    // timeoutValue is in ms
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

    TimerTime_t timeoutValue = 0;

    uint16_t milliseconds = 0;
    uint16_t seconds = now.CalendarTime.Seconds;
    uint16_t minutes = now.CalendarTime.Minutes;
    uint16_t hours = now.CalendarTime.Hours;
    uint16_t days = now.CalendarDate.Date;

    timeoutValue = timeCounter;

    milliseconds = PREDIV_S - now.CalendarTime.SubSeconds;
    milliseconds += ( timeoutValue & PREDIV_S );

    /* convert timeout  to seconds */
    timeoutValue >>= N_PREDIV_S;  /* convert timeout  in seconds */

    // Convert milliseconds to RTC format and add to now
    while( timeoutValue >= SecondsInDay )
    {
        timeoutValue -= SecondsInDay;
        days++;
    }

    // Calculate hours
    while( timeoutValue >= SecondsInHour )
    {
        timeoutValue -= SecondsInHour;
        hours++;
    }

    // Calculate minutes
    while( timeoutValue >= SecondsInMinute )
    {
        timeoutValue -= SecondsInMinute;
        minutes++;
    }

    // Calculate seconds
    seconds += timeoutValue;

    // Correct for modulo
    while( milliseconds >= ( PREDIV_S + 1 ) )
    {
        milliseconds -= ( PREDIV_S + 1 );
        seconds++;
    }

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
        days++;
    }

    if( ( now.CalendarDate.Year == 0 ) || ( now.CalendarDate.Year % 4 ) == 0 )
    {
        if( days > DaysInMonthLeapYear[now.CalendarDate.Month - 1] )
        {
            days = days % DaysInMonthLeapYear[now.CalendarDate.Month - 1];
            calendar.CalendarDate.Month++;
        }
    }
    else
    {
        if( days > DaysInMonth[now.CalendarDate.Month - 1] )
        {
            days = days % DaysInMonth[now.CalendarDate.Month - 1];
            calendar.CalendarDate.Month++;
        }
    }

    calendar.CalendarTime.SubSeconds = PREDIV_S - milliseconds;
    calendar.CalendarTime.Seconds = seconds;
    calendar.CalendarTime.Minutes = minutes;
    calendar.CalendarTime.Hours = hours;
    calendar.CalendarDate.Date = days;

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

    TimerTime_t timeoutValue = 0;

    uint16_t milliseconds = 0;
    uint16_t seconds = 0;
    uint16_t minutes = 0;
    uint16_t hours = 0;
    uint16_t days = 0;
    uint8_t months = 1; // Start at 1, month 0 does not exist
    uint16_t years = 0;

    timeoutValue = timeCounter;

    milliseconds += ( timeoutValue & PREDIV_S);

    /* convert timeout  to seconds */
    timeoutValue >>= N_PREDIV_S; // convert timeout  in seconds

    // Convert milliseconds to RTC format and add to now
    while( timeoutValue >= SecondsInDay )
    {
        timeoutValue -= SecondsInDay;
        days++;
    }

    // Calculate hours
    while( timeoutValue >= SecondsInHour )
    {
        timeoutValue -= SecondsInHour;
        hours++;
    }

    // Calculate minutes
    while( timeoutValue >= SecondsInMinute )
    {
        timeoutValue -= SecondsInMinute;
        minutes++;
    }

    // Calculate seconds
    seconds += timeoutValue;

    // Correct for modulo
    while( milliseconds >= ( PREDIV_S + 1 ) )
    {
        milliseconds -= ( PREDIV_S + 1 );
        seconds++;
    }

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
        days++;
    }

    while( days > DaysInMonthLeapYear[months - 1] )
    {
        days -= DaysInMonthLeapYear[months - 1];
        months++;
    }

    calendar.CalendarTime.SubSeconds = PREDIV_S - milliseconds;
    calendar.CalendarTime.Seconds = seconds;
    calendar.CalendarTime.Minutes = minutes;
    calendar.CalendarTime.Hours = hours;
    calendar.CalendarDate.Date = days;
    calendar.CalendarDate.Month = months;
    calendar.CalendarDate.Year = years; // on 32-bit, years will never go up

    return calendar;
}

static TimerTime_t RtcConvertCalendarTickToTimerTime( RtcCalendar_t *calendar )
{
    TimerTime_t timeCounter = 0;
    RtcCalendar_t now;
    uint32_t timeCounterTemp = 0;

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
    for( int16_t i = 0; i < now.CalendarDate.Year ; i++ )
    {
        if( ( i == 0 ) || ( i % 4 ) == 0 )
        {
            timeCounterTemp += ( uint32_t )SecondsInLeapYear;
        }
        else
        {
            timeCounterTemp += ( uint32_t )SecondsInYear;
        }
    }

    // Months (calculation valid up to year 2099)*/
    if( ( now.CalendarDate.Year == 0 ) || ( now.CalendarDate.Year % 4 ) == 0 )
    {
        for( uint8_t i = 0; i < ( now.CalendarDate.Month - 1 ); i++ )
        {
            timeCounterTemp += ( uint32_t )( DaysInMonthLeapYear[i] * SecondsInDay );
        }
    }
    else
    {
        for( uint8_t i = 0;  i < ( now.CalendarDate.Month - 1 ); i++ )
        {
            timeCounterTemp += ( uint32_t )( DaysInMonth[i] * SecondsInDay );
        }
    }

    timeCounterTemp += ( uint32_t )( ( uint32_t )now.CalendarTime.Seconds +
                     ( ( uint32_t )now.CalendarTime.Minutes * SecondsInMinute ) +
                     ( ( uint32_t )now.CalendarTime.Hours * SecondsInHour ) +
                     ( ( uint32_t )( now.CalendarDate.Date * SecondsInDay ) ) );

    timeCounter = ( timeCounterTemp << N_PREDIV_S ) + ( PREDIV_S - now.CalendarTime.SubSeconds);

    return ( timeCounter );
}

TimerTime_t RtcConvertMsToTick( TimerTime_t timeoutValue )
{
    double retVal = 0;
    retVal = round( ( ( double )timeoutValue * CONV_DENOM ) / CONV_NUMER );
    return( ( TimerTime_t )retVal );
}

TimerTime_t RtcConvertTickToMs( TimerTime_t timeoutValue )
{
    double retVal = 0.0;
    retVal = round( ( ( double )timeoutValue * CONV_NUMER ) / CONV_DENOM );
    return( ( TimerTime_t )retVal );
}

static RtcCalendar_t RtcGetCalendar( void )
{
    uint32_t firstRead = 0;
    RtcCalendar_t now;

    // Get Time and Date
    HAL_RTC_GetTime( &RtcHandle, &now.CalendarTime, RTC_FORMAT_BIN );

    // Make sure it is correct due to asynchronous nature of RTC
    do
    {
        firstRead = now.CalendarTime.SubSeconds;
        HAL_RTC_GetDate( &RtcHandle, &now.CalendarDate, RTC_FORMAT_BIN );
        HAL_RTC_GetTime( &RtcHandle, &now.CalendarTime, RTC_FORMAT_BIN );
    }while( firstRead != now.CalendarTime.SubSeconds );

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

void RtcProcess( void )
{
    // Not used on this platform.
}

TimerTime_t RtcTempCompensation( TimerTime_t period, float temperature )
{
    float k = RTC_TEMP_COEFFICIENT;
    float kDev = RTC_TEMP_DEV_COEFFICIENT;
    float t = RTC_TEMP_TURNOVER;
    float tDev = RTC_TEMP_DEV_TURNOVER;
    float interim = 0.0;
    float ppm = 0.0;

    if( k < 0.0 )
    {
        ppm = ( k - kDev );
    }
    else
    {
        ppm = ( k + kDev );
    }
    interim = ( temperature - ( t - tDev ) );
    ppm *=  interim * interim;

    // Calculate the drift in time
    interim = ( ( float ) period * ppm ) / 1e6;
    // Calculate the resulting time period
    interim += period;
    interim = floor( interim );

    if( interim < 0.0 )
    {
        interim = ( float )period;
    }

    // Calculate the resulting period
    return ( TimerTime_t ) interim;
}

/*
 * ============================================================================
 * BEGIN of mktime and localtime functions implmentation
 *
 * These functions implementation is based on mbed os mbed_mktime.h and
 * mbed_mktime.c files.
 *
 * mbed Microcontroller Library
 * Copyright (c) 2017-2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ============================================================================
 */

/*!
 * time constants 
 */
#define SECONDS_BY_MINUTES 60
#define MINUTES_BY_HOUR 60
#define SECONDS_BY_HOUR (SECONDS_BY_MINUTES * MINUTES_BY_HOUR)
#define HOURS_BY_DAY 24 
#define SECONDS_BY_DAY (SECONDS_BY_HOUR * HOURS_BY_DAY)

/*!
 * 2 dimensional array containing the number of seconds elapsed before a given 
 * month.
 * The second index map to the month while the first map to the type of year:
 *   - 0: non leap year 
 *   - 1: leap year
 */
static const uint32_t seconds_before_month[2][12] = {
    {
        0,
        31 * SECONDS_BY_DAY,
        (31 + 28) * SECONDS_BY_DAY,
        (31 + 28 + 31) * SECONDS_BY_DAY,
        (31 + 28 + 31 + 30) * SECONDS_BY_DAY,
        (31 + 28 + 31 + 30 + 31) * SECONDS_BY_DAY,
        (31 + 28 + 31 + 30 + 31 + 30) * SECONDS_BY_DAY,
        (31 + 28 + 31 + 30 + 31 + 30 + 31) * SECONDS_BY_DAY,
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31) * SECONDS_BY_DAY,
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30) * SECONDS_BY_DAY,
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31) * SECONDS_BY_DAY,
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30) * SECONDS_BY_DAY,
    },
    {
        0,
        31 * SECONDS_BY_DAY,
        (31 + 29) * SECONDS_BY_DAY,
        (31 + 29 + 31) * SECONDS_BY_DAY,
        (31 + 29 + 31 + 30) * SECONDS_BY_DAY,
        (31 + 29 + 31 + 30 + 31) * SECONDS_BY_DAY,
        (31 + 29 + 31 + 30 + 31 + 30) * SECONDS_BY_DAY,
        (31 + 29 + 31 + 30 + 31 + 30 + 31) * SECONDS_BY_DAY,
        (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31) * SECONDS_BY_DAY,
        (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30) * SECONDS_BY_DAY,
        (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31) * SECONDS_BY_DAY,
        (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30) * SECONDS_BY_DAY,
    }
};

/*!
 * Compute if a year is a leap year or not.
 *
 * \param year The year to test it shall be in the range [70:138]. Year 0 is
 * translated into year 1900 CE.
 * \return true if the year in input is a leap year and false otherwise.
 * \note - For use by the HAL only
 */
static bool _rtc_is_leap_year(int year) {
    /* 
     * since in practice, the value manipulated by this algorithm lie in the 
     * range [70 : 138], the algorith can be reduced to: year % 4.
     * The algorithm valid over the full range of value is: 

        year = 1900 + year;
        if (year % 4) {
            return false;
        } else if (year % 100) {
            return true;
        } else if (year % 400) {
            return false;
        }
        return true;

     */ 
    return (year) % 4 ? false : true;
}

static time_t _rtc_mktime(const struct tm* time) {
    // partial check for the upper bound of the range
    // normalization might happen at the end of the function 
    // this solution is faster than checking if the input is after the 19th of 
    // january 2038 at 03:14:07.  
    if ((time->tm_year < 70) || (time->tm_year > 138)) { 
        return ((time_t) -1);
    }

    uint32_t result = time->tm_sec;
    result += time->tm_min * SECONDS_BY_MINUTES;
    result += time->tm_hour * SECONDS_BY_HOUR;
    result += (time->tm_mday - 1) * SECONDS_BY_DAY;
    result += seconds_before_month[_rtc_is_leap_year(time->tm_year)][time->tm_mon];

    if (time->tm_year > 70) { 
        // valid in the range [70:138] 
        uint32_t count_of_leap_days = ((time->tm_year - 1) / 4) - (70 / 4);
        result += (((time->tm_year - 70) * 365) + count_of_leap_days) * SECONDS_BY_DAY;
    }

    if (result > INT32_MAX) { 
        return (time_t) -1;
    }

    return result;
}

static bool _rtc_localtime(time_t timestamp, struct tm* time_info) {
    if (((int32_t) timestamp) < 0) { 
        return false;
    } 

    time_info->tm_sec = timestamp % 60;
    timestamp = timestamp / 60;   // timestamp in minutes
    time_info->tm_min = timestamp % 60;
    timestamp = timestamp / 60;  // timestamp in hours
    time_info->tm_hour = timestamp % 24;
    timestamp = timestamp / 24;  // timestamp in days;

    // compute the weekday
    // The 1st of January 1970 was a Thursday which is equal to 4 in the weekday
    // representation ranging from [0:6]
    time_info->tm_wday = (timestamp + 4) % 7;

    // years start at 70
    time_info->tm_year = 70;
    while (true) { 
        if (_rtc_is_leap_year(time_info->tm_year) && timestamp >= 366) {
            ++time_info->tm_year;
            timestamp -= 366;
        } else if (!_rtc_is_leap_year(time_info->tm_year) && timestamp >= 365) {
            ++time_info->tm_year;
            timestamp -= 365;
        } else {
            // the remaining days are less than a years
            break;
        }
    }

    time_info->tm_yday = timestamp;

    // convert days into seconds and find the current month
    timestamp *= SECONDS_BY_DAY;
    time_info->tm_mon = 11;
    bool leap = _rtc_is_leap_year(time_info->tm_year);
    for (uint32_t i = 0; i < 12; ++i) {
        if ((uint32_t) timestamp < seconds_before_month[leap][i]) {
            time_info->tm_mon = i - 1;
            break;
        }
    }

    // remove month from timestamp and compute the number of days.
    // note: unlike other fields, days are not 0 indexed.
    timestamp -= seconds_before_month[leap][time_info->tm_mon];
    time_info->tm_mday = (timestamp / SECONDS_BY_DAY) + 1;

    return true;
}
