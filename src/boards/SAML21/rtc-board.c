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
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */
#include <hw_timer.h>
#include "board.h"
#include "timer.h"
#include "gpio.h"

#include "rtc-board.h"

#define RTC_DEBUG_ENABLE                            1
#define RTC_DEBUG_DISABLE                           0

#define RTC_DEBUG_GPIO_STATE                        RTC_DEBUG_DISABLE
#define RTC_DEBUG_PRINTF_STATE                      RTC_DEBUG_DISABLE

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitialized = false;
static volatile bool RtcTimeoutPendingInterrupt = false;
static volatile bool RtcTimeoutPendingPolling = false;

// in ticks
static uint32_t TimeoutStart = 0;
// in ticks
static uint32_t TimeoutDuration = 0;

#if( RTC_DEBUG_GPIO_STATE == RTC_DEBUG_ENABLE )
Gpio_t DbgRtcPin0;
Gpio_t DbgRtcPin1;
#endif

/*!
 * \brief Callback for the hal_timer when timeout expired
 */
static void RtcTimerTaskCallback( void );

void RtcInit( void )
{
    if( RtcInitialized == false )
    {
#if( RTC_DEBUG_GPIO_STATE == RTC_DEBUG_ENABLE )
        GpioInit( &DbgRtcPin0, RTC_DBG_PIN_0, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgRtcPin1, RTC_DBG_PIN_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
        // RTC timer
        HwTimerInit( );
        HwTimerSetCallback( RtcTimerTaskCallback );
        RtcInitialized = true;
    }
}

void RtcSetTimeout( uint32_t timeout )
{
    TimeoutStart = HwTimerGetTime( );
    TimeoutDuration = MS_TO_TICKS( timeout );
#if( RTC_DEBUG_PRINTF_STATE == RTC_DEBUG_ENABLE )
    printf( "TIMEOUT \t%010d\t%010d\r\n", TimeoutStart, TimeoutDuration );
#endif
#if( RTC_DEBUG_GPIO_STATE == RTC_DEBUG_ENABLE )
    GpioWrite( &DbgRtcPin0, 0 );
    GpioWrite( &DbgRtcPin1, 0 );
#endif

    RtcTimeoutPendingInterrupt = true;
    RtcTimeoutPendingPolling = false;

    if( HwTimerLoadAbsoluteTicks( TimeoutStart + TimeoutDuration ) == false )
    {
        // If timer already passed
        if( RtcTimeoutPendingInterrupt == true )
        {
            // And interrupt not handled, mark as polling
            RtcTimeoutPendingPolling = true;
            RtcTimeoutPendingInterrupt = false;
#if( RTC_DEBUG_GPIO_STATE == RTC_DEBUG_ENABLE )
            GpioWrite( &DbgRtcPin0, 1 );
#endif
        }
    }
}

TimerTime_t RtcGetAdjustedTimeoutValue( uint32_t timeout )
{
    return timeout;
}

TimerTime_t RtcGetTimerValue( void )
{
    return TICKS_TO_MS( HwTimerGetTime( ) );
}

TimerTime_t RtcGetElapsedAlarmTime( void )
{
    return TICKS_TO_MS( HwTimerGetTime( ) - TimeoutStart );
}

TimerTime_t RtcComputeElapsedTime( TimerTime_t eventInTime )
{
    // Calculate in ticks for correct 32bit wrapping.
    // Only works because ticks wrappes earlier than ms.
    return TICKS_TO_MS( HwTimerGetTime( ) - MS_TO_TICKS( eventInTime ) );
}

void RtcEnterLowPowerStopMode( void )
{

}

void RtcProcess( void )
{
    if( RtcTimeoutPendingPolling == true )
    {
        if( RtcGetElapsedAlarmTime( ) >= TimeoutDuration )
        {
            // Because of one shot the task will be removed after the callback
            RtcTimeoutPendingPolling = false;
#if( RTC_DEBUG_GPIO_STATE == RTC_DEBUG_ENABLE )
            GpioWrite( &DbgRtcPin0, 0 );
            GpioWrite( &DbgRtcPin1, 1 );
#endif
            // NOTE: The handler should take less then 1 ms otherwise the clock shifts
            TimerIrqHandler( );
#if( RTC_DEBUG_GPIO_STATE == RTC_DEBUG_ENABLE )
            GpioWrite( &DbgRtcPin1, 0 );
#endif
        }
    }
}

static void RtcTimerTaskCallback( void )
{
    // Because of one shot the task will be removed after the callback
    RtcTimeoutPendingInterrupt = false;
#if( RTC_DEBUG_GPIO_STATE == RTC_DEBUG_ENABLE )
    GpioWrite( &DbgRtcPin1, 1 );
#endif
    // NOTE: The handler should take less then 1 ms otherwise the clock shifts
    TimerIrqHandler( );
#if( RTC_DEBUG_GPIO_STATE == RTC_DEBUG_ENABLE )
    GpioWrite( &DbgRtcPin1, 0 );
#endif
}
