
/**
 * \file
 *
 * \brief SAM Power manager
 *
 * Copyright (C) 2014-2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include <hpl_sleep.h>
#include <hpl_init.h>

/**
 * \brief Set the sleep mode for the device
 */
int32_t _set_sleep_mode(const uint8_t mode)
{
	switch (mode) {
	case 2:
	case 4:
	case 5:
	case 6:
		hri_pm_write_SLEEPCFG_SLEEPMODE_bf(PM, mode);
		break;
	default:
		return ERR_INVALID_ARG;
	}

	return ERR_NONE;
}

/**
 * \brief Set performance level
 */
void _set_performance_level(const uint8_t level)
{
	hri_pm_clear_INTFLAG_reg(PM, 0xFF);
	if (hri_pm_get_PLCFG_PLSEL_bf(PM, PM_PLCFG_PLSEL_Msk) != level) {
		hri_pm_write_PLCFG_PLSEL_bf(PM, level);
	}
	while (!hri_pm_read_INTFLAG_reg(PM))
		;
}
