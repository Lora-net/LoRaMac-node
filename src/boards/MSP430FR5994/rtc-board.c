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
 *              (C)2013-2017 Semtech - STMicroelectronics
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    MCD Application Team (C)( STMicroelectronics International )
 */
#include <math.h>
#include <time.h>
#include "utilities.h"
#include "delay.h"
#include "board.h"
#include "timer.h"
#include "systime.h"
#include "gpio.h"
#include "sysIrqHandlers.h"
#include "lpm-board.h"
#include "rtc-board.h"

#define MIN_ALARM_DELAY 0 //TODO

/*!
 * RTC timer context 
 */
typedef struct
{
    uint32_t        Time;         // Reference time
    //RTC_TimeTypeDef CalendarTime; // Reference time in calendar format
    //RTC_DateTypeDef CalendarDate; // Reference date in calendar format
}RtcTimerContext_t;

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitialized = false;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 * Set with the \ref RtcSetTimerContext function
 * Value is kept as a Reference to calculate alarm
 */
static RtcTimerContext_t RtcTimerContext;

/*!
 * \brief Get the current time from calendar in ticks
 *
 * \param [IN] date           Pointer to RTC_DateStruct
 * \param [IN] time           Pointer to RTC_TimeStruct
 * \retval calendarValue Time in ticks
 */

void RtcInit( void )
{
    
}

/*!
 * \brief Sets the RTC timer reference, sets also the RTC_DateStruct and RTC_TimeStruct
 *
 * \param none
 * \retval timerValue In ticks
 */
uint32_t RtcSetTimerContext( void )
{
   // RtcTimerContext.Time = ( uint32_t )RtcGetCalendarValue( &RtcTimerContext.CalendarDate, &RtcTimerContext.CalendarTime );
    return ( uint32_t )RtcTimerContext.Time;
}

/*!
 * \brief Gets the RTC timer reference
 *
 * \param none
 * \retval timerValue In ticks
 */
uint32_t RtcGetTimerContext( void )
{
    return RtcTimerContext.Time;
}

/*!
 * \brief returns the wake up time in ticks
 *
 * \retval wake up time in ticks
 */
uint32_t RtcGetMinimumTimeout( void )
{
    return( MIN_ALARM_DELAY );
}

/*!
 * \brief converts time in ms to time in ticks
 *
 * \param[IN] milliseconds Time in milliseconds
 * \retval returns time in timer ticks
 */
uint32_t RtcMs2Tick( uint32_t milliseconds )
{
    return 0;
}

/*!
 * \brief converts time in ticks to time in ms
 *
 * \param[IN] time in timer ticks
 * \retval returns time in milliseconds
 */
uint32_t RtcTick2Ms( uint32_t tick )
{
    return 0;
}

/*!
 * \brief a delay of delay ms by polling RTC
 *
 * \param[IN] delay in ms
 */
void RtcDelayMs( uint32_t delay )
{

}

/*!
 * \brief Sets the alarm
 *
 * \note The alarm is set at now (read in this function) + timeout
 *
 * \param timeout Duration of the Timer ticks
 */
void RtcSetAlarm( uint32_t timeout )
{

}

void RtcStopAlarm( void )
{

}

void RtcStartAlarm( uint32_t timeout )
{

}

uint32_t RtcGetTimerValue( void )
{


    return 0;
}

uint32_t RtcGetTimerElapsedTime( void )
{
 
  uint32_t calendarValue = 0;

  return( ( uint32_t )( calendarValue - RtcTimerContext.Time ) );
}

static uint64_t RtcGetCalendarValue(  )
{
    
}

uint32_t RtcGetCalendarTime( uint16_t *milliseconds )
{

}

/*!
 * \brief RTC IRQ Handler of the RTC Alarm
 */
void RTC_Alarm_IRQHandler( void )
{
    
}

/*!
 * \brief  Alarm A callback.
 *
 * \param [IN] hrtc RTC handle
 */
void HAL_RTC_AlarmAEventCallback(  )
{
    TimerIrqHandler( );
}

void RtcBkupWrite( uint32_t data0, uint32_t data1 )
{

}

void RtcBkupRead( uint32_t *data0, uint32_t *data1 )
{

}

void RtcProcess( void )
{
    // Not used on this platform.
}

TimerTime_t RtcTempCompensation( TimerTime_t period, float temperature )
{
   
}
