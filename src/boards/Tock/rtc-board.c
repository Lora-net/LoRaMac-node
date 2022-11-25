/*!
 * \file      rtc-board.c
 *
 * \brief     Target board RTC timer and low power modes management
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Alistair Francis <alistair@alistair23.me>
 */
#include <math.h>
#include <time.h>
#include "utilities.h"
#include "delay.h"
#include "board.h"
#include "timer.h"
#include "systime.h"
#include "gpio.h"
#include "lpm-board.h"
#include "rtc-board.h"

#include "libtock/timer.h"
#include "libtock/alarm.h"
#include "libtock/read_only_state.h"

// MCU Wake Up Time
#define MIN_ALARM_DELAY                             3 // in ticks

// sub-second number of bits
#define N_PREDIV_S                                  10

// Synchronous prediv
#define PREDIV_S                                    ( ( 1 << N_PREDIV_S ) - 1 )

// Asynchronous prediv
#define PREDIV_A                                    ( 1 << ( 15 - N_PREDIV_S ) ) - 1

/*!
 * RTC timer context
 */
typedef struct
{
    uint32_t        Time;         // Reference time
}RtcTimerContext_t;

void* read_only_state_buffer = NULL;

static tock_timer_t timer;
static uint32_t alarm_set_time = 0;

static void timer_fired(__attribute__ ((unused)) int arg0,
                        __attribute__ ((unused)) int arg1,
                        __attribute__ ((unused)) int arg2,
                        __attribute__ ((unused)) void* ud)
{
    TimerIrqHandler();
}

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 * Set with the \ref RtcSetTimerContext function
 * Value is kept as a Reference to calculate alarm
 */
static RtcTimerContext_t RtcTimerContext;

void RtcInit( void )
{
    read_only_state_buffer = malloc(READ_ONLY_STATEBUFFER_LEN);

    read_only_state_allocate_region(read_only_state_buffer, READ_ONLY_STATEBUFFER_LEN);
}

uint32_t RtcSetTimerContext( void )
{
    RtcTimerContext.Time = RtcGetTimerValue();

    return RtcTimerContext.Time;
}

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
    uint32_t frequency;
    alarm_internal_frequency(&frequency);

    return frequency;
}

/*!
 * \brief converts time in ms to time in ticks
 *
 * \param[IN] milliseconds Time in milliseconds
 * \retval returns time in timer ticks
 */
uint32_t RtcMs2Tick( uint32_t milliseconds )
{
    uint32_t frequency;
    alarm_internal_frequency(&frequency);

    return (milliseconds / 1000) * frequency + (milliseconds % 1000) * (frequency / 1000);
}

/*!
 * \brief converts time in ticks to time in ms
 *
 * \param[IN] time in timer ticks
 * \retval returns time in milliseconds
 */
uint32_t RtcTick2Ms( uint32_t tick )
{
    uint32_t frequency;
    alarm_internal_frequency(&frequency);

    return (tick / frequency) * 1000;
}

/*!
 * \brief a delay of delay ms by polling RTC
 *
 * \param[IN] delay in ms
 */
void RtcDelayMs( uint32_t delay )
{
    delay_ms(delay);
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
    timer_in(timeout, timer_fired, NULL, &timer);
    alarm_set_time = RtcGetTimerValue();
}

void RtcStopAlarm( void )
{
    timer_cancel(&timer);
    alarm_set_time = 0;
}

void RtcStartAlarm( uint32_t timeout )
{
    RtcSetAlarm(timeout);
}

uint32_t RtcGetTimerValue( void )
{
    if (read_only_state_buffer == NULL) {
        uint32_t now;
        alarm_internal_read(&now);
        return now;
    } else {
        return read_only_state_get_ticks(read_only_state_buffer);
    }
}

uint32_t RtcGetTimerElapsedTime( void )
{
    if (read_only_state_buffer == NULL) {
        uint32_t now;
        alarm_internal_read(&now);
        return now - alarm_set_time;
    } else {
        return read_only_state_get_ticks(read_only_state_buffer) - alarm_set_time;
    }
}

uint32_t RtcGetCalendarTime( uint16_t *milliseconds )
{
    return 0;
}

void RtcBkupWrite( uint32_t data0, uint32_t data1 )
{
}

void RtcBkupRead( uint32_t *data0, uint32_t *data1 )
{
  *data0 = 0;
  *data1 = 0;
}

void RtcProcess( void )
{
    yield_no_wait();
}

TimerTime_t RtcTempCompensation( TimerTime_t period, float temperature )
{
    return period;
}
