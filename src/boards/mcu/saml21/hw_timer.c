/**
* \file  hw_timer.c
*
* \brief Wrapper used by sw_timer utility using ASF timer api's
*
* Copyright (C) 2016 Atmel Corporation. All rights reserved.
*
* \asf_license_start
*
* \page License
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3. The name of Atmel may not be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* 4. This software may only be redistributed and used in connection with an
*    Atmel microcontroller product.
*
* THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
* EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* \asf_license_stop
*
* Modified for use with Atmel START hpl_rtc functions
*/

/**************************************** INCLUDES*****************************/
#include <stdint.h>
#include <stdbool.h>
#include <utils_assert.h>
#include <utils.h>
#include <hal_atomic.h>
#include <hpl_irq.h>

#include "board-config.h"
#include "gpio.h"
#include "hw_timer.h"


/**************************************** MACROS*****************************/

#define COMPARE_COUNT_MAX_VALUE 0xFFFFFFFF

/*TODO Remove after the fix for LORA_DEV-104*/
#define RTC_COMPARE_MATCH_COUNT_ERROR   4 //TICKS

/**************************************** GLOBALS*****************************/

static bool callback_enabled = false;
hwTImerCallback_t callback = NULL;
static volatile uint32_t ticksPrevious=0,ticksCurrent = 0;

#if defined( USE_HWTMR_DEBUG )
Gpio_t DbgHwTmrPin;
#endif

/************************************** IMPLEMENTATION************************/

/**
* \brief Initializes the hw timer module
*/
void HwTimerInit(void)
{
#if defined( USE_HWTMR_DEBUG )
    GpioInit( &DbgHwTmrPin, HWTMR_DBG_PIN_0, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

    hri_mclk_set_APBAMASK_RTC_bit(MCLK);
    hri_rtcmode0_write_CTRLA_reg(RTC, RTC_MODE0_CTRLA_SWRST);
    hri_rtcmode0_wait_for_sync(RTC, RTC_MODE0_SYNCBUSY_SWRST);

    hri_rtcmode0_write_CTRLA_reg(RTC, RTC_MODE0_CTRLA_PRESCALER(0) |
                                 RTC_MODE0_CTRLA_COUNTSYNC);
    hri_rtcmode0_write_EVCTRL_reg(RTC, RTC_MODE0_EVCTRL_CMPEO0);
    hri_rtcmode0_write_COMP_reg(RTC, 0, COMPARE_COUNT_MAX_VALUE);
    hri_rtcmode0_write_COMP_reg(RTC, 1, COMPARE_COUNT_MAX_VALUE);
    hri_rtcmode0_set_INTEN_CMP0_bit(RTC);

    NVIC_EnableIRQ(RTC_IRQn);
    hri_rtcmode0_write_COUNT_reg(RTC, 0);
    hri_rtcmode0_wait_for_sync(RTC, RTC_MODE0_SYNCBUSY_COUNT);
    hri_rtcmode0_set_CTRLA_ENABLE_bit(RTC);
}

/**
* \brief This function is used to set the callback when the hw timer
* expires.
* \param callback Callback to be registered
*/
void HwTimerSetCallback(hwTImerCallback_t newCallback)
{
    callback = newCallback;
}

/**
* \brief Loads the timeout in terms of ticks into the hardware
* \ticks Time value in terms of timer ticks
*/
bool HwTimerLoadAbsoluteTicks(uint32_t ticks)
{
#if defined( USE_HWTMR_DEBUG )
    GpioWrite( &DbgHwTmrPin, 1 );
#endif

    callback_enabled = true;
    RTC_CRITICAL_SECTION_ENTER();
    hri_rtcmode0_write_COMP_reg(RTC, 0, ticks);
    hri_rtcmode0_wait_for_sync(RTC, RTC_MODE0_SYNCBUSY_MASK);
    uint32_t current = hri_rtcmode0_read_COUNT_reg(RTC);
    RTC_CRITICAL_SECTION_LEAVE();

    if((ticks - current - 1) >= (COMPARE_COUNT_MAX_VALUE >> 1)) {
        // if difference is more than half of max assume timer has passed
        return false;
    }
    if((ticks - current) < 10) {
        // if too close the matching interrupt does not trigger, so handle same as passed
        return false;
    }
    return true;
}

/**
* \brief Gets the absolute time value
* \retval Absolute time in ticks
*/
uint32_t HwTimerGetTime(void)
{
    hri_rtcmode0_wait_for_sync(RTC, RTC_MODE0_SYNCBUSY_COUNT);
    ticksCurrent = hri_rtcmode0_read_COUNT_reg(RTC);

    return ticksCurrent;
}

/**
* \brief Disables the hw timer module
*/
void HwTimerDisable(void)
{
    callback_enabled = false;
}

/**
* \brief Rtc interrupt handler
*/
void RTC_Handler(void)
{
    /* Read and mask interrupt flag register */
    uint16_t flag = hri_rtcmode0_read_INTFLAG_reg(RTC);

    if (flag & RTC_MODE0_INTFLAG_CMP0) {
#if defined( USE_HWTMR_DEBUG )
        GpioWrite( &DbgHwTmrPin, 0 );
#endif
        hri_rtcmode0_clear_interrupt_CMP0_bit(RTC);
        if (callback) {
            callback();
        }
        /* Clear interrupt flag */
    }
}

/* eof hw_timer.c */
