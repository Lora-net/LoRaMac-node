/**
 * \file
 *
 * \brief Common Delay Service
 *
 * Copyright (c) 2013-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup group_common_services_delay Busy-Wait Delay Routines
 *
 * This module provides simple loop-based delay routines for those
 * applications requiring a brief wait during execution. Common for
 * API ver. 2.
 *
 * @{
 */

#ifdef SYSTICK_MODE
#include "sam0/systick_counter.h"
#endif
#ifdef CYCLE_MODE
#include "sam0/cycle_counter.h"
#endif

void delay_init(void);

/**
 * \def delay_s
 * \brief Delay in at least specified number of seconds.
 * \param delay Delay in seconds
 */
#define delay_s(delay)          ((delay) ? cpu_delay_s(delay) : cpu_delay_us(1))

/**
 * \def delay_ms
 * \brief Delay in at least specified number of milliseconds.
 * \param delay Delay in milliseconds
 */
#define delay_ms(delay)         ((delay) ? cpu_delay_ms(delay) : cpu_delay_us(1))

/**
 * \def delay_us
 * \brief Delay in at least specified number of microseconds.
 * \param delay Delay in microseconds
 */
#define delay_us(delay)         ((delay) ? cpu_delay_us(delay) : cpu_delay_us(1))

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* DELAY_H_INCLUDED */
