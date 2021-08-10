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

// sub-second number of bits
#define N_PREDIV_S 10

// Synchronous prediv
#define PREDIV_S ((1 << N_PREDIV_S) - 1)

// Asynchronous prediv
#define PREDIV_A (1 << (15 - N_PREDIV_S)) - 1

// Sub-second mask definition
#define ALARM_SUBSECOND_MASK (N_PREDIV_S << RTC_ALRMASSR_MASKSS_Pos)

// RTC Time base in us
#define USEC_NUMBER 1000000
#define MSEC_NUMBER (USEC_NUMBER / 1000)

#define COMMON_FACTOR 3
#define CONV_NUMER (MSEC_NUMBER >> COMMON_FACTOR)
#define CONV_DENOM (1 << (N_PREDIV_S - COMMON_FACTOR))

/*!
 * \brief Days, Hours, Minutes and seconds
 */
#define DAYS_IN_LEAP_YEAR ((uint32_t)366U)
#define DAYS_IN_YEAR ((uint32_t)365U)
#define SECONDS_IN_1DAY ((uint32_t)86400U)
#define SECONDS_IN_1HOUR ((uint32_t)3600U)
#define SECONDS_IN_1MINUTE ((uint32_t)60U)
#define MINUTES_IN_1HOUR ((uint32_t)60U)
#define HOURS_IN_1DAY ((uint32_t)24U)

/*!
 * \brief Correction factors
 */
#define DAYS_IN_MONTH_CORRECTION_NORM ((uint32_t)0x99AAA0)
#define DAYS_IN_MONTH_CORRECTION_LEAP ((uint32_t)0x445550)

/*!
 * \brief Calculates ceiling( X / N )
 */
#define DIVC(X, N) (((X) + (N)-1) / (N))

/*!
 * RTC timer context 
 */
typedef struct
{
    uint32_t Time;                // Reference time
    RTC_TimeTypeDef CalendarTime; // Reference time in calendar format
    RTC_DateTypeDef CalendarDate; // Reference date in calendar format
} RtcTimerContext_t;

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitialized = false;

/*!
 * \brief Indicates if the RTC Wake Up Time is calibrated or not
 */
static bool McuWakeUpTimeInitialized = false;

/*!
 * \brief Compensates MCU wakeup time
 */
static int16_t McuWakeUpTimeCal = 0;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*!
 * \brief RTC Handle
 */
static RTC_HandleTypeDef RtcHandle =
    {
        .Instance = NULL,
        .Init =
            {
                .HourFormat = 0,
                .AsynchPrediv = 0,
                .SynchPrediv = 0,
                .OutPut = 0,
                .OutPutRemap = 0,
                .OutPutPolarity = 0,
                .OutPutType = 0},
        .Lock = HAL_UNLOCKED,
        .State = HAL_RTC_STATE_RESET};

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

/*!
 * \brief Get the current time from calendar in ticks
 *
 * \param [IN] date           Pointer to RTC_DateStruct
 * \param [IN] time           Pointer to RTC_TimeStruct
 * \retval calendarValue Time in ticks
 */
static uint64_t RtcGetCalendarValue(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);

void RtcInit(void)
{
}

/*!
 * \brief Sets the RTC timer reference, sets also the RTC_DateStruct and RTC_TimeStruct
 *
 * \param none
 * \retval timerValue In ticks
 */
uint32_t RtcSetTimerContext(void)
{
    RtcTimerContext.Time = (uint32_t)RtcGetCalendarValue(&RtcTimerContext.CalendarDate, &RtcTimerContext.CalendarTime);
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
    return (uint32_t)((((uint64_t)milliseconds) * CONV_DENOM) / CONV_NUMER);
}

/*!
 * \brief converts time in ticks to time in ms
 *
 * \param[IN] time in timer ticks
 * \retval returns time in milliseconds
 */
uint32_t RtcTick2Ms(uint32_t tick)
{
    uint32_t seconds = tick >> N_PREDIV_S;

    tick = tick & PREDIV_S;
    return ((seconds * 1000) + ((tick * 1000) >> N_PREDIV_S));
}

/*!
 * \brief a delay of delay ms by polling RTC
 *
 * \param[IN] delay in ms
 */
void RtcDelayMs(uint32_t delay)
{
    uint64_t delayTicks = 0;
    uint64_t refTicks = RtcGetTimerValue();

    delayTicks = RtcMs2Tick(delay);

    // Wait delay ms
    while (((RtcGetTimerValue() - refTicks)) < delayTicks)
    {
        __NOP();
    }
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
}

void RtcStartAlarm(uint32_t timeout)
{
    uint16_t rtcAlarmSubSeconds = 0;
    uint16_t rtcAlarmSeconds = 0;
    uint16_t rtcAlarmMinutes = 0;
    uint16_t rtcAlarmHours = 0;
    uint16_t rtcAlarmDays = 0;
    RTC_TimeTypeDef time = RtcTimerContext.CalendarTime;
    RTC_DateTypeDef date = RtcTimerContext.CalendarDate;

    RtcStopAlarm();

    /*reverse counter */
    rtcAlarmSubSeconds = PREDIV_S - time.SubSeconds;
    rtcAlarmSubSeconds += (timeout & PREDIV_S);
    // convert timeout  to seconds
    timeout >>= N_PREDIV_S;

    // Convert microsecs to RTC format and add to 'Now'
    rtcAlarmDays = date.Date;
    while (timeout >= TM_SECONDS_IN_1DAY)
    {
        timeout -= TM_SECONDS_IN_1DAY;
        rtcAlarmDays++;
    }

    // Calc hours
    rtcAlarmHours = time.Hours;
    while (timeout >= TM_SECONDS_IN_1HOUR)
    {
        timeout -= TM_SECONDS_IN_1HOUR;
        rtcAlarmHours++;
    }

    // Calc minutes
    rtcAlarmMinutes = time.Minutes;
    while (timeout >= TM_SECONDS_IN_1MINUTE)
    {
        timeout -= TM_SECONDS_IN_1MINUTE;
        rtcAlarmMinutes++;
    }

    // Calc seconds
    rtcAlarmSeconds = time.Seconds + timeout;

    //***** Correct for modulo********
    while (rtcAlarmSubSeconds >= (PREDIV_S + 1))
    {
        rtcAlarmSubSeconds -= (PREDIV_S + 1);
        rtcAlarmSeconds++;
    }

    while (rtcAlarmSeconds >= TM_SECONDS_IN_1MINUTE)
    {
        rtcAlarmSeconds -= TM_SECONDS_IN_1MINUTE;
        rtcAlarmMinutes++;
    }

    while (rtcAlarmMinutes >= TM_MINUTES_IN_1HOUR)
    {
        rtcAlarmMinutes -= TM_MINUTES_IN_1HOUR;
        rtcAlarmHours++;
    }

    while (rtcAlarmHours >= TM_HOURS_IN_1DAY)
    {
        rtcAlarmHours -= TM_HOURS_IN_1DAY;
        rtcAlarmDays++;
    }

    if (date.Year % 4 == 0)
    {
        if (rtcAlarmDays > DaysInMonthLeapYear[date.Month - 1])
        {
            rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[date.Month - 1];
        }
    }
    else
    {
        if (rtcAlarmDays > DaysInMonth[date.Month - 1])
        {
            rtcAlarmDays = rtcAlarmDays % DaysInMonth[date.Month - 1];
        }
    }

    /* Set RTC_AlarmStructure with calculated values*/
    RtcAlarm.AlarmTime.SubSeconds = PREDIV_S - rtcAlarmSubSeconds;
    RtcAlarm.AlarmSubSecondMask = ALARM_SUBSECOND_MASK;
    RtcAlarm.AlarmTime.Seconds = rtcAlarmSeconds;
    RtcAlarm.AlarmTime.Minutes = rtcAlarmMinutes;
    RtcAlarm.AlarmTime.Hours = rtcAlarmHours;
    RtcAlarm.AlarmDateWeekDay = (uint8_t)rtcAlarmDays;
    RtcAlarm.AlarmTime.TimeFormat = time.TimeFormat;
    RtcAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    RtcAlarm.AlarmMask = RTC_ALARMMASK_NONE;
    RtcAlarm.Alarm = RTC_ALARM_A;
    RtcAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    RtcAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

    // Set RTC_Alarm
}

uint32_t RtcGetTimerValue(void)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    uint32_t calendarValue = (uint32_t)RtcGetCalendarValue(&date, &time);

    return (calendarValue);
}

uint32_t RtcGetTimerElapsedTime(void)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    uint32_t calendarValue = (uint32_t)RtcGetCalendarValue(&date, &time);

    return ((uint32_t)(calendarValue - RtcTimerContext.Time));
}

void RtcSetMcuWakeUpTime(void)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    uint32_t now, hit;
    int16_t mcuWakeUpTime;
}

int16_t RtcGetMcuWakeUpTime(void)
{
    return McuWakeUpTimeCal;
}

static uint64_t RtcGetCalendarValue(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
    uint64_t calendarValue = 0;

    return (calendarValue);
}

uint32_t RtcGetCalendarTime(uint16_t *milliseconds)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    uint32_t ticks;

    uint64_t calendarValue = RtcGetCalendarValue(&date, &time);

    uint32_t seconds = (uint32_t)(calendarValue >> N_PREDIV_S);

    ticks = (uint32_t)calendarValue & PREDIV_S;

    *milliseconds = RtcTick2Ms(ticks);

    return seconds;
}

/*!
 * \brief RTC IRQ Handler of the RTC Alarm
 */
void RTC_IRQHandler(void)
{
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

void RtcBkupWrite(uint32_t data0, uint32_t data1)
{
}

void RtcBkupRead(uint32_t *data0, uint32_t *data1)
{
}

void RtcProcess(void)
{
    // Not used on this platform.
}

TimerTime_t RtcTempCompensation(TimerTime_t period, float temperature)
{
    float k = RTC_TEMP_COEFFICIENT;
    float kDev = RTC_TEMP_DEV_COEFFICIENT;
    float t = RTC_TEMP_TURNOVER;
    float tDev = RTC_TEMP_DEV_TURNOVER;
    float interim = 0.0f;
    float ppm = 0.0f;

    if (k < 0.0f)
    {
        ppm = (k - kDev);
    }
    else
    {
        ppm = (k + kDev);
    }
    interim = (temperature - (t - tDev));
    ppm *= interim * interim;

    // Calculate the drift in time
    interim = ((float)period * ppm) / 1000000.0f;
    // Calculate the resulting time period
    interim += period;
    interim = floor(interim);

    if (interim < 0.0f)
    {
        interim = (float)period;
    }

    // Calculate the resulting period
    return (TimerTime_t)interim;
}
