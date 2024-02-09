/**
 * \file
 *
 * \brief SAM Serial Peripheral Interface Driver
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
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include "sercom.h"

#define SHIFT 32
#define BAUD_INT_MAX   8192
#define BAUD_FP_MAX     8

#if !defined(__DOXYGEN__)
/**
 * \internal Configuration structure to save current gclk status.
 */
struct _sercom_conf {
	/* Status of gclk generator initialization */
	bool generator_is_set;
	/* Sercom gclk generator used */
	enum gclk_generator generator_source;
};

static struct _sercom_conf _sercom_config;


/**
 * \internal Calculate 64 bit division, ref can be found in
 * http://en.wikipedia.org/wiki/Division_algorithm#Long_division
 */
static uint64_t long_division(uint64_t n, uint64_t d)
{
	int32_t i;
	uint64_t q = 0, r = 0, bit_shift;
	for (i = 63; i >= 0; i--) {
		bit_shift = (uint64_t)1 << i;

		r = r << 1;

		if (n & bit_shift) {
			r |= 0x01;
		}

		if (r >= d) {
			r = r - d;
			q |= bit_shift;
		}
	}

	return q;
}

/**
 * \internal Calculate synchronous baudrate value (SPI/UART)
 */
enum status_code _sercom_get_sync_baud_val(
		const uint32_t baudrate,
		const uint32_t external_clock,
		uint16_t *const baudvalue)
{
	/* Baud value variable */
	uint16_t baud_calculated = 0;
	uint32_t clock_value = external_clock;


	/* Check if baudrate is outside of valid range */
	if (baudrate > (external_clock / 2)) {
		/* Return with error code */
		return STATUS_ERR_BAUDRATE_UNAVAILABLE;
	}

	/* Calculate BAUD value from clock frequency and baudrate */
	clock_value = external_clock / 2;
	while (clock_value >= baudrate) {
		clock_value = clock_value - baudrate;
		baud_calculated++;
	}
	baud_calculated = baud_calculated - 1;

	/* Check if BAUD value is more than 255, which is maximum
	 * for synchronous mode */
	if (baud_calculated > 0xFF) {
		/* Return with an error code */
		return STATUS_ERR_BAUDRATE_UNAVAILABLE;
	} else {
		*baudvalue = baud_calculated;
		return STATUS_OK;
	}
}

/**
 * \internal Calculate asynchronous baudrate value (UART)
*/
enum status_code _sercom_get_async_baud_val(
		const uint32_t baudrate,
		const uint32_t peripheral_clock,
		uint16_t *const baudval,
		enum sercom_asynchronous_operation_mode mode,
		enum sercom_asynchronous_sample_num sample_num)
{
	/* Temporary variables  */
	uint64_t ratio = 0;
	uint64_t scale = 0;
	uint64_t baud_calculated = 0;
	uint8_t baud_fp;
	uint32_t baud_int = 0;
	uint64_t temp1;

	/* Check if the baudrate is outside of valid range */
	if ((baudrate * sample_num) > peripheral_clock) {
		/* Return with error code */
		return STATUS_ERR_BAUDRATE_UNAVAILABLE;
	}

	if(mode == SERCOM_ASYNC_OPERATION_MODE_ARITHMETIC) {
		/* Calculate the BAUD value */
		temp1 = ((sample_num * (uint64_t)baudrate) << SHIFT);
		ratio = long_division(temp1, peripheral_clock);
		scale = ((uint64_t)1 << SHIFT) - ratio;
		baud_calculated = (65536 * scale) >> SHIFT;
	} else if(mode == SERCOM_ASYNC_OPERATION_MODE_FRACTIONAL) {
		temp1 = ((uint64_t)baudrate * sample_num);
		baud_int = long_division( peripheral_clock, temp1);
		if(baud_int > BAUD_INT_MAX) {
				return STATUS_ERR_BAUDRATE_UNAVAILABLE;
		}
		temp1 = long_division( 8 * (uint64_t)peripheral_clock, temp1);
		baud_fp = temp1 - 8 * baud_int;
		baud_calculated = baud_int | (baud_fp << 13);
	}

	*baudval = baud_calculated;
	return STATUS_OK;
}
#endif

/**
 * \brief Set GCLK channel to generator.
 *
 * This will set the appropriate GCLK channel to the requested GCLK generator.
 * This will set the generator for all SERCOM instances, and the user will thus
 * only be able to set the same generator that has previously been set, if any.
 *
 * After the generator has been set the first time, the generator can be changed
 * using the \c force_change flag.
 *
 * \param[in]  generator_source The generator to use for SERCOM.
 * \param[in]  force_change     Force change the generator.
 *
 * \return Status code indicating the GCLK generator change operation.
 * \retval STATUS_OK                       If the generator update request was
 *                                         successful.
 * \retval STATUS_ERR_ALREADY_INITIALIZED  If a generator was already configured
 *                                         and the new configuration was not
 *                                         forced.
 */
enum status_code sercom_set_gclk_generator(
		const enum gclk_generator generator_source,
		const bool force_change)
{
	/* Check if valid option */
	if (!_sercom_config.generator_is_set || force_change) {
		/* Create and fill a GCLK configuration structure for the new config */
		struct system_gclk_chan_config gclk_chan_conf;
		system_gclk_chan_get_config_defaults(&gclk_chan_conf);
		gclk_chan_conf.source_generator = generator_source;
		system_gclk_chan_set_config(SERCOM_GCLK_ID, &gclk_chan_conf);
		system_gclk_chan_enable(SERCOM_GCLK_ID);

		/* Save config */
		_sercom_config.generator_source = generator_source;
		_sercom_config.generator_is_set = true;

		return STATUS_OK;
	} else if (generator_source == _sercom_config.generator_source) {
		/* Return status OK if same config */
		return STATUS_OK;
	}

	/* Return invalid config to already initialized GCLK */
	return STATUS_ERR_ALREADY_INITIALIZED;
}

/** \internal
 * Creates a switch statement case entry to convert a SERCOM instance and pad
 * index to the default SERCOM pad MUX setting.
 */
#define _SERCOM_PAD_DEFAULTS_CASE(n, pad) \
		case (uintptr_t)SERCOM##n: \
			switch (pad) { \
				case 0: \
					return SERCOM##n##_PAD0_DEFAULT; \
				case 1: \
					return SERCOM##n##_PAD1_DEFAULT; \
				case 2: \
					return SERCOM##n##_PAD2_DEFAULT; \
				case 3: \
					return SERCOM##n##_PAD3_DEFAULT; \
			} \
			break;

/**
 * \internal Gets the default PAD pinout for a given SERCOM.
 *
 * Returns the pinmux settings for the given SERCOM and pad. This is used
 * for default configuration of pins.
 *
 * \param[in]  sercom_module   Pointer to the SERCOM module
 * \param[in]  pad             PAD to get default pinout for
 *
 * \returns The default pinmux for the given SERCOM instance and PAD
 *
 */
uint32_t _sercom_get_default_pad(
		Sercom *const sercom_module,
		const uint8_t pad)
{
	switch ((uintptr_t)sercom_module) {
		/* Auto-generate a lookup table for the default SERCOM pad defaults */
		MREPEAT(SERCOM_INST_NUM, _SERCOM_PAD_DEFAULTS_CASE, pad)
	}

	Assert(false);
	return 0;
}

/**
 * \internal
 * Find index of given instance.
 *
 * \param[in] sercom_instance  Instance pointer.
 *
 * \return Index of given instance.
 */
uint8_t _sercom_get_sercom_inst_index(
		Sercom *const sercom_instance)
{
	/* Save all available SERCOM instances for compare */
	Sercom *sercom_instances[SERCOM_INST_NUM] = SERCOM_INSTS;

	/* Find index for sercom instance */
	for (uint32_t i = 0; i < SERCOM_INST_NUM; i++) {
		if ((uintptr_t)sercom_instance == (uintptr_t)sercom_instances[i]) {
			return i;
		}
	}

	/* Invalid data given */
	Assert(false);
	return 0;
}
