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
#include "rtc_count_interrupt.h"

extern struct rtc_module *_rtc_instance[RTC_INST_NUM];

/**
 * \brief Registers callback for the specified callback type.
 *
 * Associates the given callback function with the
 * specified callback type.
 * To enable the callback, the \ref rtc_count_enable_callback function
 * must be used.
 *
 * \param[in,out]  module  Pointer to the software instance struct
 * \param[in]  callback      Pointer to the function desired for the specified
 *                           callback
 * \param[in]  callback_type Callback type to register
 *
 * \return                        Status of registering callback.
 * \retval STATUS_OK              Registering was done successfully
 * \retval STATUS_ERR_INVALID_ARG If trying to register a callback not available
 */
enum status_code rtc_count_register_callback(
		struct rtc_module *const module,
		rtc_count_callback_t callback,
		enum rtc_count_callback callback_type)
{

	enum status_code status = STATUS_OK;

	/* Overflow callback */
	if (callback_type == RTC_COUNT_CALLBACK_OVERFLOW
#ifdef FEATURE_RTC_TAMPER_DETECTION
		|| callback_type == RTC_COUNT_CALLBACK_TAMPER
#endif
		|| (callback_type >= RTC_COUNT_CALLBACK_PERIODIC_INTERVAL_0
			&& callback_type <= RTC_COUNT_CALLBACK_PERIODIC_INTERVAL_7)) {
		status = STATUS_OK;
	} else {
		/* Make sure callback type can be registered */
		switch (module->mode) {
		case RTC_COUNT_MODE_32BIT:
			/* Check sanity for 32-bit mode. */
			if (callback_type > (RTC_COMP32_NUM + RTC_PER_NUM)) {
				status = STATUS_ERR_INVALID_ARG;
			}

			break;
		case RTC_COUNT_MODE_16BIT:
			/* Check sanity for 16-bit mode. */
			if (callback_type > (RTC_NUM_OF_COMP16 + RTC_PER_NUM)) {
				status = STATUS_ERR_INVALID_ARG;
			}
			break;
		default:
			status = STATUS_ERR_INVALID_ARG;
		}
	}

	if (status == STATUS_OK) {
		/* Register callback */
		module->callbacks[callback_type] = callback;
		/* Set corresponding bit to set callback as registered */
		module->registered_callback |= (1 << callback_type);
	}

	return status;
}

/**
 * \brief Unregisters callback for the specified callback type.
 *
 * When called, the currently registered callback for the given callback type
 * will be removed.
 *
 * \param[in,out]  module  Pointer to the software instance struct
 * \param[in]     callback_type  Specifies the callback type to unregister
 *
 * \return                        Status of unregistering callback.
 * \retval STATUS_OK              Unregistering was done successfully
 * \retval STATUS_ERR_INVALID_ARG If trying to unregister a callback not available
 */
enum status_code rtc_count_unregister_callback(
		struct rtc_module *const module,
		enum rtc_count_callback callback_type)
{
	enum status_code status = STATUS_OK;

	/* Overflow callback */
	if (callback_type == RTC_COUNT_CALLBACK_OVERFLOW
#ifdef FEATURE_RTC_TAMPER_DETECTION
		|| callback_type == RTC_COUNT_CALLBACK_TAMPER
#endif
		|| (callback_type >= RTC_COUNT_CALLBACK_PERIODIC_INTERVAL_0
			&& callback_type <= RTC_COUNT_CALLBACK_PERIODIC_INTERVAL_7)) {
		status = STATUS_OK;
	} else {
		/* Make sure callback type can be unregistered */
		switch (module->mode) {
		case RTC_COUNT_MODE_32BIT:
			/* Check sanity for 32-bit mode. */
			if (callback_type > (RTC_COMP32_NUM + RTC_PER_NUM)) {
				status = STATUS_ERR_INVALID_ARG;
			}
			break;
		case RTC_COUNT_MODE_16BIT:
			/* Check sanity for 16-bit mode. */
			if (callback_type > (RTC_NUM_OF_COMP16 + RTC_PER_NUM)) {
				status = STATUS_ERR_INVALID_ARG;
			}
			break;
		default:
			status = STATUS_ERR_INVALID_ARG;
		}
	}
	if (status == STATUS_OK) {
		/* Unregister callback */
		module->callbacks[callback_type] = NULL;

		/* Clear corresponding bit to set callback as unregistered */
		module->registered_callback &= ~(1 << callback_type);
	}
	return status;
}

/**
 * \brief Enables callback.
 *
 * Enables the callback specified by the callback_type.
 *
 * \param[in,out]  module  Pointer to the software instance struct
 * \param[in]     callback_type Callback type to enable
 */
void rtc_count_enable_callback(
		struct rtc_module *const module,
		enum rtc_count_callback callback_type)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	if (callback_type == RTC_COUNT_CALLBACK_OVERFLOW) {
		rtc_module->MODE0.INTENSET.reg = RTC_MODE0_INTFLAG_OVF;
#ifdef FEATURE_RTC_TAMPER_DETECTION
	} else if (callback_type == RTC_COUNT_CALLBACK_TAMPER) {
		rtc_module->MODE0.INTENSET.reg = RTC_MODE0_INTFLAG_TAMPER;
#endif
	} else if (callback_type >= RTC_COUNT_CALLBACK_PERIODIC_INTERVAL_0
			&& callback_type <= RTC_COUNT_CALLBACK_PERIODIC_INTERVAL_7) {
		rtc_module->MODE0.INTENSET.reg = RTC_MODE1_INTFLAG_PER(1 << callback_type);
	}else {
		rtc_module->MODE0.INTENSET.reg = RTC_MODE1_INTFLAG_CMP(1 << (callback_type - RTC_PER_NUM));
	}
	/* Mark callback as enabled. */
	module->enabled_callback |= (1 << callback_type);
}

/**
 * \brief Disables callback.
 *
 * Disables the callback specified by the callback_type.
 *
 * \param[in,out]  module  Pointer to the software instance struct
 * \param[in]     callback_type Callback type to disable
 */
void rtc_count_disable_callback(
		struct rtc_module *const module,
		enum rtc_count_callback callback_type)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	/* Disable interrupt */
	if (callback_type == RTC_COUNT_CALLBACK_OVERFLOW) {
		rtc_module->MODE0.INTENCLR.reg = RTC_MODE0_INTFLAG_OVF;
#ifdef FEATURE_RTC_TAMPER_DETECTION
	} else if (callback_type == RTC_COUNT_CALLBACK_TAMPER) {
		rtc_module->MODE0.INTENCLR.reg = RTC_MODE0_INTFLAG_TAMPER;
#endif
	} else if(callback_type >= RTC_COUNT_CALLBACK_PERIODIC_INTERVAL_0
			&& callback_type <= RTC_COUNT_CALLBACK_PERIODIC_INTERVAL_7){
		rtc_module->MODE0.INTENCLR.reg = RTC_MODE1_INTFLAG_PER(1 << callback_type);;
	}else {
		rtc_module->MODE0.INTENCLR.reg = RTC_MODE1_INTFLAG_CMP(1 << (callback_type - RTC_PER_NUM));
	}

	/* Mark callback as disabled. */
	module->enabled_callback &= ~(1 << callback_type);
}

/**
 * \internal Interrupt handler for RTC
 *
 * \param [in] instance_index  Default value 0
 */
static void _rtc_interrupt_handler(const uint32_t instance_index)
{
	struct rtc_module *module = _rtc_instance[instance_index];

	Rtc *const rtc_module = module->hw;

	/* Combine callback registered and enabled masks */
	uint16_t callback_mask = module->enabled_callback;
	callback_mask &= module->registered_callback;

	/* Read and mask interrupt flag register */
	uint16_t interrupt_status = rtc_module->MODE0.INTFLAG.reg;
	interrupt_status &= rtc_module->MODE0.INTENSET.reg;

	if (interrupt_status & RTC_MODE0_INTFLAG_OVF) {
		/* Overflow interrupt */
		if (callback_mask & (1 << RTC_COUNT_CALLBACK_OVERFLOW)) {
			module->callbacks[RTC_COUNT_CALLBACK_OVERFLOW]();
		}

		/* Clear interrupt flag */
		rtc_module->MODE0.INTFLAG.reg = RTC_MODE0_INTFLAG_OVF;
#ifdef FEATURE_RTC_TAMPER_DETECTION
	} else if (interrupt_status & RTC_MODE0_INTFLAG_TAMPER) {
		/* Tamper interrupt */
		if (callback_mask & (1 << RTC_COUNT_CALLBACK_TAMPER)) {
			module->callbacks[RTC_COUNT_CALLBACK_TAMPER]();
		}

		/* Clear interrupt flag */
		rtc_module->MODE0.INTFLAG.reg = RTC_MODE0_INTFLAG_TAMPER;
#endif

	} else if (interrupt_status & RTC_MODE1_INTFLAG_PER(0xff)) {
		uint8_t i  = 0;
		for ( i = 0;i < RTC_PER_NUM;i++) {
			if ((interrupt_status & RTC_MODE1_INTFLAG_PER(1 << i))
			  && (callback_mask & (1 << i))) {
				module->callbacks[i]();
			}

			/* Clear interrupt flag */
			rtc_module->MODE0.INTFLAG.reg = RTC_MODE1_INTFLAG_PER(1<<i);
		}
	}else if (interrupt_status & RTC_MODE1_INTFLAG_CMP(1 << 0)) {
		/* Compare 0 interrupt */
		if (callback_mask & (1 << RTC_COUNT_CALLBACK_COMPARE_0)) {
			module->callbacks[RTC_COUNT_CALLBACK_COMPARE_0]();
		}
		/* Clear interrupt flag */
		rtc_module->MODE0.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP(1 << 0);

	} else if (interrupt_status & RTC_MODE1_INTFLAG_CMP(1 << 1)) {
		#if (RTC_NUM_OF_COMP16 > 1) || defined(__DOXYGEN__)
		/* Compare 1 interrupt */
		if (callback_mask & (1 << RTC_COUNT_CALLBACK_COMPARE_1)) {
			module->callbacks[RTC_COUNT_CALLBACK_COMPARE_1]();
		}
		/* Clear interrupt flag */
		rtc_module->MODE0.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP(1 << 1);
		#endif

	} else if (interrupt_status & RTC_MODE1_INTFLAG_CMP(1 << 2)) {
		#if (RTC_NUM_OF_COMP16 > 2)	|| defined(__DOXYGEN__)
		/* Compare 2 interrupt */
		if (callback_mask & (1 << RTC_COUNT_CALLBACK_COMPARE_2)) {
			module->callbacks[RTC_COUNT_CALLBACK_COMPARE_2]();
		}
		/* Clear interrupt flag */
		rtc_module->MODE0.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP(1 << 2);
		#endif

	} else if (interrupt_status & RTC_MODE1_INTFLAG_CMP(1 << 3)) {
		#if (RTC_NUM_OF_COMP16 > 3)	|| defined(__DOXYGEN__)
		/* Compare 3 interrupt */
		if (callback_mask & (1 << RTC_COUNT_CALLBACK_COMPARE_3)) {
			module->callbacks[RTC_COUNT_CALLBACK_COMPARE_3]();
		}
		/* Clear interrupt flag */
		rtc_module->MODE0.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP(1 << 3);
		#endif

	} else if (interrupt_status & RTC_MODE1_INTFLAG_CMP(1 << 4)) {
		#if (RTC_NUM_OF_COMP16 > 4) || defined(__DOXYGEN__)
		/* Compare 4 interrupt */
		if (callback_mask & (1 << RTC_COUNT_CALLBACK_COMPARE_4)) {
			module->callbacks[RTC_COUNT_CALLBACK_COMPARE_4]();
		}
		/* Clear interrupt flag */
		rtc_module->MODE0.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP(1 << 4);
		#endif

	} else if (interrupt_status & RTC_MODE1_INTFLAG_CMP(1 << 5)) {
		#if (RTC_NUM_OF_COMP16 > 5) || defined(__DOXYGEN__)
		/* Compare 5 interrupt */
		if (callback_mask & (1 << RTC_COUNT_CALLBACK_COMPARE_5)) {
			module->callbacks[RTC_COUNT_CALLBACK_COMPARE_5]();
		}
		/* Clear interrupt flag */
		rtc_module->MODE0.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP(1 << 5);
		#endif
	}
}

/**
 * \internal ISR handler for RTC
 */
#if (RTC_INST_NUM == 1)
void RTC_Handler(void)
{
	_rtc_interrupt_handler(0);
}
#elif (RTC_INST_NUM > 1)
#  define _RTC_INTERRUPT_HANDLER(n, unused) \
		void RTC##n##_Handler(void) \
		{ \
			_rtc_interrupt_handler(n); \
		}

MREPEAT(RTC_INST_NUM, _RTC_INTERRUPT_HANDLER, ~)
#endif /* (RTC_INST_NUM > 1) */