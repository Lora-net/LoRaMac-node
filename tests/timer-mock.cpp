/*!
 * \file      timer.c
 *
 * \brief     Timer objects and scheduling management implementation
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
 * \author Modified for mocking by Medad Newman
 * 
 * This file implements a mock of the timer.c file, using a c++ list to hold all pointers 
 * to timers. It calls the call back of each timer when it expires. 
 */

#include "CppUTestExt/MockSupport.h"
#include <list>

/*!
 * Safely execute call back
 */
#define ExecuteCallBack(_callback_, context) \
    do                                       \
    {                                        \
        if (_callback_ == NULL)              \
        {                                    \
            while (1)                        \
                ;                            \
        }                                    \
        else                                 \
        {                                    \
            _callback_(context);             \
        }                                    \
    } while (0);

extern "C"
{
#include "timer.h"
}
/*!
 * Timers list head pointer
 */
TimerTime_t current_time = 0;

TimerEvent_t *list_of_timer_event_pointers[100000]; /* There won't be more than 100 timers in the project */
int number_of_timers = 0;

void TimerInit(TimerEvent_t *obj, void (*callback)(void *context))
{
    obj->Timestamp = 0;
    obj->ReloadValue = 0;
    obj->IsStarted = false;
    obj->IsNext2Expire = false;
    obj->Callback = callback;
    obj->Context = NULL;
    obj->Next = NULL;

    list_of_timer_event_pointers[number_of_timers] = obj;
    number_of_timers++;
}

void TimerStart(TimerEvent_t *obj)
{
    obj->Timestamp = obj->ReloadValue;
    obj->IsStarted = true;
    obj->IsNext2Expire = false;
}

void TimerIrqHandler(void)
{
    /**
     * @brief Check through countdown timers, and decrement them if they are active.
     * If hits zero, set them inactive(IsStarted = false), and set IsNext2Expire
     * 
     * @param list_of_timer_event_pointers 
     */
    for (int i = 0; i < number_of_timers; ++i)
    {
        TimerEvent_t *timer_event = list_of_timer_event_pointers[i];
        // printf("Timestamp[ms]: %d\n", timer_event->Timestamp);

        if (timer_event->IsStarted)
        {
            timer_event->Timestamp--;

            if (timer_event->Timestamp == 0)
            {
                timer_event->IsNext2Expire = true;
                timer_event->IsStarted = false;
            }
        }
    }

    // printf("\n");

    /**
     * Now here, execute the call back of first expiring timer
     * Leave the execution of the next expiring timer to the next time 
     * TimerIrqHandler() is called, to allow stuff to update on the main
     * main loop.
     * 
     */
    for (int i = 0; i < number_of_timers; ++i)
    {
        TimerEvent_t *timer_event = list_of_timer_event_pointers[i];
        if (timer_event->IsNext2Expire == true)
        {
            ExecuteCallBack(timer_event->Callback, timer_event->Context);
            timer_event->IsNext2Expire = false;
            return;
        }
    }
}

void TimerStop(TimerEvent_t *obj)
{
    obj->IsStarted = false;
}

void TimerSetValue(TimerEvent_t *obj, uint32_t value)
{

    TimerStop(obj);

    obj->Timestamp = value;
    obj->ReloadValue = value;
}

TimerTime_t TimerGetCurrentTime(void)
{
    return current_time;
}

TimerTime_t TimerGetElapsedTime(TimerTime_t past)
{
    return current_time - past;
}
