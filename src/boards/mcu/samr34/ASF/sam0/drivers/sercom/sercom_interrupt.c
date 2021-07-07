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
#include "sercom_interrupt.h"

void *_sercom_instances[SERCOM_INST_NUM];

/** Save status of initialized handlers */
static bool _handler_table_initialized = false;

/** Void pointers for saving device instance structures */
static void (*_sercom_interrupt_handlers[SERCOM_INST_NUM])(const uint8_t instance);

/**
 * \internal
 * Default interrupt handler.
 *
 * \param[in] instance SERCOM instance used.
 */
static void _sercom_default_handler(
		const uint8_t instance)
{
	Assert(false);
}

/**
 * \internal
 * Saves the given callback handler.
 *
 * \param[in]  instance           Instance index.
 * \param[in]  interrupt_handler  Pointer to instance callback handler.
 */
void _sercom_set_handler(
		const uint8_t instance,
		const sercom_handler_t interrupt_handler)
{
	/* Initialize handlers with default handler and device instances with 0 */
	if (_handler_table_initialized == false) {
		for (uint32_t i = 0; i < SERCOM_INST_NUM; i++) {
			_sercom_interrupt_handlers[i] = &_sercom_default_handler;
			_sercom_instances[i] = NULL;
		}

		_handler_table_initialized = true;
	}

	/* Save interrupt handler */
	_sercom_interrupt_handlers[instance] = interrupt_handler;
}


/** \internal
 * Converts a given SERCOM index to its interrupt vector index.
 */
#define _SERCOM_INTERRUPT_VECT_NUM(n, unused) \
		SYSTEM_INTERRUPT_MODULE_SERCOM##n,

/** \internal
 * Generates a SERCOM interrupt handler function for a given SERCOM index.
 */
#define _SERCOM_INTERRUPT_HANDLER(n, unused) \
		void SERCOM##n##_Handler(void) \
		{ \
			_sercom_interrupt_handlers[n](n); \
		}

/**
 * \internal
 * Returns the system interrupt vector.
 *
 * \param[in]  sercom_instance  Instance pointer
 *
 * \return Enum of system interrupt vector
 * \retval SYSTEM_INTERRUPT_MODULE_SERCOM0
 * \retval SYSTEM_INTERRUPT_MODULE_SERCOM1
 * \retval SYSTEM_INTERRUPT_MODULE_SERCOM2
 * \retval SYSTEM_INTERRUPT_MODULE_SERCOM3
 * \retval SYSTEM_INTERRUPT_MODULE_SERCOM4
 * \retval SYSTEM_INTERRUPT_MODULE_SERCOM5
 * \retval SYSTEM_INTERRUPT_MODULE_SERCOM6
 * \retval SYSTEM_INTERRUPT_MODULE_SERCOM7
 */
enum system_interrupt_vector _sercom_get_interrupt_vector(
		Sercom *const sercom_instance)
{
	const uint8_t sercom_int_vectors[SERCOM_INST_NUM] =
		{
			MREPEAT(SERCOM_INST_NUM, _SERCOM_INTERRUPT_VECT_NUM, ~)
		};

	/* Retrieve the index of the SERCOM being requested */
	uint8_t instance_index = _sercom_get_sercom_inst_index(sercom_instance);

	/* Get the vector number from the lookup table for the requested SERCOM */
	return (enum system_interrupt_vector)sercom_int_vectors[instance_index];
}

/** Auto-generate a set of interrupt handlers for each SERCOM in the device */
MREPEAT(SERCOM_INST_NUM, _SERCOM_INTERRUPT_HANDLER, ~)
