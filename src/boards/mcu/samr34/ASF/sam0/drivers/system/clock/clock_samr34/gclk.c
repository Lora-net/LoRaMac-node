/**
 * \file
 *
 * \brief SAMR34 Generic Clock Driver
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

#include <gclk.h>
#include <clock.h>
#include <system_interrupt.h>


 /**
  * \brief Determines if the hardware module(s) are currently synchronizing to the bus.
  *
  * Checks to see if the underlying hardware peripheral module(s) are currently
  * synchronizing across multiple clock domains to the hardware bus, This
  * function can be used to delay further operations on a module until such time
  * that it is ready, to prevent blocking delays for synchronization in the
  * user application.
  * \param[in] generator  Generic Clock Generator index to sync
  *
  * \return Synchronization status of the underlying hardware module(s).
  *
  * \retval false if the module has completed synchronization
  * \retval true if the module synchronization is ongoing
  */
static inline bool system_gclk_is_syncing(const uint8_t generator)
{

	 if (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL(1 << generator )){
		 return true;
	}

	 return false;
}


/**
 * \brief Initializes the GCLK driver.
 *
 * Initializes the Generic Clock module, disabling and resetting all active
 * Generic Clock Generators and Channels to their power-on default values.
 */
void system_gclk_init(void)
{
	/* Turn on the digital interface clock */
	system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBA, MCLK_APBAMASK_GCLK);

	/* Software reset the module to ensure it is re-initialized correctly */
	GCLK->CTRLA.reg = GCLK_CTRLA_SWRST;
	while (GCLK->CTRLA.reg & GCLK_CTRLA_SWRST) {
		/* Wait for reset to complete */
	}
}

/**
 * \brief Writes a Generic Clock Generator configuration to the hardware module.
 *
 * Writes out a given configuration of a Generic Clock Generator configuration
 * to the hardware module.
 *
 * \note Changing the clock source on the fly (on a running
 *       generator) can take additional time if the clock source is configured
 *       to only run on-demand (ONDEMAND bit is set) and it is not currently
 *       running (no peripheral is requesting the clock source). In this case
 *       the GCLK will request the new clock while still keeping a request to
 *       the old clock source until the new clock source is ready.
 *
 * \note This function will not start a generator that is not already running;
 *       to start the generator, call \ref system_gclk_gen_enable()
 *       after configuring a generator.
 *
 * \param[in] generator  Generic Clock Generator index to configure
 * \param[in] config     Configuration settings for the generator
 */
void system_gclk_gen_set_config(
		const uint8_t generator,
		struct system_gclk_gen_config *const config)
{
	/* Sanity check arguments */
	Assert(config);

	/* Cache new register configurations to minimize sync requirements. */
	uint32_t new_genctrl_config ;


	/* Select the requested source clock for the generator */
	new_genctrl_config = config->source_clock << GCLK_GENCTRL_SRC_Pos;

	/* Configure the clock to be either high or low when disabled */
	if (config->high_when_disabled) {
		new_genctrl_config |= GCLK_GENCTRL_OOV;
	}

	/* Configure if the clock output to I/O pin should be enabled. */
	if (config->output_enable) {
		new_genctrl_config |= GCLK_GENCTRL_OE;
	}

	/* Set division factor */
	if (config->division_factor > 1) {
		/* Check if division is a power of two */
		if (((config->division_factor & (config->division_factor - 1)) == 0)) {
			/* Determine the index of the highest bit set to get the
			 * division factor that must be loaded into the division
			 * register */

			uint32_t div2_count = 0;

			uint32_t mask;
			for (mask = (1UL << 1); mask < config->division_factor;
						mask <<= 1) {
				div2_count++;
			}

			/* Set binary divider power of 2 division factor */
			new_genctrl_config  |= div2_count << GCLK_GENCTRL_DIV_Pos;
			new_genctrl_config |= GCLK_GENCTRL_DIVSEL;
		} else {
			/* Set integer division factor */

			new_genctrl_config  |=
					(config->division_factor) << GCLK_GENCTRL_DIV_Pos;

			/* Enable non-binary division with increased duty cycle accuracy */
			new_genctrl_config |= GCLK_GENCTRL_IDC;
		}

	}

	/* Enable or disable the clock in standby mode */
	if (config->run_in_standby) {
		new_genctrl_config |= GCLK_GENCTRL_RUNSTDBY;
	}

	while (system_gclk_is_syncing(generator)) {
		/* Wait for synchronization */
	};

	system_interrupt_enter_critical_section();

	GCLK->GENCTRL[generator].reg = new_genctrl_config | (GCLK->GENCTRL[generator].reg & GCLK_GENCTRL_GENEN);

	while (system_gclk_is_syncing(generator)) {
		/* Wait for synchronization */
	};

	system_interrupt_leave_critical_section();
}

/**
 * \brief Enables a Generic Clock Generator that was previously configured.
 *
 * Starts the clock generation of a Generic Clock Generator that was previously
 * configured via a call to \ref system_gclk_gen_set_config().
 *
 * \param[in] generator  Generic Clock Generator index to enable
 */
void system_gclk_gen_enable(
		const uint8_t generator)
{
	while (system_gclk_is_syncing(generator)) {
		/* Wait for synchronization */
	};

	system_interrupt_enter_critical_section();

	/* Enable generator */
	GCLK->GENCTRL[generator].reg |= GCLK_GENCTRL_GENEN;

	system_interrupt_leave_critical_section();
}

/**
 * \brief Disables a Generic Clock Generator that was previously enabled.
 *
 * Stops the clock generation of a Generic Clock Generator that was previously
 * started via a call to \ref system_gclk_gen_enable().
 *
 * \param[in] generator  Generic Clock Generator index to disable
 */
void system_gclk_gen_disable(
		const uint8_t generator)
{
	while (system_gclk_is_syncing(generator)) {
		/* Wait for synchronization */
	};

	system_interrupt_enter_critical_section();

	/* Disable generator */
	GCLK->GENCTRL[generator].reg &= ~GCLK_GENCTRL_GENEN;
	while (GCLK->GENCTRL[generator].reg & GCLK_GENCTRL_GENEN) {
		/* Wait for clock to become disabled */
	}

	system_interrupt_leave_critical_section();
}

/**
 * \brief Determins if the specified Generic Clock Generator is enabled.
 *
 * \param[in] generator  Generic Clock Generator index to check
 *
 * \return The enabled status.
 * \retval true The Generic Clock Generator is enabled
 * \retval false The Generic Clock Generator is disabled
 */
bool system_gclk_gen_is_enabled(
		const uint8_t generator)
{
	bool enabled;

	system_interrupt_enter_critical_section();

	/* Obtain the enabled status */
	enabled = (GCLK->GENCTRL[generator].reg & GCLK_GENCTRL_GENEN);

	system_interrupt_leave_critical_section();

	return enabled;
}

/**
 * \brief Retrieves the clock frequency of a Generic Clock generator.
 *
 * Determines the clock frequency (in Hz) of a specified Generic Clock
 * generator, used as a source to a Generic Clock Channel module.
 *
 * \param[in] generator  Generic Clock Generator index
 *
 * \return The frequency of the generic clock generator, in Hz.
 */
uint32_t system_gclk_gen_get_hz(
		const uint8_t generator)
{
	while (system_gclk_is_syncing(generator)) {
		/* Wait for synchronization */
	};

	system_interrupt_enter_critical_section();

	/* Get the frequency of the source connected to the GCLK generator */
	uint32_t gen_input_hz = system_clock_source_get_hz(
			(enum system_clock_source)GCLK->GENCTRL[generator].bit.SRC);

	uint8_t divsel = GCLK->GENCTRL[generator].bit.DIVSEL;
	uint32_t divider = GCLK->GENCTRL[generator].bit.DIV;

	system_interrupt_leave_critical_section();

	/* Check if the generator is using fractional or binary division */
	if (!divsel && divider > 1) {
		gen_input_hz /= divider;
	} else if (divsel) {
		gen_input_hz >>= (divider+1);
	}

	return gen_input_hz;
}

/**
 * \brief Writes a Generic Clock configuration to the hardware module.
 *
 * Writes out a given configuration of a Generic Clock configuration to the
 * hardware module. If the clock is currently running, it will be stopped.
 *
 * \note Once called the clock will not be running; to start the clock,
 *       call \ref system_gclk_chan_enable() after configuring a clock channel.
 *
 * \param[in] channel   Generic Clock channel to configure
 * \param[in] config    Configuration settings for the clock
 *
 */
void system_gclk_chan_set_config(
		const uint8_t channel,
		struct system_gclk_chan_config *const config)
{
	/* Sanity check arguments */
	Assert(config);

	/* Disable generic clock channel */
	system_gclk_chan_disable(channel);

	/* Configure the peripheral channel */
	GCLK->PCHCTRL[channel].reg = GCLK_PCHCTRL_GEN(config->source_generator);


}

/**
 * \brief Enables a Generic Clock that was previously configured.
 *
 * Starts the clock generation of a Generic Clock that was previously
 * configured via a call to \ref system_gclk_chan_set_config().
 *
 * \param[in] channel   Generic Clock channel to enable
 */
void system_gclk_chan_enable(
		const uint8_t channel)
{
	system_interrupt_enter_critical_section();

	/* Enable the peripheral channel */
	GCLK->PCHCTRL[channel].reg |= GCLK_PCHCTRL_CHEN;

	while (!(GCLK->PCHCTRL[channel].reg & GCLK_PCHCTRL_CHEN)) {
		/* Wait for clock synchronization */
	}

	system_interrupt_leave_critical_section();
}

/**
 * \brief Disables a Generic Clock that was previously enabled.
 *
 * Stops the clock generation of a Generic Clock that was previously started
 * via a call to \ref system_gclk_chan_enable().
 *
 * \param[in] channel  Generic Clock channel to disable
 */
void system_gclk_chan_disable(
		const uint8_t channel)
{
	system_interrupt_enter_critical_section();

	/* Sanity check WRTLOCK */
	Assert(!GCLK->PCHCTRL[channel].bit.WRTLOCK);

	/* Disable the peripheral channel */
	GCLK->PCHCTRL[channel].reg &= ~GCLK_PCHCTRL_CHEN;

	while (GCLK->PCHCTRL[channel].reg & GCLK_PCHCTRL_CHEN) {
		/* Wait for clock synchronization */
	}

	system_interrupt_leave_critical_section();
}

/**
 * \brief Determins if the specified Generic Clock channel is enabled.
 *
 * \param[in] channel  Generic Clock Channel index
 *
 * \return The enabled status.
 * \retval true The Generic Clock channel is enabled
 * \retval false The Generic Clock channel is disabled
 */
bool system_gclk_chan_is_enabled(
		const uint8_t channel)
{
	bool enabled;

	system_interrupt_enter_critical_section();

	/* Select the requested generic clock channel */
	enabled = GCLK->PCHCTRL[channel].bit.CHEN;

	system_interrupt_leave_critical_section();

	return enabled;
}

/**
 * \brief Locks a Generic Clock channel from further configuration writes.
 *
 * Locks a generic clock channel from further configuration writes. It is only
 * possible to unlock the channel configuration through a power on reset.
 *
 * \param[in] channel   Generic Clock channel to enable
 */
void system_gclk_chan_lock(
		const uint8_t channel)
{
	system_interrupt_enter_critical_section();

	GCLK->PCHCTRL[channel].reg |= GCLK_PCHCTRL_WRTLOCK | GCLK_PCHCTRL_CHEN;
	system_interrupt_leave_critical_section();
}

/**
 * \brief Determins if the specified Generic Clock channel is locked.
 *
 * \param[in] channel  Generic Clock Channel index
 *
 * \return The lock status.
 * \retval true The Generic Clock channel is locked
 * \retval false The Generic Clock channel is not locked
 */
bool system_gclk_chan_is_locked(
		const uint8_t channel)
{
	bool locked;

	system_interrupt_enter_critical_section();
	locked = GCLK->PCHCTRL[channel].bit.WRTLOCK;
	system_interrupt_leave_critical_section();

	return locked;
}

/**
 * \brief Retrieves the clock frequency of a Generic Clock channel.
 *
 * Determines the clock frequency (in Hz) of a specified Generic Clock
 * channel, used as a source to a device peripheral module.
 *
 * \param[in] channel  Generic Clock Channel index
 *
 * \return The frequency of the generic clock channel, in Hz.
 */
uint32_t system_gclk_chan_get_hz(
		const uint8_t channel)
{
	uint8_t gen_id;

	system_interrupt_enter_critical_section();
	/* Select the requested generic clock channel */
	gen_id = GCLK->PCHCTRL[channel].bit.GEN;
	system_interrupt_leave_critical_section();

	/* Return the clock speed of the associated GCLK generator */
	return system_gclk_gen_get_hz(gen_id);
}
