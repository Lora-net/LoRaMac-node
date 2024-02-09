/**
 * \file
 *
 * \brief SAM RTC Driver (Count Mode)
 *
 * Copyright (c) 2012-2020 Microchip Technology Inc. and its subsidiaries.
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
#include "rtc_count.h"
#include <clock.h>
#include <gclk.h>
#include "conf_rtc.h"

#if !defined(__DOXYGEN__)
struct rtc_module *_rtc_instance[RTC_INST_NUM];
#endif

#if !defined(RTC_CLOCK_SOURCE)
#  warning  RTC_CLOCK_SOURCE is not defined, assuming RTC_CLOCK_SELECTION_ULP1K.
#  define RTC_CLOCK_SOURCE RTC_CLOCK_SELECTION_ULP1K
#endif

/**
 * \brief Determines if the hardware module(s) are currently synchronizing to the bus.
 *
 * Checks to see if the underlying hardware peripheral module(s) are currently
 * synchronizing across multiple clock domains to the hardware bus, This
 * function can be used to delay further operations on a module until such time
 * that it is ready, to prevent blocking delays for synchronization in the
 * user application.
 *
 * \param[in]  module  RTC hardware module
 *
 * \return Synchronization status of the underlying hardware module(s).
 *
 * \retval true  if the module synchronization is ongoing
 * \retval false if the module has completed synchronization
 */
static bool rtc_count_is_syncing(struct rtc_module *const module)
{
 	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	if (rtc_module->MODE0.SYNCBUSY.reg) {
		return true;
	}

	return false;
}

/**
 * \brief Enables the RTC module.
 *
 * Enables the RTC module once it has been configured, ready for use. Most
 * module configuration parameters cannot be altered while the module is enabled.
 *
 * \param[in,out]  module  RTC hardware module
 */
void rtc_count_enable(struct rtc_module *const module)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

#if RTC_COUNT_ASYNC == true
	system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_RTC);
#endif

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Enable RTC module. */
	rtc_module->MODE0.CTRLA.reg |= RTC_MODE0_CTRLA_ENABLE;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}
}

/**
 * \brief Disables the RTC module.
 *
 * Disables the RTC module.
 *
 * \param[in,out]  module  RTC hardware module
 */
void rtc_count_disable(struct rtc_module *const module)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

#if RTC_COUNT_ASYNC == true
	system_interrupt_disable(SYSTEM_INTERRUPT_MODULE_RTC);
#endif

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Disbale interrupt */
	rtc_module->MODE0.INTENCLR.reg = RTC_MODE0_INTENCLR_MASK;
	/* Clear interrupt flag */
	rtc_module->MODE0.INTFLAG.reg = RTC_MODE0_INTFLAG_MASK;

	/* Disable RTC module. */
	rtc_module->MODE0.CTRLA.reg &= ~RTC_MODE0_CTRLA_ENABLE;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}
}

/**
 * \brief Resets the RTC module.
 * Resets the RTC to hardware defaults.
 *
 * \param[in,out]  module  Pointer to the software instance struct
 */
void rtc_count_reset(struct rtc_module *const module)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	/* Disable module before reset. */
	rtc_count_disable(module);

#if RTC_COUNT_ASYNC == true
	module->registered_callback = 0;
	module->enabled_callback    = 0;
#endif

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Initiate software reset. */
	rtc_module->MODE0.CTRLA.reg |= RTC_MODE0_CTRLA_SWRST;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}
}

/**
 * \internal Applies the given configuration.
 *
 * Sets the configurations given from the configuration structure to the
 * hardware module
 *
 * \param[in,out]  module  Pointer to the software instance struct
 * \param[in] config  Pointer to the configuration structure
 *
 * \return Status of the configuration procedure.
 * \retval STATUS_OK               RTC configurations was set successfully
 * \retval STATUS_ERR_INVALID_ARG  If invalid argument(s) were given
 */
static enum status_code _rtc_count_set_config(
		struct rtc_module *const module,
		const struct rtc_count_config *const config)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

#if SAML21 || SAMR30 || (SAMR34) || (SAMR35) || (WLR089)
	rtc_module->MODE0.CTRLA.reg = RTC_MODE0_CTRLA_MODE(0)
#if (SAML21XXXB) || (SAMR30) || (SAMR34) || (SAMR35) || (WLR089)
				    | (config->enable_read_sync << RTC_MODE0_CTRLA_COUNTSYNC_Pos)
#endif
				    | config->prescaler;
#endif
#if (SAMC20) || (SAMC21) || (SAML22)
	rtc_module->MODE0.CTRLA.reg = RTC_MODE0_CTRLA_MODE(0) | config->prescaler
			| (config->enable_read_sync << RTC_MODE0_CTRLA_COUNTSYNC_Pos);
#endif

	/* Set mode and clear on match if applicable. */
	switch (config->mode) {
		case RTC_COUNT_MODE_32BIT:
			/* Set 32-bit mode and clear on match if applicable. */
			rtc_module->MODE0.CTRLA.reg |= RTC_MODE0_CTRLA_MODE(0);

			/* Check if clear on compare match should be set. */
			if (config->clear_on_match) {
				/* Set clear on match. */
				rtc_module->MODE0.CTRLA.reg |= RTC_MODE0_CTRLA_MATCHCLR;
			}
			/* Set compare values. */
			for (uint8_t i = 0; i < RTC_COMP32_NUM; i++) {
				rtc_count_set_compare(module, config->compare_values[i],
						(enum rtc_count_compare)i);
			}
			break;

		case RTC_COUNT_MODE_16BIT:
			/* Set 16bit mode. */
			rtc_module->MODE1.CTRLA.reg |= RTC_MODE1_CTRLA_MODE(1);

			/* Check if match on clear is set, and return invalid
			 * argument if set. */
			if (config->clear_on_match) {
				Assert(false);
				return STATUS_ERR_INVALID_ARG;
			}
			/* Set compare values. */
			for (uint8_t i = 0; i < RTC_NUM_OF_COMP16; i++) {
				rtc_count_set_compare(module, config->compare_values[i],
						(enum rtc_count_compare)i);
			}
			break;
		default:
			Assert(false);
			return STATUS_ERR_INVALID_ARG;
	}

	/* Return status OK if everything was configured. */
	return STATUS_OK;
}

/**
 * \brief Initializes the RTC module with given configurations.
 *
 * Initializes the module, setting up all given configurations to provide
 * the desired functionality of the RTC.
 *
 * \param[out] module  Pointer to the software instance struct
 * \param[in]   hw      Pointer to hardware instance
 * \param[in] config  Pointer to the configuration structure
 *
 * \return Status of the initialization procedure.
 * \retval STATUS_OK               If the initialization was run stressfully
 * \retval STATUS_ERR_INVALID_ARG  If invalid argument(s) were given
 */
enum status_code rtc_count_init(
		struct rtc_module *const module,
		Rtc *const hw,
		const struct rtc_count_config *const config)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(hw);
	Assert(config);

	/* Initialize device instance */
	module->hw = hw;

	/* Turn on the digital interface clock */
	system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBA, MCLK_APBAMASK_RTC);

	/* Select RTC clock */
	OSC32KCTRL->RTCCTRL.reg = RTC_CLOCK_SOURCE;

	/* Reset module to hardware defaults. */
	rtc_count_reset(module);

	/* Save conf_struct internally for continued use. */
	module->mode                = config->mode;

#  if (RTC_INST_NUM == 1)
	_rtc_instance[0] = module;
#  else
	/* Register this instance for callbacks*/
	_rtc_instance[_rtc_get_inst_index(hw)] = module;
#  endif

	/* Set config and return status. */
	return _rtc_count_set_config(module, config);
}

/**
 * \brief Set the current count value to desired value.
 *
 * Sets the value of the counter to the specified value.
 *
 * \param[in,out] module  Pointer to the software instance struct
 * \param[in] count_value  The value to be set in count register
 *
 * \return Status of setting the register.
 * \retval STATUS_OK               If everything was executed correctly
 * \retval STATUS_ERR_INVALID_ARG  If invalid argument(s) were provided
 */
enum status_code rtc_count_set_count(
		struct rtc_module *const module,
		const uint32_t count_value)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Set count according to mode */
	switch(module->mode){
		case RTC_COUNT_MODE_32BIT:
			/* Write value to register. */
			rtc_module->MODE0.COUNT.reg = count_value;
			break;
		case RTC_COUNT_MODE_16BIT:
			/* Check if 16-bit value is provided. */
			if(count_value > 0xffff){
				return STATUS_ERR_INVALID_ARG;
			}

			/* Write value to register. */
			rtc_module->MODE1.COUNT.reg = (uint32_t)count_value;

			break;

		default:
			Assert(false);
			return STATUS_ERR_INVALID_ARG;
	}

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}
	return STATUS_OK;
}

/**
 * \brief Get the current count value.
 *
 * \param[in,out] module  Pointer to the software instance struct
 *
 * Returns the current count value.
 *
 * \return The current counter value as a 32-bit unsigned integer.
 */
uint32_t rtc_count_get_count(struct rtc_module *const module)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	/* Initialize return value. */
	uint32_t ret_val;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Read value based on mode. */
	switch (module->mode) {
		case RTC_COUNT_MODE_32BIT:
			/* Return count value in 32-bit mode. */
			ret_val = rtc_module->MODE0.COUNT.reg;

			break;

		case RTC_COUNT_MODE_16BIT:
			/* Return count value in 16-bit mode. */
			ret_val = (uint32_t)rtc_module->MODE1.COUNT.reg;

			break;

		default:
			Assert(false);
			/* Counter not initialized. Assume counter value 0.*/
			ret_val = 0;
			break;
	}

	return ret_val;
}

/**
 * \brief Set the compare value for the specified compare.
 *
 * Sets the value specified by the implementer to the requested compare.
 *
 * \note Compare 4 and 5 are only available in 16-bit mode.
 *
 * \param[in,out] module  Pointer to the software instance struct
 * \param[in] comp_value  The value to be written to the compare
 * \param[in] comp_index  Index of the compare to set
 *
 * \return Status indicating if compare was successfully set.
 * \retval STATUS_OK               If compare was successfully set
 * \retval STATUS_ERR_INVALID_ARG  If invalid argument(s) were provided
 * \retval STATUS_ERR_BAD_FORMAT   If the module was not initialized in a mode
 */
enum status_code rtc_count_set_compare(
		struct rtc_module *const module,
		const uint32_t comp_value,
		const enum rtc_count_compare comp_index)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Set compare values based on operation mode. */
	switch (module->mode) {
		case RTC_COUNT_MODE_32BIT:
			/* Check sanity of comp_index. */
			if ((uint32_t)comp_index > RTC_COMP32_NUM) {
				return STATUS_ERR_INVALID_ARG;
			}

			/* Set compare value for COMP. */
			rtc_module->MODE0.COMP[comp_index].reg = comp_value;

			break;

		case RTC_COUNT_MODE_16BIT:
			/* Check sanity of comp_index. */
			if ((uint32_t)comp_index > RTC_NUM_OF_COMP16) {
				return STATUS_ERR_INVALID_ARG;
			}

			/* Check that 16-bit value is provided. */
			if (comp_value > 0xffff) {
				Assert(false);
				return STATUS_ERR_INVALID_ARG;
			}

			/* Set compare value for COMP. */
			rtc_module->MODE1.COMP[comp_index].reg = comp_value & 0xffff;

			break;

		default:
			Assert(false);
			return STATUS_ERR_BAD_FORMAT;
	}

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Return status if everything is OK. */
	return STATUS_OK;
}

/**
 * \brief Get the current compare value of specified compare.
 *
 * Retrieves the current value of the specified compare.
 *
 * \note Compare 4 and 5 are only available in 16-bit mode.
 *
 * \param[in,out] module  Pointer to the software instance struct
 * \param[out] comp_value  Pointer to 32-bit integer that will be populated with
 *                         the current compare value
 * \param[in]  comp_index  Index of compare to check
 *
 * \return Status of the reading procedure.
 * \retval STATUS_OK               If the value was read correctly
 * \retval STATUS_ERR_INVALID_ARG  If invalid argument(s) were provided
 * \retval STATUS_ERR_BAD_FORMAT   If the module was not initialized in a mode
 */
enum status_code rtc_count_get_compare(
		struct rtc_module *const module,
		uint32_t *const comp_value,
		const enum rtc_count_compare comp_index)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	switch (module->mode) {
		case RTC_COUNT_MODE_32BIT:
			/* Check sanity of comp_index. */
			if ((uint32_t)comp_index > RTC_COMP32_NUM) {
				return STATUS_ERR_INVALID_ARG;
			}

			/* Get compare value for COMP. */
			*comp_value = rtc_module->MODE0.COMP[comp_index].reg;

			break;

		case RTC_COUNT_MODE_16BIT:
			/* Check sanity of comp_index. */
			if ((uint32_t)comp_index > RTC_NUM_OF_COMP16) {
				return STATUS_ERR_INVALID_ARG;
			}

			/* Get compare value for COMP. */
			*comp_value = (uint32_t)rtc_module->MODE1.COMP[comp_index].reg;

			break;

		default:
			Assert(false);
			return STATUS_ERR_BAD_FORMAT;
	}
	/* Return status showing everything is OK. */
	return STATUS_OK;
}

/**
 * \brief Retrieves the value of period.
 *
 * Retrieves the value of the period for the 16-bit mode counter.
 *
 * \note Only available in 16-bit mode.
 *
 * \param[in,out] module  Pointer to the software instance struct
 * \param[out] period_value  Pointer to value for return argument
 *
 * \return Status of getting the period value.
 * \retval STATUS_OK                   If the period value was read correctly
 * \retval STATUS_ERR_UNSUPPORTED_DEV  If incorrect mode was set
 */
enum status_code rtc_count_get_period(
		struct rtc_module *const module,
		uint16_t *const period_value)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Check that correct mode is set. */
	if (module->mode != RTC_COUNT_MODE_16BIT) {
		return STATUS_ERR_UNSUPPORTED_DEV;
	}

	/* Returns the value. */
	*period_value = rtc_module->MODE1.PER.reg;

	return STATUS_OK;
}

/**
 * \brief Set the given value to the period.
 *
 * Sets the given value to the period.
 *
 * \note Only available in 16-bit mode.
 *
 * \param[in,out] module  Pointer to the software instance struct
 * \param[in] period_value  The value to set to the period
 *
 * \return Status of setting the period value.
 * \retval STATUS_OK                   If the period was set correctly
 * \retval STATUS_ERR_UNSUPPORTED_DEV  If module is not operated in 16-bit mode
 */
enum status_code rtc_count_set_period(
		struct rtc_module *const module,
		const uint16_t period_value)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	/* Check that correct mode is set. */
	if (module->mode != RTC_COUNT_MODE_16BIT) {
		return STATUS_ERR_UNSUPPORTED_DEV;
	}

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Write value to register. */
	rtc_module->MODE1.PER.reg = period_value;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	return STATUS_OK;
}

/**
 * \brief Check if RTC compare match has occurred.
 *
 * Checks the compare flag to see if a match has occurred. The compare flag is
 * set when there is a compare match between counter and the compare.
 *
 * \note Compare 4 and 5 are only available in 16-bit mode.
 *
 * \param[in,out] module  Pointer to the software instance struct
 * \param[in] comp_index  Index of compare to check current flag
 */
bool rtc_count_is_compare_match(
		struct rtc_module *const module,
		const enum rtc_count_compare comp_index)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	/* Check sanity. */
	switch (module->mode) {
		case RTC_COUNT_MODE_32BIT:
			/* Check sanity for 32-bit mode. */
			if (comp_index > RTC_COMP32_NUM) {
				return false;
			}

			break;

		case RTC_COUNT_MODE_16BIT:
			/* Check sanity for 16-bit mode. */
			if (comp_index > RTC_NUM_OF_COMP16) {
				return false;
			}

			break;

		default:
			Assert(false);
			return false;
	}

	/* Set status of INTFLAG as return argument. */
	return (rtc_module->MODE0.INTFLAG.reg & RTC_MODE1_INTFLAG_CMP(1 << comp_index)) ? true : false;
}

/**
 * \brief Clears RTC compare match flag.
 *
 * Clears the compare flag. The compare flag is set when there is a compare
 * match between the counter and the compare.
 *
 * \note Compare 4 and 5 are only available in 16-bit mode.
 *
 * \param[in,out] module  Pointer to the software instance struct
 * \param[in] comp_index  Index of compare to check current flag
 *
 * \return Status indicating if flag was successfully cleared.
 * \retval STATUS_OK               If flag was successfully cleared
 * \retval STATUS_ERR_INVALID_ARG  If invalid argument(s) were provided
 * \retval STATUS_ERR_BAD_FORMAT   If the module was not initialized in a mode
 */
enum status_code rtc_count_clear_compare_match(
		struct rtc_module *const module,
		const enum rtc_count_compare comp_index)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	/* Check sanity. */
	switch (module->mode){
		case RTC_COUNT_MODE_32BIT:
			/* Check sanity for 32-bit mode. */
			if (comp_index > RTC_COMP32_NUM) {
				return STATUS_ERR_INVALID_ARG;
			}

			break;

		case RTC_COUNT_MODE_16BIT:
			/* Check sanity for 16-bit mode. */
			if (comp_index > RTC_NUM_OF_COMP16) {
				return STATUS_ERR_INVALID_ARG;
			}

			break;

		default:
			Assert(false);
			return STATUS_ERR_BAD_FORMAT;
	}

	/* Clear INTFLAG. */
	rtc_module->MODE0.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP(1 << comp_index);

	return STATUS_OK;
}

/**
 * \brief Calibrate for too-slow or too-fast oscillator.
 *
 * When used, the RTC will compensate for an inaccurate oscillator. The
 * RTC module will add or subtract cycles from the RTC prescaler to adjust the
 * frequency in approximately 1 PPM steps. The provided correction value should
 * be between 0 and 127, allowing for a maximum 127 PPM correction.
 *
 * If no correction is needed, set value to zero.
 *
 * \note Can only be used when the RTC is operated in 1Hz.
 *
 * \param[in,out] module  Pointer to the software instance struct
 * \param[in] value  Ranging from -127 to 127 used for the correction
 *
 * \return Status of the calibration procedure.
 * \retval STATUS_OK               If calibration was executed correctly
 * \retval STATUS_ERR_INVALID_ARG  If invalid argument(s) were provided
 */
enum status_code rtc_count_frequency_correction(
		struct rtc_module *const module,
		const int8_t value)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	/* Check if valid argument. */
	if (abs(value) > 0x7F) {
		/* Value bigger than allowed, return invalid argument. */
		return STATUS_ERR_INVALID_ARG;
	}

	uint32_t new_correction_value;

	/* Load the new correction value as a positive value, sign added later */
	new_correction_value = abs(value);

	/* Convert to positive value and adjust register sign bit. */
	if (value < 0) {
		new_correction_value |= RTC_FREQCORR_SIGN;
	}

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Set value. */
	rtc_module->MODE0.FREQCORR.reg = new_correction_value;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	return STATUS_OK;
}

#ifdef FEATURE_RTC_TAMPER_DETECTION
/**
 * \brief Applies the given configuration.
 *
 * Sets the configurations given from the configuration structure to the
 * RTC tamper and it should be called before RTC module enable. 
 *
 * \param[in,out]  module  Pointer to the software instance struct
 * \param[in] config  Pointer to the configuration structure
 *
 * \return Status of the configuration procedure.
 * \retval STATUS_OK               RTC configurations was set successfully
 * \note If tamper input configured as active layer protection, RTC prescaler
 *       output automatically enabled in the function.
 */
enum status_code rtc_tamper_set_config ( 
		struct rtc_module *const module,
		struct rtc_tamper_config *const tamper_cfg)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);
	Assert(tamper_cfg);

	Rtc *const rtc_module = module->hw;
	uint16_t ctrl_b = 0;

	/* Configure enable backup and GP register reset on tamper or not. */
	if(tamper_cfg->bkup_reset_on_tamper) {
		rtc_module->MODE0.CTRLA.reg |= RTC_MODE0_CTRLA_BKTRST;
	} else {
		rtc_module->MODE0.CTRLA.reg &= ~RTC_MODE0_CTRLA_BKTRST;
	}
	
	if (tamper_cfg->gp_reset_on_tamper) {
		rtc_module->MODE0.CTRLA.reg |= RTC_MODE0_CTRLA_GPTRST;
	} else {
		rtc_module->MODE0.CTRLA.reg &= ~RTC_MODE0_CTRLA_GPTRST;
	}

	/* Configure tamper detection of frequency and debounce setting. */
	ctrl_b = tamper_cfg->actl_freq_div | tamper_cfg->deb_freq_div;
	if(tamper_cfg->deb_seq == RTC_TAMPER_DEBOUNCE_ASYNC) {
		ctrl_b |= RTC_MODE0_CTRLB_DEBASYNC;
	} else if (tamper_cfg->deb_seq == RTC_TAMPER_DEBOUNCE_MAJORITY) {
		ctrl_b |= RTC_MODE0_CTRLB_DEBMAJ;
	}
	if(tamper_cfg->dma_tamper_enable) {
		ctrl_b |= RTC_MODE0_CTRLB_DMAEN;
	}
	if (tamper_cfg->gp0_enable) {
		ctrl_b |= RTC_MODE0_CTRLB_GP0EN;
	}

	/* Configure tamper input. */
	volatile RTC_TAMPCTRL_Type *tamper_ctrl = &(rtc_module->MODE0.TAMPCTRL);

	struct rtc_tamper_input_config in_cfg;
	for (uint8_t tamper_id = 0; tamper_id < RTC_TAMPER_NUM; tamper_id++) {
		in_cfg = tamper_cfg->in_cfg[tamper_id];

		if(in_cfg.action == RTC_TAMPER_INPUT_ACTION_ACTL) {
			ctrl_b |= RTC_MODE0_CTRLB_RTCOUT;
		}
	
		switch(tamper_id) {
			case 0:
				tamper_ctrl->bit.IN0ACT = in_cfg.action;
				tamper_ctrl->bit.TAMLVL0 = in_cfg.level;
				tamper_ctrl->bit.DEBNC0 = in_cfg.debounce_enable;
				break;
			case 1:
				tamper_ctrl->bit.IN1ACT = in_cfg.action;
				tamper_ctrl->bit.TAMLVL1 = in_cfg.level;
				tamper_ctrl->bit.DEBNC1 = in_cfg.debounce_enable;
				break;
			case 2:
				tamper_ctrl->bit.IN2ACT = in_cfg.action;
				tamper_ctrl->bit.TAMLVL2 = in_cfg.level;
				tamper_ctrl->bit.DEBNC2 = in_cfg.debounce_enable;
				break;
			case 3:
				tamper_ctrl->bit.IN3ACT = in_cfg.action;
				tamper_ctrl->bit.TAMLVL3 = in_cfg.level;
				tamper_ctrl->bit.DEBNC3 = in_cfg.debounce_enable;
				break;
			case 4:
				tamper_ctrl->bit.IN4ACT = in_cfg.action;
				tamper_ctrl->bit.TAMLVL4 = in_cfg.level;
				tamper_ctrl->bit.DEBNC4 = in_cfg.debounce_enable;
				break;
			default:
				Assert(false);
				break;
		}
	}

	rtc_module->MODE0.CTRLB.reg = ctrl_b;

	/* Return status OK if everything was configured. */
	return STATUS_OK;
}

/**
 * \brief Get the tamper stamp value.
 *
 * \param[in,out] module  Pointer to the software instance struct
 *
 * \return The current tamper stamp value as a 32-bit unsigned integer.
 */
uint32_t rtc_tamper_get_stamp (struct rtc_module *const module)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	Rtc *const rtc_module = module->hw;

	/* Initialize return value. */
	uint32_t tamper_stamp = 0;

	while (rtc_count_is_syncing(module)) {
		/* Wait for synchronization */
	}

	/* Read value based on mode. */
	switch (module->mode) {
		case RTC_COUNT_MODE_32BIT:
			/* Return stamp value in 32-bit mode. */
			tamper_stamp = rtc_module->MODE0.TIMESTAMP.reg;

			break;

		case RTC_COUNT_MODE_16BIT:
			/* Return stamp value in 16-bit mode. */
			tamper_stamp = (uint32_t)rtc_module->MODE1.TIMESTAMP.reg;

			break;

		default:
			Assert(false);
			break;
	}

	return tamper_stamp;
}

#endif
