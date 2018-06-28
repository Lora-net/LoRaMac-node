/**
* \file  hw_timer.h
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
*/

#ifndef HW_TIMER_H_INCLUDED
#define HW_TIMER_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** INCLUDES*****************************/

/**************************************** MACROS******************************/

/**************************************** TYPES*****************************/

typedef void (*HwTimerCallback_t) (void);

/***************************************PROTOTYPES**************************/

/**
* \brief Initializes the hw timer module
*/
void HwTimerInit(void);

/**
* \brief This function is used to set the callback when the hw timer
* expires.
* \param callback Callback to be registered
*/
void HwTimerAlarmSetCallback(HwTimerCallback_t callback);

/**
* \brief This function is used to set the callback when the hw timer
* overflows.
* \param callback Callback to be registered
*/
void HwTimerOverflowSetCallback(HwTimerCallback_t callback);

/**
* \brief Loads the absolute timeout in terms of ticks into the hardware
* \ticks Absolute time value in terms of timer ticks
*/
bool HwTimerLoadAbsoluteTicks(uint32_t ticks);

/**
* \brief Gets the absolute time value
* \retval Absolute time in ticks
*/
uint32_t HwTimerGetTime(void);

/**
* \brief Disables the hw timer module
*/
void HwTimerDisable(void);

#ifdef	__cplusplus
}
#endif
#endif /* HW_TIMER_H_INCLUDED */
