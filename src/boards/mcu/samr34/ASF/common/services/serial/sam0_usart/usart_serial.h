/**
 * \file
 *
 * \brief USART Serial wrapper service for the SAM D/L/C/R devices.
 *
 * Copyright (c) 2009-2018 Microchip Technology Inc. and its subsidiaries.
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
#ifndef _USART_SERIAL_H_
#define _USART_SERIAL_H_

#include "compiler.h"
#include "status_codes.h"
#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \name Serial Management Configuration */

typedef Sercom * usart_inst_t;

//struct usart_module usart;

/*! \brief Initializes the Usart in serial mode.
 *
 * \param[in,out] module  Software instance of the USART to initialize.
 * \param[in]     hw      Base address of the hardware USART.
 * \param[in]     config  Configuration settings for the USART.
 *
 * \retval true if the initialization was successful
 * \retval false if initialization failed (error in baud rate calculation)
 */
static inline bool usart_serial_init(
		struct usart_module *const module,
		usart_inst_t const hw,
		const struct usart_config *const config)
{
	if (usart_init(module, hw, config) == STATUS_OK) {
		return true;
	}
	else {
		return false;
	}
}

/** \brief Sends a character with the USART.
 *
 * \param[in,out] module  Software instance of the USART.
 * \param[in]     c       Character to write.
 *
 * \return Status code
 */
static inline enum status_code usart_serial_putchar(
		struct usart_module *const module,
		uint8_t c)
{
	while(STATUS_OK !=usart_write_wait(module, c));

	return STATUS_OK;
}

/** \brief Waits until a character is received, and returns it.
 *
 * \param[in,out] module  Software instance of the USART.
 * \param[out]    c       Destination for the read character.
 */
static inline void usart_serial_getchar(
		struct usart_module *const module,
		uint8_t *c)
{
	uint16_t temp = 0;

	while(STATUS_OK != usart_read_wait(module, &temp));

	*c = temp;
}

/**
 * \brief Send a sequence of bytes to USART device
 *
 * \param[in,out] module   Software instance of the USART.
 * \param[in]     tx_data  Data buffer to read the data to write from.
 * \param[in]     length   Length of data to write.
 */
static inline enum status_code usart_serial_write_packet(
		struct usart_module *const module,
		const uint8_t *tx_data,
		uint16_t length)
{
	return usart_write_buffer_wait(module, tx_data, length);
}

/**
 * \brief Receive a sequence of bytes from USART device
 *
 * \param[in,out] module   Software instance of the USART.
 * \param[out]    rx_data  Data buffer to store the read data into.
 * \param[in]     length   Length of data to read.
 */
static inline enum status_code usart_serial_read_packet(
		struct usart_module *const module,
		uint8_t *rx_data,
		uint16_t length)
{
	return usart_read_buffer_wait(module, rx_data, length);
}

#ifdef __cplusplus
}
#endif

#endif  // _USART_SERIAL_H_
