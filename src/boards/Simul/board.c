/*!
 * \file      board.c
 *
 * \brief     Target board general functions implementation
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

#include "board.h"
#include <stdlib.h>
#include "timer.h"
#include <stdio.h>

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit( void );

/*!
 * System Clock Configuration
 */
static void SystemClockConfig( void );

/*!
 * Used to measure and calibrate the system wake-up time from STOP mode
 */
static void CalibrateSystemWakeupTime( void );

/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
static void SystemClockReConfig( void );


/*!
 * Callback indicating the end of the system wake-up time calibration
 */
static void OnCalibrateSystemWakeupTimeTimerEvent( void* context )
{
}

/*!
 * Holds the bord version.
 */
static Version_t BoardVersion = { 0 };

void BoardCriticalSectionBegin( uint32_t *mask )
{

}

void BoardCriticalSectionEnd( uint32_t *mask )
{
}

void BoardInitPeriph( void )
{
}

void BoardInitMcu( void )
{
}

void BoardResetMcu( void )
{
}

void BoardDeInitMcu( void )
{
}

uint32_t BoardGetRandomSeed( void )
{
    return 0;
}

void BoardGetUniqueId( uint8_t *id )
{
    id[7] = 0xDE;
    id[6] = 0xAD;
    id[5] = 0xBE;
    id[4] = 0xEF;
    id[3] = 0xDE;
    id[2] = 0xAD;
    id[1] = 0xBE;
    id[0] = 0xEF;
}


uint16_t BoardBatteryMeasureVolage( void )
{
 
}

uint32_t BoardGetBatteryVoltage( void )
{
    return 355;
}

uint8_t BoardGetBatteryLevel( void )
{
  
    return 233;
}

static void BoardUnusedIoInit( void )
{
}


void SystemClockConfig( void )
{
}

void CalibrateSystemWakeupTime( void )
{

}

void SystemClockReConfig( void )
{
}

void SysTick_Handler( void )
{
}

uint8_t GetBoardPowerSource( void )
{

}

void BoardLowPowerHandler( void )
{
}


#include <stddef.h>
#include "radio.h"


void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size ){
    memcpy(dst, src, size);
}
void memset1( uint8_t *dst, uint8_t value, uint16_t size ){
    memset(dst, value, size);
}

void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

/*!
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behavior across
 * different compiler toolchains implementations.
 */
// Standard random functions redefinition start
#define RAND_LOCAL_MAX 2147483647L

int32_t rand1( void )
{
    return random();
}

void srand1( uint32_t seed )
{
    srandom(seed);
}

// Standard random functions redefinition end

int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand1( ) % ( max - min + 1 ) + min;
}

typedef uint32_t TimerTime_t;


/*!
 * Timers list head pointer
 */
static TimerEvent_t *TimerListHead = NULL;

/*!
 * \brief Adds or replace the head timer of the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be become the new head
 * \param [IN]  remainingTime Remaining time of the previous head to be replaced
 */
static void TimerInsertNewHeadTimer( TimerEvent_t *obj );

/*!
 * \brief Adds a timer to the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be added to the list
 * \param [IN]  remainingTime Remaining time of the running head after which the object may be added
 */
static void TimerInsertTimer( TimerEvent_t *obj );

/*!
 * \brief Sets a timeout with the duration "timestamp"
 *
 * \param [IN] timestamp Delay duration
 */
static void TimerSetTimeout( TimerEvent_t *obj );

/*!
 * \brief Check if the Object to be added is not already in the list
 *
 * \param [IN] timestamp Delay duration
 * \retval true (the object is already in the list) or false
 */
static bool TimerExists( TimerEvent_t *obj );


/*!
 * These are hard-coded to help make debug more readable
 */
const char* TimerString[8] =
{ 
    "OnTxDelayedTimerEvent",
    "OnRxWindow1TimerEvent",
    "OnRxWindow2TimerEvent",
    "OnAckTimeoutTimerEvent",
    "Unlabelled 5",
    "Unlabelled 6",
    "Unlabelled 7",
    "Unlabelled 8",
};

TimerEvent_t * timer[8];
uint num_timers = 0;

void poll_timers() {
    int ret, res;
    struct itimerspec ts;
    for(uint i=0; i<num_timers; i++) {
        if(timer[i]->IsStarted) {
            uint time = timer_gettime(timer[i]->t, &ts);
            if(ts.it_value.tv_sec == 0 && ts.it_value.tv_nsec == 0){
                // fire the timer
                if(timer[i]->Callback!=NULL){
                    printf("fire the cb %p\n", timer[i]->t);
                    (*timer[i]->Callback)(timer[i]->Context);
                }
                TimerStart(timer[i]);
            }
        }
    }
}

int get_timer_index(TimerEvent_t *obj) {
    for(uint i=0; i<num_timers; i++) {
        if(timer[i] == obj){
            return i;
        }
    }
    return -1;
}



void TimerInit( TimerEvent_t *obj, void ( *callback )( void *context ) )
{
    timer[num_timers] = obj;
    obj->sev.sigev_notify = SIGEV_NONE;
    timer_create(CLOCK_REALTIME, &obj->sev, &obj->t);
    printf("Creating timer %s : %p\r\n", TimerString[num_timers], obj->t);

    obj->Timestamp = 0;
    obj->ReloadValue = 0;
    obj->IsStarted = false;
    obj->IsNext2Expire = false;
    obj->Callback = callback;
    obj->Context = NULL;

    num_timers++;
}

void TimerSetContext( TimerEvent_t *obj, void* context )
{
    obj->Context = context;
}

void TimerStart( TimerEvent_t *obj )
{

    int i = get_timer_index(obj);
    if(!i){
        printf("Index query failed\r\n");
        return;
    }

    if(TimerString[i] == "OnRxWindow1TimerEvent") {
        (*timer[i]->Callback)(timer[i]->Context);
    } else {

    }

    printf("Starting timer %p for %u ms\r\n", obj->t, obj->ReloadValue);
    struct itimerspec ts;

    ts.it_value.tv_sec = obj->ReloadValue/1000.0;
    ts.it_value.tv_nsec = (obj->ReloadValue % 1000) * 1000;

    // we will manually rearm the timer
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
  
    obj->IsStarted = true;

    if (timer_settime(obj->t, 0, &ts, NULL) < 0) {
        printf("timer_settime() failed");
        return;
    }
}

static void TimerInsertTimer( TimerEvent_t *obj )
{
}

static void TimerInsertNewHeadTimer( TimerEvent_t *obj )
{
}

bool TimerIsStarted( TimerEvent_t *obj )
{

}

void TimerIrqHandler( void )
{
}

void TimerStop( TimerEvent_t *obj )
{
    printf("Stopping timer %p\r\n", obj->t);
    struct itimerspec ts;

    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
  
    obj->IsStarted = false;

    if (timer_settime(obj->t, 0, &ts, NULL) < 0) {
        printf("timer_settime() failed");
        return;
    }
}

static bool TimerExists( TimerEvent_t *obj )
{
}

void TimerReset( TimerEvent_t *obj )
{

}

// given in 
void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    printf("setting timervalue\r\n");
    uint32_t minValue = 0;

    TimerStop( obj );

    obj->Timestamp = value;
    obj->ReloadValue = value;
}

TimerTime_t TimerGetCurrentTime( void )
{
    return  0;
}

TimerTime_t TimerGetElapsedTime( TimerTime_t past )
{

}

static void TimerSetTimeout( TimerEvent_t *obj )
{
    
}

TimerTime_t TimerTempCompensation( TimerTime_t period, float temperature )
{
    return 0;
}

void TimerProcess( void )
{
}
