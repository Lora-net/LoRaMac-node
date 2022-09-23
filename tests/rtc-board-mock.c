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
#include "stm32l0xx.h"
#include "utilities.h"
#include "delay.h"
#include "board.h"
#include "timer.h"
#include "systime.h"
#include "gpio.h"
#include "sysIrqHandlers.h"
#include "lpm-board.h"
#include "rtc-board.h"

// MCU Wake Up Time
#define MIN_ALARM_DELAY 3 // in ticks

TimerTime_t current_time = 0;

/*!
 * RTC timer context
 */
typedef struct
{
    uint32_t Time; // Reference time
} RtcTimerContext_t;

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitialized = false;

uint32_t countdown = 0;
bool alarm_stopped = true;

/*!
 * \brief RTC Alarm
 */
static RTC_AlarmTypeDef RtcAlarm;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 * Set with the \ref RtcSetTimerContext function
 * Value is kept as a Reference to calculate alarm
 */
static RtcTimerContext_t RtcTimerContext;

void RtcInit(void)
{
    if (RtcInitialized == false)
    {
        RtcSetTimerContext();
        RtcInitialized = true;
    }
}

/*!
 * \brief Sets the RTC timer reference, sets also the RTC_DateStruct and RTC_TimeStruct
 *
 * \param none
 * \retval timerValue In ticks
 */
uint32_t RtcSetTimerContext(void)
{
    RtcTimerContext.Time = current_time;
    return (uint32_t)RtcTimerContext.Time;
}

/*!
 * \brief Gets the RTC timer reference
 *
 * \param none
 * \retval timerValue In ticks
 */
uint32_t RtcGetTimerContext(void)
{
    return RtcTimerContext.Time;
}

/*!
 * \brief returns the wake up time in ticks
 *
 * \retval wake up time in ticks
 */
uint32_t RtcGetMinimumTimeout(void)
{
    return (MIN_ALARM_DELAY);
}

/*!
 * \brief converts time in ms to time in ticks
 *
 * \param[IN] milliseconds Time in milliseconds
 * \retval returns time in timer ticks
 */
uint32_t RtcMs2Tick(uint32_t milliseconds)
{
    return milliseconds;
}

/*!
 * \brief converts time in ticks to time in ms
 *
 * \param[IN] time in timer ticks
 * \retval returns time in milliseconds
 */
uint32_t RtcTick2Ms(uint32_t tick)
{
    return tick;
}

/*!
 * \brief Sets the alarm
 *
 * \note The alarm is set at now (read in this function) + timeout
 *
 * \param timeout Duration of the Timer ticks
 */
void RtcSetAlarm(uint32_t timeout)
{

    RtcStartAlarm(timeout);
}

void RtcStopAlarm(void)
{
    alarm_stopped = true;
}

void RtcStartAlarm(uint32_t timeout)
{

    RtcStopAlarm();
    countdown = timeout;

    alarm_stopped = false;
}

uint32_t RtcGetTimerValue(void)
{
    return current_time;
}

uint32_t RtcGetTimerElapsedTime(void)
{
    return ((uint32_t)(current_time - RtcTimerContext.Time));
}

/*!
 * \brief  Alarm A callback.
 *
 * \param [IN] hrtc RTC handle
 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    TimerIrqHandler();
}
void RtcProcess(void)
{
    // Not used on this platform.
}

TimerTime_t RtcTempCompensation(TimerTime_t period, float temperature)
{

    return (TimerTime_t)1;
}
