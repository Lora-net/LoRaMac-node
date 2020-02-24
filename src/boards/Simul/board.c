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


/*!
 * \brief Timer object description
 */
typedef struct TimerEvent_s
{
    uint32_t Timestamp;                  //! Current timer value
    uint32_t ReloadValue;                //! Timer delay value
    bool IsStarted;                      //! Is the timer currently running
    bool IsNext2Expire;                  //! Is the next timer to expire
    void ( *Callback )( void* context ); //! Timer IRQ callback function
    void *Context;                       //! User defined data object pointer to pass back
    struct TimerEvent_s *Next;           //! Pointer to the next Timer object.
}TimerEvent_t;

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

void TimerInit( TimerEvent_t *obj, void ( *callback )( void *context ) )
{
}

void TimerSetContext( TimerEvent_t *obj, void* context )
{
}

void TimerStart( TimerEvent_t *obj )
{
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
}

static bool TimerExists( TimerEvent_t *obj )
{
}

void TimerReset( TimerEvent_t *obj )
{
}

void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{

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
