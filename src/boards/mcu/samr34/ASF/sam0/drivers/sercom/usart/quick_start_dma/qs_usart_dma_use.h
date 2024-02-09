/**
 * \file
 *
 * \brief SAM Quick Start Guide for Using Usart driver with DMA
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
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

/**
 * \page asfdoc_sam0_sercom_usart_dma_use_case Quick Start Guide for Using DMA with SERCOM USART
 *
 * The supported board list:
 *    - SAM D21 Xplained Pro
 *    - SAM R21 Xplained Pro
 *    - SAM D11 Xplained Pro
 *    - SAM DA1 Xplained Pro
 *    - SAM HA1G16A Xplained Pro
 *    - SAM L21 Xplained Pro
 *    - SAM L22 Xplained Pro
 *    - SAM C21 Xplained Pro
 *
 * This quick start will receive eight bytes of data from the PC terminal and transmit back the string
 * to the terminal through DMA. In this use case the USART will be configured with the following
 * settings:
 * - Asynchronous mode
 * - 9600 Baudrate
 * - 8-bits, No Parity and one Stop Bit
 * - TX and RX enabled and connected to the Xplained Pro Embedded Debugger virtual COM port
 *
 * \section asfdoc_sam0_sercom_usart_dma_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_sercom_usart_dma_use_case_prereq Prerequisites
 * There are no special setup requirements for this use-case.
 *
 * \subsection asfdoc_sam0_usart_dma_use_case_setup_code Code
 * Add to the main application source file, outside of any functions:
 * \snippet qs_usart_dma_use.c module_inst
 * \snippet qs_usart_dma_use.c dma_resource
 * \snippet qs_usart_dma_use.c usart_buffer
 * \snippet qs_usart_dma_use.c transfer_descriptor
 *
 * Copy-paste the following setup code to your user application:
 * \snippet qs_usart_dma_use.c setup
 *
 * Add to user application initialization (typically the start of \c main()):
 * \snippet qs_usart_dma_use.c setup_init
 *
 * \subsection asfdoc_sam0_usart_dma_use_case_setup_flow Workflow
 *
 * \subsubsection asfdoc_sam0_usart_dma_use_case_setup_flow_inst Create variables
 * -# Create a module software instance structure for the USART module to store
 *    the USART driver state while it is in use.
 *    \snippet qs_usart_dma_use.c module_inst
 *    \note This should never go out of scope as long as the module is in use.
 *          In most cases, this should be global.
 *
 * -# Create module software instance structures for DMA resources to store
 *    the DMA resource state while it is in use.
 *    \snippet qs_usart_dma_use.c dma_resource
 *    \note This should never go out of scope as long as the module is in use.
 *          In most cases, this should be global.
 *
 * -# Create a buffer to store the data to be transferred /received.
 *    \snippet qs_usart_dma_use.c usart_buffer
 * -# Create DMA transfer descriptors for RX/TX.
 *    \snippet qs_usart_dma_use.c transfer_descriptor
 *
 * \subsubsection asfdoc_sam0_usart_dma_use_case_setup_flow_usart Configure the USART
 * -# Create a USART module configuration struct, which can be filled out to
 *    adjust the configuration of a physical USART peripheral.
 *    \snippet qs_usart_dma_use.c setup_config
 * -# Initialize the USART configuration struct with the module's default values.
 *    \snippet qs_usart_dma_use.c setup_config_defaults
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 * -# Alter the USART settings to configure the physical pinout, baudrate, and
 *    other relevant parameters.
 *    \snippet qs_usart_dma_use.c setup_change_config
 * -# Configure the USART module with the desired settings, retrying while the
 *    driver is busy until the configuration is stressfully set.
 *    \snippet qs_usart_dma_use.c setup_set_config
 * -# Enable the USART module.
 *    \snippet qs_usart_dma_use.c setup_enable
 *
 * \subsubsection asfdoc_sam0_usart_dma_use_case_setup_flow_dma Configure DMA
 * -# Create a callback function of receiver done.
 *    \snippet qs_usart_dma_use.c transfer_done_rx
 *
 * -# Create a callback function of transmission done.
 *    \snippet qs_usart_dma_use.c transfer_done_tx
 *
 * -# Create a DMA resource configuration structure, which can be filled out to
 *    adjust the configuration of a single DMA transfer.
 *    \snippet qs_usart_dma_use.c setup_rx_1
 *
 * -# Initialize the DMA resource configuration struct with the module's
 *    default values.
 *    \snippet qs_usart_dma_use.c setup_rx_2
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 * -# Set extra configurations for the DMA resource. It is using peripheral
 *    trigger. SERCOM TX empty trigger causes a beat transfer in
 *    this example.
 *    \snippet qs_usart_dma_use.c setup_rx_3
 *
 * -# Allocate a DMA resource with the configurations.
 *    \snippet qs_usart_dma_use.c setup_rx_4
 *
 * -# Create a DMA transfer descriptor configuration structure, which can be
 *    filled out to adjust the configuration of a single DMA transfer.
 *    \snippet qs_usart_dma_use.c setup_rx_5
 *
 * -# Initialize the DMA transfer descriptor configuration struct with the module's
 *    default values.
 *    \snippet qs_usart_dma_use.c setup_rx_6
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 * -# Set the specific parameters for a DMA transfer with transfer size, source
 *    address, and destination address.
 *    \snippet qs_usart_dma_use.c setup_rx_7
 *
 * -# Create the DMA transfer descriptor.
 *    \snippet qs_usart_dma_use.c setup_rx_8
 *
 * -# Create a DMA resource configuration structure for TX, which can be filled
 *    out to adjust the configuration of a single DMA transfer.
 *    \snippet qs_usart_dma_use.c setup_tx_1
 *
 * -# Initialize the DMA resource configuration struct with the module's
 *    default values.
 *    \snippet qs_usart_dma_use.c setup_tx_2
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 * -# Set extra configurations for the DMA resource. It is using peripheral
 *    trigger. SERCOM RX Ready trigger causes a beat transfer in
 *    this example.
 *    \snippet qs_usart_dma_use.c setup_tx_3
 *
 * -# Allocate a DMA resource with the configurations.
 *    \snippet qs_usart_dma_use.c setup_tx_4
 *
 * -# Create a DMA transfer descriptor configuration structure, which can be
 *    filled out to adjust the configuration of a single DMA transfer.
 *    \snippet qs_usart_dma_use.c setup_tx_5
 *
 * -# Initialize the DMA transfer descriptor configuration struct with the module's
 *    default values.
 *    \snippet qs_usart_dma_use.c setup_tx_6
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 * -# Set the specific parameters for a DMA transfer with transfer size, source
 *    address, and destination address.
 *    \snippet qs_usart_dma_use.c setup_tx_7
 *
 * -# Create the DMA transfer descriptor.
 *    \snippet qs_usart_dma_use.c setup_tx_8
 *
 * \section asfdoc_sam0_usart_dma_use_case_main Use Case
 *
 * \subsection asfdoc_sam0_usart_dma_use_case_main_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_usart_dma_use.c main
 *
 * \subsection asfdoc_sam0_usart_dma_use_case_main_flow Workflow
 * -# Wait for receiving data.
 *    \snippet qs_usart_dma_use.c main_1
 *
 * -# Enter endless loop.
 *    \snippet qs_usart_dma_use.c endless_loop
 */
