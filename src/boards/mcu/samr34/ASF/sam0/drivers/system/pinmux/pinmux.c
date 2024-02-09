/**
 * \file
 *
 * \brief SAM Pin Multiplexer Driver
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
#include <pinmux.h>

/**
 * \internal
 * Writes out a given configuration of a Port pin configuration to the
 * hardware module.
 *
 * \note If the pin direction is set as an output, the pull-up/pull-down input
 *       configuration setting is ignored.
 *
 * \param[in] port      Base of the PORT module to configure
 * \param[in] pin_mask  Mask of the port pin to configure
 * \param[in] config    Configuration settings for the pin
 */
static void _system_pinmux_config(
		PortGroup *const port,
		const uint32_t pin_mask,
		const struct system_pinmux_config *const config)
{
	Assert(port);
	Assert(config);

	/* Track the configuration bits into a temporary variable before writing */
	uint32_t pin_cfg = 0;

	/* Enabled powersave mode, don't create configuration */
	if (!config->powersave) {
		/* Enable the pin peripheral MUX flag if non-GPIO selected (pinmux will
		 * be written later) and store the new MUX mask */
		if (config->mux_position != SYSTEM_PINMUX_GPIO) {
			pin_cfg |= PORT_WRCONFIG_PMUXEN;
			pin_cfg |= (config->mux_position << PORT_WRCONFIG_PMUX_Pos);
		}

		/* Check if the user has requested that the input buffer be enabled */
		if ((config->direction == SYSTEM_PINMUX_PIN_DIR_INPUT) ||
				(config->direction == SYSTEM_PINMUX_PIN_DIR_OUTPUT_WITH_READBACK)) {
			/* Enable input buffer flag */
			pin_cfg |= PORT_WRCONFIG_INEN;

			/* Enable pull-up/pull-down control flag if requested */
			if (config->input_pull != SYSTEM_PINMUX_PIN_PULL_NONE) {
				pin_cfg |= PORT_WRCONFIG_PULLEN;
			}

			/* Clear the port DIR bits to disable the output buffer */
			port->DIRCLR.reg = pin_mask;
		}

		/* Check if the user has requested that the output buffer be enabled */
		if ((config->direction == SYSTEM_PINMUX_PIN_DIR_OUTPUT) ||
				(config->direction == SYSTEM_PINMUX_PIN_DIR_OUTPUT_WITH_READBACK)) {
			/* Cannot use a pull-up if the output driver is enabled,
			 * if requested the input buffer can only sample the current
			 * output state */
			pin_cfg &= ~PORT_WRCONFIG_PULLEN;
		}
	} else {
		port->DIRCLR.reg = pin_mask;
	}

	/* The Write Configuration register (WRCONFIG) requires the
	 * pins to to grouped into two 16-bit half-words - split them out here */
	uint32_t lower_pin_mask = (pin_mask & 0xFFFF);
	uint32_t upper_pin_mask = (pin_mask >> 16);

	/* Configure the lower 16-bits of the port to the desired configuration,
	 * including the pin peripheral multiplexer just in case it is enabled */
	port->WRCONFIG.reg
		= (lower_pin_mask << PORT_WRCONFIG_PINMASK_Pos) |
			pin_cfg | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_WRPINCFG;

	/* Configure the upper 16-bits of the port to the desired configuration,
	 * including the pin peripheral multiplexer just in case it is enabled */
	port->WRCONFIG.reg
		= (upper_pin_mask << PORT_WRCONFIG_PINMASK_Pos) |
			pin_cfg | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_WRPINCFG |
			PORT_WRCONFIG_HWSEL;

	if(!config->powersave) {
		/* Set the pull-up state once the port pins are configured if one was
		 * requested and it does not violate the valid set of port
		 * configurations */
		if (pin_cfg & PORT_WRCONFIG_PULLEN) {
			/* Set the OUT register bits to enable the pull-up if requested,
			 * clear to enable pull-down */
			if (config->input_pull == SYSTEM_PINMUX_PIN_PULL_UP) {
				port->OUTSET.reg = pin_mask;
			} else {
				port->OUTCLR.reg = pin_mask;
			}
		}

		/* Check if the user has requested that the output buffer be enabled */
		if ((config->direction == SYSTEM_PINMUX_PIN_DIR_OUTPUT) ||
				(config->direction == SYSTEM_PINMUX_PIN_DIR_OUTPUT_WITH_READBACK)) {
			/* Set the port DIR bits to enable the output buffer */
			port->DIRSET.reg = pin_mask;
		}
	}
}

/**
 * \brief Writes a Port pin configuration to the hardware module.
 *
 * Writes out a given configuration of a Port pin configuration to the hardware
 * module.
 *
 * \note If the pin direction is set as an output, the pull-up/pull-down input
 *       configuration setting is ignored.
 *
 * \param[in] gpio_pin  Index of the GPIO pin to configure
 * \param[in] config    Configuration settings for the pin
 */
void system_pinmux_pin_set_config(
		const uint8_t gpio_pin,
		const struct system_pinmux_config *const config)
{
	PortGroup *const port = system_pinmux_get_group_from_gpio_pin(gpio_pin);
	uint32_t pin_mask = (1UL << (gpio_pin % 32));

	_system_pinmux_config(port, pin_mask, config);
}

/**
 * \brief Writes a Port pin group configuration to the hardware module.
 *
 * Writes out a given configuration of a Port pin group configuration to the
 * hardware module.
 *
 * \note If the pin direction is set as an output, the pull-up/pull-down input
 *       configuration setting is ignored.
 *
 * \param[in] port      Base of the PORT module to configure
 * \param[in] mask      Mask of the port pin(s) to configure
 * \param[in] config    Configuration settings for the pin
 */
void system_pinmux_group_set_config(
		PortGroup *const port,
		const uint32_t mask,
		const struct system_pinmux_config *const config)
{
	Assert(port);

	for (int i = 0; i < 32; i++) {
		if (mask & (1UL << i)) {
			_system_pinmux_config(port, (1UL << i), config);
		}
	}
}

/**
 * \brief Configures the input sampling mode for a group of pins.
 *
 * Configures the input sampling mode for a group of pins, to
 * control when the physical I/O pin value is sampled and
 * stored inside the microcontroller.
 *
 * \param[in] port     Base of the PORT module to configure
 * \param[in] mask     Mask of the port pin(s) to configure
 * \param[in] mode     New pin sampling mode to configure
 */
void system_pinmux_group_set_input_sample_mode(
		PortGroup *const port,
		const uint32_t mask,
		const enum system_pinmux_pin_sample mode)
{
	Assert(port);

	if (mode == SYSTEM_PINMUX_PIN_SAMPLE_ONDEMAND) {
		port->CTRL.reg |= mask;
	} else {
		port->CTRL.reg &= ~mask;
	}
}

#ifdef FEATURE_SYSTEM_PINMUX_SLEWRATE_LIMITER
/**
 * \brief Configures the output slew rate mode for a group of pins.
 *
 * Configures the output slew rate mode for a group of pins, to
 * control the speed at which the physical output pin can react to
 * logical changes of the I/O pin value.
 *
 * \param[in] port     Base of the PORT module to configure
 * \param[in] mask     Mask of the port pin(s) to configure
 * \param[in] mode     New pin slew rate mode to configure
 */
void system_pinmux_group_set_output_slew_rate(
		PortGroup *const port,
		const uint32_t mask,
		const enum system_pinmux_pin_slew_rate mode)
{
	Assert(port);

	for (int i = 0; i < 32; i++) {
		if (mask & (1UL << i)) {
			if (mode == SYSTEM_PINMUX_PIN_SLEW_RATE_LIMITED) {
				port->PINCFG[i].reg |=  PORT_PINCFG_SLEWLIM;
			} else {
				port->PINCFG[i].reg &= ~PORT_PINCFG_SLEWLIM;
			}
		}
	}
}
#endif

#ifdef FEATURE_SYSTEM_PINMUX_DRIVE_STRENGTH
/**
 * \brief Configures the output driver strength mode for a group of pins.
 *
 * Configures the output drive strength for a group of pins, to
 * control the amount of current the pad is able to sink/source.
 *
 * \param[in] port     Base of the PORT module to configure
 * \param[in] mask     Mask of the port pin(s) to configure
 * \param[in] mode     New output driver strength mode to configure
 */
void system_pinmux_group_set_output_strength(
		PortGroup *const port,
		const uint32_t mask,
		const enum system_pinmux_pin_strength mode)
{
	Assert(port);

	for (int i = 0; i < 32; i++) {
		if (mask & (1UL << i)) {
			if (mode == SYSTEM_PINMUX_PIN_STRENGTH_HIGH) {
				port->PINCFG[i].reg |=  PORT_PINCFG_DRVSTR;
			} else {
				port->PINCFG[i].reg &= ~PORT_PINCFG_DRVSTR;
			}
		}
	}
}
#endif

#ifdef FEATURE_SYSTEM_PINMUX_OPEN_DRAIN
/**
 * \brief Configures the output driver mode for a group of pins.
 *
 * Configures the output driver mode for a group of pins, to
 * control the pad behavior.
 *
 * \param[in] port Base of the PORT module to configure
 * \param[in] mask Mask of the port pin(s) to configure
 * \param[in] mode New pad output driver mode to configure
 */
void system_pinmux_group_set_output_drive(
		PortGroup *const port,
		const uint32_t mask,
		const enum system_pinmux_pin_drive mode)
{
	Assert(port);

	for (int i = 0; i < 32; i++) {
		if (mask & (1UL << i)) {
			if (mode == SYSTEM_PINMUX_PIN_DRIVE_OPEN_DRAIN) {
				port->PINCFG[i].reg |= PORT_PINCFG_ODRAIN;
			} else {
				port->PINCFG[i].reg &= ~PORT_PINCFG_ODRAIN;
			}
		}
	}
}
#endif
