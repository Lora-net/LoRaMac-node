/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Timer objects and scheduling management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "timer-board.h"
#include "rtc-board.h"

/*!
 * This flag is set to "true" when the timer is stopped due to an external IRQ
 */
volatile bool TimerEarlyStopFlag = false;

/*!
 * This flag is used to make sure we have looped through all condition to avoid race issues
 */
static bool HasLoopedTwice = false;

/*!
 * Timers list head pointer
 */
static TimerEvent_t *TimerListHead = NULL;

/*!
 * \brief Adds a timer to the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param obj Timer object to be added to the list
 */
static void TimerListAddObj( TimerEvent_t *obj );

/*!
 * \brief Removes the given timer from the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param obj Timer object to be removed from the list
 */
static void TimerListRemoveObj( TimerEvent_t *obj );

/*!
 * \brief Parses the timer list and updates all timestamps.
 *
 * \remark The list is sorted.
 *
 * \param elapsedTime Elapsed time since last call. This value can be either
 *                    the return value of TimerGetTimerValue function or the
 *                    list head current time stamp.
 */
static void TimerListUpdateTime( TimerEvent_t *obj, uint32_t elapsedTime );

/*!
 * \brief Parses the timer list and executes the callback function of expired
 *        timers.
 *
 * \remark Once the callback function is executed the timer is removed from
 *         the list.
 */
static void TimerListExpiredExecute( void );

/*!
 * \brief Sets a timeout with the duration "timestamp"
 * 
 * \param [IN] timestamp Delay duration
 */
static void TimerSetTimeout( uint32_t timestamp );

/*!
 * \brief Read the timer value of the currently running timer
 * 
 * \param [IN] Timer object running
 */
uint32_t TimerReadTimers( TimerEvent_t *obj );


void TimerInit( TimerEvent_t *obj, void ( *callback )( void ) )
{
    obj->Timestamp = 0;
    obj->ReloadValue = 0;
    obj->IsStoppedEarly = false;
    obj->IsRunning = false;
    obj->Callback = callback;
}

void TimerStart( TimerEvent_t *obj )
{
    __disable_irq( );

    // Remove the timer from the list
    TimerListRemoveObj( obj );
    
    // Reset the object
    obj->Timestamp = obj->ReloadValue;
    obj->IsRunning = false;
    obj->IsStoppedEarly = false;

    // Add the object to the timer list
    TimerListAddObj( obj );
        
    __enable_irq( );
}

void TimerStop( TimerEvent_t *obj ) 
{
    __disable_irq( );

    TimerEvent_t* cur = TimerListHead;

    if( ( cur == obj ) && ( cur->IsRunning == true ) )
    {   
#ifdef LOW_POWER_MODE_ENABLE
        RtcStopTimer( );
#else    
        TimerHwStop( );
#endif
        cur->IsRunning = false;

        // At least one Timer event has been stopped before its due time
        TimerEarlyStopFlag = true;
        // Indicates which timer has stopped
        cur->IsStoppedEarly = true;
    }
    else
    {
        // Remove the timer from the list
        TimerListRemoveObj( obj );
    }

    __enable_irq( );
}

void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    TimerStop( obj );
    obj->Timestamp = value;
    obj->ReloadValue = value;
}

uint32_t TimerReadTimers( TimerEvent_t *obj )
{
    uint32_t valTimer = 0;

#ifdef LOW_POWER_MODE_ENABLE
    valTimer = RtcGetTimerValue( );
#else
    valTimer = obj->ReloadValue - TimerHwGetTimerValue( );    
#endif

    return valTimer;
}

void TimerReset( TimerEvent_t *obj )
{
    TimerStop( obj );
    TimerStart( obj );
}

void TimerIrqHandler( void )
{
    // Update all listed timers
    TimerListUpdateTime( TimerListHead, TimerListHead->Timestamp );
    TimerListExpiredExecute( );
}

static void TimerListAddObj( TimerEvent_t *obj )
{
    TimerEvent_t* prev = TimerListHead;
    TimerEvent_t* cur = TimerListHead;

    if( TimerListHead == NULL )
    {
        TimerListHead = obj;
    }
    else
    {
        while( ( cur != NULL ) && ( cur->Timestamp < obj->Timestamp ) )
        {
            prev = cur;
            cur = cur->Next;
        }
        
        if( TimerListHead == cur )
        {
            cur->IsRunning = false;
            obj->Next = cur;
            TimerListHead = obj;
            TimerListHead->IsRunning = false;
        }
        else
        {
            obj->Next = cur;
            prev->Next = obj;
        }
    }
}

static void TimerListRemoveObj( TimerEvent_t *obj ) 
{
    TimerEvent_t* prev = TimerListHead;
    TimerEvent_t* cur = TimerListHead;

    if( cur == NULL )
    {
        // List empty
        return;
    }
    
    if( cur == obj )
    {
        // object is the first element
        TimerListHead = cur->Next;
        obj->Next = NULL;
        return;
    }
    // Search the object in the list
    while( ( cur != obj ) && ( cur->Next != NULL ) )
    {
        prev = cur;
        cur = cur->Next;
    }
    if( cur == obj )
    {
        prev->Next = cur->Next;
        obj->Next = NULL;
    }
}

static void TimerListUpdateTime( TimerEvent_t *obj, uint32_t elapsedTime )
{
    TimerEvent_t* cur = obj;

    while( cur != NULL )
    {
        if( elapsedTime > cur->Timestamp )
        {
            cur->Timestamp = 0; 
        }
        else
        {
            cur->Timestamp -= elapsedTime; 
        }
        cur = cur->Next;
    }
}

static void TimerListExpiredExecute( void )
{
    TimerEvent_t* prev = TimerListHead;
    TimerEvent_t* cur = TimerListHead;
    
    while( cur != NULL )
    {
        prev = cur;
        cur = cur->Next;
        if( ( int )prev->Timestamp <= 0 )
        {
            if( prev->Callback != NULL )
            {
                prev->Callback( );
            }
            TimerListRemoveObj( prev );
        }
    }
}

static void TimerSetTimeout( uint32_t timestamp )
{
#ifdef LOW_POWER_MODE_ENABLE
        RtcSetTimeout( timestamp );
#else
        TimerHwStart( timestamp );
#endif
}

void TimerHandleEvent( void )
{
    uint32_t currentTimerValue = 0;
    
    if( TimerEarlyStopFlag == true )    // the last timer running has been stopped before the end due to an IRQ
    {
        HasLoopedTwice = false;
    
        TimerEvent_t* cur = TimerListHead;
    
        while( cur != NULL )
        {
            if( cur->IsStoppedEarly == true )
            {
                break;
            }
            cur = cur->Next;
        }
    
        // read the value at which it stopped
        currentTimerValue = TimerReadTimers( cur );
    
        // update the timestamps for the following events
        TimerListUpdateTime( cur, currentTimerValue );
    
        // Remove the timer which stop early from the list
        TimerListRemoveObj( cur ); 
    
        TimerEarlyStopFlag = false;
    }

    if( HasLoopedTwice == false )
    {
        HasLoopedTwice = true;
    }
    else
    {        
        HasLoopedTwice = false;
    
        // start the next timer
        if( ( TimerListHead != NULL ) && ( TimerListHead->IsRunning == false ) )
        {    
            TimerListHead->IsRunning = true; 
            TimerSetTimeout( TimerListHead->Timestamp ); 
        }
        else if( ( TimerListHead != NULL ) && ( TimerListHead->IsRunning == true ) ) 
        {
#ifdef LOW_POWER_MODE_ENABLE
            RtcEnterLowPowerStopMode( );
#else
#ifndef USE_DEBUGGER
            __WFI( );
#endif
#endif
        }
    }

    if( TimerListHead == NULL )
    {
        HasLoopedTwice = false;
    
#ifdef LOW_POWER_MODE_ENABLE
        RtcStopTimer( );
#else
        TimerHwStop( );
#endif
    }
}

