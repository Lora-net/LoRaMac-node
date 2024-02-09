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
#include "extint.h"
#include "extint_callback.h"

/**
 * \internal
 * Internal driver device instance struct, declared in the main module driver.
 */
extern struct _extint_module _extint_dev;

/**
 * \internal
 * This is the number of the channel whose callback is currently running.
 */
uint8_t _current_channel;

/**
 * \brief Registers an asynchronous callback function with the driver.
 *
 * Registers an asynchronous callback with the EXTINT driver, fired when a
 * channel detects the configured channel detection criteria
 * (e.g. edge or level). Callbacks are fired once for each detected channel.
 *
 * \note NMI channel callbacks cannot be registered via this function; the
 *       device's NMI interrupt should be hooked directly in the user
 *       application and the NMI flags manually cleared via
 *       \ref extint_nmi_clear_detected().
 *
 * \param[in] callback  Pointer to the callback function to register
 * \param[in] channel   Logical channel to register callback for
 * \param[in] type      Type of callback function to register
 *
 * \return Status of the registration operation.
 * \retval STATUS_OK               The callback was registered successfully
 * \retval STATUS_ERR_INVALID_ARG  If an invalid callback type was supplied
 * \retval STATUS_ERR_ALREADY_INITIALIZED    Callback function has been
 *                                  registered, need unregister first
 */
enum status_code extint_register_callback(
	const extint_callback_t callback,
	const uint8_t channel,
	const enum extint_callback_type type)
{
	/* Sanity check arguments */
	Assert(callback);

	if (type != EXTINT_CALLBACK_TYPE_DETECT) {
		Assert(false);
		return STATUS_ERR_INVALID_ARG;
	}

	if (_extint_dev.callbacks[channel] == NULL) {
		_extint_dev.callbacks[channel] = callback;
		return STATUS_OK;
	} else if (_extint_dev.callbacks[channel] == callback) {
		return STATUS_OK;
	}

	return STATUS_ERR_ALREADY_INITIALIZED;
}

/**
 * \brief Unregisters an asynchronous callback function with the driver.
 *
 * Unregisters an asynchronous callback with the EXTINT driver, removing it
 * from the internal callback registration table.
 *
 * \param[in] callback  Pointer to the callback function to unregister
 * \param[in] channel   Logical channel to unregister callback for
 * \param[in] type      Type of callback function to unregister
 *
 * \return Status of the de-registration operation.
 * \retval STATUS_OK               The callback was unregistered successfully
 * \retval STATUS_ERR_INVALID_ARG  If an invalid callback type was supplied
 * \retval STATUS_ERR_BAD_ADDRESS  No matching entry was found in the
 *                                 registration table
 */
enum status_code extint_unregister_callback(
	const extint_callback_t callback,
	const uint8_t channel,
	const enum extint_callback_type type)
{
	/* Sanity check arguments */
	Assert(callback);

	if (type != EXTINT_CALLBACK_TYPE_DETECT) {
		Assert(false);
		return STATUS_ERR_INVALID_ARG;
	}

	if (_extint_dev.callbacks[channel] == callback) {
		_extint_dev.callbacks[channel] = NULL;
		return STATUS_OK;
	}

	return STATUS_ERR_BAD_ADDRESS;
}

/**
 * \brief Enables asynchronous callback generation for a given channel and type.
 *
 * Enables asynchronous callbacks for a given logical external interrupt channel
 * and type. This must be called before an external interrupt channel will
 * generate callback events.
 *
 * \param[in] channel  Logical channel to enable callback generation for
 * \param[in] type     Type of callback function callbacks to enable
 *
 * \return Status of the callback enable operation.
 * \retval STATUS_OK               The callback was enabled successfully
 * \retval STATUS_ERR_INVALID_ARG  If an invalid callback type was supplied
 */
enum status_code extint_chan_enable_callback(
	const uint8_t channel,
	const enum extint_callback_type type)
{
	if (type == EXTINT_CALLBACK_TYPE_DETECT) {
		Eic *const eic = _extint_get_eic_from_channel(channel);

		eic->INTENSET.reg = (1UL << channel);
	}
	else {
		Assert(false);
		return STATUS_ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

/**
 * \brief Disables asynchronous callback generation for a given channel and type.
 *
 * Disables asynchronous callbacks for a given logical external interrupt
 * channel and type.
 *
 * \param[in] channel  Logical channel to disable callback generation for
 * \param[in] type     Type of callback function callbacks to disable
 *
 * \return Status of the callback disable operation.
 * \retval STATUS_OK               The callback was disabled successfully
 * \retval STATUS_ERR_INVALID_ARG  If an invalid callback type was supplied
 */
enum status_code extint_chan_disable_callback(
	const uint8_t channel,
	const enum extint_callback_type type)
{
	if (type == EXTINT_CALLBACK_TYPE_DETECT) {
		Eic *const eic = _extint_get_eic_from_channel(channel);

		eic->INTENCLR.reg = (1UL << channel);
	}
	else {
		Assert(false);
		return STATUS_ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

/**
 * \brief Find what channel caused the callback.
 *
 * Can be used in an EXTINT callback function to find what channel caused
 * the callback in case the same callback is used by multiple channels.
 *
 * \return Channel number.
 */
uint8_t extint_get_current_channel(void)
{
	return _current_channel;
}

/** Handler for the EXTINT hardware module interrupt. */
void EIC_Handler(void)
{
	/* Find any triggered channels, run associated callback handlers */
	for (_current_channel = 0; _current_channel < EIC_NUMBER_OF_INTERRUPTS ; _current_channel++) {
		if (extint_chan_is_detected(_current_channel)) {
			/* Clear flag */
			extint_chan_clear_detected(_current_channel);
			/* Find any associated callback entries in the callback table */
			if (_extint_dev.callbacks[_current_channel] != NULL) {
				/* Run the registered callback */
				_extint_dev.callbacks[_current_channel]();
			}
		}
	}
}
