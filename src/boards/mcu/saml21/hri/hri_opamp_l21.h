/**
 * \file
 *
 * \brief SAM OPAMP
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

#ifdef _SAML21_OPAMP_COMPONENT_
#ifndef _HRI_OPAMP_L21_H_INCLUDED_
#define _HRI_OPAMP_L21_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <hal_atomic.h>

#if defined(ENABLE_OPAMP_CRITICAL_SECTIONS)
#define OPAMP_CRITICAL_SECTION_ENTER() CRITICAL_SECTION_ENTER()
#define OPAMP_CRITICAL_SECTION_LEAVE() CRITICAL_SECTION_LEAVE()
#else
#define OPAMP_CRITICAL_SECTION_ENTER()
#define OPAMP_CRITICAL_SECTION_LEAVE()
#endif

typedef uint32_t hri_opamp_opampctrl_reg_t;
typedef uint8_t  hri_opamp_ctrla_reg_t;
typedef uint8_t  hri_opamp_status_reg_t;

static inline void hri_opamp_set_CTRLA_SWRST_bit(const void *const hw)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg |= OPAMP_CTRLA_SWRST;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_CTRLA_SWRST_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Opamp *)hw)->CTRLA.reg;
	tmp = (tmp & OPAMP_CTRLA_SWRST) >> OPAMP_CTRLA_SWRST_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_set_CTRLA_ENABLE_bit(const void *const hw)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg |= OPAMP_CTRLA_ENABLE;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_CTRLA_ENABLE_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Opamp *)hw)->CTRLA.reg;
	tmp = (tmp & OPAMP_CTRLA_ENABLE) >> OPAMP_CTRLA_ENABLE_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_CTRLA_ENABLE_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->CTRLA.reg;
	tmp &= ~OPAMP_CTRLA_ENABLE;
	tmp |= value << OPAMP_CTRLA_ENABLE_Pos;
	((Opamp *)hw)->CTRLA.reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_CTRLA_ENABLE_bit(const void *const hw)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg &= ~OPAMP_CTRLA_ENABLE;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_CTRLA_ENABLE_bit(const void *const hw)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg ^= OPAMP_CTRLA_ENABLE;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_CTRLA_LPMUX_bit(const void *const hw)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg |= OPAMP_CTRLA_LPMUX;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_CTRLA_LPMUX_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Opamp *)hw)->CTRLA.reg;
	tmp = (tmp & OPAMP_CTRLA_LPMUX) >> OPAMP_CTRLA_LPMUX_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_CTRLA_LPMUX_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->CTRLA.reg;
	tmp &= ~OPAMP_CTRLA_LPMUX;
	tmp |= value << OPAMP_CTRLA_LPMUX_Pos;
	((Opamp *)hw)->CTRLA.reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_CTRLA_LPMUX_bit(const void *const hw)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg &= ~OPAMP_CTRLA_LPMUX;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_CTRLA_LPMUX_bit(const void *const hw)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg ^= OPAMP_CTRLA_LPMUX;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_CTRLA_reg(const void *const hw, hri_opamp_ctrla_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg |= mask;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_ctrla_reg_t hri_opamp_get_CTRLA_reg(const void *const hw, hri_opamp_ctrla_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Opamp *)hw)->CTRLA.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_opamp_write_CTRLA_reg(const void *const hw, hri_opamp_ctrla_reg_t data)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg = data;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_CTRLA_reg(const void *const hw, hri_opamp_ctrla_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg &= ~mask;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_CTRLA_reg(const void *const hw, hri_opamp_ctrla_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->CTRLA.reg ^= mask;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_ctrla_reg_t hri_opamp_read_CTRLA_reg(const void *const hw)
{
	return ((Opamp *)hw)->CTRLA.reg;
}

static inline void hri_opamp_set_OPAMPCTRL_ENABLE_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_ENABLE;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_OPAMPCTRL_ENABLE_bit(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_ENABLE) >> OPAMP_OPAMPCTRL_ENABLE_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_ENABLE_bit(const void *const hw, uint8_t index, bool value)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_ENABLE;
	tmp |= value << OPAMP_OPAMPCTRL_ENABLE_Pos;
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_ENABLE_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_ENABLE;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_ENABLE_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_ENABLE;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_OPAMPCTRL_ANAOUT_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_ANAOUT;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_OPAMPCTRL_ANAOUT_bit(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_ANAOUT) >> OPAMP_OPAMPCTRL_ANAOUT_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_ANAOUT_bit(const void *const hw, uint8_t index, bool value)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_ANAOUT;
	tmp |= value << OPAMP_OPAMPCTRL_ANAOUT_Pos;
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_ANAOUT_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_ANAOUT;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_ANAOUT_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_ANAOUT;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_OPAMPCTRL_RUNSTDBY_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_RUNSTDBY;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_OPAMPCTRL_RUNSTDBY_bit(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_RUNSTDBY) >> OPAMP_OPAMPCTRL_RUNSTDBY_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_RUNSTDBY_bit(const void *const hw, uint8_t index, bool value)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_RUNSTDBY;
	tmp |= value << OPAMP_OPAMPCTRL_RUNSTDBY_Pos;
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_RUNSTDBY_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_RUNSTDBY;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_RUNSTDBY_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_RUNSTDBY;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_OPAMPCTRL_ONDEMAND_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_ONDEMAND;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_OPAMPCTRL_ONDEMAND_bit(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_ONDEMAND) >> OPAMP_OPAMPCTRL_ONDEMAND_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_ONDEMAND_bit(const void *const hw, uint8_t index, bool value)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_ONDEMAND;
	tmp |= value << OPAMP_OPAMPCTRL_ONDEMAND_Pos;
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_ONDEMAND_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_ONDEMAND;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_ONDEMAND_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_ONDEMAND;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_OPAMPCTRL_RES2OUT_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_RES2OUT;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_OPAMPCTRL_RES2OUT_bit(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_RES2OUT) >> OPAMP_OPAMPCTRL_RES2OUT_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_RES2OUT_bit(const void *const hw, uint8_t index, bool value)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_RES2OUT;
	tmp |= value << OPAMP_OPAMPCTRL_RES2OUT_Pos;
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_RES2OUT_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_RES2OUT;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_RES2OUT_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_RES2OUT;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_OPAMPCTRL_RES2VCC_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_RES2VCC;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_OPAMPCTRL_RES2VCC_bit(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_RES2VCC) >> OPAMP_OPAMPCTRL_RES2VCC_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_RES2VCC_bit(const void *const hw, uint8_t index, bool value)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_RES2VCC;
	tmp |= value << OPAMP_OPAMPCTRL_RES2VCC_Pos;
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_RES2VCC_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_RES2VCC;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_RES2VCC_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_RES2VCC;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_OPAMPCTRL_RES1EN_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_RES1EN;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_opamp_get_OPAMPCTRL_RES1EN_bit(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_RES1EN) >> OPAMP_OPAMPCTRL_RES1EN_Pos;
	return (bool)tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_RES1EN_bit(const void *const hw, uint8_t index, bool value)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_RES1EN;
	tmp |= value << OPAMP_OPAMPCTRL_RES1EN_Pos;
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_RES1EN_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_RES1EN;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_RES1EN_bit(const void *const hw, uint8_t index)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_RES1EN;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_set_OPAMPCTRL_BIAS_bf(const void *const hw, uint8_t index, hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_BIAS(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_get_OPAMPCTRL_BIAS_bf(const void *const hw, uint8_t index,
                                                                        hri_opamp_opampctrl_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_BIAS(mask)) >> OPAMP_OPAMPCTRL_BIAS_Pos;
	return tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_BIAS_bf(const void *const hw, uint8_t index,
                                                     hri_opamp_opampctrl_reg_t data)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_BIAS_Msk;
	tmp |= OPAMP_OPAMPCTRL_BIAS(data);
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_BIAS_bf(const void *const hw, uint8_t index,
                                                     hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_BIAS(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_BIAS_bf(const void *const hw, uint8_t index,
                                                      hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_BIAS(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_read_OPAMPCTRL_BIAS_bf(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_BIAS_Msk) >> OPAMP_OPAMPCTRL_BIAS_Pos;
	return tmp;
}

static inline void hri_opamp_set_OPAMPCTRL_RES1MUX_bf(const void *const hw, uint8_t index,
                                                      hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_RES1MUX(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_get_OPAMPCTRL_RES1MUX_bf(const void *const hw, uint8_t index,
                                                                           hri_opamp_opampctrl_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_RES1MUX(mask)) >> OPAMP_OPAMPCTRL_RES1MUX_Pos;
	return tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_RES1MUX_bf(const void *const hw, uint8_t index,
                                                        hri_opamp_opampctrl_reg_t data)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_RES1MUX_Msk;
	tmp |= OPAMP_OPAMPCTRL_RES1MUX(data);
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_RES1MUX_bf(const void *const hw, uint8_t index,
                                                        hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_RES1MUX(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_RES1MUX_bf(const void *const hw, uint8_t index,
                                                         hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_RES1MUX(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_read_OPAMPCTRL_RES1MUX_bf(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_RES1MUX_Msk) >> OPAMP_OPAMPCTRL_RES1MUX_Pos;
	return tmp;
}

static inline void hri_opamp_set_OPAMPCTRL_POTMUX_bf(const void *const hw, uint8_t index,
                                                     hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_POTMUX(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_get_OPAMPCTRL_POTMUX_bf(const void *const hw, uint8_t index,
                                                                          hri_opamp_opampctrl_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_POTMUX(mask)) >> OPAMP_OPAMPCTRL_POTMUX_Pos;
	return tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_POTMUX_bf(const void *const hw, uint8_t index,
                                                       hri_opamp_opampctrl_reg_t data)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_POTMUX_Msk;
	tmp |= OPAMP_OPAMPCTRL_POTMUX(data);
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_POTMUX_bf(const void *const hw, uint8_t index,
                                                       hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_POTMUX(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_POTMUX_bf(const void *const hw, uint8_t index,
                                                        hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_POTMUX(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_read_OPAMPCTRL_POTMUX_bf(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_POTMUX_Msk) >> OPAMP_OPAMPCTRL_POTMUX_Pos;
	return tmp;
}

static inline void hri_opamp_set_OPAMPCTRL_MUXPOS_bf(const void *const hw, uint8_t index,
                                                     hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_MUXPOS(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_get_OPAMPCTRL_MUXPOS_bf(const void *const hw, uint8_t index,
                                                                          hri_opamp_opampctrl_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_MUXPOS(mask)) >> OPAMP_OPAMPCTRL_MUXPOS_Pos;
	return tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_MUXPOS_bf(const void *const hw, uint8_t index,
                                                       hri_opamp_opampctrl_reg_t data)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_MUXPOS_Msk;
	tmp |= OPAMP_OPAMPCTRL_MUXPOS(data);
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_MUXPOS_bf(const void *const hw, uint8_t index,
                                                       hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_MUXPOS(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_MUXPOS_bf(const void *const hw, uint8_t index,
                                                        hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_MUXPOS(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_read_OPAMPCTRL_MUXPOS_bf(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_MUXPOS_Msk) >> OPAMP_OPAMPCTRL_MUXPOS_Pos;
	return tmp;
}

static inline void hri_opamp_set_OPAMPCTRL_MUXNEG_bf(const void *const hw, uint8_t index,
                                                     hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= OPAMP_OPAMPCTRL_MUXNEG(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_get_OPAMPCTRL_MUXNEG_bf(const void *const hw, uint8_t index,
                                                                          hri_opamp_opampctrl_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_MUXNEG(mask)) >> OPAMP_OPAMPCTRL_MUXNEG_Pos;
	return tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_MUXNEG_bf(const void *const hw, uint8_t index,
                                                       hri_opamp_opampctrl_reg_t data)
{
	uint32_t tmp;
	OPAMP_CRITICAL_SECTION_ENTER();
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= ~OPAMP_OPAMPCTRL_MUXNEG_Msk;
	tmp |= OPAMP_OPAMPCTRL_MUXNEG(data);
	((Opamp *)hw)->OPAMPCTRL[index].reg = tmp;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_MUXNEG_bf(const void *const hw, uint8_t index,
                                                       hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~OPAMP_OPAMPCTRL_MUXNEG(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_MUXNEG_bf(const void *const hw, uint8_t index,
                                                        hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= OPAMP_OPAMPCTRL_MUXNEG(mask);
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_read_OPAMPCTRL_MUXNEG_bf(const void *const hw, uint8_t index)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp = (tmp & OPAMP_OPAMPCTRL_MUXNEG_Msk) >> OPAMP_OPAMPCTRL_MUXNEG_Pos;
	return tmp;
}

static inline void hri_opamp_set_OPAMPCTRL_reg(const void *const hw, uint8_t index, hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg |= mask;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_get_OPAMPCTRL_reg(const void *const hw, uint8_t index,
                                                                    hri_opamp_opampctrl_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Opamp *)hw)->OPAMPCTRL[index].reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_opamp_write_OPAMPCTRL_reg(const void *const hw, uint8_t index, hri_opamp_opampctrl_reg_t data)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg = data;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_clear_OPAMPCTRL_reg(const void *const hw, uint8_t index, hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg &= ~mask;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline void hri_opamp_toggle_OPAMPCTRL_reg(const void *const hw, uint8_t index, hri_opamp_opampctrl_reg_t mask)
{
	OPAMP_CRITICAL_SECTION_ENTER();
	((Opamp *)hw)->OPAMPCTRL[index].reg ^= mask;
	OPAMP_CRITICAL_SECTION_LEAVE();
}

static inline hri_opamp_opampctrl_reg_t hri_opamp_read_OPAMPCTRL_reg(const void *const hw, uint8_t index)
{
	return ((Opamp *)hw)->OPAMPCTRL[index].reg;
}

static inline bool hri_opamp_get_STATUS_READY0_bit(const void *const hw)
{
	return (((Opamp *)hw)->STATUS.reg & OPAMP_STATUS_READY0) >> OPAMP_STATUS_READY0_Pos;
}

static inline bool hri_opamp_get_STATUS_READY1_bit(const void *const hw)
{
	return (((Opamp *)hw)->STATUS.reg & OPAMP_STATUS_READY1) >> OPAMP_STATUS_READY1_Pos;
}

static inline bool hri_opamp_get_STATUS_READY2_bit(const void *const hw)
{
	return (((Opamp *)hw)->STATUS.reg & OPAMP_STATUS_READY2) >> OPAMP_STATUS_READY2_Pos;
}

static inline hri_opamp_status_reg_t hri_opamp_get_STATUS_reg(const void *const hw, hri_opamp_status_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Opamp *)hw)->STATUS.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_opamp_status_reg_t hri_opamp_read_STATUS_reg(const void *const hw)
{
	return ((Opamp *)hw)->STATUS.reg;
}

#ifdef __cplusplus
}
#endif

#endif /* _HRI_OPAMP_L21_H_INCLUDED */
#endif /* _SAML21_OPAMP_COMPONENT_ */
