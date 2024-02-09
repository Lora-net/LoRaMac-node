/**
 * \file
 *
 * \brief SAM TC - Timer Counter Driver
 *
 * Copyright (c) 2014-2020 Microchip Technology Inc. and its subsidiaries.
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

#include "tc.h"

#if TC_ASYNC == true
#  include "tc_interrupt.h"
#  include <system_interrupt.h>
#endif

/**
 * \internal Find the index of given TC module instance.
 *
 * \param[in] TC module instance pointer
 *
 * \return Index of the given TC module instance.
 */
uint8_t _tc_get_inst_index(
		Tc *const hw)
{
	/* List of available TC modules. */
	Tc *const tc_modules[TC_INST_NUM] = TC_INSTS;

	/* Find index for TC instance. */
	for (uint32_t i = 0; i < TC_INST_NUM; i++) {
		if (hw == tc_modules[i]) {
			return i;
		}
	}

	/* Invalid data given. */
	Assert(false);
	return 0;
}


/**
 * \brief Initializes a hardware TC module instance.
 *
 * Enables the clock and initializes the TC module, based on the given
 * configuration values.
 *
 * \param[in,out] module_inst  Pointer to the software module instance struct
 * \param[in]     hw           Pointer to the TC hardware module
 * \param[in]     config       Pointer to the TC configuration options struct
 *
 * \return Status of the initialization procedure.
 *
 * \retval STATUS_OK           The module was initialized successfully
 * \retval STATUS_BUSY         Hardware module was busy when the
 *                             initialization procedure was attempted
 * \retval STATUS_INVALID_ARG  An invalid configuration option or argument
 *                             was supplied
 * \retval STATUS_ERR_DENIED   Hardware module was already enabled, or the
 *                             hardware module is configured in 32-bit
 *                             slave mode
 */
enum status_code tc_init(
		struct tc_module *const module_inst,
		Tc *const hw,
		const struct tc_config *const config)
{
	/* Sanity check arguments */
	Assert(hw);
	Assert(module_inst);
	Assert(config);

	/* Temporary variable to hold all updates to the CTRLA
	 * register before they are written to it */
	uint32_t ctrla_tmp = 0;
	/* Temporary variable to hold all updates to the CTRLBSET
	 * register before they are written to it */
	uint8_t ctrlbset_tmp = 0;
	/* Temporary variable to hold TC instance number */
	uint8_t instance = _tc_get_inst_index(hw);

#if (SAMC20) || (SAMC21)
	/* Array of GLCK ID for different TC instances */
	uint8_t inst_gclk_id[] = {TC0_GCLK_ID, TC1_GCLK_ID, TC2_GCLK_ID, TC3_GCLK_ID, TC4_GCLK_ID};
	/* Array of MCLK APB mask bit position for different TC instances */
	uint32_t inst_mclk_apbmask[] = {SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC0,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC1,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC2,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC3,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC4};
#elif (SAML21J) || (SAMR34J) || (SAMR35J) || (WLR089U0)
	/* Array of GLCK ID for different TC instances */
	uint8_t inst_gclk_id[] = {TC0_GCLK_ID, TC1_GCLK_ID, TC2_GCLK_ID, TC3_GCLK_ID, TC4_GCLK_ID};
	/* Array of MCLK APB mask bit position for different TC instances */
	uint32_t inst_mclk_apbmask[] = {SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC0,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC1,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC2,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC3,
									SYSTEM_CLOCK_APB_APBD, MCLK_APBDMASK_TC4};
#elif (SAML22)
	/* Array of GLCK ID for different TC instances */
	uint8_t inst_gclk_id[] = {TC0_GCLK_ID, TC1_GCLK_ID, TC2_GCLK_ID, TC3_GCLK_ID};
	/* Array of MCLK APB mask bit position for different TC instances */
	uint32_t inst_mclk_apbmask[] = {SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC0,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC1,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC2,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC3};
#else
	/* Array of GLCK ID for different TC instances */
	uint8_t inst_gclk_id[] = {TC0_GCLK_ID, TC1_GCLK_ID, TC4_GCLK_ID};
	/* Array of PM APB mask bit position for different TC instances */
	uint32_t inst_mclk_apbmask[] = {SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC0,
									SYSTEM_CLOCK_APB_APBC, MCLK_APBCMASK_TC1,
									SYSTEM_CLOCK_APB_APBD, MCLK_APBDMASK_TC4};
#endif

	struct system_pinmux_config pin_config;
	struct system_gclk_chan_config gclk_chan_config;

#if TC_ASYNC == true
	/* Initialize parameters */
	for (uint8_t i = 0; i < TC_CALLBACK_N; i++) {
		module_inst->callback[i]        = NULL;
	}
	module_inst->register_callback_mask     = 0x00;
	module_inst->enable_callback_mask       = 0x00;

	/* Register this instance for callbacks*/
	_tc_instances[instance] = module_inst;
#endif

	/* Associate the given device instance with the hardware module */
	module_inst->hw = hw;

	module_inst->double_buffering_enabled = config->double_buffering_enabled;

	/* Check if odd numbered TC modules are being configured in 32-bit
	 * counter size. Only even numbered counters are allowed to be
	 * configured in 32-bit counter size.
	 */
	if ((config->counter_size == TC_COUNTER_SIZE_32BIT) &&
			((instance + TC_INSTANCE_OFFSET) & 0x01)) {
		Assert(false);
		return STATUS_ERR_INVALID_ARG;
	}

	/* Make the counter size variable in the module_inst struct reflect
	 * the counter size in the module
	 */
	module_inst->counter_size = config->counter_size;

	if (hw->COUNT8.CTRLA.reg & TC_CTRLA_SWRST) {
		/* We are in the middle of a reset. Abort. */
		return STATUS_BUSY;
	}

	if (hw->COUNT8.STATUS.reg & TC_STATUS_SLAVE) {
		/* Module is used as a slave */
		return STATUS_ERR_DENIED;
	}

	if (hw->COUNT8.CTRLA.reg & TC_CTRLA_ENABLE) {
		/* Module must be disabled before initialization. Abort. */
		return STATUS_ERR_DENIED;
	}

	/* Set up the TC PWM out pin for channel 0 */
	if (config->pwm_channel[0].enabled) {
		system_pinmux_get_config_defaults(&pin_config);
		pin_config.mux_position = config->pwm_channel[0].pin_mux;
		pin_config.direction = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
		system_pinmux_pin_set_config(
				config->pwm_channel[0].pin_out, &pin_config);
	}

	/* Set up the TC PWM out pin for channel 1 */
	if (config->pwm_channel[1].enabled) {
		system_pinmux_get_config_defaults(&pin_config);
		pin_config.mux_position = config->pwm_channel[1].pin_mux;
		pin_config.direction = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
		system_pinmux_pin_set_config(
				config->pwm_channel[1].pin_out, &pin_config);
	}

	/* Enable the user interface clock in the MCLK */
	system_apb_clock_set_mask((enum system_clock_apb_bus)inst_mclk_apbmask[instance*2],
			inst_mclk_apbmask[2*instance+1]);

	/* Enable the slave counter if counter_size is 32-bit */
	if ((config->counter_size == TC_COUNTER_SIZE_32BIT) && (instance+1 < TC_INST_NUM))
	{
		/* Enable the user interface clock in the MCLK */
		system_apb_clock_set_mask((enum system_clock_apb_bus)inst_mclk_apbmask[(instance+1)*2],
				inst_mclk_apbmask[(instance+1)*2+1]);
	}


	/* Setup clock for module */
	system_gclk_chan_get_config_defaults(&gclk_chan_config);
	gclk_chan_config.source_generator = config->clock_source;
	system_gclk_chan_set_config(inst_gclk_id[instance], &gclk_chan_config);
	system_gclk_chan_enable(inst_gclk_id[instance]);

	/* Set ctrla register */
	ctrla_tmp =
			(uint32_t)config->counter_size |
			(uint32_t)config->reload_action |
			(uint32_t)config->clock_prescaler;

	for (uint8_t i = 0; i < NUMBER_OF_COMPARE_CAPTURE_CHANNELS; i++) {
		if (config->enable_capture_on_channel[i] == true) {
			ctrla_tmp |= (TC_CTRLA_CAPTEN(1) << i);
		}
	}

	for (uint8_t i = 0; i < NUMBER_OF_COMPARE_CAPTURE_CHANNELS; i++) {
		if (config->enable_capture_on_IO[i] == true) {
			ctrla_tmp |= (TC_CTRLA_COPEN(1) << i);
		}
	}

	ctrla_tmp |= (config->run_in_standby << TC_CTRLA_RUNSTDBY_Pos)
				|(config->on_demand << TC_CTRLA_ONDEMAND_Pos);

	/* Write configuration to register */
	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}
	hw->COUNT8.CTRLA.reg = ctrla_tmp;

	/* Write configuration to register */
	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}
	hw->COUNT8.WAVE.reg = config->wave_generation;

	/* Set ctrlb register */
	if (config->oneshot) {
		ctrlbset_tmp = TC_CTRLBSET_ONESHOT;
	}

	if (config->count_direction) {
		ctrlbset_tmp |= TC_CTRLBSET_DIR;
	}

	/* Clear old ctrlb configuration */
	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}
	hw->COUNT8.CTRLBCLR.reg = 0xFF;

	/* Check if we actually need to go into a wait state. */
	if (ctrlbset_tmp) {
		while (tc_is_syncing(module_inst)) {
			/* Wait for sync */
		}
		/* Write configuration to register */
		hw->COUNT8.CTRLBSET.reg = ctrlbset_tmp;
	}

	/* Set drvvtrl register*/
	hw->COUNT8.DRVCTRL.reg = config->waveform_invert_output;

	/* Write configuration to register */
	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}

	/* Switch for TC counter size  */
	switch (module_inst->counter_size) {
		case TC_COUNTER_SIZE_8BIT:
			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT8.COUNT.reg =
					config->counter_8_bit.value;


			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT8.PER.reg =
					config->counter_8_bit.period;

			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT8.CC[0].reg =
					config->counter_8_bit.compare_capture_channel[0];

			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT8.CC[1].reg =
					config->counter_8_bit.compare_capture_channel[1];

			return STATUS_OK;

		case TC_COUNTER_SIZE_16BIT:
			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT16.COUNT.reg
				= config->counter_16_bit.value;

			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT16.CC[0].reg =
					config->counter_16_bit.compare_capture_channel[0];

			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT16.CC[1].reg =
					config->counter_16_bit.compare_capture_channel[1];

			return STATUS_OK;

		case TC_COUNTER_SIZE_32BIT:
			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT32.COUNT.reg
				= config->counter_32_bit.value;

			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT32.CC[0].reg =
					config->counter_32_bit.compare_capture_channel[0];

			while (tc_is_syncing(module_inst)) {
				/* Wait for sync */
			}

			hw->COUNT32.CC[1].reg =
					config->counter_32_bit.compare_capture_channel[1];

			return STATUS_OK;
	}

	Assert(false);
	return STATUS_ERR_INVALID_ARG;
}

/**
 * \brief Sets TC module count value.
 *
 * Sets the current timer count value of a initialized TC module. The
 * specified TC module may be started or stopped.
 *
 * \param[in] module_inst  Pointer to the software module instance struct
 * \param[in] count        New timer count value to set
 *
 * \return Status of the count update procedure.
 *
 * \retval STATUS_OK               The timer count was updated successfully
 * \retval STATUS_ERR_INVALID_ARG  An invalid timer counter size was specified
 */
enum status_code tc_set_count_value(
		const struct tc_module *const module_inst,
		const uint32_t count)
{
	/* Sanity check arguments */
	Assert(module_inst);
	Assert(module_inst->hw);

	/* Get a pointer to the module's hardware instance*/
	Tc *const tc_module = module_inst->hw;

	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}

	/* Write to based on the TC counter_size */
	switch (module_inst->counter_size) {
		case TC_COUNTER_SIZE_8BIT:
			tc_module->COUNT8.COUNT.reg  = (uint8_t)count;
			return STATUS_OK;

		case TC_COUNTER_SIZE_16BIT:
			tc_module->COUNT16.COUNT.reg = (uint16_t)count;
			return STATUS_OK;

		case TC_COUNTER_SIZE_32BIT:
			tc_module->COUNT32.COUNT.reg = (uint32_t)count;
			return STATUS_OK;

		default:
			return STATUS_ERR_INVALID_ARG;
	}
}

/**
 * \brief Get TC module count value.
 *
 * Retrieves the current count value of a TC module. The specified TC module
 * may be started or stopped.
 *
 * \param[in] module_inst  Pointer to the software module instance struct
 *
 * \return Count value of the specified TC module.
 */
uint32_t tc_get_count_value(
		const struct tc_module *const module_inst)
{
	/* Sanity check arguments */
	Assert(module_inst);
	Assert(module_inst->hw);

	/* Read synchronization */
	tc_sync_read_count(module_inst);
	
	/* Get a pointer to the module's hardware instance */
	Tc *const tc_module = module_inst->hw;

	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}

	/* Read from based on the TC counter size */
	switch (module_inst->counter_size) {
		case TC_COUNTER_SIZE_8BIT:
			return (uint32_t)tc_module->COUNT8.COUNT.reg;

		case TC_COUNTER_SIZE_16BIT:
			return (uint32_t)tc_module->COUNT16.COUNT.reg;

		case TC_COUNTER_SIZE_32BIT:
			return tc_module->COUNT32.COUNT.reg;
	}

	Assert(false);
	return 0;
}

/**
 * \brief Gets the TC module capture value.
 *
 * Retrieves the capture value in the indicated TC module capture channel.
 *
 * \param[in]  module_inst    Pointer to the software module instance struct
 * \param[in]  channel_index  Index of the Compare Capture channel to read
 *
 * \return Capture value stored in the specified timer channel.
 */
uint32_t tc_get_capture_value(
		const struct tc_module *const module_inst,
		const enum tc_compare_capture_channel channel_index)
{
	/* Sanity check arguments */
	Assert(module_inst);
	Assert(module_inst->hw);

	/* Get a pointer to the module's hardware instance */
	Tc *const tc_module = module_inst->hw;

	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}

	/* Read out based on the TC counter size */
	switch (module_inst->counter_size) {
		case TC_COUNTER_SIZE_8BIT:
			if (channel_index <
					NUMBER_OF_COMPARE_CAPTURE_CHANNELS) {
				return tc_module->COUNT8.CC[channel_index].reg;
			}

		case TC_COUNTER_SIZE_16BIT:
			if (channel_index <
					NUMBER_OF_COMPARE_CAPTURE_CHANNELS) {
				return tc_module->COUNT16.CC[channel_index].reg;
			}

		case TC_COUNTER_SIZE_32BIT:
			if (channel_index <
					NUMBER_OF_COMPARE_CAPTURE_CHANNELS) {
				return tc_module->COUNT32.CC[channel_index].reg;
			}
	}

	Assert(false);
	return 0;
}

/**
 * \brief Sets a TC module compare value.
 *
 * Writes a compare value to the given TC module compare/capture channel.
 *
 * \param[in]  module_inst    Pointer to the software module instance struct
 * \param[in]  channel_index  Index of the compare channel to write to
 * \param[in]  compare        New compare value to set
 *
 * \return Status of the compare update procedure.
 *
 * \retval  STATUS_OK               The compare value was updated successfully
 * \retval  STATUS_ERR_INVALID_ARG  An invalid channel index was supplied
 */
enum status_code tc_set_compare_value(
		const struct tc_module *const module_inst,
		const enum tc_compare_capture_channel channel_index,
		const uint32_t compare)
{
	/* Sanity check arguments */
	Assert(module_inst);
	Assert(module_inst->hw);
	Assert(compare);

	/* Get a pointer to the module's hardware instance */
	Tc *const tc_module = module_inst->hw;

	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}

	/* Read out based on the TC counter size */
	switch (module_inst->counter_size) {
		case TC_COUNTER_SIZE_8BIT:
			if (channel_index <
					NUMBER_OF_COMPARE_CAPTURE_CHANNELS) {
				if (module_inst->double_buffering_enabled){
					tc_module->COUNT8.CCBUF[channel_index].reg =
							(uint8_t)compare;
				} else {
					tc_module->COUNT8.CC[channel_index].reg  =
							(uint8_t)compare;
				}
				return STATUS_OK;
			}
		case TC_COUNTER_SIZE_16BIT:
			if (channel_index <
					NUMBER_OF_COMPARE_CAPTURE_CHANNELS) {
				if (module_inst->double_buffering_enabled){
					tc_module->COUNT16.CCBUF[channel_index].reg =
							(uint16_t)compare;
				} else {
					tc_module->COUNT16.CC[channel_index].reg =
							(uint16_t)compare;
				}
				return STATUS_OK;
			}

		case TC_COUNTER_SIZE_32BIT:
			if (channel_index <
					NUMBER_OF_COMPARE_CAPTURE_CHANNELS) {
				if (module_inst->double_buffering_enabled){
					tc_module->COUNT32.CCBUF[channel_index].reg =
							(uint32_t)compare;
				} else {
					tc_module->COUNT32.CC[channel_index].reg =
						(uint32_t)compare;
				}
				return STATUS_OK;
			}
	}

	return STATUS_ERR_INVALID_ARG;
}

/**
 * \brief Resets the TC module.
 *
 * Resets the TC module, restoring all hardware module registers to their
 * default values and disabling the module. The TC module will not be
 * accessible while the reset is being performed.
 *
 * \note When resetting a 32-bit counter only the master TC module's instance
 *       structure should be passed to the function.
 *
 * \param[in]  module_inst    Pointer to the software module instance struct
 *
 * \return Status of the procedure.
 * \retval STATUS_OK                   The module was reset successfully
 * \retval STATUS_ERR_UNSUPPORTED_DEV  A 32-bit slave TC module was passed to
 *                                     the function. Only use reset on master
 *                                     TC
 */
enum status_code tc_reset(
		const struct tc_module *const module_inst)
{
	/* Sanity check arguments  */
	Assert(module_inst);
	Assert(module_inst->hw);

	/* Get a pointer to the module hardware instance */
	TcCount8 *const tc_module = &(module_inst->hw->COUNT8);

	if (tc_module->STATUS.reg & TC_STATUS_SLAVE) {
		return STATUS_ERR_UNSUPPORTED_DEV;
	}

	/* Disable this module if it is running */
	if (tc_module->CTRLA.reg & TC_CTRLA_ENABLE) {
		tc_disable(module_inst);
		while (tc_is_syncing(module_inst)) {
			/* wait while module is disabling */
		}
	}

	/* Reset this TC module */
	tc_module->CTRLA.reg  |= TC_CTRLA_SWRST;

	return STATUS_OK;
}

/**
 * \brief Set the timer TOP/period value.
 *
 * For 8-bit counter size this function writes the top value to the period
 * register.
 *
 * For 16- and 32-bit counter size this function writes the top value to
 * Capture Compare register 0. The value in this register can not be used for
 * any other purpose.
 *
 * \note This function is designed to be used in PWM or frequency
 *       match modes only, when the counter is set to 16- or 32-bit counter
 *       size. In 8-bit counter size it will always be possible to change the
 *       top value even in normal mode.
 *
 * \param[in]  module_inst   Pointer to the software module instance struct
 * \param[in]  top_value     New timer TOP value to set
 *
 * \return Status of the TOP set procedure.
 *
 * \retval STATUS_OK              The timer TOP value was updated successfully
 * \retval STATUS_ERR_INVALID_ARG The configured TC module counter size in the
 *                                module instance is invalid
 */
enum status_code tc_set_top_value (
		const struct tc_module *const module_inst,
		const uint32_t top_value)
{
	Assert(module_inst);
	Assert(module_inst->hw);
	Assert(top_value);

	Tc *const tc_module = module_inst->hw;

	while (tc_is_syncing(module_inst)) {
		/* Wait for sync */
	}

	switch (module_inst->counter_size) {
		case TC_COUNTER_SIZE_8BIT:
			if (module_inst->double_buffering_enabled){
				tc_module->COUNT8.PERBUF.reg = (uint8_t)top_value;
			} else {
				tc_module->COUNT8.PER.reg = (uint8_t)top_value;
			}
			return STATUS_OK;

		case TC_COUNTER_SIZE_16BIT:
			if (module_inst->double_buffering_enabled){
				tc_module->COUNT16.CCBUF[0].reg = (uint16_t)top_value;
			} else {
				tc_module->COUNT16.CC[0].reg = (uint16_t)top_value;
			}
			return STATUS_OK;

		case TC_COUNTER_SIZE_32BIT:
			if (module_inst->double_buffering_enabled){
				tc_module->COUNT32.CCBUF[0].reg = (uint32_t)top_value;
			} else {
				tc_module->COUNT32.CC[0].reg = (uint32_t)top_value;
			}
			return STATUS_OK;

		default:
			Assert(false);
			return STATUS_ERR_INVALID_ARG;
	}
}
