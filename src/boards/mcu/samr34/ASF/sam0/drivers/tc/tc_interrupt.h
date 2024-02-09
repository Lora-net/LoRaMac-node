/**
 * \file
 *
 * \brief SAM TC - Timer Counter Callback Driver
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

#ifndef TC_INTERRUPT_H_INCLUDED
#define TC_INTERRUPT_H_INCLUDED

#include "tc.h"
#include <system_interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__DOXYGEN__)
extern void *_tc_instances[TC_INST_NUM];

#  define _TC_INTERRUPT_VECT_NUM(n, unused) \
		  SYSTEM_INTERRUPT_MODULE_TC##n,
/**
 * \internal Get the interrupt vector for the given device instance
 *
 * \param[in] TC module instance number
 *
 * \return Interrupt vector for of the given TC module instance.
 */
static enum system_interrupt_vector _tc_interrupt_get_interrupt_vector(
		uint32_t inst_num)
{
	static uint8_t tc_interrupt_vectors[TC_INST_NUM] =
		{
#if (SAML21E) || (SAML21G) || (SAMR30E) || (SAMR30G)
			SYSTEM_INTERRUPT_MODULE_TC0,
			SYSTEM_INTERRUPT_MODULE_TC1,
			SYSTEM_INTERRUPT_MODULE_TC4
#else
			MRECURSION(TC_INST_NUM, _TC_INTERRUPT_VECT_NUM, TC_INST_MAX_ID)
#endif
		};

	return (enum system_interrupt_vector)tc_interrupt_vectors[inst_num];
}
#endif /* !defined(__DOXYGEN__) */

/**
 * \name Callback Management
 * {@
 */

enum status_code tc_register_callback(
		struct tc_module *const module,
		tc_callback_t callback_func,
		const enum tc_callback callback_type);

enum status_code tc_unregister_callback(
		struct tc_module *const module,
		const enum tc_callback callback_type);

/**
 * \brief Enables callback.
 *
 * Enables the callback function registered by the \ref
 * tc_register_callback. The callback function will be called from the
 * interrupt handler when the conditions for the callback type are
 * met. This function will also enable the appropriate interrupts.
 *
 * \param[in]     module        Pointer to TC software instance struct
 * \param[in]     callback_type Callback type given by an enum
 */
static inline void tc_enable_callback(
		struct tc_module *const module,
		const enum tc_callback callback_type)
{
	/* Sanity check arguments */
	Assert(module);


	/* Enable interrupts for this TC module */
	system_interrupt_enable(_tc_interrupt_get_interrupt_vector(_tc_get_inst_index(module->hw)));

	/* Enable callback */
	if (callback_type == TC_CALLBACK_CC_CHANNEL0) {
		module->enable_callback_mask |= TC_INTFLAG_MC(1);
		module->hw->COUNT8.INTENSET.reg = TC_INTFLAG_MC(1);
	}
	else if (callback_type == TC_CALLBACK_CC_CHANNEL1) {
		module->enable_callback_mask |= TC_INTFLAG_MC(2);
		module->hw->COUNT8.INTENSET.reg = TC_INTFLAG_MC(2);
	}
	else {
		module->enable_callback_mask |= (1 << callback_type);
		module->hw->COUNT8.INTENSET.reg = (1 << callback_type);
	}
}

/**
 * \brief Disables callback.
 *
 * Disables the callback function registered by the \ref
 * tc_register_callback, and the callback will not be called from the
 * interrupt routine. The function will also disable the appropriate
 * interrupts.
 *
 * \param[in]     module        Pointer to TC software instance struct
 * \param[in]     callback_type Callback type given by an enum
 */
static inline void tc_disable_callback(
		struct tc_module *const module,
		const enum tc_callback callback_type){
	/* Sanity check arguments */
	Assert(module);

	/* Disable callback */
	if (callback_type == TC_CALLBACK_CC_CHANNEL0) {
		module->hw->COUNT8.INTENCLR.reg = TC_INTFLAG_MC(1);
		module->enable_callback_mask &= ~TC_INTFLAG_MC(1);
	}
	else if (callback_type == TC_CALLBACK_CC_CHANNEL1) {
		module->hw->COUNT8.INTENCLR.reg = TC_INTFLAG_MC(2);
		module->enable_callback_mask &= ~TC_INTFLAG_MC(2);
	}
	else {
		module->hw->COUNT8.INTENCLR.reg = (1 << callback_type);
		module->enable_callback_mask &= ~(1 << callback_type);
	}
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* TC_INTERRUPT_H_INCLUDED */
