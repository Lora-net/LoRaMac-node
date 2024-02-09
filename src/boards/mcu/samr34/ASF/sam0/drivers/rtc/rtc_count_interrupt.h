/**
 * \file
 *
 * \brief SAM RTC Driver (Count Interrupt Mode)
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

#ifndef RTC_COUNT_INTERRUPT_H_INCLUDED
#define RTC_COUNT_INTERRUPT_H_INCLUDED

#include "rtc_count.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup asfdoc_sam0_rtc_count_group
 * @{
 */

 /**
 * \name Callbacks
 * @{
 */
enum status_code rtc_count_register_callback(
		struct rtc_module *const module,
		rtc_count_callback_t callback,
		enum rtc_count_callback callback_type);

enum status_code rtc_count_unregister_callback(
		struct rtc_module *const module,
		enum rtc_count_callback callback_type);

void rtc_count_enable_callback(
		struct rtc_module *const module,
		enum rtc_count_callback callback_type);

void rtc_count_disable_callback(
		struct rtc_module *const module,
		enum rtc_count_callback callback_type);

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* RTC_COUNT_INTERRUPT_H_INCLUDED */
