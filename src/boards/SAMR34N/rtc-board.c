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
 */
#include "asf.h"

#include "utilities.h"
#include "board-config.h"
#include "board.h"
#include "timer.h"
#include "systime.h"
#include "gpio.h"

#include "rtc-board.h"

#define RTC_DEBUG_ENABLE 1
#define RTC_DEBUG_DISABLE 0

#define RTC_DEBUG_GPIO_STATE RTC_DEBUG_DISABLE
#define RTC_DEBUG_PRINTF_STATE RTC_DEBUG_DISABLE

#define MIN_ALARM_DELAY 3  // in ticks

#define COMPARE_COUNT_MAX_VALUE ( 0xFFFFFFFF )

#define US_TO_SLEEP_TICKS( u ) ( ( u ) * ( 0.03278f ) )
#define SLEEP_TICKS_TO_US( s ) ( ( s ) * ( 30.5175f ) )

#define MS_TO_SLEEP_TICKS( m ) ( ( m ) * ( 32.769f ) )
#define SLEEP_TICKS_TO_MS( s ) ( ( s ) * ( 0.0306f ) )

/*!
 * RTC timer context
 */
typedef struct
{
    uint32_t Time;   // Reference time
    uint32_t Delay;  // Reference Timeout duration
} RtcTimerContext_t;

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitialized = false;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 * Set with the \ref RtcSetTimerContext function
 * Value is kept as a Reference to calculate alarm
 */
static RtcTimerContext_t RtcTimerContext;

static struct rtc_module rtc_instance;

void RtcInit( void )
{
    if( RtcInitialized == false )
    {
        struct rtc_count_config rtc_config;
        rtc_count_get_config_defaults( &rtc_config );

        rtc_config.prescaler         = RTC_COUNT_PRESCALER_OFF;
        rtc_config.enable_read_sync  = true;
        rtc_config.compare_values[0] = COMPARE_COUNT_MAX_VALUE;
        rtc_config.compare_values[1] = COMPARE_COUNT_MAX_VALUE;
        rtc_count_init( &rtc_instance, RTC, &rtc_config );
        rtc_count_set_count( &rtc_instance, 0 );
        rtc_count_register_callback( &rtc_instance, TimerIrqHandler, RTC_COUNT_CALLBACK_COMPARE_0 );
        rtc_count_enable( &rtc_instance );

        RtcInitialized = true;
    }
}

uint32_t RtcSetTimerContext( void )
{
    RtcTimerContext.Time = ( uint32_t ) RtcGetTimerValue( );
    return ( uint32_t ) RtcTimerContext.Time;
}

uint32_t RtcGetTimerContext( void )
{
    return RtcTimerContext.Time;
}

uint32_t RtcGetMinimumTimeout( void )
{
    return ( MIN_ALARM_DELAY );
}

uint32_t RtcMs2Tick( TimerTime_t milliseconds )
{
    return ( uint32_t )( ( ( ( uint64_t )milliseconds ) << 10 ) / 1000 );
}

TimerTime_t RtcTick2Ms( uint32_t tick )
{
    uint32_t seconds = tick >> 10;

    tick = tick & 0x3FF;
    return ( ( seconds * 1000 ) + ( ( tick * 1000 ) >> 10 ) );
}

void RtcDelayMs( TimerTime_t milliseconds )
{
    uint32_t delayTicks = 0;
    uint32_t refTicks   = RtcGetTimerValue( );

    delayTicks = RtcMs2Tick( milliseconds );

    // Wait delay ms
    while( ( ( RtcGetTimerValue( ) - refTicks ) ) < delayTicks )
    {
        __NOP( );
    }
}

void RtcSetAlarm( uint32_t timeout )
{
    RtcStartAlarm( timeout );
}

void RtcStopAlarm( void )
{
    rtc_count_disable_callback( &rtc_instance, RTC_COUNT_CALLBACK_COMPARE_0 );
}

void RtcStartAlarm( uint32_t timeout )
{
    // rtc_count_set_count(&rtc_instance, 0);
    rtc_count_set_compare( &rtc_instance, timeout, RTC_COUNT_COMPARE_0 );
    rtc_count_enable_callback( &rtc_instance, RTC_COUNT_CALLBACK_COMPARE_0 );
}

uint32_t RtcGetTimerValue( void )
{
    // return ( uint32_t )HwTimerGetTime( );
    return rtc_count_get_count( &rtc_instance );
}

uint32_t RtcGetTimerElapsedTime( void )
{
    return ( uint32_t )( RtcGetTimerValue( ) - RtcTimerContext.Time );
}

uint32_t RtcGetCalendarTime( uint16_t* milliseconds )
{
    uint32_t ticks = 0;

    uint32_t calendarValue = RtcGetTimerValue( );

    uint32_t seconds = ( uint32_t ) calendarValue >> 10;

    ticks = ( uint32_t ) calendarValue & 0x3FF;

    *milliseconds = RtcTick2Ms( ticks );

    return seconds;
}

void RtcBkupWrite( uint32_t data0, uint32_t data1 )
{
    CRITICAL_SECTION_BEGIN( );
    rtc_instance.hw->MODE0.GP[0].reg = data0;
    rtc_instance.hw->MODE0.GP[1].reg = data1;
    CRITICAL_SECTION_END( );
}

void RtcBkupRead( uint32_t* data0, uint32_t* data1 )
{
    CRITICAL_SECTION_BEGIN( );
    *data0 = rtc_instance.hw->MODE0.GP[0].reg;
    *data1 = ( uint32_t ) rtc_instance.hw->MODE0.GP[1].reg;
    CRITICAL_SECTION_END( );
}

void RtcProcess( void )
{
}

TimerTime_t RtcTempCompensation( TimerTime_t period, float temperature )
{
    return period;
}
