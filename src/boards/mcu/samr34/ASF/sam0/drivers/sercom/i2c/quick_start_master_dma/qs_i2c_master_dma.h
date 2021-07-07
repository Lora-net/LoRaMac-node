/**
 * \file
 *
 * \brief SAM SERCOM I2C Master Driver with DMA Quick Start Guide
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
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
 * \page asfdoc_sam0_sercom_i2c_master_dma_use_case Quick Start Guide for Using DMA with SERCOM I2C Master
 *
 * The supported board list:
 *    - SAMD21 Xplained Pro
 *    - SAMR21 Xplained Pro
 *    - SAML21 Xplained Pro
 *    - SAML22 Xplained Pro
 *    - SAMDA1 Xplained Pro
 *    - SAMC21 Xplained Pro
 *    - SAMHA1G16A Xplained Pro
 *
 * In this use case, the I<SUP>2</SUP>C will used and set up as follows:
 *  - Master mode
 *  - 100KHz operation speed
 *  - Not operational in standby
 *  - 10000 packet timeout value
 *  - 65535 unknown bus state timeout value
 *
 *
 * \section asfdoc_sam0_sercom_i2c_master_dma_use_case_prereq Prerequisites
 * The device must be connected to an I<SUP>2</SUP>C slave.
 *
 * \section asfdoc_sam0_sercom_i2c_master_dma_use_setup Setup
 *
 * \subsection asfdoc_sam0_sercom_i2c_master_dma_use_setup_code Code
 * The following must be added to the user application:
 *
 * - A sample buffer to send, number of entries to send and address of slave:
 * \snippet qs_i2c_master_dma.c packet_data
 *
 * Number of times to try to send packet if it fails:
 * \snippet qs_i2c_master_dma.c timeout
 *
 * - Globally accessible module structure:
 * \snippet qs_i2c_master_dma.c dev_i2c_inst
 *
 * - Function for setting up the module:
 * \snippet qs_i2c_master_dma.c initialize_i2c
 *
 * - Globally accessible DMA module structure:
 * \snippet qs_i2c_master_dma.c dma_resource
 *
 * - Globally transfer done flag:
 * \snippet qs_i2c_master_dma.c transfer_done_flag
 *
 * - Globally accessible DMA transfer descriptor:
 * \snippet qs_i2c_master_dma.c transfer_descriptor
 *
 * - Function for transfer done callback:
 * \snippet qs_i2c_master_dma.c transfer_done
 *
 * - Function for setting up the DMA resource:
 * \snippet qs_i2c_master_dma.c config_dma_resource
 *
 * - Function for setting up the DMA transfer descriptor:
 * \snippet qs_i2c_master_dma.c setup_dma_transfer_descriptor
 * - Add to user application \c main():
 * \snippet qs_i2c_master_dma.c init
 *
 * \subsection asfdoc_sam0_sercom_i2c_master_dma_use_setup_workflow Workflow
 * -# Configure and enable module:
 * \snippet qs_i2c_master_dma.c config_i2c
 *  -# Create and initialize configuration structure.
 *    \snippet qs_i2c_master_dma.c init_conf
 *  -# Change settings in the configuration.
 *    \snippet qs_i2c_master_dma.c conf_change
 *  -# Initialize the module with the set configurations.
 *    \snippet qs_i2c_master_dma.c init_module
 *  -# Enable the module.
 *    \snippet qs_i2c_master_dma.c enable_module
 *
 * -# Configure DMA
 *  -# Create a DMA resource configuration structure, which can be filled out to
 *    adjust the configuration of a single DMA transfer.
 *    \snippet qs_i2c_master_dma.c dma_setup_1
 *
 *  -# Initialize the DMA resource configuration struct with the module's
 *    default values.
 *    \snippet qs_i2c_master_dma.c dma_setup_2
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 *  -# Set extra configurations for the DMA resource. It is using peripheral
 *    trigger. SERCOM TX trigger causes a transaction transfer in
 *    this example.
 *    \snippet qs_i2c_master_dma.c dma_setup_3
 *
 *  -# Allocate a DMA resource with the configurations.
 *    \snippet qs_i2c_master_dma.c dma_setup_4
 *
 *  -# Create a DMA transfer descriptor configuration structure, which can be
 *    filled out to adjust the configuration of a single DMA transfer.
 *    \snippet qs_i2c_master_dma.c dma_setup_5
 *
 *  -# Initialize the DMA transfer descriptor configuration struct with the module's
 *    default values.
 *    \snippet qs_i2c_master_dma.c dma_setup_6
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 *  -# Set the specific parameters for a DMA transfer with transfer size, source
 *    address, and destination address.
 *    \snippet qs_i2c_master_dma.c dma_setup_7
 *
 *  -# Create the DMA transfer descriptor.
 *    \snippet qs_i2c_master_dma.c dma_setup_8
 *
 * \section asfdoc_sam0_sercom_i2c_master_dma_use_implemenation Implementation
 * \subsection asfdoc_sam0_sercom_i2c_master_dma_use_implemenation_code Code
 * Add to user application \c main():
 * \snippet qs_i2c_master_dma.c main
 *
 * \subsection asfdoc_sam0_sercom_i2c_master_dma_use_implemenation_workflow Workflow
 * -# Start the DMA transfer job.
 *    \snippet qs_i2c_master_dma.c start_transfer_job
 *
 * -# Set the auto address length and enable flag.
 *    \snippet qs_i2c_master_dma.c set_i2c_addr
 * 
 * -# Waiting for transfer complete.
 *    \snippet qs_i2c_master_dma.c waiting_for_complete
 *
 * -# Enter an infinite loop once transfer complete.
 *    \snippet qs_i2c_master_dma.c inf_loop
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
