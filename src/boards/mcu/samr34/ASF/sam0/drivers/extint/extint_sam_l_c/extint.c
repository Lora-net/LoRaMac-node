/**
 * \file
 *
 * \brief SAM External Interrupt Driver
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
#include <system.h>
#include <system_interrupt.h>
#include <extint.h>
#include <conf_extint.h>

#if !defined(EXTINT_CLOCK_SELECTION) || defined(__DOXYGEN__)
#  warning  EXTINT_CLOCK_SELECTION is not defined, assuming EXTINT_CLK_GCLK.

/** Configuration option, setting the EIC clock source which can be used for
 *  EIC edge detection or filtering. This option may be overridden in the module
 *  configuration header file \c conf_extint.h.
 */
#  define EXTINT_CLOCK_SELECTION EXTINT_CLK_GCLK
#endif

#if (EXTINT_CLOCK_SELECTION == EXTINT_CLK_GCLK)
#if !defined(EXTINT_CLOCK_SOURCE) || defined(__DOXYGEN__)
#  warning  EXTINT_CLOCK_SOURCE is not defined, assuming GCLK_GENERATOR_0.

/** Configuration option, setting the EIC clock source which can be used for
 *  EIC edge detection or filtering. This option may be overridden in the module
 *  configuration header file \c conf_extint.h.
 */
#  define EXTINT_CLOCK_SOURCE GCLK_GENERATOR_0
#endif
#endif

/**
 * \internal
 * Internal driver device instance struct.
 */
struct _extint_module _extint_dev;

/**
 * \brief Determin if the general clock is required.
 *
 * \param[in] filter_input_signal Filter the raw input signal to prevent noise
 * \param[in] detection_criteria  Edge detection mode to use (\ref extint_detect)
 */
#define _extint_is_gclk_required(filter_input_signal, detection_criteria) \
		((filter_input_signal) ? true : (\
			(EXTINT_DETECT_RISING == (detection_criteria)) ? true : (\
			(EXTINT_DETECT_FALLING == (detection_criteria)) ? true : (\
			(EXTINT_DETECT_BOTH == (detection_criteria)) ? true : false))))

static void _extint_enable(void);
static void _extint_disable(void);

/**
 * \brief Determines if the hardware module(s) are currently synchronizing to the bus.
 *
 * Checks to see if the underlying hardware peripheral module(s) are currently
 * synchronizing across multiple clock domains to the hardware bus, This
 * function can be used to delay further operations on a module until such time
 * that it is ready, to prevent blocking delays for synchronization in the
 * user application.
 *
 * \return Synchronization status of the underlying hardware module(s).
 *
 * \retval true  If the module synchronization is ongoing
 * \retval false If the module has completed synchronization
 */
static inline bool extint_is_syncing(void)
{
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;

	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		if((eics[i]->SYNCBUSY.reg & EIC_SYNCBUSY_ENABLE)
		 || (eics[i]->SYNCBUSY.reg & EIC_SYNCBUSY_SWRST)){
			return true;
		}
	}
	return false;
}

/**
 * \internal
 * \brief Initializes and enables the External Interrupt driver.
 *
 * Enable the clocks used by External Interrupt driver.
 *
 * Resets the External Interrupt driver, resetting all hardware
 * module registers to their power-on defaults, then enable it for further use.
 *
 * Reset the callback list if callback mode is used.
 *
 * This function must be called before attempting to use any NMI or standard
 * external interrupt channel functions.
 *
 * \note When SYSTEM module is used, this function will be invoked by
 * \ref system_init() automatically if the module is included.
 */
void _system_extint_init(void);
void _system_extint_init(void)
{
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;

	/* Turn on the digital interface clock */
	system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBA, MCLK_APBAMASK_EIC);

#if (EXTINT_CLOCK_SELECTION == EXTINT_CLK_GCLK)
	/* Configure the generic clock for the module and enable it */
	struct system_gclk_chan_config gclk_chan_conf;
	system_gclk_chan_get_config_defaults(&gclk_chan_conf);
	gclk_chan_conf.source_generator = EXTINT_CLOCK_SOURCE;
	system_gclk_chan_set_config(EIC_GCLK_ID, &gclk_chan_conf);

	/* Enable the clock anyway, since when needed it will be requested
	 * by External Interrupt driver */
	system_gclk_chan_enable(EIC_GCLK_ID);
#endif

	/* Reset all EIC hardware modules. */
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		eics[i]->CTRLA.reg |= EIC_CTRLA_SWRST;
	}

	while (extint_is_syncing()) {
		/* Wait for all hardware modules to complete synchronization */
	}

#if (EXTINT_CLOCK_SELECTION == EXTINT_CLK_GCLK)
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		eics[i]->CTRLA.bit.CKSEL = EXTINT_CLK_GCLK;
	}
#else
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		eics[i]->CTRLA.bit.CKSEL = EXTINT_CLK_ULP32K;
	}
#endif

	/* Reset the software module */
#if EXTINT_CALLBACK_MODE == true
	/* Clear callback registration table */
	for (uint8_t j = 0; j < EIC_NUMBER_OF_INTERRUPTS; j++) {
		_extint_dev.callbacks[j] = NULL;
	}
	system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_EIC);
#endif

	/* Enables the driver for further use */
	_extint_enable();
}

/**
 * \internal
 * \brief Enables the External Interrupt driver.
 *
 * Enables EIC modules.
 * Registered callback list will not be affected if callback mode is used.
 */
void _extint_enable(void)
{
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;

	/* Enable all EIC hardware modules. */
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		eics[i]->CTRLA.reg |= EIC_CTRLA_ENABLE;
	}

	while (extint_is_syncing()) {
		/* Wait for all hardware modules to complete synchronization */
	}
}

/**
 * \internal
 * \brief Disables the External Interrupt driver.
 *
 * Disables EIC modules that were previously started via a call to
 * \ref _extint_enable().
 * Registered callback list will not be affected if callback mode is used.
 */
void _extint_disable(void)
{
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;

	/* Disable all EIC hardware modules. */
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		eics[i]->CTRLA.reg &= ~EIC_CTRLA_ENABLE;
	}

	while (extint_is_syncing()) {
		/* Wait for all hardware modules to complete synchronization */
	}
}

/**
 * \brief Initializes an External Interrupt channel configuration structure to defaults.
 *
 * Initializes a given External Interrupt channel configuration structure to a
 * set of known default values. This function should be called on all new
 * instances of these configuration structures before being modified by the
 * user application.
 *
 * The default configuration is as follows:
 * \li Input filtering disabled
 * \li Internal pull-up enabled
 * \li Detect falling edges of a signal
 * \li Asynchronous edge detection is disabled
 *
 * \param[out] config  Configuration structure to initialize to default values
 */
void extint_chan_get_config_defaults(
		struct extint_chan_conf *const config)
{
	/* Sanity check arguments */
	Assert(config);

	/* Default configuration values */
	config->gpio_pin            = 0;
	config->gpio_pin_mux        = 0;
	config->gpio_pin_pull       = EXTINT_PULL_UP;
	config->filter_input_signal = false;
	config->detection_criteria  = EXTINT_DETECT_FALLING;
	config->enable_async_edge_detection = false;
}

/**
 * \brief Writes an External Interrupt channel configuration to the hardware module.
 *
 * Writes out a given configuration of an External Interrupt channel
 * configuration to the hardware module. If the channel is already configured,
 * the new configuration will replace the existing one.
 *
 * \param[in] channel   External Interrupt channel to configure
 * \param[in] config    Configuration settings for the channel

 */
void extint_chan_set_config(
		const uint8_t channel,
		const struct extint_chan_conf *const config)
{
	/* Sanity check arguments */
	Assert(config);
	_extint_disable();
#if(EXTINT_CLOCK_SELECTION == EXTINT_CLK_GCLK)
	/* Sanity check clock requirements */
	Assert(!(!system_gclk_gen_is_enabled(EXTINT_CLOCK_SOURCE) &&
		_extint_is_gclk_required(config->filter_input_signal,
			config->detection_criteria)));
#endif
	struct system_pinmux_config pinmux_config;
	system_pinmux_get_config_defaults(&pinmux_config);

	pinmux_config.mux_position = config->gpio_pin_mux;
	pinmux_config.direction    = SYSTEM_PINMUX_PIN_DIR_INPUT;
	pinmux_config.input_pull   = (enum system_pinmux_pin_pull)config->gpio_pin_pull;
	system_pinmux_pin_set_config(config->gpio_pin, &pinmux_config);

	/* Get a pointer to the module hardware instance */
	Eic *const EIC_module = _extint_get_eic_from_channel(channel);

	uint32_t config_pos = (4 * (channel % 8));
	uint32_t new_config;

	/* Determine the channel's new edge detection configuration */
	new_config = (config->detection_criteria << EIC_CONFIG_SENSE0_Pos);

	/* Enable the hardware signal filter if requested in the config */
	if (config->filter_input_signal) {
		new_config |= EIC_CONFIG_FILTEN0;
	}

	/* Clear the existing and set the new channel configuration */
	EIC_module->CONFIG[channel / 8].reg
		= (EIC_module->CONFIG[channel / 8].reg &
			~((EIC_CONFIG_SENSE0_Msk | EIC_CONFIG_FILTEN0) << config_pos)) |
			(new_config << config_pos);
#if (SAML22) || (SAML21XXXB) || (SAMC20) || (SAMR30) || (SAMR34) || (SAMR35) || (WLR089)
	/* Config asynchronous edge detection */
	if (config->enable_async_edge_detection) {
		EIC_module->ASYNCH.reg |= (1UL << channel);
	} else {
		EIC_module->ASYNCH.reg &= (EIC_ASYNCH_MASK & (~(1UL << channel)));
	}
#endif
#if (SAMC21)
	/* Config asynchronous edge detection */
	if (config->enable_async_edge_detection) {
		EIC_module->EIC_ASYNCH.reg |= (1UL << channel);
	} else {
		EIC_module->EIC_ASYNCH.reg &= (EIC_EIC_ASYNCH_MASK & (~(1UL << channel)));
	}
#endif
	_extint_enable();
}

/**
 * \brief Writes an External Interrupt NMI channel configuration to the hardware module.
 *
 *  Writes out a given configuration of an External Interrupt NMI channel
 *  configuration to the hardware module. If the channel is already configured,
 *  the new configuration will replace the existing one.
 *
 *  \param[in] nmi_channel   External Interrupt NMI channel to configure
 *  \param[in] config        Configuration settings for the channel
 *
 * \returns Status code indicating the success or failure of the request.
 * \retval  STATUS_OK                   Configuration succeeded
 * \retval  STATUS_ERR_PIN_MUX_INVALID  An invalid pin mux value was supplied
 * \retval  STATUS_ERR_BAD_FORMAT       An invalid detection mode was requested
 */
enum status_code extint_nmi_set_config(
		const uint8_t nmi_channel,
		const struct extint_nmi_conf *const config)
{
	/* Sanity check arguments */
	Assert(config);

	/* Sanity check clock requirements */
	Assert(!(!system_gclk_gen_is_enabled(EXTINT_CLOCK_SOURCE) &&
		_extint_is_gclk_required(config->filter_input_signal,
			config->detection_criteria)));

	struct system_pinmux_config pinmux_config;
	system_pinmux_get_config_defaults(&pinmux_config);

	pinmux_config.mux_position = config->gpio_pin_mux;
	pinmux_config.direction    = SYSTEM_PINMUX_PIN_DIR_INPUT;
	pinmux_config.input_pull   = SYSTEM_PINMUX_PIN_PULL_UP;
	pinmux_config.input_pull   = (enum system_pinmux_pin_pull)config->gpio_pin_pull;
	system_pinmux_pin_set_config(config->gpio_pin, &pinmux_config);

	/* Get a pointer to the module hardware instance */
	Eic *const EIC_module = _extint_get_eic_from_channel(nmi_channel);

	uint32_t new_config;

	/* Determine the NMI's new edge detection configuration */
	new_config = (config->detection_criteria << EIC_NMICTRL_NMISENSE_Pos);

	/* Enable the hardware signal filter if requested in the config */
	if (config->filter_input_signal) {
		new_config |= EIC_NMICTRL_NMIFILTEN;
	}

#if (SAML21XXXB) || (SAML22) || (SAMC21) || (SAMR30) || (SAMR34) || (SAMR35) || (WLR089)
	/* Enable asynchronous edge detection if requested in the config */
	if (config->enable_async_edge_detection) {
		new_config |= EIC_NMICTRL_NMIASYNCH;
	}
#endif
	
	/* Disable EIC and general clock to configure NMI */
	_extint_disable();
#if(EXTINT_CLOCK_SELECTION == EXTINT_CLK_GCLK)
	system_gclk_chan_disable(EIC_GCLK_ID);
#else
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;
	for (uint32_t i = 0; i < EIC_INST_NUM; i++){
		eics[i]->CTRLA.bit.CKSEL = EXTINT_CLK_GCLK;
		system_gclk_chan_disable(EIC_GCLK_ID);
	}
#endif

	EIC_module->NMICTRL.reg = new_config;

	/* Enable the EIC clock and EIC after configure NMI */
#if(EXTINT_CLOCK_SELECTION == EXTINT_CLK_GCLK)
	system_gclk_chan_enable(EIC_GCLK_ID);
#else
	for (uint32_t i = 0; i < EIC_INST_NUM; i++){
		eics[i]->CTRLA.bit.CKSEL = EXTINT_CLK_ULP32K;
	}
#endif
	_extint_enable();

	return STATUS_OK;
}

/**
 * \brief Enables an External Interrupt event output.
 *
 *  Enables one or more output events from the External Interrupt module. See
 *  \ref extint_events "here" for a list of events this module supports.
 *
 *  \note Events cannot be altered while the module is enabled.
 *
 *  \param[in] events    Struct containing flags of events to enable
 */
void extint_enable_events(
		struct extint_events *const events)
{
	/* Sanity check arguments */
	Assert(events);

	/* Array of available EICs. */
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;

	_extint_disable();

	/* Update the event control register for each physical EIC instance */
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		uint32_t event_mask = 0;

		/* Create an enable mask for the current EIC module */
		for (uint32_t j = 0; j < 32; j++) {
			if (events->generate_event_on_detect[(32 * i) + j]) {
				event_mask |= (1UL << j);
			}
		}

		/* Enable the masked events */
		eics[i]->EVCTRL.reg |= event_mask;
	}
	_extint_enable();
}

/**
 * \brief Disables an External Interrupt event output.
 *
 *  Disables one or more output events from the External Interrupt module. See
 *  \ref extint_events "here" for a list of events this module supports.
 *
 *  \note Events cannot be altered while the module is enabled.
 *
 *  \param[in] events    Struct containing flags of events to disable
 */
void extint_disable_events(
		struct extint_events *const events)
{
	/* Sanity check arguments */
	Assert(events);

	/* Array of available EICs. */
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;

	_extint_disable();

	/* Update the event control register for each physical EIC instance */
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		uint32_t event_mask = 0;

		/* Create a disable mask for the current EIC module */
		for (uint32_t j = 0; j < 32; j++) {
			if (events->generate_event_on_detect[(32 * i) + j]) {
				event_mask |= (1UL << j);
			}
		}

		/* Disable the masked events */
		eics[i]->EVCTRL.reg &= ~event_mask;
	}
	_extint_enable();
}
