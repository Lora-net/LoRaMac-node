/**
 * \file sio2host.c
 *
 * \brief Handles Serial I/O  Functionalities For the Host Device
 *
 * Copyright (c) 2013-2020 Microchip Technology Inc. and its subsidiaries.
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
 */

/* === INCLUDES ============================================================ */

#include "asf.h"
#include "sio2host.h"
#include "conf_sio2host.h"

/* === TYPES =============================================================== */

/* === MACROS ============================================================== */

/* === PROTOTYPES ========================================================== */

/* === GLOBALS ========================================================== */
#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35 || (WLR089)
static struct usart_module host_uart_module;
#else
static usart_serial_options_t usart_serial_options = {
	.baudrate     = USART_HOST_BAUDRATE,
	.charlength   = USART_HOST_CHAR_LENGTH,
	.paritytype   = USART_HOST_PARITY,
	.stopbits     = USART_HOST_STOP_BITS
};
#endif

/**
 * Receive buffer
 * The buffer size is defined in sio2host.h
 */
static uint8_t serial_rx_buf[SERIAL_RX_BUF_SIZE_HOST];

/**
 * Receive buffer head
 */
static uint8_t serial_rx_buf_head;

/**
 * Receive buffer tail
 */
static uint8_t serial_rx_buf_tail;

/**
 * Number of bytes in receive buffer
 */
static uint8_t serial_rx_count;

/* === IMPLEMENTATION ====================================================== */

void sio2host_init(void)
{
#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35  || (WLR089)
	struct usart_config host_uart_config;
	/* Configure USART for unit test output */
	usart_get_config_defaults(&host_uart_config);
	host_uart_config.mux_setting = HOST_SERCOM_MUX_SETTING;

	host_uart_config.pinmux_pad0 = HOST_SERCOM_PINMUX_PAD0;
	host_uart_config.pinmux_pad1 = HOST_SERCOM_PINMUX_PAD1;
	host_uart_config.pinmux_pad2 = HOST_SERCOM_PINMUX_PAD2;
	host_uart_config.pinmux_pad3 = HOST_SERCOM_PINMUX_PAD3;
	host_uart_config.baudrate    = USART_HOST_BAUDRATE;
	stdio_serial_init(&host_uart_module, USART_HOST, &host_uart_config);
	usart_enable(&host_uart_module);
	/* Enable transceivers */
	usart_enable_transceiver(&host_uart_module, USART_TRANSCEIVER_TX);
	usart_enable_transceiver(&host_uart_module, USART_TRANSCEIVER_RX);
#else
	stdio_serial_init(USART_HOST, &usart_serial_options);
#endif
	USART_HOST_RX_ISR_ENABLE();
}
void sio2host_deinit(void)
{
#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35 || (WLR089)	
		usart_disable(&host_uart_module);
	
		/* Disable transceivers */
		usart_disable_transceiver(&host_uart_module, USART_TRANSCEIVER_TX);
		usart_disable_transceiver(&host_uart_module, USART_TRANSCEIVER_RX);
#endif	
}
uint8_t sio2host_tx(uint8_t *data, uint8_t length)
{
#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35 || (WLR089)
	status_code_genare_t status;
#else
	status_code_t status;
#endif /*SAMD || SAMR21 || SAML21 */

	do {
#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35 || (WLR089)
		status
			= usart_serial_write_packet(&host_uart_module,
				(const uint8_t *)data, length);
#elif SAM4S || SAM4E
        status = usart_serial_write_packet((Usart *)USART_HOST,
				(const uint8_t *)data,
				length);
#else
	    status = usart_serial_write_packet(USART_HOST,
				(const uint8_t *)data,
				length);
#endif
	} while (status != STATUS_OK);
	return length;
}

uint8_t sio2host_rx(uint8_t *data, uint8_t max_length)
{
	uint8_t data_received = 0;
	if(serial_rx_buf_tail >= serial_rx_buf_head)
	{
		serial_rx_count = serial_rx_buf_tail - serial_rx_buf_head;
	}
	else
	{
		serial_rx_count = serial_rx_buf_tail + (SERIAL_RX_BUF_SIZE_HOST - serial_rx_buf_head);
	}
	
	if (0 == serial_rx_count) {
		return 0;
	}

	if (SERIAL_RX_BUF_SIZE_HOST <= serial_rx_count) {
		/*
		 * Bytes between head and tail are overwritten by new data.
		 * The oldest data in buffer is the one to which the tail is
		 * pointing. So reading operation should start from the tail.
		 */
		serial_rx_buf_head = serial_rx_buf_tail;

		/*
		 * This is a buffer overflow case. But still only the number of
		 * bytes equivalent to
		 * full buffer size are useful.
		 */
		serial_rx_count = SERIAL_RX_BUF_SIZE_HOST;

		/* Bytes received is more than or equal to buffer. */
		if (SERIAL_RX_BUF_SIZE_HOST <= max_length) {
			/*
			 * Requested receive length (max_length) is more than
			 * the
			 * max size of receive buffer, but at max the full
			 * buffer can be read.
			 */
			max_length = SERIAL_RX_BUF_SIZE_HOST;
		}
	} else {
		/* Bytes received is less than receive buffer maximum length. */
		if (max_length > serial_rx_count) {
			/*
			 * Requested receive length (max_length) is more than
			 * the data
			 * present in receive buffer. Hence only the number of
			 * bytes
			 * present in receive buffer are read.
			 */
			max_length = serial_rx_count;
		}
	}

	data_received = max_length;
	while (max_length > 0) {
		/* Start to copy from head. */
		*data = serial_rx_buf[serial_rx_buf_head];
		data++;
		max_length--;
		if ((SERIAL_RX_BUF_SIZE_HOST - 1) == serial_rx_buf_head) {
			serial_rx_buf_head = 0;
		}
		else
		{
			serial_rx_buf_head++;
		}
	}
	return data_received;
}

uint8_t sio2host_getchar(void)
{
	uint8_t c;
	while (0 == sio2host_rx(&c, 1)) {
	}
	return c;
}

void sio2host_putchar(uint8_t ch)
{
	sio2host_tx(&ch, 1);
}

int sio2host_getchar_nowait(void)
{
	uint8_t c;
	int back = sio2host_rx(&c, 1);
	if (back >= 1) {
		return c;
	} else {
		return (-1);
	}
}

#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35 || WLR089
void USART_HOST_ISR_VECT(uint8_t instance)
#else
USART_HOST_ISR_VECT()
#endif
{
	uint8_t temp;
#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35 || WLR089
	usart_serial_read_packet(&host_uart_module, &temp, 1);
#elif SAM4E || SAM4S
	usart_serial_read_packet((Usart *)USART_HOST, &temp, 1);
#else
    usart_serial_read_packet(USART_HOST, &temp, 1);
#endif

	/* Introducing critical section to avoid buffer corruption. */
	cpu_irq_disable();

	/* The number of data in the receive buffer is incremented and the
	 * buffer is updated. */

	serial_rx_buf[serial_rx_buf_tail] = temp;

	if ((SERIAL_RX_BUF_SIZE_HOST - 1) == serial_rx_buf_tail) {
		/* Reached the end of buffer, revert back to beginning of
		 * buffer. */
		serial_rx_buf_tail = 0x00;
	} else {
		serial_rx_buf_tail++;
	}

	cpu_irq_enable();
}

void sio2host_disable(void)
{
#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35 || (WLR089) 
	usart_disable(&host_uart_module);
#endif
}

void sio2host_enable(void)
{
#if SAMD || SAMR21 || SAML21 || SAMR30 || SAMR34 || SAMR35 || (WLR089)
	usart_enable(&host_uart_module);
#endif
}
/* EOF */
