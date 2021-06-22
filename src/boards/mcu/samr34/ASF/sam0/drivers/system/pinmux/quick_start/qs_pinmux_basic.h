/**
 * \file
 *
 * \brief SAM PINMUX Driver Quick Start
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

/**
 * \page asfdoc_sam0_system_pinmux_basic_use_case Quick Start Guide for SYSTEM PINMUX - Basic
 *
 * In this use case, the PINMUX module is configured for:
 *  \li One pin in input mode, with pull-up enabled, connected to the GPIO
 *      module
 *  \li Sampling mode of the pin changed to sample on demand
 *
 * This use case sets up the PINMUX to configure a physical I/O pin set as
 * an input with pull-up and changes the sampling mode of the pin to reduce
 * power by only sampling the physical pin state when the user application
 * attempts to read it.
 *
 * \section asfdoc_sam0_system_pinmux_basic_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_system_pinmux_basic_use_case_setup_prereq Prerequisites
 * There are no special setup requirements for this use-case.
 *
 * \subsection asfdoc_sam0_system_pinmux_basic_use_case_setup_code Code
 * Copy-paste the following setup code to your application:
 * \snippet qs_pinmux_basic.c setup
 *
 * \subsection asfdoc_sam0_system_pinmux_basic_use_case_setup_flow Workflow
 * -# Create a PINMUX module pin configuration struct, which can be filled out
 *    to adjust the configuration of a single port pin.
 *    \snippet qs_pinmux_basic.c pinmux_config
 * -# Initialize the pin configuration struct with the module's default values.
 *    \snippet qs_pinmux_basic.c pinmux_config_defaults
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 * -# Adjust the configuration struct to request an input pin with pull-up
 *    connected to the GPIO peripheral.
 *  \snippet qs_pinmux_basic.c pinmux_update_config_values
 * -# Configure GPIO10 with the initialized pin configuration struct, to enable
 *    the input sampler on the pin.
 *    \snippet qs_pinmux_basic.c pinmux_set_config
 *
 * \section asfdoc_sam0_system_pinmux_basic_use_case_use_main Use Case
 *
 * \subsection asfdoc_sam0_system_pinmux_basic_use_case_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_pinmux_basic.c main
 *
 * \subsection asfdoc_sam0_system_pinmux_basic_use_case_flow Workflow

 * -# Adjust the configuration of the pin to enable on-demand sampling mode.
 *    \snippet qs_pinmux_basic.c pinmux_change_input_sampling
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
