/**
 * \file
 *
 * \brief SAM SERCOM I2C Master Interface Driver
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
 * \page asfdoc_sam0_sercom_i2c_master_callback_use_case Quick Start Guide for SERCOM I2C Master - Callback
 *
 * In this use case, the I<SUP>2</SUP>C will used and set up as follows:
 *  - Master mode
 *  - 100KHz operation speed
 *  - Not operational in standby
 *  - 65535 unknown bus state timeout value
 *
 * \section asfdoc_sam0_sercom_i2c_master_callback_use_case_prereq Prerequisites
 * The device must be connected to an I<SUP>2</SUP>C slave.
 *
 * \section asfdoc_sam0_sercom_i2c_master_callback_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_sercom_i2c_master_callback_use_case_setup_code Code
 * The following must be added to the user application:
 *
 * A sample buffer to write from, a reversed buffer to write from and length of
 * buffers.
 * \snippet qs_i2c_master_callback.c packet_data
 *
 * Address of slave:
 * \snippet qs_i2c_master_callback.c address
 *
 * Globally accessible module structure:
 * \snippet qs_i2c_master_callback.c dev_inst
 *
 * Globally accessible packet:
 * \snippet qs_i2c_master_callback.c packet_glob
 *
 * Function for setting up module:
 * \snippet qs_i2c_master_callback.c initialize_i2c
 *
 * Callback function for write complete:
 * \snippet qs_i2c_master_callback.c callback_func
 *
 * Function for setting up the callback functionality of the driver:
 * \snippet qs_i2c_master_callback.c setup_callback
 *
 * Add to user application \c main():
 * \snippet qs_i2c_master_callback.c run_initialize_i2c
 *
 * \subsection asfdoc_sam0_sercom_i2c_master_callback_use_case_setup_workflow Workflow
 * -# Configure and enable module.
 *    \snippet qs_i2c_master_callback.c config
 *  -# Create and initialize configuration structure.
 *     \snippet qs_i2c_master_callback.c init_conf
 *  -# Change settings in the configuration.
 *     \snippet qs_i2c_master_callback.c conf_change
 *  -# Initialize the module with the set configurations.
 *     \snippet qs_i2c_master_callback.c init_module
 *  -# Enable the module.
 *     \snippet qs_i2c_master_callback.c enable_module
 * -# Configure callback functionality.
 *    \snippet qs_i2c_master_callback.c config_callback
 *  -# Register write complete callback.
 *     \snippet qs_i2c_master_callback.c callback_reg
 *  -# Enable write complete callback.
 *     \snippet qs_i2c_master_callback.c callback_en
 * -# Create a packet to send to slave.
 *    \snippet qs_i2c_master_callback.c write_packet
 *
 * \section asfdoc_sam0_sercom_i2c_master_callback_use_case_implementation Implementation
 * \subsection asfdoc_sam0_sercom_i2c_master_callback_use_case_code Code
 * Add to user application \c main():
 * \snippet qs_i2c_master_callback.c while
 * \subsection asfdoc_sam0_sercom_i2c_master_callback_use_case_implementation_workflow Workflow
 * -# Write packet to slave.
 *    \snippet qs_i2c_master_callback.c write_packet
 * -# Infinite while loop, while waiting for interaction with slave.
 *    \snippet qs_i2c_master_callback.c while
 *
 * \section asfdoc_sam0_sercom_i2c_master_callback_use_case_callback Callback
 * Each time a packet is sent, the callback function will be called.
 *
 * \subsection asfdoc_sam0_sercom_i2c_master_callback_use_case_callback_workflow Workflow
 * - Write complete callback:
 *  -# Send every other packet in reversed order.
 *     \snippet qs_i2c_master_callback.c revert_order
 *  -# Write new packet to slave.
 *     \snippet qs_i2c_master_callback.c write_packet
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <conf_clocks.h>

