/**
 * @file deep_sleep_delay.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "deep_sleep_delay.h"
#include "timer.h"
#include "stdbool.h"
#include "board.h"

static TimerEvent_t DeepSleepDelayTimeoutTimer;
static void DeepSleepDelayTimeoutEvent(void *context);
volatile bool timeout;

void DeepSleepDelayMsInit()
{
    TimerInit(&DeepSleepDelayTimeoutTimer, DeepSleepDelayTimeoutEvent);
}

void DeepSleepDelayMs(uint32_t ms)
{
    TimerSetValue(&DeepSleepDelayTimeoutTimer, ms);
    TimerStart(&DeepSleepDelayTimeoutTimer);

    timeout = false;

    while (1)
    {

        BoardLowPowerHandler();
        if (timeout == true)
        {
            break;
        }
    }

    TimerStop(&DeepSleepDelayTimeoutTimer);
}

static void DeepSleepDelayTimeoutEvent(void *context)
{
    timeout = true;
}
