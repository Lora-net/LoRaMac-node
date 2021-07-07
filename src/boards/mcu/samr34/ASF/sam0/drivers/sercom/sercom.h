/**
 * \file
 *
 * \brief SAM Serial Peripheral Interface Driver
 *
 * Copyright (c) 2012-2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef SERCOM_H_INCLUDED
#define SERCOM_H_INCLUDED

#include <compiler.h>
#include <system.h>
#include <clock.h>
#include <system_interrupt.h>
#include "sercom_pinout.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SERCOM modules should share  same slow GCLK channel ID */
#define SERCOM_GCLK_ID SERCOM0_GCLK_ID_SLOW

#if (0x1ff >= REV_SERCOM)
#  define FEATURE_SERCOM_SYNCBUSY_SCHEME_VERSION_1
#elif (0x400 >= REV_SERCOM)
#  define FEATURE_SERCOM_SYNCBUSY_SCHEME_VERSION_2
#else
#  error "Unknown SYNCBUSY scheme for this SERCOM revision"
#endif

/**
 * \brief sercom asynchronous operation mode
 *
 * Select sercom asynchronous operation mode
 */
enum sercom_asynchronous_operation_mode {
	SERCOM_ASYNC_OPERATION_MODE_ARITHMETIC = 0,
	SERCOM_ASYNC_OPERATION_MODE_FRACTIONAL,
};

/**
 * \brief sercom asynchronous samples per bit
 *
 * Select number of samples per bit
 */
enum sercom_asynchronous_sample_num {
	SERCOM_ASYNC_SAMPLE_NUM_3 = 3,
	SERCOM_ASYNC_SAMPLE_NUM_8 = 8,
	SERCOM_ASYNC_SAMPLE_NUM_16 = 16,
};

enum status_code sercom_set_gclk_generator(
		const enum gclk_generator generator_source,
		const bool force_change);

enum status_code _sercom_get_sync_baud_val(
		const uint32_t baudrate,
		const uint32_t external_clock,
		uint16_t *const baudval);

enum status_code _sercom_get_async_baud_val(
		const uint32_t baudrate,
		const uint32_t peripheral_clock,
		uint16_t *const baudval,
		enum sercom_asynchronous_operation_mode mode,
		enum sercom_asynchronous_sample_num sample_num);

uint32_t _sercom_get_default_pad(
		Sercom *const sercom_module,
		const uint8_t pad);

uint8_t _sercom_get_sercom_inst_index(
		Sercom *const sercom_instance);
#ifdef __cplusplus
}
#endif

#endif //__SERCOM_H_INCLUDED
