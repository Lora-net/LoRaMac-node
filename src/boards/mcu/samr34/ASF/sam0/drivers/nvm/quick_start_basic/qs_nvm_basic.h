/**
 * \file
 *
 * \brief SAM Non Volatile Memory Driver Quick Start
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
 * \page asfdoc_sam0_nvm_basic_use_case Quick Start Guide for NVM - Basic
 *
 * In this use case, the NVM module is configured for:
 *  \li Power reduction mode enabled after sleep mode until first NVM access
 *  \li Automatic page write commands issued to commit data as pages are written
 *      to the internal buffer
 *  \li Zero wait states when reading FLASH memory
 *  \li No memory space for the EEPROM
 *  \li No protected bootloader section
 *
 * This use case sets up the NVM controller to write a page of data to flash,
 * and then read it back into the same buffer.
 *
 * \section asfdoc_sam0_nvm_basic_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_nvm_basic_use_case_setup_prereq Prerequisites
 * There are no special setup requirements for this use-case.
 *
 * \subsection asfdoc_sam0_nvm_basic_use_case_setup_code Code
 * Copy-paste the following setup code to your user application:
 * \snippet qs_nvm_basic.c setup
 *
 * Add to user application initialization (typically the start of \c main()):
 * \snippet qs_nvm_basic.c setup_init
 *
 * \subsection asfdoc_sam0_nvm_basic_use_case_setup_flow Workflow
 * -# Create an NVM module configuration struct, which can be filled
 *    out to adjust the configuration of the NVM controller.
 *    \snippet qs_nvm_basic.c setup_1
 * -# Initialize the NVM configuration struct with the module's default
 *    values.
 *    \snippet qs_nvm_basic.c setup_2
 *    \note This should always be performed before using the configuration
 *          struct to ensure that all values are initialized to known default
 *          settings.
 * -# Enable automatic page write mode. The new data will be written to NVM automaticly.
 *    \snippet qs_nvm_basic.c setup_3
 *    \note If automatic page write mode is disabled, the data will not write to NVM
 *          until the NVM write command has been invoked. For safe use of the NVM module,
 *          disable automatic page write mode and use write command to commit data is
 *          recommended.
 *
 * -# Configure NVM controller with the created configuration struct settings.
 *    \snippet qs_nvm_basic.c setup_4
 *
 * \section asfdoc_sam0_nvm_basic_use_case_main Use Case
 *
 * \subsection asfdoc_sam0_nvm_basic_use_case_main_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_nvm_basic.c main
 *
 * \subsection asfdoc_sam0_nvm_basic_use_case_main_flow Workflow
 * -# Set up a buffer, one NVM page in size, to hold data to read or write into
 *    NVM memory.
 *    \snippet qs_nvm_basic.c main_1
 * -# Fill the buffer with a pattern of data.
 *    \snippet qs_nvm_basic.c main_2
 * -# Create a variable to hold the error status from the called NVM functions.
 *    \snippet qs_nvm_basic.c main_3
 * -# Erase a page of NVM data. As the NVM could be busy initializing or
 *    completing a previous operation, a loop is used to retry the command while
 *    the NVM controller is busy.
 *    \snippet qs_nvm_basic.c main_4
 *    \note This must be performed before writing new data into an NVM page.
 *
 * -# Write the data buffer to the previously erased page of the NVM.
 *    \snippet qs_nvm_basic.c main_5
 *    \note The new data will be written to NVM memory automatically, as the
 *          NVM controller is configured in automatic page write mode.
 *
 * -# Read back the written page of page from the NVM into the buffer.
 *    \snippet qs_nvm_basic.c main_6
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

