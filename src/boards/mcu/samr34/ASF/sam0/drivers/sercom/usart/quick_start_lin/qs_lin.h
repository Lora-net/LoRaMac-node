/**
 * \file
 *
 * \brief SAM USART LIN Quick Start
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
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
 * \page asfdoc_sam0_sercom_usart_lin_use_case Quick Start Guide for SERCOM USART LIN
 *
 * The supported board list:
 *    - SAMC21 Xplained Pro
 *
 * This quick start will set up LIN frame format transmission according to your
 * configuration \c CONF_LIN_NODE_TYPE.
 * For LIN master, it will send LIN command after startup.
 * For LIN salve, once received a format from LIN master with ID \c LIN_ID_FIELD_VALUE,
 * it will reply four data bytes plus a checksum.
 *
 * \section asfdoc_sam0_sercom_usart_lin_use_case_setup Setup
 *
 * \subsection asfdoc_sam0_sercom_usart_lin_use_case_prereq Prerequisites
 * When verify data transmission between LIN master and slave, two boards are needed:
 * one is for LIN master and the other is for LIN slave.
 * connect LIN master LIN PIN with LIN slave LIN PIN.
 *
 * \subsection asfdoc_sam0_usart_lin_use_case_setup_code Code
 * Add to the main application source file, outside of any functions:
 * \snippet qs_lin.c module_var
 *
 * Copy-paste the following setup code to your user application:
 * \snippet qs_lin.c setup
 *
 * Add to user application initialization (typically the start of \c main()):
 * \snippet qs_lin.c setup_init
 *
 * \subsection asfdoc_sam0_usart_lin_use_case_setup_flow Workflow
 * -# Create USART CDC and LIN module software instance structure for the USART module to store
 *    the USART driver state while it is in use.
 *    \snippet qs_lin.c module_inst
 * -# Define LIN ID field for header format.
 *    \snippet qs_lin.c lin_id
 *    \note The ID \c LIN_ID_FIELD_VALUE is eight bits as [P1,P0,ID5...ID0], when it's 0x64, the
 * 		data field length is four bytes plus a checksum byte.
 *
 * -# Define LIN RX/TX buffer.
 *    \snippet qs_lin.c lin_buffer
 *    \note For \c tx_buffer and \c rx_buffer, the last byte is for checksum.
 *
 * -# Configure the USART CDC for output message.
 *     \snippet qs_lin.c CDC_setup
 *
 * -# Configure the USART LIN module.
 *     \snippet qs_lin.c lin_setup
 *    \note The LIN frame format can be configured as master or slave, refer to \c CONF_LIN_NODE_TYPE .
 *
 * \section asfdoc_sam0_usart_lin_use_case_main Use Case
 *
 * \subsection asfdoc_sam0_usart_lin_use_case_main_code Code
 * Copy-paste the following code to your user application:
 * \snippet qs_lin.c main_setup
 *
 * \subsection asfdoc_sam0_usart_lin_use_case_main_flow Workflow
 * -# Set up USART LIN module.
 *     \snippet qs_lin.c configure_lin
 * -# For LIN master, sending LIN command. For LIN slaver, start reading data .
 *     \snippet qs_lin.c lin_master_cmd
 */
