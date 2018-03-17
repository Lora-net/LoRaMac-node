/**
 * \file
 *
 * \brief SAM ADC
 *
 * Copyright (C) 2016 Atmel Corporation. All rights reserved.
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
 */

#ifdef _SAML21_ADC_COMPONENT_
#ifndef _HRI_ADC_L21_H_INCLUDED_
#define _HRI_ADC_L21_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <hal_atomic.h>

#if defined(ENABLE_ADC_CRITICAL_SECTIONS)
#define ADC_CRITICAL_SECTION_ENTER() CRITICAL_SECTION_ENTER()
#define ADC_CRITICAL_SECTION_LEAVE() CRITICAL_SECTION_LEAVE()
#else
#define ADC_CRITICAL_SECTION_ENTER()
#define ADC_CRITICAL_SECTION_LEAVE()
#endif

typedef uint16_t hri_adc_calib_reg_t;
typedef uint16_t hri_adc_ctrlc_reg_t;
typedef uint16_t hri_adc_gaincorr_reg_t;
typedef uint16_t hri_adc_inputctrl_reg_t;
typedef uint16_t hri_adc_offsetcorr_reg_t;
typedef uint16_t hri_adc_result_reg_t;
typedef uint16_t hri_adc_syncbusy_reg_t;
typedef uint16_t hri_adc_winlt_reg_t;
typedef uint16_t hri_adc_winut_reg_t;
typedef uint32_t hri_adc_seqctrl_reg_t;
typedef uint8_t  hri_adc_avgctrl_reg_t;
typedef uint8_t  hri_adc_ctrla_reg_t;
typedef uint8_t  hri_adc_ctrlb_reg_t;
typedef uint8_t  hri_adc_dbgctrl_reg_t;
typedef uint8_t  hri_adc_evctrl_reg_t;
typedef uint8_t  hri_adc_intenset_reg_t;
typedef uint8_t  hri_adc_intflag_reg_t;
typedef uint8_t  hri_adc_refctrl_reg_t;
typedef uint8_t  hri_adc_sampctrl_reg_t;
typedef uint8_t  hri_adc_seqstatus_reg_t;
typedef uint8_t  hri_adc_swtrig_reg_t;

static inline void hri_adc_wait_for_sync(const void *const hw, hri_adc_syncbusy_reg_t reg)
{
	while (((Adc *)hw)->SYNCBUSY.reg & reg) {
	};
}

static inline bool hri_adc_is_syncing(const void *const hw, hri_adc_syncbusy_reg_t reg)
{
	return ((Adc *)hw)->SYNCBUSY.reg & reg;
}

static inline void hri_adc_set_INTEN_RESRDY_bit(const void *const hw)
{
	((Adc *)hw)->INTENSET.reg = ADC_INTENSET_RESRDY;
}

static inline bool hri_adc_get_INTEN_RESRDY_bit(const void *const hw)
{
	return (((Adc *)hw)->INTENSET.reg & ADC_INTENSET_RESRDY) >> ADC_INTENSET_RESRDY_Pos;
}

static inline void hri_adc_write_INTEN_RESRDY_bit(const void *const hw, bool value)
{
	if (value == 0x0) {
		((Adc *)hw)->INTENCLR.reg = ADC_INTENSET_RESRDY;
	} else {
		((Adc *)hw)->INTENSET.reg = ADC_INTENSET_RESRDY;
	}
}

static inline void hri_adc_clear_INTEN_RESRDY_bit(const void *const hw)
{
	((Adc *)hw)->INTENCLR.reg = ADC_INTENSET_RESRDY;
}

static inline void hri_adc_set_INTEN_OVERRUN_bit(const void *const hw)
{
	((Adc *)hw)->INTENSET.reg = ADC_INTENSET_OVERRUN;
}

static inline bool hri_adc_get_INTEN_OVERRUN_bit(const void *const hw)
{
	return (((Adc *)hw)->INTENSET.reg & ADC_INTENSET_OVERRUN) >> ADC_INTENSET_OVERRUN_Pos;
}

static inline void hri_adc_write_INTEN_OVERRUN_bit(const void *const hw, bool value)
{
	if (value == 0x0) {
		((Adc *)hw)->INTENCLR.reg = ADC_INTENSET_OVERRUN;
	} else {
		((Adc *)hw)->INTENSET.reg = ADC_INTENSET_OVERRUN;
	}
}

static inline void hri_adc_clear_INTEN_OVERRUN_bit(const void *const hw)
{
	((Adc *)hw)->INTENCLR.reg = ADC_INTENSET_OVERRUN;
}

static inline void hri_adc_set_INTEN_WINMON_bit(const void *const hw)
{
	((Adc *)hw)->INTENSET.reg = ADC_INTENSET_WINMON;
}

static inline bool hri_adc_get_INTEN_WINMON_bit(const void *const hw)
{
	return (((Adc *)hw)->INTENSET.reg & ADC_INTENSET_WINMON) >> ADC_INTENSET_WINMON_Pos;
}

static inline void hri_adc_write_INTEN_WINMON_bit(const void *const hw, bool value)
{
	if (value == 0x0) {
		((Adc *)hw)->INTENCLR.reg = ADC_INTENSET_WINMON;
	} else {
		((Adc *)hw)->INTENSET.reg = ADC_INTENSET_WINMON;
	}
}

static inline void hri_adc_clear_INTEN_WINMON_bit(const void *const hw)
{
	((Adc *)hw)->INTENCLR.reg = ADC_INTENSET_WINMON;
}

static inline void hri_adc_set_INTEN_reg(const void *const hw, hri_adc_intenset_reg_t mask)
{
	((Adc *)hw)->INTENSET.reg = mask;
}

static inline hri_adc_intenset_reg_t hri_adc_get_INTEN_reg(const void *const hw, hri_adc_intenset_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->INTENSET.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_adc_intenset_reg_t hri_adc_read_INTEN_reg(const void *const hw)
{
	return ((Adc *)hw)->INTENSET.reg;
}

static inline void hri_adc_write_INTEN_reg(const void *const hw, hri_adc_intenset_reg_t data)
{
	((Adc *)hw)->INTENSET.reg = data;
	((Adc *)hw)->INTENCLR.reg = ~data;
}

static inline void hri_adc_clear_INTEN_reg(const void *const hw, hri_adc_intenset_reg_t mask)
{
	((Adc *)hw)->INTENCLR.reg = mask;
}

static inline bool hri_adc_get_INTFLAG_RESRDY_bit(const void *const hw)
{
	return (((Adc *)hw)->INTFLAG.reg & ADC_INTFLAG_RESRDY) >> ADC_INTFLAG_RESRDY_Pos;
}

static inline void hri_adc_clear_INTFLAG_RESRDY_bit(const void *const hw)
{
	((Adc *)hw)->INTFLAG.reg = ADC_INTFLAG_RESRDY;
}

static inline bool hri_adc_get_INTFLAG_OVERRUN_bit(const void *const hw)
{
	return (((Adc *)hw)->INTFLAG.reg & ADC_INTFLAG_OVERRUN) >> ADC_INTFLAG_OVERRUN_Pos;
}

static inline void hri_adc_clear_INTFLAG_OVERRUN_bit(const void *const hw)
{
	((Adc *)hw)->INTFLAG.reg = ADC_INTFLAG_OVERRUN;
}

static inline bool hri_adc_get_INTFLAG_WINMON_bit(const void *const hw)
{
	return (((Adc *)hw)->INTFLAG.reg & ADC_INTFLAG_WINMON) >> ADC_INTFLAG_WINMON_Pos;
}

static inline void hri_adc_clear_INTFLAG_WINMON_bit(const void *const hw)
{
	((Adc *)hw)->INTFLAG.reg = ADC_INTFLAG_WINMON;
}

static inline bool hri_adc_get_interrupt_RESRDY_bit(const void *const hw)
{
	return (((Adc *)hw)->INTFLAG.reg & ADC_INTFLAG_RESRDY) >> ADC_INTFLAG_RESRDY_Pos;
}

static inline void hri_adc_clear_interrupt_RESRDY_bit(const void *const hw)
{
	((Adc *)hw)->INTFLAG.reg = ADC_INTFLAG_RESRDY;
}

static inline bool hri_adc_get_interrupt_OVERRUN_bit(const void *const hw)
{
	return (((Adc *)hw)->INTFLAG.reg & ADC_INTFLAG_OVERRUN) >> ADC_INTFLAG_OVERRUN_Pos;
}

static inline void hri_adc_clear_interrupt_OVERRUN_bit(const void *const hw)
{
	((Adc *)hw)->INTFLAG.reg = ADC_INTFLAG_OVERRUN;
}

static inline bool hri_adc_get_interrupt_WINMON_bit(const void *const hw)
{
	return (((Adc *)hw)->INTFLAG.reg & ADC_INTFLAG_WINMON) >> ADC_INTFLAG_WINMON_Pos;
}

static inline void hri_adc_clear_interrupt_WINMON_bit(const void *const hw)
{
	((Adc *)hw)->INTFLAG.reg = ADC_INTFLAG_WINMON;
}

static inline hri_adc_intflag_reg_t hri_adc_get_INTFLAG_reg(const void *const hw, hri_adc_intflag_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->INTFLAG.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_adc_intflag_reg_t hri_adc_read_INTFLAG_reg(const void *const hw)
{
	return ((Adc *)hw)->INTFLAG.reg;
}

static inline void hri_adc_clear_INTFLAG_reg(const void *const hw, hri_adc_intflag_reg_t mask)
{
	((Adc *)hw)->INTFLAG.reg = mask;
}

static inline void hri_adc_set_CTRLA_SWRST_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_SWRST);
	((Adc *)hw)->CTRLA.reg |= ADC_CTRLA_SWRST;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_CTRLA_SWRST_bit(const void *const hw)
{
	uint8_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_SWRST);
	tmp = ((Adc *)hw)->CTRLA.reg;
	tmp = (tmp & ADC_CTRLA_SWRST) >> ADC_CTRLA_SWRST_Pos;
	return (bool)tmp;
}

static inline void hri_adc_set_CTRLA_ENABLE_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE);
	((Adc *)hw)->CTRLA.reg |= ADC_CTRLA_ENABLE;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_CTRLA_ENABLE_bit(const void *const hw)
{
	uint8_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE);
	tmp = ((Adc *)hw)->CTRLA.reg;
	tmp = (tmp & ADC_CTRLA_ENABLE) >> ADC_CTRLA_ENABLE_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_CTRLA_ENABLE_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE);
	tmp = ((Adc *)hw)->CTRLA.reg;
	tmp &= ~ADC_CTRLA_ENABLE;
	tmp |= value << ADC_CTRLA_ENABLE_Pos;
	((Adc *)hw)->CTRLA.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLA_ENABLE_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE);
	((Adc *)hw)->CTRLA.reg &= ~ADC_CTRLA_ENABLE;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLA_ENABLE_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE);
	((Adc *)hw)->CTRLA.reg ^= ADC_CTRLA_ENABLE;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_CTRLA_RUNSTDBY_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLA.reg |= ADC_CTRLA_RUNSTDBY;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_CTRLA_RUNSTDBY_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->CTRLA.reg;
	tmp = (tmp & ADC_CTRLA_RUNSTDBY) >> ADC_CTRLA_RUNSTDBY_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_CTRLA_RUNSTDBY_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->CTRLA.reg;
	tmp &= ~ADC_CTRLA_RUNSTDBY;
	tmp |= value << ADC_CTRLA_RUNSTDBY_Pos;
	((Adc *)hw)->CTRLA.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLA_RUNSTDBY_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLA.reg &= ~ADC_CTRLA_RUNSTDBY;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLA_RUNSTDBY_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLA.reg ^= ADC_CTRLA_RUNSTDBY;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_CTRLA_ONDEMAND_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLA.reg |= ADC_CTRLA_ONDEMAND;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_CTRLA_ONDEMAND_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->CTRLA.reg;
	tmp = (tmp & ADC_CTRLA_ONDEMAND) >> ADC_CTRLA_ONDEMAND_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_CTRLA_ONDEMAND_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->CTRLA.reg;
	tmp &= ~ADC_CTRLA_ONDEMAND;
	tmp |= value << ADC_CTRLA_ONDEMAND_Pos;
	((Adc *)hw)->CTRLA.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLA_ONDEMAND_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLA.reg &= ~ADC_CTRLA_ONDEMAND;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLA_ONDEMAND_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLA.reg ^= ADC_CTRLA_ONDEMAND;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_CTRLA_reg(const void *const hw, hri_adc_ctrla_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLA.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrla_reg_t hri_adc_get_CTRLA_reg(const void *const hw, hri_adc_ctrla_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->CTRLA.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_CTRLA_reg(const void *const hw, hri_adc_ctrla_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLA.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLA_reg(const void *const hw, hri_adc_ctrla_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLA.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLA_reg(const void *const hw, hri_adc_ctrla_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLA.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrla_reg_t hri_adc_read_CTRLA_reg(const void *const hw)
{
	return ((Adc *)hw)->CTRLA.reg;
}

static inline void hri_adc_set_CTRLB_PRESCALER_bf(const void *const hw, hri_adc_ctrlb_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLB.reg |= ADC_CTRLB_PRESCALER(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlb_reg_t hri_adc_get_CTRLB_PRESCALER_bf(const void *const hw, hri_adc_ctrlb_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->CTRLB.reg;
	tmp = (tmp & ADC_CTRLB_PRESCALER(mask)) >> ADC_CTRLB_PRESCALER_Pos;
	return tmp;
}

static inline void hri_adc_write_CTRLB_PRESCALER_bf(const void *const hw, hri_adc_ctrlb_reg_t data)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->CTRLB.reg;
	tmp &= ~ADC_CTRLB_PRESCALER_Msk;
	tmp |= ADC_CTRLB_PRESCALER(data);
	((Adc *)hw)->CTRLB.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLB_PRESCALER_bf(const void *const hw, hri_adc_ctrlb_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLB.reg &= ~ADC_CTRLB_PRESCALER(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLB_PRESCALER_bf(const void *const hw, hri_adc_ctrlb_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLB.reg ^= ADC_CTRLB_PRESCALER(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlb_reg_t hri_adc_read_CTRLB_PRESCALER_bf(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->CTRLB.reg;
	tmp = (tmp & ADC_CTRLB_PRESCALER_Msk) >> ADC_CTRLB_PRESCALER_Pos;
	return tmp;
}

static inline void hri_adc_set_CTRLB_reg(const void *const hw, hri_adc_ctrlb_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLB.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlb_reg_t hri_adc_get_CTRLB_reg(const void *const hw, hri_adc_ctrlb_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->CTRLB.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_CTRLB_reg(const void *const hw, hri_adc_ctrlb_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLB.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLB_reg(const void *const hw, hri_adc_ctrlb_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLB.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLB_reg(const void *const hw, hri_adc_ctrlb_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLB.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlb_reg_t hri_adc_read_CTRLB_reg(const void *const hw)
{
	return ((Adc *)hw)->CTRLB.reg;
}

static inline void hri_adc_set_REFCTRL_REFCOMP_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg |= ADC_REFCTRL_REFCOMP;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_REFCTRL_REFCOMP_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->REFCTRL.reg;
	tmp = (tmp & ADC_REFCTRL_REFCOMP) >> ADC_REFCTRL_REFCOMP_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_REFCTRL_REFCOMP_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->REFCTRL.reg;
	tmp &= ~ADC_REFCTRL_REFCOMP;
	tmp |= value << ADC_REFCTRL_REFCOMP_Pos;
	((Adc *)hw)->REFCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_REFCTRL_REFCOMP_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg &= ~ADC_REFCTRL_REFCOMP;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_REFCTRL_REFCOMP_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg ^= ADC_REFCTRL_REFCOMP;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_REFCTRL_REFSEL_bf(const void *const hw, hri_adc_refctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg |= ADC_REFCTRL_REFSEL(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_refctrl_reg_t hri_adc_get_REFCTRL_REFSEL_bf(const void *const hw, hri_adc_refctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->REFCTRL.reg;
	tmp = (tmp & ADC_REFCTRL_REFSEL(mask)) >> ADC_REFCTRL_REFSEL_Pos;
	return tmp;
}

static inline void hri_adc_write_REFCTRL_REFSEL_bf(const void *const hw, hri_adc_refctrl_reg_t data)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->REFCTRL.reg;
	tmp &= ~ADC_REFCTRL_REFSEL_Msk;
	tmp |= ADC_REFCTRL_REFSEL(data);
	((Adc *)hw)->REFCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_REFCTRL_REFSEL_bf(const void *const hw, hri_adc_refctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg &= ~ADC_REFCTRL_REFSEL(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_REFCTRL_REFSEL_bf(const void *const hw, hri_adc_refctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg ^= ADC_REFCTRL_REFSEL(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_refctrl_reg_t hri_adc_read_REFCTRL_REFSEL_bf(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->REFCTRL.reg;
	tmp = (tmp & ADC_REFCTRL_REFSEL_Msk) >> ADC_REFCTRL_REFSEL_Pos;
	return tmp;
}

static inline void hri_adc_set_REFCTRL_reg(const void *const hw, hri_adc_refctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_refctrl_reg_t hri_adc_get_REFCTRL_reg(const void *const hw, hri_adc_refctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->REFCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_REFCTRL_reg(const void *const hw, hri_adc_refctrl_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_REFCTRL_reg(const void *const hw, hri_adc_refctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_REFCTRL_reg(const void *const hw, hri_adc_refctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->REFCTRL.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_refctrl_reg_t hri_adc_read_REFCTRL_reg(const void *const hw)
{
	return ((Adc *)hw)->REFCTRL.reg;
}

static inline void hri_adc_set_EVCTRL_FLUSHEI_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg |= ADC_EVCTRL_FLUSHEI;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_EVCTRL_FLUSHEI_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp = (tmp & ADC_EVCTRL_FLUSHEI) >> ADC_EVCTRL_FLUSHEI_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_EVCTRL_FLUSHEI_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp &= ~ADC_EVCTRL_FLUSHEI;
	tmp |= value << ADC_EVCTRL_FLUSHEI_Pos;
	((Adc *)hw)->EVCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_EVCTRL_FLUSHEI_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg &= ~ADC_EVCTRL_FLUSHEI;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_EVCTRL_FLUSHEI_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg ^= ADC_EVCTRL_FLUSHEI;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_EVCTRL_STARTEI_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg |= ADC_EVCTRL_STARTEI;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_EVCTRL_STARTEI_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp = (tmp & ADC_EVCTRL_STARTEI) >> ADC_EVCTRL_STARTEI_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_EVCTRL_STARTEI_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp &= ~ADC_EVCTRL_STARTEI;
	tmp |= value << ADC_EVCTRL_STARTEI_Pos;
	((Adc *)hw)->EVCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_EVCTRL_STARTEI_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg &= ~ADC_EVCTRL_STARTEI;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_EVCTRL_STARTEI_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg ^= ADC_EVCTRL_STARTEI;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_EVCTRL_FLUSHINV_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg |= ADC_EVCTRL_FLUSHINV;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_EVCTRL_FLUSHINV_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp = (tmp & ADC_EVCTRL_FLUSHINV) >> ADC_EVCTRL_FLUSHINV_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_EVCTRL_FLUSHINV_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp &= ~ADC_EVCTRL_FLUSHINV;
	tmp |= value << ADC_EVCTRL_FLUSHINV_Pos;
	((Adc *)hw)->EVCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_EVCTRL_FLUSHINV_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg &= ~ADC_EVCTRL_FLUSHINV;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_EVCTRL_FLUSHINV_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg ^= ADC_EVCTRL_FLUSHINV;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_EVCTRL_STARTINV_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg |= ADC_EVCTRL_STARTINV;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_EVCTRL_STARTINV_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp = (tmp & ADC_EVCTRL_STARTINV) >> ADC_EVCTRL_STARTINV_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_EVCTRL_STARTINV_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp &= ~ADC_EVCTRL_STARTINV;
	tmp |= value << ADC_EVCTRL_STARTINV_Pos;
	((Adc *)hw)->EVCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_EVCTRL_STARTINV_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg &= ~ADC_EVCTRL_STARTINV;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_EVCTRL_STARTINV_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg ^= ADC_EVCTRL_STARTINV;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_EVCTRL_RESRDYEO_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg |= ADC_EVCTRL_RESRDYEO;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_EVCTRL_RESRDYEO_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp = (tmp & ADC_EVCTRL_RESRDYEO) >> ADC_EVCTRL_RESRDYEO_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_EVCTRL_RESRDYEO_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp &= ~ADC_EVCTRL_RESRDYEO;
	tmp |= value << ADC_EVCTRL_RESRDYEO_Pos;
	((Adc *)hw)->EVCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_EVCTRL_RESRDYEO_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg &= ~ADC_EVCTRL_RESRDYEO;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_EVCTRL_RESRDYEO_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg ^= ADC_EVCTRL_RESRDYEO;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_EVCTRL_WINMONEO_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg |= ADC_EVCTRL_WINMONEO;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_EVCTRL_WINMONEO_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp = (tmp & ADC_EVCTRL_WINMONEO) >> ADC_EVCTRL_WINMONEO_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_EVCTRL_WINMONEO_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp &= ~ADC_EVCTRL_WINMONEO;
	tmp |= value << ADC_EVCTRL_WINMONEO_Pos;
	((Adc *)hw)->EVCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_EVCTRL_WINMONEO_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg &= ~ADC_EVCTRL_WINMONEO;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_EVCTRL_WINMONEO_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg ^= ADC_EVCTRL_WINMONEO;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_EVCTRL_reg(const void *const hw, hri_adc_evctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_evctrl_reg_t hri_adc_get_EVCTRL_reg(const void *const hw, hri_adc_evctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->EVCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_EVCTRL_reg(const void *const hw, hri_adc_evctrl_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_EVCTRL_reg(const void *const hw, hri_adc_evctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_EVCTRL_reg(const void *const hw, hri_adc_evctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->EVCTRL.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_evctrl_reg_t hri_adc_read_EVCTRL_reg(const void *const hw)
{
	return ((Adc *)hw)->EVCTRL.reg;
}

static inline void hri_adc_set_INPUTCTRL_MUXPOS_bf(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->INPUTCTRL.reg |= ADC_INPUTCTRL_MUXPOS(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_inputctrl_reg_t hri_adc_get_INPUTCTRL_MUXPOS_bf(const void *const       hw,
                                                                      hri_adc_inputctrl_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->INPUTCTRL.reg;
	tmp = (tmp & ADC_INPUTCTRL_MUXPOS(mask)) >> ADC_INPUTCTRL_MUXPOS_Pos;
	return tmp;
}

static inline void hri_adc_write_INPUTCTRL_MUXPOS_bf(const void *const hw, hri_adc_inputctrl_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->INPUTCTRL.reg;
	tmp &= ~ADC_INPUTCTRL_MUXPOS_Msk;
	tmp |= ADC_INPUTCTRL_MUXPOS(data);
	((Adc *)hw)->INPUTCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_INPUTCTRL_MUXPOS_bf(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->INPUTCTRL.reg &= ~ADC_INPUTCTRL_MUXPOS(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_INPUTCTRL_MUXPOS_bf(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->INPUTCTRL.reg ^= ADC_INPUTCTRL_MUXPOS(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_inputctrl_reg_t hri_adc_read_INPUTCTRL_MUXPOS_bf(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->INPUTCTRL.reg;
	tmp = (tmp & ADC_INPUTCTRL_MUXPOS_Msk) >> ADC_INPUTCTRL_MUXPOS_Pos;
	return tmp;
}

static inline void hri_adc_set_INPUTCTRL_MUXNEG_bf(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->INPUTCTRL.reg |= ADC_INPUTCTRL_MUXNEG(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_inputctrl_reg_t hri_adc_get_INPUTCTRL_MUXNEG_bf(const void *const       hw,
                                                                      hri_adc_inputctrl_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->INPUTCTRL.reg;
	tmp = (tmp & ADC_INPUTCTRL_MUXNEG(mask)) >> ADC_INPUTCTRL_MUXNEG_Pos;
	return tmp;
}

static inline void hri_adc_write_INPUTCTRL_MUXNEG_bf(const void *const hw, hri_adc_inputctrl_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->INPUTCTRL.reg;
	tmp &= ~ADC_INPUTCTRL_MUXNEG_Msk;
	tmp |= ADC_INPUTCTRL_MUXNEG(data);
	((Adc *)hw)->INPUTCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_INPUTCTRL_MUXNEG_bf(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->INPUTCTRL.reg &= ~ADC_INPUTCTRL_MUXNEG(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_INPUTCTRL_MUXNEG_bf(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->INPUTCTRL.reg ^= ADC_INPUTCTRL_MUXNEG(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_inputctrl_reg_t hri_adc_read_INPUTCTRL_MUXNEG_bf(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->INPUTCTRL.reg;
	tmp = (tmp & ADC_INPUTCTRL_MUXNEG_Msk) >> ADC_INPUTCTRL_MUXNEG_Pos;
	return tmp;
}

static inline void hri_adc_set_INPUTCTRL_reg(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->INPUTCTRL.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_inputctrl_reg_t hri_adc_get_INPUTCTRL_reg(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->INPUTCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_INPUTCTRL_reg(const void *const hw, hri_adc_inputctrl_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->INPUTCTRL.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_INPUTCTRL_reg(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->INPUTCTRL.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_INPUTCTRL_reg(const void *const hw, hri_adc_inputctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->INPUTCTRL.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_inputctrl_reg_t hri_adc_read_INPUTCTRL_reg(const void *const hw)
{
	return ((Adc *)hw)->INPUTCTRL.reg;
}

static inline void hri_adc_set_CTRLC_DIFFMODE_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg |= ADC_CTRLC_DIFFMODE;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_CTRLC_DIFFMODE_bit(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp = (tmp & ADC_CTRLC_DIFFMODE) >> ADC_CTRLC_DIFFMODE_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_CTRLC_DIFFMODE_bit(const void *const hw, bool value)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp &= ~ADC_CTRLC_DIFFMODE;
	tmp |= value << ADC_CTRLC_DIFFMODE_Pos;
	((Adc *)hw)->CTRLC.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLC_DIFFMODE_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg &= ~ADC_CTRLC_DIFFMODE;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLC_DIFFMODE_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg ^= ADC_CTRLC_DIFFMODE;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_CTRLC_LEFTADJ_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg |= ADC_CTRLC_LEFTADJ;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_CTRLC_LEFTADJ_bit(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp = (tmp & ADC_CTRLC_LEFTADJ) >> ADC_CTRLC_LEFTADJ_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_CTRLC_LEFTADJ_bit(const void *const hw, bool value)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp &= ~ADC_CTRLC_LEFTADJ;
	tmp |= value << ADC_CTRLC_LEFTADJ_Pos;
	((Adc *)hw)->CTRLC.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLC_LEFTADJ_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg &= ~ADC_CTRLC_LEFTADJ;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLC_LEFTADJ_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg ^= ADC_CTRLC_LEFTADJ;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_CTRLC_FREERUN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg |= ADC_CTRLC_FREERUN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_CTRLC_FREERUN_bit(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp = (tmp & ADC_CTRLC_FREERUN) >> ADC_CTRLC_FREERUN_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_CTRLC_FREERUN_bit(const void *const hw, bool value)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp &= ~ADC_CTRLC_FREERUN;
	tmp |= value << ADC_CTRLC_FREERUN_Pos;
	((Adc *)hw)->CTRLC.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLC_FREERUN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg &= ~ADC_CTRLC_FREERUN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLC_FREERUN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg ^= ADC_CTRLC_FREERUN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_CTRLC_CORREN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg |= ADC_CTRLC_CORREN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_CTRLC_CORREN_bit(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp = (tmp & ADC_CTRLC_CORREN) >> ADC_CTRLC_CORREN_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_CTRLC_CORREN_bit(const void *const hw, bool value)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp &= ~ADC_CTRLC_CORREN;
	tmp |= value << ADC_CTRLC_CORREN_Pos;
	((Adc *)hw)->CTRLC.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLC_CORREN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg &= ~ADC_CTRLC_CORREN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLC_CORREN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg ^= ADC_CTRLC_CORREN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_CTRLC_RESSEL_bf(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg |= ADC_CTRLC_RESSEL(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlc_reg_t hri_adc_get_CTRLC_RESSEL_bf(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp = (tmp & ADC_CTRLC_RESSEL(mask)) >> ADC_CTRLC_RESSEL_Pos;
	return tmp;
}

static inline void hri_adc_write_CTRLC_RESSEL_bf(const void *const hw, hri_adc_ctrlc_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp &= ~ADC_CTRLC_RESSEL_Msk;
	tmp |= ADC_CTRLC_RESSEL(data);
	((Adc *)hw)->CTRLC.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLC_RESSEL_bf(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg &= ~ADC_CTRLC_RESSEL(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLC_RESSEL_bf(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg ^= ADC_CTRLC_RESSEL(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlc_reg_t hri_adc_read_CTRLC_RESSEL_bf(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp = (tmp & ADC_CTRLC_RESSEL_Msk) >> ADC_CTRLC_RESSEL_Pos;
	return tmp;
}

static inline void hri_adc_set_CTRLC_WINMODE_bf(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg |= ADC_CTRLC_WINMODE(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlc_reg_t hri_adc_get_CTRLC_WINMODE_bf(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp = (tmp & ADC_CTRLC_WINMODE(mask)) >> ADC_CTRLC_WINMODE_Pos;
	return tmp;
}

static inline void hri_adc_write_CTRLC_WINMODE_bf(const void *const hw, hri_adc_ctrlc_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp &= ~ADC_CTRLC_WINMODE_Msk;
	tmp |= ADC_CTRLC_WINMODE(data);
	((Adc *)hw)->CTRLC.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLC_WINMODE_bf(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg &= ~ADC_CTRLC_WINMODE(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLC_WINMODE_bf(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->CTRLC.reg ^= ADC_CTRLC_WINMODE(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlc_reg_t hri_adc_read_CTRLC_WINMODE_bf(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp = (tmp & ADC_CTRLC_WINMODE_Msk) >> ADC_CTRLC_WINMODE_Pos;
	return tmp;
}

static inline void hri_adc_set_CTRLC_reg(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLC.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlc_reg_t hri_adc_get_CTRLC_reg(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CTRLC.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_CTRLC_reg(const void *const hw, hri_adc_ctrlc_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLC.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CTRLC_reg(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLC.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CTRLC_reg(const void *const hw, hri_adc_ctrlc_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CTRLC.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_ctrlc_reg_t hri_adc_read_CTRLC_reg(const void *const hw)
{
	return ((Adc *)hw)->CTRLC.reg;
}

static inline void hri_adc_set_AVGCTRL_SAMPLENUM_bf(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->AVGCTRL.reg |= ADC_AVGCTRL_SAMPLENUM(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_avgctrl_reg_t hri_adc_get_AVGCTRL_SAMPLENUM_bf(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->AVGCTRL.reg;
	tmp = (tmp & ADC_AVGCTRL_SAMPLENUM(mask)) >> ADC_AVGCTRL_SAMPLENUM_Pos;
	return tmp;
}

static inline void hri_adc_write_AVGCTRL_SAMPLENUM_bf(const void *const hw, hri_adc_avgctrl_reg_t data)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->AVGCTRL.reg;
	tmp &= ~ADC_AVGCTRL_SAMPLENUM_Msk;
	tmp |= ADC_AVGCTRL_SAMPLENUM(data);
	((Adc *)hw)->AVGCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_AVGCTRL_SAMPLENUM_bf(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->AVGCTRL.reg &= ~ADC_AVGCTRL_SAMPLENUM(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_AVGCTRL_SAMPLENUM_bf(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->AVGCTRL.reg ^= ADC_AVGCTRL_SAMPLENUM(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_avgctrl_reg_t hri_adc_read_AVGCTRL_SAMPLENUM_bf(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->AVGCTRL.reg;
	tmp = (tmp & ADC_AVGCTRL_SAMPLENUM_Msk) >> ADC_AVGCTRL_SAMPLENUM_Pos;
	return tmp;
}

static inline void hri_adc_set_AVGCTRL_ADJRES_bf(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->AVGCTRL.reg |= ADC_AVGCTRL_ADJRES(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_avgctrl_reg_t hri_adc_get_AVGCTRL_ADJRES_bf(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->AVGCTRL.reg;
	tmp = (tmp & ADC_AVGCTRL_ADJRES(mask)) >> ADC_AVGCTRL_ADJRES_Pos;
	return tmp;
}

static inline void hri_adc_write_AVGCTRL_ADJRES_bf(const void *const hw, hri_adc_avgctrl_reg_t data)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->AVGCTRL.reg;
	tmp &= ~ADC_AVGCTRL_ADJRES_Msk;
	tmp |= ADC_AVGCTRL_ADJRES(data);
	((Adc *)hw)->AVGCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_AVGCTRL_ADJRES_bf(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->AVGCTRL.reg &= ~ADC_AVGCTRL_ADJRES(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_AVGCTRL_ADJRES_bf(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->AVGCTRL.reg ^= ADC_AVGCTRL_ADJRES(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_avgctrl_reg_t hri_adc_read_AVGCTRL_ADJRES_bf(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->AVGCTRL.reg;
	tmp = (tmp & ADC_AVGCTRL_ADJRES_Msk) >> ADC_AVGCTRL_ADJRES_Pos;
	return tmp;
}

static inline void hri_adc_set_AVGCTRL_reg(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->AVGCTRL.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_avgctrl_reg_t hri_adc_get_AVGCTRL_reg(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->AVGCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_AVGCTRL_reg(const void *const hw, hri_adc_avgctrl_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->AVGCTRL.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_AVGCTRL_reg(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->AVGCTRL.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_AVGCTRL_reg(const void *const hw, hri_adc_avgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->AVGCTRL.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_avgctrl_reg_t hri_adc_read_AVGCTRL_reg(const void *const hw)
{
	return ((Adc *)hw)->AVGCTRL.reg;
}

static inline void hri_adc_set_SAMPCTRL_OFFCOMP_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SAMPCTRL.reg |= ADC_SAMPCTRL_OFFCOMP;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_SAMPCTRL_OFFCOMP_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->SAMPCTRL.reg;
	tmp = (tmp & ADC_SAMPCTRL_OFFCOMP) >> ADC_SAMPCTRL_OFFCOMP_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_SAMPCTRL_OFFCOMP_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->SAMPCTRL.reg;
	tmp &= ~ADC_SAMPCTRL_OFFCOMP;
	tmp |= value << ADC_SAMPCTRL_OFFCOMP_Pos;
	((Adc *)hw)->SAMPCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_SAMPCTRL_OFFCOMP_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SAMPCTRL.reg &= ~ADC_SAMPCTRL_OFFCOMP;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_SAMPCTRL_OFFCOMP_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SAMPCTRL.reg ^= ADC_SAMPCTRL_OFFCOMP;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_SAMPCTRL_SAMPLEN_bf(const void *const hw, hri_adc_sampctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SAMPCTRL.reg |= ADC_SAMPCTRL_SAMPLEN(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_sampctrl_reg_t hri_adc_get_SAMPCTRL_SAMPLEN_bf(const void *const hw, hri_adc_sampctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->SAMPCTRL.reg;
	tmp = (tmp & ADC_SAMPCTRL_SAMPLEN(mask)) >> ADC_SAMPCTRL_SAMPLEN_Pos;
	return tmp;
}

static inline void hri_adc_write_SAMPCTRL_SAMPLEN_bf(const void *const hw, hri_adc_sampctrl_reg_t data)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->SAMPCTRL.reg;
	tmp &= ~ADC_SAMPCTRL_SAMPLEN_Msk;
	tmp |= ADC_SAMPCTRL_SAMPLEN(data);
	((Adc *)hw)->SAMPCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_SAMPCTRL_SAMPLEN_bf(const void *const hw, hri_adc_sampctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SAMPCTRL.reg &= ~ADC_SAMPCTRL_SAMPLEN(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_SAMPCTRL_SAMPLEN_bf(const void *const hw, hri_adc_sampctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SAMPCTRL.reg ^= ADC_SAMPCTRL_SAMPLEN(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_sampctrl_reg_t hri_adc_read_SAMPCTRL_SAMPLEN_bf(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->SAMPCTRL.reg;
	tmp = (tmp & ADC_SAMPCTRL_SAMPLEN_Msk) >> ADC_SAMPCTRL_SAMPLEN_Pos;
	return tmp;
}

static inline void hri_adc_set_SAMPCTRL_reg(const void *const hw, hri_adc_sampctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SAMPCTRL.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_sampctrl_reg_t hri_adc_get_SAMPCTRL_reg(const void *const hw, hri_adc_sampctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->SAMPCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_SAMPCTRL_reg(const void *const hw, hri_adc_sampctrl_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SAMPCTRL.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_SAMPCTRL_reg(const void *const hw, hri_adc_sampctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SAMPCTRL.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_SAMPCTRL_reg(const void *const hw, hri_adc_sampctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SAMPCTRL.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_sampctrl_reg_t hri_adc_read_SAMPCTRL_reg(const void *const hw)
{
	return ((Adc *)hw)->SAMPCTRL.reg;
}

static inline void hri_adc_set_WINLT_WINLT_bf(const void *const hw, hri_adc_winlt_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINLT);
	((Adc *)hw)->WINLT.reg |= ADC_WINLT_WINLT(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_winlt_reg_t hri_adc_get_WINLT_WINLT_bf(const void *const hw, hri_adc_winlt_reg_t mask)
{
	uint16_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINLT);
	tmp = ((Adc *)hw)->WINLT.reg;
	tmp = (tmp & ADC_WINLT_WINLT(mask)) >> ADC_WINLT_WINLT_Pos;
	return tmp;
}

static inline void hri_adc_write_WINLT_WINLT_bf(const void *const hw, hri_adc_winlt_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINLT);
	tmp = ((Adc *)hw)->WINLT.reg;
	tmp &= ~ADC_WINLT_WINLT_Msk;
	tmp |= ADC_WINLT_WINLT(data);
	((Adc *)hw)->WINLT.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_WINLT_WINLT_bf(const void *const hw, hri_adc_winlt_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINLT);
	((Adc *)hw)->WINLT.reg &= ~ADC_WINLT_WINLT(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_WINLT_WINLT_bf(const void *const hw, hri_adc_winlt_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINLT);
	((Adc *)hw)->WINLT.reg ^= ADC_WINLT_WINLT(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_winlt_reg_t hri_adc_read_WINLT_WINLT_bf(const void *const hw)
{
	uint16_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINLT);
	tmp = ((Adc *)hw)->WINLT.reg;
	tmp = (tmp & ADC_WINLT_WINLT_Msk) >> ADC_WINLT_WINLT_Pos;
	return tmp;
}

static inline void hri_adc_set_WINLT_reg(const void *const hw, hri_adc_winlt_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->WINLT.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_winlt_reg_t hri_adc_get_WINLT_reg(const void *const hw, hri_adc_winlt_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->WINLT.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_WINLT_reg(const void *const hw, hri_adc_winlt_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->WINLT.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_WINLT_reg(const void *const hw, hri_adc_winlt_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->WINLT.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_WINLT_reg(const void *const hw, hri_adc_winlt_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->WINLT.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_winlt_reg_t hri_adc_read_WINLT_reg(const void *const hw)
{
	return ((Adc *)hw)->WINLT.reg;
}

static inline void hri_adc_set_WINUT_WINUT_bf(const void *const hw, hri_adc_winut_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINUT);
	((Adc *)hw)->WINUT.reg |= ADC_WINUT_WINUT(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_winut_reg_t hri_adc_get_WINUT_WINUT_bf(const void *const hw, hri_adc_winut_reg_t mask)
{
	uint16_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINUT);
	tmp = ((Adc *)hw)->WINUT.reg;
	tmp = (tmp & ADC_WINUT_WINUT(mask)) >> ADC_WINUT_WINUT_Pos;
	return tmp;
}

static inline void hri_adc_write_WINUT_WINUT_bf(const void *const hw, hri_adc_winut_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINUT);
	tmp = ((Adc *)hw)->WINUT.reg;
	tmp &= ~ADC_WINUT_WINUT_Msk;
	tmp |= ADC_WINUT_WINUT(data);
	((Adc *)hw)->WINUT.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_WINUT_WINUT_bf(const void *const hw, hri_adc_winut_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINUT);
	((Adc *)hw)->WINUT.reg &= ~ADC_WINUT_WINUT(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_WINUT_WINUT_bf(const void *const hw, hri_adc_winut_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINUT);
	((Adc *)hw)->WINUT.reg ^= ADC_WINUT_WINUT(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_winut_reg_t hri_adc_read_WINUT_WINUT_bf(const void *const hw)
{
	uint16_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_WINUT);
	tmp = ((Adc *)hw)->WINUT.reg;
	tmp = (tmp & ADC_WINUT_WINUT_Msk) >> ADC_WINUT_WINUT_Pos;
	return tmp;
}

static inline void hri_adc_set_WINUT_reg(const void *const hw, hri_adc_winut_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->WINUT.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_winut_reg_t hri_adc_get_WINUT_reg(const void *const hw, hri_adc_winut_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->WINUT.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_WINUT_reg(const void *const hw, hri_adc_winut_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->WINUT.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_WINUT_reg(const void *const hw, hri_adc_winut_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->WINUT.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_WINUT_reg(const void *const hw, hri_adc_winut_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->WINUT.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_winut_reg_t hri_adc_read_WINUT_reg(const void *const hw)
{
	return ((Adc *)hw)->WINUT.reg;
}

static inline void hri_adc_set_GAINCORR_GAINCORR_bf(const void *const hw, hri_adc_gaincorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_GAINCORR);
	((Adc *)hw)->GAINCORR.reg |= ADC_GAINCORR_GAINCORR(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_gaincorr_reg_t hri_adc_get_GAINCORR_GAINCORR_bf(const void *const hw, hri_adc_gaincorr_reg_t mask)
{
	uint16_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_GAINCORR);
	tmp = ((Adc *)hw)->GAINCORR.reg;
	tmp = (tmp & ADC_GAINCORR_GAINCORR(mask)) >> ADC_GAINCORR_GAINCORR_Pos;
	return tmp;
}

static inline void hri_adc_write_GAINCORR_GAINCORR_bf(const void *const hw, hri_adc_gaincorr_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_GAINCORR);
	tmp = ((Adc *)hw)->GAINCORR.reg;
	tmp &= ~ADC_GAINCORR_GAINCORR_Msk;
	tmp |= ADC_GAINCORR_GAINCORR(data);
	((Adc *)hw)->GAINCORR.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_GAINCORR_GAINCORR_bf(const void *const hw, hri_adc_gaincorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_GAINCORR);
	((Adc *)hw)->GAINCORR.reg &= ~ADC_GAINCORR_GAINCORR(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_GAINCORR_GAINCORR_bf(const void *const hw, hri_adc_gaincorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_GAINCORR);
	((Adc *)hw)->GAINCORR.reg ^= ADC_GAINCORR_GAINCORR(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_gaincorr_reg_t hri_adc_read_GAINCORR_GAINCORR_bf(const void *const hw)
{
	uint16_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_GAINCORR);
	tmp = ((Adc *)hw)->GAINCORR.reg;
	tmp = (tmp & ADC_GAINCORR_GAINCORR_Msk) >> ADC_GAINCORR_GAINCORR_Pos;
	return tmp;
}

static inline void hri_adc_set_GAINCORR_reg(const void *const hw, hri_adc_gaincorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->GAINCORR.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_gaincorr_reg_t hri_adc_get_GAINCORR_reg(const void *const hw, hri_adc_gaincorr_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->GAINCORR.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_GAINCORR_reg(const void *const hw, hri_adc_gaincorr_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->GAINCORR.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_GAINCORR_reg(const void *const hw, hri_adc_gaincorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->GAINCORR.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_GAINCORR_reg(const void *const hw, hri_adc_gaincorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->GAINCORR.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_gaincorr_reg_t hri_adc_read_GAINCORR_reg(const void *const hw)
{
	return ((Adc *)hw)->GAINCORR.reg;
}

static inline void hri_adc_set_OFFSETCORR_OFFSETCORR_bf(const void *const hw, hri_adc_offsetcorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_OFFSETCORR);
	((Adc *)hw)->OFFSETCORR.reg |= ADC_OFFSETCORR_OFFSETCORR(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_offsetcorr_reg_t hri_adc_get_OFFSETCORR_OFFSETCORR_bf(const void *const        hw,
                                                                            hri_adc_offsetcorr_reg_t mask)
{
	uint16_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_OFFSETCORR);
	tmp = ((Adc *)hw)->OFFSETCORR.reg;
	tmp = (tmp & ADC_OFFSETCORR_OFFSETCORR(mask)) >> ADC_OFFSETCORR_OFFSETCORR_Pos;
	return tmp;
}

static inline void hri_adc_write_OFFSETCORR_OFFSETCORR_bf(const void *const hw, hri_adc_offsetcorr_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_OFFSETCORR);
	tmp = ((Adc *)hw)->OFFSETCORR.reg;
	tmp &= ~ADC_OFFSETCORR_OFFSETCORR_Msk;
	tmp |= ADC_OFFSETCORR_OFFSETCORR(data);
	((Adc *)hw)->OFFSETCORR.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_OFFSETCORR_OFFSETCORR_bf(const void *const hw, hri_adc_offsetcorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_OFFSETCORR);
	((Adc *)hw)->OFFSETCORR.reg &= ~ADC_OFFSETCORR_OFFSETCORR(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_OFFSETCORR_OFFSETCORR_bf(const void *const hw, hri_adc_offsetcorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_OFFSETCORR);
	((Adc *)hw)->OFFSETCORR.reg ^= ADC_OFFSETCORR_OFFSETCORR(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_offsetcorr_reg_t hri_adc_read_OFFSETCORR_OFFSETCORR_bf(const void *const hw)
{
	uint16_t tmp;
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_OFFSETCORR);
	tmp = ((Adc *)hw)->OFFSETCORR.reg;
	tmp = (tmp & ADC_OFFSETCORR_OFFSETCORR_Msk) >> ADC_OFFSETCORR_OFFSETCORR_Pos;
	return tmp;
}

static inline void hri_adc_set_OFFSETCORR_reg(const void *const hw, hri_adc_offsetcorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->OFFSETCORR.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_offsetcorr_reg_t hri_adc_get_OFFSETCORR_reg(const void *const hw, hri_adc_offsetcorr_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->OFFSETCORR.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_OFFSETCORR_reg(const void *const hw, hri_adc_offsetcorr_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->OFFSETCORR.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_OFFSETCORR_reg(const void *const hw, hri_adc_offsetcorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->OFFSETCORR.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_OFFSETCORR_reg(const void *const hw, hri_adc_offsetcorr_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->OFFSETCORR.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_offsetcorr_reg_t hri_adc_read_OFFSETCORR_reg(const void *const hw)
{
	return ((Adc *)hw)->OFFSETCORR.reg;
}

static inline void hri_adc_set_SWTRIG_FLUSH_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SWTRIG.reg |= ADC_SWTRIG_FLUSH;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_SWTRIG_FLUSH_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->SWTRIG.reg;
	tmp = (tmp & ADC_SWTRIG_FLUSH) >> ADC_SWTRIG_FLUSH_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_SWTRIG_FLUSH_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->SWTRIG.reg;
	tmp &= ~ADC_SWTRIG_FLUSH;
	tmp |= value << ADC_SWTRIG_FLUSH_Pos;
	((Adc *)hw)->SWTRIG.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_SWTRIG_FLUSH_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SWTRIG.reg &= ~ADC_SWTRIG_FLUSH;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_SWTRIG_FLUSH_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SWTRIG.reg ^= ADC_SWTRIG_FLUSH;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_SWTRIG_START_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SWTRIG.reg |= ADC_SWTRIG_START;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_SWTRIG_START_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->SWTRIG.reg;
	tmp = (tmp & ADC_SWTRIG_START) >> ADC_SWTRIG_START_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_SWTRIG_START_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	tmp = ((Adc *)hw)->SWTRIG.reg;
	tmp &= ~ADC_SWTRIG_START;
	tmp |= value << ADC_SWTRIG_START_Pos;
	((Adc *)hw)->SWTRIG.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_SWTRIG_START_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SWTRIG.reg &= ~ADC_SWTRIG_START;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_SWTRIG_START_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	hri_adc_wait_for_sync(hw, ADC_SYNCBUSY_MASK);
	((Adc *)hw)->SWTRIG.reg ^= ADC_SWTRIG_START;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_SWTRIG_reg(const void *const hw, hri_adc_swtrig_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SWTRIG.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_swtrig_reg_t hri_adc_get_SWTRIG_reg(const void *const hw, hri_adc_swtrig_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->SWTRIG.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_SWTRIG_reg(const void *const hw, hri_adc_swtrig_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SWTRIG.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_SWTRIG_reg(const void *const hw, hri_adc_swtrig_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SWTRIG.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_SWTRIG_reg(const void *const hw, hri_adc_swtrig_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SWTRIG.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_swtrig_reg_t hri_adc_read_SWTRIG_reg(const void *const hw)
{
	return ((Adc *)hw)->SWTRIG.reg;
}

static inline void hri_adc_set_DBGCTRL_DBGRUN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->DBGCTRL.reg |= ADC_DBGCTRL_DBGRUN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_adc_get_DBGCTRL_DBGRUN_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->DBGCTRL.reg;
	tmp = (tmp & ADC_DBGCTRL_DBGRUN) >> ADC_DBGCTRL_DBGRUN_Pos;
	return (bool)tmp;
}

static inline void hri_adc_write_DBGCTRL_DBGRUN_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->DBGCTRL.reg;
	tmp &= ~ADC_DBGCTRL_DBGRUN;
	tmp |= value << ADC_DBGCTRL_DBGRUN_Pos;
	((Adc *)hw)->DBGCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_DBGCTRL_DBGRUN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->DBGCTRL.reg &= ~ADC_DBGCTRL_DBGRUN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_DBGCTRL_DBGRUN_bit(const void *const hw)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->DBGCTRL.reg ^= ADC_DBGCTRL_DBGRUN;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_set_DBGCTRL_reg(const void *const hw, hri_adc_dbgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->DBGCTRL.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_dbgctrl_reg_t hri_adc_get_DBGCTRL_reg(const void *const hw, hri_adc_dbgctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->DBGCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_DBGCTRL_reg(const void *const hw, hri_adc_dbgctrl_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->DBGCTRL.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_DBGCTRL_reg(const void *const hw, hri_adc_dbgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->DBGCTRL.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_DBGCTRL_reg(const void *const hw, hri_adc_dbgctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->DBGCTRL.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_dbgctrl_reg_t hri_adc_read_DBGCTRL_reg(const void *const hw)
{
	return ((Adc *)hw)->DBGCTRL.reg;
}

static inline void hri_adc_set_SEQCTRL_SEQEN_bf(const void *const hw, hri_adc_seqctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SEQCTRL.reg |= ADC_SEQCTRL_SEQEN(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_seqctrl_reg_t hri_adc_get_SEQCTRL_SEQEN_bf(const void *const hw, hri_adc_seqctrl_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Adc *)hw)->SEQCTRL.reg;
	tmp = (tmp & ADC_SEQCTRL_SEQEN(mask)) >> ADC_SEQCTRL_SEQEN_Pos;
	return tmp;
}

static inline void hri_adc_write_SEQCTRL_SEQEN_bf(const void *const hw, hri_adc_seqctrl_reg_t data)
{
	uint32_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->SEQCTRL.reg;
	tmp &= ~ADC_SEQCTRL_SEQEN_Msk;
	tmp |= ADC_SEQCTRL_SEQEN(data);
	((Adc *)hw)->SEQCTRL.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_SEQCTRL_SEQEN_bf(const void *const hw, hri_adc_seqctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SEQCTRL.reg &= ~ADC_SEQCTRL_SEQEN(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_SEQCTRL_SEQEN_bf(const void *const hw, hri_adc_seqctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SEQCTRL.reg ^= ADC_SEQCTRL_SEQEN(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_seqctrl_reg_t hri_adc_read_SEQCTRL_SEQEN_bf(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Adc *)hw)->SEQCTRL.reg;
	tmp = (tmp & ADC_SEQCTRL_SEQEN_Msk) >> ADC_SEQCTRL_SEQEN_Pos;
	return tmp;
}

static inline void hri_adc_set_SEQCTRL_reg(const void *const hw, hri_adc_seqctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SEQCTRL.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_seqctrl_reg_t hri_adc_get_SEQCTRL_reg(const void *const hw, hri_adc_seqctrl_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Adc *)hw)->SEQCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_SEQCTRL_reg(const void *const hw, hri_adc_seqctrl_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SEQCTRL.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_SEQCTRL_reg(const void *const hw, hri_adc_seqctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SEQCTRL.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_SEQCTRL_reg(const void *const hw, hri_adc_seqctrl_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->SEQCTRL.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_seqctrl_reg_t hri_adc_read_SEQCTRL_reg(const void *const hw)
{
	return ((Adc *)hw)->SEQCTRL.reg;
}

static inline void hri_adc_set_CALIB_BIASCOMP_bf(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg |= ADC_CALIB_BIASCOMP(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_calib_reg_t hri_adc_get_CALIB_BIASCOMP_bf(const void *const hw, hri_adc_calib_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CALIB.reg;
	tmp = (tmp & ADC_CALIB_BIASCOMP(mask)) >> ADC_CALIB_BIASCOMP_Pos;
	return tmp;
}

static inline void hri_adc_write_CALIB_BIASCOMP_bf(const void *const hw, hri_adc_calib_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->CALIB.reg;
	tmp &= ~ADC_CALIB_BIASCOMP_Msk;
	tmp |= ADC_CALIB_BIASCOMP(data);
	((Adc *)hw)->CALIB.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CALIB_BIASCOMP_bf(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg &= ~ADC_CALIB_BIASCOMP(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CALIB_BIASCOMP_bf(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg ^= ADC_CALIB_BIASCOMP(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_calib_reg_t hri_adc_read_CALIB_BIASCOMP_bf(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CALIB.reg;
	tmp = (tmp & ADC_CALIB_BIASCOMP_Msk) >> ADC_CALIB_BIASCOMP_Pos;
	return tmp;
}

static inline void hri_adc_set_CALIB_BIASREFBUF_bf(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg |= ADC_CALIB_BIASREFBUF(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_calib_reg_t hri_adc_get_CALIB_BIASREFBUF_bf(const void *const hw, hri_adc_calib_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CALIB.reg;
	tmp = (tmp & ADC_CALIB_BIASREFBUF(mask)) >> ADC_CALIB_BIASREFBUF_Pos;
	return tmp;
}

static inline void hri_adc_write_CALIB_BIASREFBUF_bf(const void *const hw, hri_adc_calib_reg_t data)
{
	uint16_t tmp;
	ADC_CRITICAL_SECTION_ENTER();
	tmp = ((Adc *)hw)->CALIB.reg;
	tmp &= ~ADC_CALIB_BIASREFBUF_Msk;
	tmp |= ADC_CALIB_BIASREFBUF(data);
	((Adc *)hw)->CALIB.reg = tmp;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CALIB_BIASREFBUF_bf(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg &= ~ADC_CALIB_BIASREFBUF(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CALIB_BIASREFBUF_bf(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg ^= ADC_CALIB_BIASREFBUF(mask);
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_calib_reg_t hri_adc_read_CALIB_BIASREFBUF_bf(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CALIB.reg;
	tmp = (tmp & ADC_CALIB_BIASREFBUF_Msk) >> ADC_CALIB_BIASREFBUF_Pos;
	return tmp;
}

static inline void hri_adc_set_CALIB_reg(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg |= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_calib_reg_t hri_adc_get_CALIB_reg(const void *const hw, hri_adc_calib_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->CALIB.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_adc_write_CALIB_reg(const void *const hw, hri_adc_calib_reg_t data)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg = data;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_clear_CALIB_reg(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg &= ~mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline void hri_adc_toggle_CALIB_reg(const void *const hw, hri_adc_calib_reg_t mask)
{
	ADC_CRITICAL_SECTION_ENTER();
	((Adc *)hw)->CALIB.reg ^= mask;
	ADC_CRITICAL_SECTION_LEAVE();
}

static inline hri_adc_calib_reg_t hri_adc_read_CALIB_reg(const void *const hw)
{
	return ((Adc *)hw)->CALIB.reg;
}

static inline bool hri_adc_get_SEQSTATUS_SEQBUSY_bit(const void *const hw)
{
	return (((Adc *)hw)->SEQSTATUS.reg & ADC_SEQSTATUS_SEQBUSY) >> ADC_SEQSTATUS_SEQBUSY_Pos;
}

static inline hri_adc_seqstatus_reg_t hri_adc_get_SEQSTATUS_SEQSTATE_bf(const void *const       hw,
                                                                        hri_adc_seqstatus_reg_t mask)
{
	return (((Adc *)hw)->SEQSTATUS.reg & ADC_SEQSTATUS_SEQSTATE(mask)) >> ADC_SEQSTATUS_SEQSTATE_Pos;
}

static inline hri_adc_seqstatus_reg_t hri_adc_read_SEQSTATUS_SEQSTATE_bf(const void *const hw)
{
	return (((Adc *)hw)->SEQSTATUS.reg & ADC_SEQSTATUS_SEQSTATE_Msk) >> ADC_SEQSTATUS_SEQSTATE_Pos;
}

static inline hri_adc_seqstatus_reg_t hri_adc_get_SEQSTATUS_reg(const void *const hw, hri_adc_seqstatus_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Adc *)hw)->SEQSTATUS.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_adc_seqstatus_reg_t hri_adc_read_SEQSTATUS_reg(const void *const hw)
{
	return ((Adc *)hw)->SEQSTATUS.reg;
}

static inline bool hri_adc_get_SYNCBUSY_SWRST_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_SWRST) >> ADC_SYNCBUSY_SWRST_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_ENABLE_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE) >> ADC_SYNCBUSY_ENABLE_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_INPUTCTRL_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_INPUTCTRL) >> ADC_SYNCBUSY_INPUTCTRL_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_CTRLC_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_CTRLC) >> ADC_SYNCBUSY_CTRLC_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_AVGCTRL_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_AVGCTRL) >> ADC_SYNCBUSY_AVGCTRL_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_SAMPCTRL_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_SAMPCTRL) >> ADC_SYNCBUSY_SAMPCTRL_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_WINLT_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_WINLT) >> ADC_SYNCBUSY_WINLT_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_WINUT_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_WINUT) >> ADC_SYNCBUSY_WINUT_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_GAINCORR_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_GAINCORR) >> ADC_SYNCBUSY_GAINCORR_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_OFFSETCORR_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_OFFSETCORR) >> ADC_SYNCBUSY_OFFSETCORR_Pos;
}

static inline bool hri_adc_get_SYNCBUSY_SWTRIG_bit(const void *const hw)
{
	return (((Adc *)hw)->SYNCBUSY.reg & ADC_SYNCBUSY_SWTRIG) >> ADC_SYNCBUSY_SWTRIG_Pos;
}

static inline hri_adc_syncbusy_reg_t hri_adc_get_SYNCBUSY_reg(const void *const hw, hri_adc_syncbusy_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->SYNCBUSY.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_adc_syncbusy_reg_t hri_adc_read_SYNCBUSY_reg(const void *const hw)
{
	return ((Adc *)hw)->SYNCBUSY.reg;
}

static inline hri_adc_result_reg_t hri_adc_get_RESULT_RESULT_bf(const void *const hw, hri_adc_result_reg_t mask)
{
	return (((Adc *)hw)->RESULT.reg & ADC_RESULT_RESULT(mask)) >> ADC_RESULT_RESULT_Pos;
}

static inline hri_adc_result_reg_t hri_adc_read_RESULT_RESULT_bf(const void *const hw)
{
	return (((Adc *)hw)->RESULT.reg & ADC_RESULT_RESULT_Msk) >> ADC_RESULT_RESULT_Pos;
}

static inline hri_adc_result_reg_t hri_adc_get_RESULT_reg(const void *const hw, hri_adc_result_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Adc *)hw)->RESULT.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_adc_result_reg_t hri_adc_read_RESULT_reg(const void *const hw)
{
	return ((Adc *)hw)->RESULT.reg;
}

#ifdef __cplusplus
}
#endif

#endif /* _HRI_ADC_L21_H_INCLUDED */
#endif /* _SAML21_ADC_COMPONENT_ */
