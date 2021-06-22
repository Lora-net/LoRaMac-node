/**
 * \file
 *
 * \brief SAM D21/R21/L21/L22/DA1/C21/R30 Quick Start Guide for Using SPI driver with DMA
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
 * \page asfdoc_sam0_sercom_spi_dma_use_case Quick Start Guide for Using DMA with SERCOM SPI
 *
 * The supported board list:
 *    - SAM D21 Xplained Pro
 *    - SAM R21 Xplained Pro
 *    - SAM L21 Xplained Pro
 *    - SAM L22 Xplained Pro
 *    - SAM DA1 Xplained Pro
 *    - SAM C21 Xplained Pro
 *    - SAM R30 Xplained Pro
 *
 * This quick start will transmit a buffer data from master to slave through DMA.
 * In this use case the SPI master will be configured with the following
 * settings on SAM Xplained Pro:
 * - Master Mode enabled
 * - MSB of the data is transmitted first
 * - Transfer mode 0
 * - SPI MUX Setting E
 * - 8-bit character size
 * - Not enabled in sleep mode
 * - Baudrate 100000
 * - GLCK generator 0
 *
 * The SPI slave will be configured with the following settings:
 * - Slave mode enabled
 * - Preloading of shift register enabled
 * - MSB of the data is transmitted first
 * - Transfer mode 0
 * - SPI MUX Setting E
 * - 8-bit character size
 * - Not enabled in sleep mode
 * - GLCK generator 0
 *
 * Note that the pinouts on other boards may different, see next sector for
 * details.
 *
 * \section asfdoc_sam0_sercom_spi_dma_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_sercom_spi_dma_use_case_prereq Prerequisites
 * The following connections has to be made using wires:
 * - SAM D21/DA1 Xplained Pro.
 *  - \b SS_0:  EXT1 PIN15 (PA05) <--> EXT2 PIN15 (PA17)
 *  - \b DO/DI: EXT1 PIN16 (PA06) <--> EXT2 PIN17 (PA16)
 *  - \b DI/DO: EXT1 PIN17 (PA04) <--> EXT2 PIN16 (PA18)
 *  - \b SCK:   EXT1 PIN18 (PA07) <--> EXT2 PIN18 (PA19)
 * - SAM R21 Xplained Pro.
 *  - \b SS_0:  EXT1 PIN15 (PB03) <--> EXT1 PIN10 (PA23)
 *  - \b DO/DI: EXT1 PIN16 (PB22) <--> EXT1 PIN9  (PA22)
 *  - \b DI/DO: EXT1 PIN17 (PB02) <--> EXT1 PIN7  (PA18)
 *  - \b SCK:   EXT1 PIN18 (PB23) <--> EXT1 PIN8  (PA19)
 * - SAM L21 Xplained Pro.
 *  - \b SS_0:  EXT1 PIN15 (PA05) <--> EXT1 PIN12 (PA09)
 *  - \b DO/DI: EXT1 PIN16 (PA06) <--> EXT1 PIN11 (PA08)
 *  - \b DI/DO: EXT1 PIN17 (PA04) <--> EXT2 PIN03 (PA10)
 *  - \b SCK:   EXT1 PIN18 (PA07) <--> EXT2 PIN04 (PA11)
 * - SAM L22 Xplained Pro.
 *  - \b SS_0:  EXT1 PIN15 (PB21) <--> EXT2 PIN15 (PA17)
 *  - \b DO/DI: EXT1 PIN16 (PB00) <--> EXT2 PIN17 (PA16)
 *  - \b DI/DO: EXT1 PIN17 (PB02) <--> EXT2 PIN16 (PA18)
 *  - \b SCK:   EXT1 PIN18 (PB01) <--> EXT2 PIN18 (PA19)
 * - SAM C21 Xplained Pro.
 *  - \b SS_0:  EXT1 PIN15 (PA17) <--> EXT2 PIN15 (PB03)
 *  - \b DO/DI: EXT1 PIN16 (PA18) <--> EXT2 PIN17 (PB02)
 *  - \b DI/DO: EXT1 PIN17 (PA16) <--> EXT2 PIN16 (PB00)
 *  - \b SCK:   EXT1 PIN18 (PA19) <--> EXT2 PIN18 (PB01)
 *
 * \subsection asfdoc_sam0_spi_dma_use_case_setup_code Code
 *
 * Add to the main application source file, before user definitions and
 * functions according to your board:
 *
 * For SAM D21 Xplained Pro:
 * \snippet samd21_xplained_pro/conf_quick_start.h definition_master
 * \snippet samd21_xplained_pro/conf_quick_start.h definition_slave
 * \snippet samd21_xplained_pro/conf_quick_start.h definition_peripheral_trigger
 * For SAM R21 Xplained Pro:
 * \snippet samr21_xplained_pro/conf_quick_start.h definition_master
 * \snippet samr21_xplained_pro/conf_quick_start.h definition_slave
 * \snippet samr21_xplained_pro/conf_quick_start.h definition_peripheral_trigger
 * For SAM L21 Xplained Pro:
 * \snippet saml21_xplained_pro/conf_quick_start.h definition_master
 * \snippet saml21_xplained_pro/conf_quick_start.h definition_slave
 * \snippet saml21_xplained_pro/conf_quick_start.h definition_peripheral_trigger
 * For SAM L22 Xplained Pro:
 * \snippet saml22_xplained_pro/conf_quick_start.h definition_master
 * \snippet saml22_xplained_pro/conf_quick_start.h definition_slave
 * \snippet saml22_xplained_pro/conf_quick_start.h definition_peripheral_trigger
 * For SAM DA1 Xplained Pro:
 * \snippet samda1_xplained_pro/conf_quick_start.h definition_master
 * \snippet samda1_xplained_pro/conf_quick_start.h definition_slave
 * \snippet samda1_xplained_pro/conf_quick_start.h definition_peripheral_trigger
 * For SAM C21 Xplained Pro:
 * \snippet samc21_xplained_pro/conf_quick_start.h definition_master
 * \snippet samc21_xplained_pro/conf_quick_start.h definition_slave
 * \snippet samc21_xplained_pro/conf_quick_start.h definition_peripheral_trigger
 *
 * Add to the main application source file, outside of any functions:
 * \snippet qs_spi_dma_use.c buf_length
 * \snippet qs_spi_dma_use.c spi_baudrate
 * \snippet qs_spi_dma_use.c slave_select_pin
 * \snippet qs_spi_dma_use.c spi_buffer
 * \snippet qs_spi_dma_use.c spi_module_inst
 * \snippet qs_spi_dma_use.c dma_transfer_done_flag
 * \snippet qs_spi_dma_use.c slave_dev_inst
 * \snippet qs_spi_dma_use.c dma_transfer_descriptor
 *
 * Copy-paste the following setup code to your user application:
 * \snippet qs_spi_dma_use.c setup
 *
 * Add to user application initialization (typically the start of \c main()):
 * \snippet qs_spi_dma_use.c setup_init
 *
 * \subsection asfdoc_sam0_spi_dma_use_case_setup_flow Workflow
 * -# Create a module software instance structure for the SPI module to store
 *    the SPI driver state while it is in use.
 *    \snippet qs_spi_dma_use.c spi_module_inst
 *    \note This should never go out of scope as long as the module is in use.
 *          In most cases, this should be global.
 *
 * -# Create a module software instance structure for DMA resource to store
 *    the DMA resource state while it is in use.
 *    \snippet qs_spi_dma_use.c dma_resource
 *    \note This should never go out of scope as long as the module is in use.
 *          In most cases, this should be global.
 *
 * -# Create transfer done flag to indication DMA transfer done.
 *    \snippet qs_spi_dma_use.c dma_transfer_done_flag
 * -# Define the buffer length for TX/RX.
 *    \snippet qs_spi_dma_use.c buf_length
 * -# Create buffer to store the data to be transferred.
 *    \snippet qs_spi_dma_use.c spi_buffer
 * -# Create the SPI module configuration struct, which can be filled out to
 *     adjust the configuration of a physical SPI peripheral.
 *     \snippet qs_spi_dma_use.c spi_master_config
 *     \snippet qs_spi_dma_use.c spi_slave_config
 * -# Initialize the SPI configuration struct with the module's default values.
 *     \snippet qs_spi_dma_use.c spi_master_conf_defaults
 *     \snippet qs_spi_dma_use.c spi_slave_conf_defaults
 *     \note This should always be performed before using the configuration
 *           struct to ensure that all values are initialized to known default
 *           settings.
 *
 * -# Alter the SPI settings to configure the physical pinout, baudrate, and
 *     other relevant parameters.
 *     \snippet qs_spi_dma_use.c spi_master_mux_setting
 *     \snippet qs_spi_dma_use.c spi_slave_mux_setting
 * -# Configure the SPI module with the desired settings, retrying while the
 *     driver is busy until the configuration is stressfully set.
 *     \snippet qs_spi_dma_use.c spi_master_init
 *     \snippet qs_spi_dma_use.c spi_slave_init
 * -# Enable the SPI module.
 *     \snippet qs_spi_dma_use.c spi_master_enable
 *     \snippet qs_spi_dma_use.c spi_slave_enable
 *
 * -# Create the DMA resource configuration structure, which can be filled out to
 *    adjust the configuration of a single DMA transfer.
 *    \snippet qs_spi_dma_use.c dma_tx_setup_1
 *    \snippet qs_spi_dma_use.c dma_rx_setup_1
 *
 * -# Initialize the DMA resource configuration struct with the module's
 *    default values.
 *    \snippet qs_spi_dma_use.c dma_tx_setup_2
 *    \snippet qs_spi_dma_use.c dma_rx_setup_2
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 * -# Set extra configurations for the DMA resource. It is using peripheral
 *    trigger. SERCOM TX empty and RX complete trigger causes a beat transfer in
 *    this example.
 *    \snippet qs_spi_dma_use.c dma_tx_setup_3
 *    \snippet qs_spi_dma_use.c dma_rx_setup_3
 *
 * -# Allocate a DMA resource with the configurations.
 *    \snippet qs_spi_dma_use.c dma_tx_setup_4
 *    \snippet qs_spi_dma_use.c dma_rx_setup_4
 *
 * -# Create a DMA transfer descriptor configuration structure, which can be
 *    filled out to adjust the configuration of a single DMA transfer.
 *    \snippet qs_spi_dma_use.c dma_tx_setup_5
 *    \snippet qs_spi_dma_use.c dma_rx_setup_5
 *
 * -# Initialize the DMA transfer descriptor configuration struct with the module's
 *    default values.
 *    \snippet qs_spi_dma_use.c dma_tx_setup_6
 *    \snippet qs_spi_dma_use.c dma_rx_setup_6
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 *
 * -# Set the specific parameters for a DMA transfer with transfer size, source
 *    address, and destination address.
 *    \snippet qs_spi_dma_use.c dma_tx_setup_7
 *    \snippet qs_spi_dma_use.c dma_rx_setup_7
 *
 * -# Create the DMA transfer descriptor.
 *    \snippet qs_spi_dma_use.c dma_tx_setup_8
 *    \snippet qs_spi_dma_use.c dma_rx_setup_8
 *
 * \section asfdoc_sam0_spi_dma_use_case_main Use Case
 *
 * \subsection asfdoc_sam0_spi_dma_use_case_main_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_spi_dma_use.c main
 *
 * \subsection asfdoc_sam0_spi_dma_use_case_main_flow Workflow
 * -# Select the slave.
 *    \snippet qs_spi_dma_use.c select_slave
 *
 * -# Start the transfer job.
 *    \snippet qs_spi_dma_use.c main_1
 *
 * -# Wait for transfer done.
 *    \snippet qs_spi_dma_use.c main_2
 *
 * -# Deselect the slave.
 *    \snippet qs_spi_dma_use.c deselect_slave
 *
 * -# Enter endless loop.
 *    \snippet qs_spi_dma_use.c endless_loop
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
