/**
 * \file
 *
 * \brief SAM Peripheral Analog-to-Digital Converter Driver
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

#ifndef ADC_CALLBACK_H_INCLUDED
#define ADC_CALLBACK_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup asfdoc_sam0_adc_group
 *
 * @{
 */

#include <adc.h>

/**
 * Enum for the possible types of ADC asynchronous jobs that may be issued to
 * the driver.
 */
enum adc_job_type {
	/** Asynchronous ADC read into a user provided buffer */
	ADC_JOB_READ_BUFFER,
};

/**
 * \name Callback Management
 * @{
 */
void adc_register_callback(
		struct adc_module *const module,
		adc_callback_t callback_func,
		enum adc_callback callback_type);

void adc_unregister_callback(
		struct adc_module *module,
		enum adc_callback callback_type);

/**
 * \brief Enables callback.
 *
 * Enables the callback function registered by \ref
 * adc_register_callback. The callback function will be called from the
 * interrupt handler when the conditions for the callback type are met.
 *
 * \param[in]     module Pointer to ADC software instance struct
 * \param[in]     callback_type Callback type given by an enum
 *
 * \return    Status of the operation.
 * \retval     STATUS_OK              If operation was completed
 * \retval     STATUS_ERR_INVALID     If operation was not completed,
 *                                    due to invalid callback_type
 *
 */
static inline void adc_enable_callback(
		struct adc_module *const module,
		enum adc_callback callback_type)
{
	/* Sanity check arguments */
	Assert(module);

	/* Enable callback */
	module->enabled_callback_mask |= (1 << callback_type);

	/* Enable window interrupt if this is a window callback */
	if (callback_type == ADC_CALLBACK_WINDOW) {
		adc_enable_interrupt(module, ADC_INTERRUPT_WINDOW);
	}
	/* Enable overrun interrupt if error callback is registered */
	if (callback_type == ADC_CALLBACK_ERROR) {
		adc_enable_interrupt(module, ADC_INTERRUPT_OVERRUN);
	}
}

/**
 * \brief Disables callback.
 *
 * Disables the callback function registered by the \ref
 * adc_register_callback.
 *
 * \param[in]     module Pointer to ADC software instance struct
 * \param[in]     callback_type Callback type given by an enum
 *
 * \return    Status of the operation.
 * \retval     STATUS_OK              If operation was completed
 * \retval     STATUS_ERR_INVALID     If operation was not completed,
 *                                    due to invalid callback_type
 *
 */
static inline void adc_disable_callback(
		struct adc_module *const module,
		enum adc_callback callback_type)
{
	/* Sanity check arguments */
	Assert(module);

	/* Disable callback */
	module->enabled_callback_mask &= ~(1 << callback_type);

	/* Disable window interrupt if this is a window callback */
	if (callback_type == ADC_CALLBACK_WINDOW) {
		adc_disable_interrupt(module, ADC_INTERRUPT_WINDOW);
	}
	/* Disable overrun interrupt if this is the error callback */
	if (callback_type == ADC_CALLBACK_ERROR) {
		adc_disable_interrupt(module, ADC_INTERRUPT_OVERRUN);
	}
}

/** @} */


/**
 * \name Job Management
 * @{
 */
enum status_code adc_read_buffer_job(
		struct adc_module *const module_inst,
		uint16_t *buffer,
		uint16_t samples);

enum status_code adc_get_job_status(
		struct adc_module *module_inst,
		enum adc_job_type type);

void adc_abort_job(
		struct adc_module *module_inst,
		enum adc_job_type type);
/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ADC_CALLBACK_H_INCLUDED */
