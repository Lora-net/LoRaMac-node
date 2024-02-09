/**
 * \file
 *
 * \brief SAMR34 System Interrupt Driver
 *
 * Copyright (c) 2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef SYSTEM_INTERRUPT_FEATURES_H_INCLUDED
#define SYSTEM_INTERRUPT_FEATURES_H_INCLUDED

#if !defined(__DOXYGEN__)

/* Generates a interrupt vector table enum list entry for a given module type
   and index (e.g. "SYSTEM_INTERRUPT_MODULE_TC0 = TC0_IRQn,"). */
#  define _MODULE_IRQn(n, module) \
		SYSTEM_INTERRUPT_MODULE_##module##n = module##n##_IRQn,

/* Generates interrupt vector table enum list entries for all instances of a
   given module type on the selected device. */
#  define _SYSTEM_INTERRUPT_MODULES(name) \
		MREPEAT(name##_INST_NUM, _MODULE_IRQn, name)

#  define _SYSTEM_INTERRUPT_IPSR_MASK              0x0000003f
#  define _SYSTEM_INTERRUPT_PRIORITY_MASK          0x00000003

#  define _SYSTEM_INTERRUPT_EXTERNAL_VECTOR_START  0

#  define _SYSTEM_INTERRUPT_SYSTICK_PRI_POS        30
#endif

/**
 * \addtogroup asfdoc_sam0_system_interrupt_group
 * @{
 */

/**
 * \brief Table of possible system interrupt/exception vector numbers.
 *
 * Table of all possible interrupt and exception vector indexes within the
 * SAM L21 device.
 */
#if defined(__DOXYGEN__)
/** \note The actual enumeration name is "system_interrupt_vector". */
enum system_interrupt_vector_samr34 {
#else
enum system_interrupt_vector {
#endif
	/** Interrupt vector index for a NMI interrupt */
	SYSTEM_INTERRUPT_NON_MASKABLE      = NonMaskableInt_IRQn,
	/** Interrupt vector index for a Hard Fault memory access exception */
	SYSTEM_INTERRUPT_HARD_FAULT        = HardFault_IRQn,
	/** Interrupt vector index for a Supervisor Call exception */
	SYSTEM_INTERRUPT_SV_CALL           = SVCall_IRQn,
	/** Interrupt vector index for a Pending Supervisor interrupt */
	SYSTEM_INTERRUPT_PENDING_SV        = PendSV_IRQn,
	/** Interrupt vector index for a System Tick interrupt */
	SYSTEM_INTERRUPT_SYSTICK           = SysTick_IRQn,

	/** Interrupt vector index for MCLK, OSCCTRL, OSC32KCTRL, PAC, PM, SUPC, TAL peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_SYSTEM     = SYSTEM_IRQn,
	/** Interrupt vector index for a Watch Dog peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_WDT        = WDT_IRQn,
	/** Interrupt vector index for a Real Time Clock peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_RTC        = RTC_IRQn,
	/** Interrupt vector index for an External Interrupt peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_EIC        = EIC_IRQn,
	/** Interrupt vector index for a Non Volatile Memory Controller interrupt */
	SYSTEM_INTERRUPT_MODULE_NVMCTRL    = NVMCTRL_IRQn,
	/** Interrupt vector index for a Direct Memory Access interrupt */
	SYSTEM_INTERRUPT_MODULE_DMA        = DMAC_IRQn,
	/** Interrupt vector index for a Universal Serial Bus interrupt */
	SYSTEM_INTERRUPT_MODULE_USB        = USB_IRQn,
	/** Interrupt vector index for an Event System interrupt */
	SYSTEM_INTERRUPT_MODULE_EVSYS      = EVSYS_IRQn,
#if defined(__DOXYGEN__)
	/** Interrupt vector index for a SERCOM peripheral interrupt.
	 *
	 *  Each specific device may contain several SERCOM peripherals; each module
	 *  instance will have its own entry in the table, with the instance number
	 *  substituted for "n" in the entry name (e.g.
	 *  \c SYSTEM_INTERRUPT_MODULE_SERCOM0).
	 */
	SYSTEM_INTERRUPT_MODULE_SERCOMn    = SERCOMn_IRQn,

	/** Interrupt vector index for a Timer/Counter Control peripheral interrupt.
	 *
	 *  Each specific device may contain several TCC peripherals; each module
	 *  instance will have its own entry in the table, with the instance number
	 *  substituted for "n" in the entry name (e.g.
	 *  \c SYSTEM_INTERRUPT_MODULE_TCC0).
	 */
	SYSTEM_INTERRUPT_MODULE_TCCn        = TCCn_IRQn,

	/** Interrupt vector index for a Timer/Counter peripheral interrupt.
	 *
	 *  Each specific device may contain several TC peripherals; each module
	 *  instance will have its own entry in the table, with the instance number
	 *  substituted for "n" in the entry name (e.g.
	 *  \c SYSTEM_INTERRUPT_MODULE_TC3).
	 */
	SYSTEM_INTERRUPT_MODULE_TCn        = TCn_IRQn,
#else
	_SYSTEM_INTERRUPT_MODULES(SERCOM)

	_SYSTEM_INTERRUPT_MODULES(TCC)
#if	(SAML21J) || (SAMR34J) || (SAMR35J)
	_SYSTEM_INTERRUPT_MODULES(TC)
#else
	SYSTEM_INTERRUPT_MODULE_TC0        = TC0_IRQn,
	SYSTEM_INTERRUPT_MODULE_TC1        = TC1_IRQn,
	SYSTEM_INTERRUPT_MODULE_TC4        = TC4_IRQn,
#endif
#endif

	/** Interrupt vector index for an Analog Comparator peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_AC         = AC_IRQn,
	/** Interrupt vector index for an Analog-to-Digital peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_ADC        = ADC_IRQn,
	/** Interrupt vector index for a Peripheral Touch Controller peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_PTC        = PTC_IRQn,
	/** Interrupt vector index for a AES peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_AES        = AES_IRQn,
	/** Interrupt vector index for a TRNG peripheral interrupt */
	SYSTEM_INTERRUPT_MODULE_TRNG       = TRNG_IRQn,
};

/** @} */

#endif
