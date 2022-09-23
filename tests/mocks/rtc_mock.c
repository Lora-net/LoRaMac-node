/**
 * @file rtc_mock.cpp
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief
 * @version 0.1
 * @date 2022-08-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "rtc_mock.h"
#include "stdint.h"
#include "stdbool.h"
#include "stm32l0xx.h"
#include "rtc-board.h"


extern uint32_t countdown;
extern bool alarm_stopped;
extern TimerTime_t current_time;


void bump_rtc_tick()
{
    current_time += 1; /* simulate 1 millisecond per loop */
    if (countdown > 0 && !alarm_stopped)
    {
        countdown--;
    }

    // printf("current time: %d ms, countdown: %d ms, alarm_stopped: %d\n", current_time, countdown, alarm_stopped);

    if (countdown == 0)
    {
        RTC_HandleTypeDef dummy;
        HAL_RTC_AlarmAEventCallback(&dummy);
    }
}
