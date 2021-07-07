/**
 * \file
 *
 * \brief SAM External Interrupt Driver
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
#ifndef EXTINT_CALLBACK_H_INCLUDED
#define EXTINT_CALLBACK_H_INCLUDED

#include <compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup asfdoc_sam0_extint_group
 *
 * @{
 */

/** \name Callback Configuration and Initialization
 * @{
 */

/** Enum for the possible callback types for the EXTINT module. */
enum extint_callback_type
{
	/** Callback type for when an external interrupt detects the configured
	 *  channel criteria (i.e. edge or level detection)
	 */
	EXTINT_CALLBACK_TYPE_DETECT,
};

enum status_code extint_register_callback(
	const extint_callback_t callback,
	const uint8_t channel,
	const enum extint_callback_type type);

enum status_code extint_unregister_callback(
	const extint_callback_t callback,
	const uint8_t channel,
	const enum extint_callback_type type);

uint8_t extint_get_current_channel(void);

/** @} */

/** \name Callback Enabling and Disabling (Channel)
 * @{
 */

enum status_code extint_chan_enable_callback(
	const uint8_t channel,
	const enum extint_callback_type type);

enum status_code extint_chan_disable_callback(
	const uint8_t channel,
	const enum extint_callback_type type);

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif
