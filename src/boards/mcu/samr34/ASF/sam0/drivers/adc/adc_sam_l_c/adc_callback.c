/**
 * \file
 *
 * \brief SAM Peripheral Analog-to-Digital Converter Driver
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
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
#include "adc_callback.h"

struct adc_module *_adc_instances[ADC_INST_NUM];

static void _adc_interrupt_handler(const uint8_t instance)
{
	struct adc_module *module = _adc_instances[instance];

	/* get interrupt flags and mask out enabled callbacks */
	uint32_t flags = module->hw->INTFLAG.reg & module->hw->INTENSET.reg;

	if (flags & ADC_INTFLAG_RESRDY) {
		/* clear interrupt flag */
		module->hw->INTFLAG.reg = ADC_INTFLAG_RESRDY;

		/* store ADC result in job buffer */
		*(module->job_buffer++) = module->hw->RESULT.reg;

		if (--module->remaining_conversions > 0) {
			if (module->software_trigger == true
				&& (!(module->hw->SEQSTATUS.reg & ADC_SEQSTATUS_SEQBUSY))) {
				adc_start_conversion(module);
			}
		} else {
			adc_disable_interrupt(module, ADC_INTERRUPT_RESULT_READY);
			if (module->job_status == STATUS_BUSY) {
				/* job is complete. update status,disable interrupt
				 *and call callback */
				module->job_status = STATUS_OK;

				if ((module->enabled_callback_mask &
						(1 << ADC_CALLBACK_READ_BUFFER)) &&
					(module->registered_callback_mask &
						(1 << ADC_CALLBACK_READ_BUFFER))) {
					(module->callback[ADC_CALLBACK_READ_BUFFER])(module);
				}
			}
		}
	}

	if (flags & ADC_INTFLAG_WINMON) {
		module->hw->INTFLAG.reg = ADC_INTFLAG_WINMON;
		if ((module->enabled_callback_mask & (1 << ADC_CALLBACK_WINDOW)) &&
				(module->registered_callback_mask & (1 << ADC_CALLBACK_WINDOW))) {
			(module->callback[ADC_CALLBACK_WINDOW])(module);
		}

	}

	if (flags & ADC_INTFLAG_OVERRUN) {
		module->hw->INTFLAG.reg = ADC_INTFLAG_OVERRUN;
		if ((module->enabled_callback_mask & (1 << ADC_CALLBACK_ERROR)) &&
				(module->registered_callback_mask & (1 << ADC_CALLBACK_ERROR))) {
			(module->callback[ADC_CALLBACK_ERROR])(module);
		}
	}
}

/** Interrupt handler for the ADC module. */
#if (ADC_INST_NUM > 1) || (SAMC20)
#   define _ADC_INTERRUPT_HANDLER(n, m) \
		void ADC##n##_Handler(void) \
		{ \
			_adc_interrupt_handler(n); \
		}

	MREPEAT(ADC_INST_NUM, _ADC_INTERRUPT_HANDLER, 0)
#else
void ADC_Handler(void)
{
	_adc_interrupt_handler(0);
}
#endif

/**
 * \brief Registers a callback.
 *
 * Registers a callback function which is implemented by the user.
 *
 * \note The callback must be enabled for the interrupt handler to call it
 * when the condition for the callback is met.
 *
 * \param[in] module         Pointer to ADC software instance struct
 * \param[in] callback_func  Pointer to callback function
 * \param[in] callback_type  Callback type given by an enum
 *
 */
void adc_register_callback(
		struct adc_module *const module,
		adc_callback_t callback_func,
		enum adc_callback callback_type)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(callback_func);

	/* Register callback function */
	module->callback[callback_type] = callback_func;

	/* Set the bit corresponding to the callback_type */
	module->registered_callback_mask |= (1 << callback_type);
}

/**
 * \brief Unregisters a callback.
 *
 * Unregisters a callback function which is implemented by the user.
 *
 * \param[in] module         Pointer to ADC software instance struct
 * \param[in] callback_type  Callback type given by an enum
 *
 */
void adc_unregister_callback(
		struct adc_module *const module,
		enum adc_callback callback_type)
{
	/* Sanity check arguments */
	Assert(module);

	/* Unregister callback function */
	module->callback[callback_type] = NULL;

	/* Clear the bit corresponding to the callback_type */
	module->registered_callback_mask &= ~(1 << callback_type);
}

/**
 * \brief Read multiple samples from ADC.
 *
 * Read \c samples from the ADC into the \c buffer.
 * If there is no hardware trigger defined (event action) the
 * driver will retrigger the ADC conversion whenever a conversion
 * is complete until \c samples has been acquired. To avoid
 * jitter in the sampling frequency using an event trigger is advised.
 *
 * \param[in]  module_inst  Pointer to the ADC software instance struct
 * \param[in]  samples      Number of samples to acquire
 * \param[out] buffer       Buffer to store the ADC samples
 *
 * \return Status of the job start.
 * \retval STATUS_OK        The conversion job was started successfully and is
 *                          in progress
 * \retval STATUS_BUSY      The ADC is already busy with another job
 */
enum status_code adc_read_buffer_job(
		struct adc_module *const module_inst,
		uint16_t *buffer,
		uint16_t samples)
{
	Assert(module_inst);
	Assert(samples);
	Assert(buffer);

	if(module_inst->remaining_conversions != 0 ||
			module_inst->job_status == STATUS_BUSY){
		return STATUS_BUSY;
	}

	module_inst->job_status = STATUS_BUSY;
	module_inst->remaining_conversions = samples;
	module_inst->job_buffer = buffer;

	adc_enable_interrupt(module_inst, ADC_INTERRUPT_RESULT_READY);

	if(module_inst->software_trigger == true) {
		adc_start_conversion(module_inst);
	}

	return STATUS_OK;
}

/**
 * \brief Gets the status of a job.
 *
 * Gets the status of an ongoing or the last job.
 *
 * \param [in]  module_inst Pointer to the ADC software instance struct
 * \param [in]  type        Type of to get status
 *
 * \return Status of the job.
 */
enum status_code adc_get_job_status(
		struct adc_module *module_inst,
		enum adc_job_type type)
{
	/* Sanity check arguments */
	Assert(module_inst);

	if (type == ADC_JOB_READ_BUFFER ) {
		return module_inst->job_status;
	} else {
		return STATUS_ERR_INVALID_ARG;
	}
}

/**
 * \brief Aborts an ongoing job.
 *
 * Aborts an ongoing job.
 *
 * \param [in]  module_inst Pointer to the ADC software instance struct
 * \param [in]  type        Type of job to abort
 */
void adc_abort_job(
		struct adc_module *module_inst,
		enum adc_job_type type)
{
	/* Sanity check arguments */
	Assert(module_inst);

	if (type == ADC_JOB_READ_BUFFER) {
		/* Disable interrupt */
		adc_disable_interrupt(module_inst, ADC_INTERRUPT_RESULT_READY);
		/* Mark job as aborted */
		module_inst->job_status = STATUS_ABORTED;
		module_inst->remaining_conversions = 0;
	}
}

