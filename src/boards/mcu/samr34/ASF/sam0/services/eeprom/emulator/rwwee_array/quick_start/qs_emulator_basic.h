/**
 * \file
 *
 * \brief SAM RWW EEPROM Emulator Service Quick Start
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
 * \page asfdoc_sam0_rww_eeprom_basic_use_case Quick Start Guide for the Emulated RWW EEPROM Module - Basic Use Case
 *
 * In this use case, the RWW EEPROM emulator module is configured, and a sample page
 * is read and written. The first byte of the first RWW EEPROM page is toggled,
 * and a LED is turned ON or OFF to reflect the new state. Each time the device
 * is reset, the LED should toggle to a different state to indicate correct
 * non-volatile storage and retrieval.
 *
 *
 * \section asfdoc_sam0_rww_eeprom_basic_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_rww_eeprom_basic_use_case_setup_prereq Prerequisites
 * There are no special setup requirements for this use-case.
 *
 * \subsection asfdoc_sam0_rww_eeprom_basic_use_case_setup_code Code
 * Copy-paste the following setup code to your user application:
 * \snippet qs_emulator_basic.c setup
 *
 * Add to user application initialization (typically the start of \c main()):
 * \snippet qs_emulator_basic.c setup_init
 *
 * \subsection asfdoc_sam0_rww_eeprom_basic_use_case_setup_flow Workflow
 * -# Attempt to initialize the RWW EEPROM emulator service, storing the error code
 *    from the initialization function into a temporary variable.
 *    \snippet qs_emulator_basic.c init_eeprom_service
 * -# Check if the emulator service failed to initialize for any other reason;
 *    if so, assume the emulator physical memory is unformatted or corrupt and
 *    erase/re-try initialization.
 *    \snippet qs_emulator_basic.c check_re-init
 *
 * Config BOD to give an early warning to prevent data loss.
 * \snippet qs_emulator_basic.c setup_bod
 *
 * \section asfdoc_sam0_rww_eeprom_basic_use_case_main Use Case
 *
 * \subsection asfdoc_sam0_rww_eeprom_basic_use_case_main_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_emulator_basic.c main
 *
 * \subsection asfdoc_sam0_rww_eeprom_basic_use_case_main_flow Workflow
 * -# Create a buffer to hold a single emulated RWW EEPROM page of memory, and read
 *    out logical RWW EEPROM page zero into it.
 *    \snippet qs_emulator_basic.c read_page
 * -# Toggle the first byte of the read page.
 *    \snippet qs_emulator_basic.c toggle_first_byte
 * -# Output the toggled LED state onto the board LED.
 *    \snippet qs_emulator_basic.c set_led
 * -# Write the modified page back to logical RWW EEPROM page zero, flushing the
 *    internal emulator write cache afterwards to ensure it is immediately
 *    written to physical non-volatile memory.
 *    \snippet qs_emulator_basic.c write_page
 * -# Modify data and write back to logical EEPROM page zero.
 *    The data is not committed and should call \c rww_eeprom_emulator_commit_page_buffer
 *    to ensure that any outstanding cache data is fully written to prevent data loss
 *    when detecting a BOD early warning.
 *    \snippet qs_emulator_basic.c write_page_not_commit
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
