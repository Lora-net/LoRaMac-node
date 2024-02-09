/**
 * \file
 *
 * \brief SAM USART Quick Start
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
 * \page asfdoc_sam0_sercom_usart_callback_use_case Quick Start Guide for SERCOM USART - Callback
 *
 * This quick start will echo back characters typed into the terminal, using
 * asynchronous TX and RX callbacks from the USART peripheral. In this use case
 * the USART will be configured with the following settings:
 * - Asynchronous mode
 * - 9600 Baudrate
 * - 8-bits, No Parity and one Stop Bit
 * - TX and RX enabled and connected to the Xplained Pro Embedded Debugger virtual COM port
 *
 * \section asfdoc_sam0_sercom_usart_callback_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_sercom_usart_callback_use_case_prereq Prerequisites
 * There are no special setup requirements for this use-case.
 *
 * \subsection asfdoc_sam0_usart_callback_use_case_setup_code Code
 * Add to the main application source file, outside of any functions:
 * \snippet qs_usart_callback.c module_inst
 * \snippet qs_usart_callback.c rx_buffer_var
 *
 * Copy-paste the following callback function code to your user application:
 * \snippet qs_usart_callback.c callback_funcs
 *
 * Copy-paste the following setup code to your user application:
 * \snippet qs_usart_callback.c setup
 *
 * Add to user application initialization (typically the start of \c main()):
 * \snippet qs_usart_callback.c setup_init
 *
 * \subsection asfdoc_sam0_usart_callback_use_case_setup_flow Workflow
 * -# Create a module software instance structure for the USART module to store
 *    the USART driver state while it is in use.
 *    \snippet qs_usart_callback.c module_inst
 *    \note This should never go out of scope as long as the module is in use.
 *          In most cases, this should be global.
 *
 * -# Configure the USART module.
 *  -# Create a USART module configuration struct, which can be filled out to
 *     adjust the configuration of a physical USART peripheral.
 *     \snippet qs_usart_callback.c setup_config
 *  -# Initialize the USART configuration struct with the module's default values.
 *     \snippet qs_usart_callback.c setup_config_defaults
 *     \note This should always be performed before using the configuration
 *           struct to ensure that all values are initialized to known default
 *           settings.
 *
 *  -# Alter the USART settings to configure the physical pinout, baudrate, and
 *     other relevant parameters.
 *     \snippet qs_usart_callback.c setup_change_config
 *  -# Configure the USART module with the desired settings, retrying while the
 *     driver is busy until the configuration is stressfully set.
 *     \snippet qs_usart_callback.c setup_set_config
 *  -# Enable the USART module.
 *     \snippet qs_usart_callback.c setup_enable
 * -# Configure the USART callbacks.
 *  -# Register the TX and RX callback functions with the driver.
 *     \snippet qs_usart_callback.c setup_register_callbacks
 *  -# Enable the TX and RX callbacks so that they will be called by the driver
 *     when appropriate.
 *     \snippet qs_usart_callback.c setup_enable_callbacks
 *
 * \section asfdoc_sam0_usart_callback_use_case_main Use Case
 *
 * \subsection asfdoc_sam0_usart_callback_use_case_main_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_usart_callback.c main
 *
 * \subsection asfdoc_sam0_usart_callback_use_case_main_flow Workflow
 * -# Enable global interrupts, so that the callbacks can be fired.
 *    \snippet qs_usart_callback.c enable_global_interrupts
 * -# Send a string to the USART to show the demo is running, blocking until
 *    all characters have been sent.
 *    \snippet qs_usart_callback.c main_send_string
 * -# Enter an infinite loop to continuously echo received values on the USART.
 *    \snippet qs_usart_callback.c main_loop
 * -# Perform an asynchronous read of the USART, which will fire the registered
 *    callback when characters are received.
 *    \snippet qs_usart_callback.c main_read
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <conf_clocks.h>

