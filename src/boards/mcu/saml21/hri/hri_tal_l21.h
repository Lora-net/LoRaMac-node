/**
 * \file
 *
 * \brief SAM TAL
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

#ifdef _SAML21_TAL_COMPONENT_
#ifndef _HRI_TAL_L21_H_INCLUDED_
#define _HRI_TAL_L21_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <hal_atomic.h>

#if defined(ENABLE_TAL_CRITICAL_SECTIONS)
#define TAL_CRITICAL_SECTION_ENTER() CRITICAL_SECTION_ENTER()
#define TAL_CRITICAL_SECTION_LEAVE() CRITICAL_SECTION_LEAVE()
#else
#define TAL_CRITICAL_SECTION_ENTER()
#define TAL_CRITICAL_SECTION_LEAVE()
#endif

typedef uint16_t hri_tal_brkstatus_reg_t;
typedef uint16_t hri_tal_irqtrig_reg_t;
typedef uint32_t hri_tal_cpuirqs_reg_t;
typedef uint32_t hri_tal_dmacpusel0_reg_t;
typedef uint32_t hri_tal_eiccpusel0_reg_t;
typedef uint32_t hri_tal_evcpusel0_reg_t;
typedef uint32_t hri_tal_intcpusel0_reg_t;
typedef uint32_t hri_tal_intcpusel1_reg_t;
typedef uint8_t  hri_tal_ctictrla_reg_t;
typedef uint8_t  hri_tal_ctimask_reg_t;
typedef uint8_t  hri_tal_ctrla_reg_t;
typedef uint8_t  hri_tal_evctrl_reg_t;
typedef uint8_t  hri_tal_extctrl_reg_t;
typedef uint8_t  hri_tal_globmask_reg_t;
typedef uint8_t  hri_tal_halt_reg_t;
typedef uint8_t  hri_tal_intenset_reg_t;
typedef uint8_t  hri_tal_intflag_reg_t;
typedef uint8_t  hri_tal_intstatus_reg_t;
typedef uint8_t  hri_tal_restart_reg_t;
typedef uint8_t  hri_tal_rstctrl_reg_t;
typedef uint8_t  hri_talctis_ctictrla_reg_t;
typedef uint8_t  hri_talctis_ctimask_reg_t;

static inline void hri_tal_set_INTEN_BRK_bit(const void *const hw)
{
	((Tal *)hw)->INTENSET.reg = TAL_INTENSET_BRK;
}

static inline bool hri_tal_get_INTEN_BRK_bit(const void *const hw)
{
	return (((Tal *)hw)->INTENSET.reg & TAL_INTENSET_BRK) >> TAL_INTENSET_BRK_Pos;
}

static inline void hri_tal_write_INTEN_BRK_bit(const void *const hw, bool value)
{
	if (value == 0x0) {
		((Tal *)hw)->INTENCLR.reg = TAL_INTENSET_BRK;
	} else {
		((Tal *)hw)->INTENSET.reg = TAL_INTENSET_BRK;
	}
}

static inline void hri_tal_clear_INTEN_BRK_bit(const void *const hw)
{
	((Tal *)hw)->INTENCLR.reg = TAL_INTENSET_BRK;
}

static inline void hri_tal_set_INTEN_reg(const void *const hw, hri_tal_intenset_reg_t mask)
{
	((Tal *)hw)->INTENSET.reg = mask;
}

static inline hri_tal_intenset_reg_t hri_tal_get_INTEN_reg(const void *const hw, hri_tal_intenset_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->INTENSET.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_tal_intenset_reg_t hri_tal_read_INTEN_reg(const void *const hw)
{
	return ((Tal *)hw)->INTENSET.reg;
}

static inline void hri_tal_write_INTEN_reg(const void *const hw, hri_tal_intenset_reg_t data)
{
	((Tal *)hw)->INTENSET.reg = data;
	((Tal *)hw)->INTENCLR.reg = ~data;
}

static inline void hri_tal_clear_INTEN_reg(const void *const hw, hri_tal_intenset_reg_t mask)
{
	((Tal *)hw)->INTENCLR.reg = mask;
}

static inline bool hri_tal_get_INTFLAG_BRK_bit(const void *const hw)
{
	return (((Tal *)hw)->INTFLAG.reg & TAL_INTFLAG_BRK) >> TAL_INTFLAG_BRK_Pos;
}

static inline void hri_tal_clear_INTFLAG_BRK_bit(const void *const hw)
{
	((Tal *)hw)->INTFLAG.reg = TAL_INTFLAG_BRK;
}

static inline bool hri_tal_get_interrupt_BRK_bit(const void *const hw)
{
	return (((Tal *)hw)->INTFLAG.reg & TAL_INTFLAG_BRK) >> TAL_INTFLAG_BRK_Pos;
}

static inline void hri_tal_clear_interrupt_BRK_bit(const void *const hw)
{
	((Tal *)hw)->INTFLAG.reg = TAL_INTFLAG_BRK;
}

static inline hri_tal_intflag_reg_t hri_tal_get_INTFLAG_reg(const void *const hw, hri_tal_intflag_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->INTFLAG.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_tal_intflag_reg_t hri_tal_read_INTFLAG_reg(const void *const hw)
{
	return ((Tal *)hw)->INTFLAG.reg;
}

static inline void hri_tal_clear_INTFLAG_reg(const void *const hw, hri_tal_intflag_reg_t mask)
{
	((Tal *)hw)->INTFLAG.reg = mask;
}

static inline void hri_tal_write_HALT_reg(const void *const hw, hri_tal_halt_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->HALT.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_write_RESTART_reg(const void *const hw, hri_tal_restart_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->RESTART.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_CTRLA_SWRST_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->CTRLA.reg |= TAL_CTRLA_SWRST;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_CTRLA_SWRST_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->CTRLA.reg;
	tmp = (tmp & TAL_CTRLA_SWRST) >> TAL_CTRLA_SWRST_Pos;
	return (bool)tmp;
}

static inline void hri_tal_set_CTRLA_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->CTRLA.reg |= TAL_CTRLA_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_CTRLA_ENABLE_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->CTRLA.reg;
	tmp = (tmp & TAL_CTRLA_ENABLE) >> TAL_CTRLA_ENABLE_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_CTRLA_ENABLE_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->CTRLA.reg;
	tmp &= ~TAL_CTRLA_ENABLE;
	tmp |= value << TAL_CTRLA_ENABLE_Pos;
	((Tal *)hw)->CTRLA.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTRLA_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->CTRLA.reg &= ~TAL_CTRLA_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTRLA_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->CTRLA.reg ^= TAL_CTRLA_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_CTRLA_reg(const void *const hw, hri_tal_ctrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->CTRLA.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctrla_reg_t hri_tal_get_CTRLA_reg(const void *const hw, hri_tal_ctrla_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->CTRLA.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_CTRLA_reg(const void *const hw, hri_tal_ctrla_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->CTRLA.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTRLA_reg(const void *const hw, hri_tal_ctrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->CTRLA.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTRLA_reg(const void *const hw, hri_tal_ctrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->CTRLA.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctrla_reg_t hri_tal_read_CTRLA_reg(const void *const hw)
{
	return ((Tal *)hw)->CTRLA.reg;
}

static inline void hri_tal_set_RSTCTRL_reg(const void *const hw, hri_tal_rstctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->RSTCTRL.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_rstctrl_reg_t hri_tal_get_RSTCTRL_reg(const void *const hw, hri_tal_rstctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->RSTCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_RSTCTRL_reg(const void *const hw, hri_tal_rstctrl_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->RSTCTRL.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_RSTCTRL_reg(const void *const hw, hri_tal_rstctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->RSTCTRL.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_RSTCTRL_reg(const void *const hw, hri_tal_rstctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->RSTCTRL.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_rstctrl_reg_t hri_tal_read_RSTCTRL_reg(const void *const hw)
{
	return ((Tal *)hw)->RSTCTRL.reg;
}

static inline void hri_tal_set_EXTCTRL_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg |= TAL_EXTCTRL_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EXTCTRL_ENABLE_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->EXTCTRL.reg;
	tmp = (tmp & TAL_EXTCTRL_ENABLE) >> TAL_EXTCTRL_ENABLE_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EXTCTRL_ENABLE_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EXTCTRL.reg;
	tmp &= ~TAL_EXTCTRL_ENABLE;
	tmp |= value << TAL_EXTCTRL_ENABLE_Pos;
	((Tal *)hw)->EXTCTRL.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EXTCTRL_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg &= ~TAL_EXTCTRL_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EXTCTRL_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg ^= TAL_EXTCTRL_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EXTCTRL_INV_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg |= TAL_EXTCTRL_INV;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EXTCTRL_INV_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->EXTCTRL.reg;
	tmp = (tmp & TAL_EXTCTRL_INV) >> TAL_EXTCTRL_INV_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EXTCTRL_INV_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EXTCTRL.reg;
	tmp &= ~TAL_EXTCTRL_INV;
	tmp |= value << TAL_EXTCTRL_INV_Pos;
	((Tal *)hw)->EXTCTRL.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EXTCTRL_INV_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg &= ~TAL_EXTCTRL_INV;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EXTCTRL_INV_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg ^= TAL_EXTCTRL_INV;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EXTCTRL_reg(const void *const hw, hri_tal_extctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_extctrl_reg_t hri_tal_get_EXTCTRL_reg(const void *const hw, hri_tal_extctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->EXTCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_EXTCTRL_reg(const void *const hw, hri_tal_extctrl_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EXTCTRL_reg(const void *const hw, hri_tal_extctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EXTCTRL_reg(const void *const hw, hri_tal_extctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EXTCTRL.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_extctrl_reg_t hri_tal_read_EXTCTRL_reg(const void *const hw)
{
	return ((Tal *)hw)->EXTCTRL.reg;
}

static inline void hri_tal_set_EVCTRL_BRKEI_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg |= TAL_EVCTRL_BRKEI;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCTRL_BRKEI_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->EVCTRL.reg;
	tmp = (tmp & TAL_EVCTRL_BRKEI) >> TAL_EVCTRL_BRKEI_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCTRL_BRKEI_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCTRL.reg;
	tmp &= ~TAL_EVCTRL_BRKEI;
	tmp |= value << TAL_EVCTRL_BRKEI_Pos;
	((Tal *)hw)->EVCTRL.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCTRL_BRKEI_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg &= ~TAL_EVCTRL_BRKEI;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCTRL_BRKEI_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg ^= TAL_EVCTRL_BRKEI;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCTRL_BRKEO_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg |= TAL_EVCTRL_BRKEO;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCTRL_BRKEO_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->EVCTRL.reg;
	tmp = (tmp & TAL_EVCTRL_BRKEO) >> TAL_EVCTRL_BRKEO_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCTRL_BRKEO_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCTRL.reg;
	tmp &= ~TAL_EVCTRL_BRKEO;
	tmp |= value << TAL_EVCTRL_BRKEO_Pos;
	((Tal *)hw)->EVCTRL.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCTRL_BRKEO_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg &= ~TAL_EVCTRL_BRKEO;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCTRL_BRKEO_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg ^= TAL_EVCTRL_BRKEO;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCTRL_reg(const void *const hw, hri_tal_evctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_evctrl_reg_t hri_tal_get_EVCTRL_reg(const void *const hw, hri_tal_evctrl_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->EVCTRL.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_EVCTRL_reg(const void *const hw, hri_tal_evctrl_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCTRL_reg(const void *const hw, hri_tal_evctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCTRL_reg(const void *const hw, hri_tal_evctrl_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCTRL.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_evctrl_reg_t hri_tal_read_EVCTRL_reg(const void *const hw)
{
	return ((Tal *)hw)->EVCTRL.reg;
}

static inline void hri_tal_set_GLOBMASK_CM0P_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg |= TAL_GLOBMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_GLOBMASK_CM0P_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp = (tmp & TAL_GLOBMASK_CM0P) >> TAL_GLOBMASK_CM0P_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_GLOBMASK_CM0P_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp &= ~TAL_GLOBMASK_CM0P;
	tmp |= value << TAL_GLOBMASK_CM0P_Pos;
	((Tal *)hw)->GLOBMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_GLOBMASK_CM0P_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg &= ~TAL_GLOBMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_GLOBMASK_CM0P_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg ^= TAL_GLOBMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_GLOBMASK_PPP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg |= TAL_GLOBMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_GLOBMASK_PPP_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp = (tmp & TAL_GLOBMASK_PPP) >> TAL_GLOBMASK_PPP_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_GLOBMASK_PPP_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp &= ~TAL_GLOBMASK_PPP;
	tmp |= value << TAL_GLOBMASK_PPP_Pos;
	((Tal *)hw)->GLOBMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_GLOBMASK_PPP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg &= ~TAL_GLOBMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_GLOBMASK_PPP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg ^= TAL_GLOBMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_GLOBMASK_EVBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg |= TAL_GLOBMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_GLOBMASK_EVBRK_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp = (tmp & TAL_GLOBMASK_EVBRK) >> TAL_GLOBMASK_EVBRK_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_GLOBMASK_EVBRK_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp &= ~TAL_GLOBMASK_EVBRK;
	tmp |= value << TAL_GLOBMASK_EVBRK_Pos;
	((Tal *)hw)->GLOBMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_GLOBMASK_EVBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg &= ~TAL_GLOBMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_GLOBMASK_EVBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg ^= TAL_GLOBMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_GLOBMASK_EXTBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg |= TAL_GLOBMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_GLOBMASK_EXTBRK_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp = (tmp & TAL_GLOBMASK_EXTBRK) >> TAL_GLOBMASK_EXTBRK_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_GLOBMASK_EXTBRK_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp &= ~TAL_GLOBMASK_EXTBRK;
	tmp |= value << TAL_GLOBMASK_EXTBRK_Pos;
	((Tal *)hw)->GLOBMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_GLOBMASK_EXTBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg &= ~TAL_GLOBMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_GLOBMASK_EXTBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg ^= TAL_GLOBMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_GLOBMASK_reg(const void *const hw, hri_tal_globmask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_globmask_reg_t hri_tal_get_GLOBMASK_reg(const void *const hw, hri_tal_globmask_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->GLOBMASK.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_GLOBMASK_reg(const void *const hw, hri_tal_globmask_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_GLOBMASK_reg(const void *const hw, hri_tal_globmask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_GLOBMASK_reg(const void *const hw, hri_tal_globmask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->GLOBMASK.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_globmask_reg_t hri_tal_read_GLOBMASK_reg(const void *const hw)
{
	return ((Tal *)hw)->GLOBMASK.reg;
}

static inline void hri_tal_set_DMACPUSEL0_CH0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH0_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH0_Msk) >> TAL_DMACPUSEL0_CH0_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH0_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH0_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH0_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH1_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH1_Msk) >> TAL_DMACPUSEL0_CH1_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH1_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH1_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH1_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH2_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH2_Msk) >> TAL_DMACPUSEL0_CH2_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH2_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH2_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH2_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH3_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH3_Msk) >> TAL_DMACPUSEL0_CH3_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH3_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH3_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH3_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH4_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH4_Msk) >> TAL_DMACPUSEL0_CH4_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH4_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH4_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH4_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH5_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH5_Msk) >> TAL_DMACPUSEL0_CH5_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH5_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH5_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH5_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH6_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH6_Msk) >> TAL_DMACPUSEL0_CH6_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH6_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH6_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH6_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH7_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH7_Msk) >> TAL_DMACPUSEL0_CH7_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH7_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH7_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH7_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH8_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH8_Msk) >> TAL_DMACPUSEL0_CH8_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH8_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH8_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH8_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH9_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH9_Msk) >> TAL_DMACPUSEL0_CH9_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH9_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH9_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH9_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH10_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH10_Msk) >> TAL_DMACPUSEL0_CH10_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH10_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH10_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH10_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH11_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH11_Msk) >> TAL_DMACPUSEL0_CH11_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH11_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH11_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH11_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH12_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH12_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH12_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH12_Msk) >> TAL_DMACPUSEL0_CH12_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH12_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH12_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH12_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH12_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH12_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH12_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH12_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH13_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH13_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH13_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH13_Msk) >> TAL_DMACPUSEL0_CH13_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH13_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH13_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH13_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH13_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH13_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH13_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH13_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH14_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH14_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH14_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH14_Msk) >> TAL_DMACPUSEL0_CH14_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH14_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH14_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH14_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH14_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH14_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH14_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH14_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_CH15_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= TAL_DMACPUSEL0_CH15_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_DMACPUSEL0_CH15_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp = (tmp & TAL_DMACPUSEL0_CH15_Msk) >> TAL_DMACPUSEL0_CH15_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_DMACPUSEL0_CH15_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= ~TAL_DMACPUSEL0_CH15_Msk;
	tmp |= value << TAL_DMACPUSEL0_CH15_Pos;
	((Tal *)hw)->DMACPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_CH15_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~TAL_DMACPUSEL0_CH15_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_CH15_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= TAL_DMACPUSEL0_CH15_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_DMACPUSEL0_reg(const void *const hw, hri_tal_dmacpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_dmacpusel0_reg_t hri_tal_get_DMACPUSEL0_reg(const void *const hw, hri_tal_dmacpusel0_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->DMACPUSEL0.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_DMACPUSEL0_reg(const void *const hw, hri_tal_dmacpusel0_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_DMACPUSEL0_reg(const void *const hw, hri_tal_dmacpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_DMACPUSEL0_reg(const void *const hw, hri_tal_dmacpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->DMACPUSEL0.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_dmacpusel0_reg_t hri_tal_read_DMACPUSEL0_reg(const void *const hw)
{
	return ((Tal *)hw)->DMACPUSEL0.reg;
}

static inline void hri_tal_set_EVCPUSEL0_CH0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH0_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH0_Msk) >> TAL_EVCPUSEL0_CH0_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH0_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH0_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH0_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH1_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH1_Msk) >> TAL_EVCPUSEL0_CH1_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH1_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH1_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH1_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH2_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH2_Msk) >> TAL_EVCPUSEL0_CH2_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH2_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH2_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH2_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH3_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH3_Msk) >> TAL_EVCPUSEL0_CH3_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH3_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH3_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH3_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH4_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH4_Msk) >> TAL_EVCPUSEL0_CH4_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH4_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH4_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH4_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH5_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH5_Msk) >> TAL_EVCPUSEL0_CH5_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH5_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH5_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH5_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH6_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH6_Msk) >> TAL_EVCPUSEL0_CH6_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH6_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH6_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH6_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH7_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH7_Msk) >> TAL_EVCPUSEL0_CH7_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH7_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH7_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH7_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH8_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH8_Msk) >> TAL_EVCPUSEL0_CH8_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH8_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH8_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH8_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH9_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH9_Msk) >> TAL_EVCPUSEL0_CH9_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH9_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH9_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH9_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH10_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH10_Msk) >> TAL_EVCPUSEL0_CH10_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH10_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH10_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH10_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_CH11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= TAL_EVCPUSEL0_CH11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EVCPUSEL0_CH11_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp = (tmp & TAL_EVCPUSEL0_CH11_Msk) >> TAL_EVCPUSEL0_CH11_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EVCPUSEL0_CH11_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= ~TAL_EVCPUSEL0_CH11_Msk;
	tmp |= value << TAL_EVCPUSEL0_CH11_Pos;
	((Tal *)hw)->EVCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_CH11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~TAL_EVCPUSEL0_CH11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_CH11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= TAL_EVCPUSEL0_CH11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EVCPUSEL0_reg(const void *const hw, hri_tal_evcpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_evcpusel0_reg_t hri_tal_get_EVCPUSEL0_reg(const void *const hw, hri_tal_evcpusel0_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EVCPUSEL0.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_EVCPUSEL0_reg(const void *const hw, hri_tal_evcpusel0_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EVCPUSEL0_reg(const void *const hw, hri_tal_evcpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EVCPUSEL0_reg(const void *const hw, hri_tal_evcpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EVCPUSEL0.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_evcpusel0_reg_t hri_tal_read_EVCPUSEL0_reg(const void *const hw)
{
	return ((Tal *)hw)->EVCPUSEL0.reg;
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT0_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT0_Msk) >> TAL_EICCPUSEL0_EXTINT0_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT0_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT0_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT0_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT1_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT1_Msk) >> TAL_EICCPUSEL0_EXTINT1_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT1_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT1_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT1_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT2_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT2_Msk) >> TAL_EICCPUSEL0_EXTINT2_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT2_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT2_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT2_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT3_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT3_Msk) >> TAL_EICCPUSEL0_EXTINT3_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT3_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT3_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT3_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT4_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT4_Msk) >> TAL_EICCPUSEL0_EXTINT4_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT4_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT4_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT4_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT5_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT5_Msk) >> TAL_EICCPUSEL0_EXTINT5_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT5_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT5_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT5_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT6_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT6_Msk) >> TAL_EICCPUSEL0_EXTINT6_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT6_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT6_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT6_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT6_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT6_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT7_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT7_Msk) >> TAL_EICCPUSEL0_EXTINT7_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT7_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT7_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT7_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT7_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT7_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT8_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT8_Msk) >> TAL_EICCPUSEL0_EXTINT8_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT8_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT8_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT8_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT8_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT8_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT9_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT9_Msk) >> TAL_EICCPUSEL0_EXTINT9_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT9_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT9_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT9_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT9_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT9_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT10_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT10_Msk) >> TAL_EICCPUSEL0_EXTINT10_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT10_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT10_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT10_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT10_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT10_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT11_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT11_Msk) >> TAL_EICCPUSEL0_EXTINT11_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT11_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT11_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT11_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT11_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT11_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT12_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT12_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT12_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT12_Msk) >> TAL_EICCPUSEL0_EXTINT12_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT12_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT12_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT12_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT12_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT12_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT12_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT12_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT13_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT13_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT13_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT13_Msk) >> TAL_EICCPUSEL0_EXTINT13_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT13_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT13_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT13_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT13_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT13_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT13_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT13_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT14_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT14_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT14_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT14_Msk) >> TAL_EICCPUSEL0_EXTINT14_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT14_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT14_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT14_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT14_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT14_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT14_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT14_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_EXTINT15_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= TAL_EICCPUSEL0_EXTINT15_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_EICCPUSEL0_EXTINT15_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp = (tmp & TAL_EICCPUSEL0_EXTINT15_Msk) >> TAL_EICCPUSEL0_EXTINT15_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_EICCPUSEL0_EXTINT15_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= ~TAL_EICCPUSEL0_EXTINT15_Msk;
	tmp |= value << TAL_EICCPUSEL0_EXTINT15_Pos;
	((Tal *)hw)->EICCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_EXTINT15_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~TAL_EICCPUSEL0_EXTINT15_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_EXTINT15_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= TAL_EICCPUSEL0_EXTINT15_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_EICCPUSEL0_reg(const void *const hw, hri_tal_eiccpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_eiccpusel0_reg_t hri_tal_get_EICCPUSEL0_reg(const void *const hw, hri_tal_eiccpusel0_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->EICCPUSEL0.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_EICCPUSEL0_reg(const void *const hw, hri_tal_eiccpusel0_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_EICCPUSEL0_reg(const void *const hw, hri_tal_eiccpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_EICCPUSEL0_reg(const void *const hw, hri_tal_eiccpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->EICCPUSEL0.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_eiccpusel0_reg_t hri_tal_read_EICCPUSEL0_reg(const void *const hw)
{
	return ((Tal *)hw)->EICCPUSEL0.reg;
}

static inline void hri_tal_set_INTCPUSEL0_SYSTEM_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_SYSTEM_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_SYSTEM_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_SYSTEM_Msk) >> TAL_INTCPUSEL0_SYSTEM_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_SYSTEM_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_SYSTEM_Msk;
	tmp |= value << TAL_INTCPUSEL0_SYSTEM_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_SYSTEM_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_SYSTEM_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_SYSTEM_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_SYSTEM_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_WDT_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_WDT_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_WDT_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_WDT_Msk) >> TAL_INTCPUSEL0_WDT_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_WDT_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_WDT_Msk;
	tmp |= value << TAL_INTCPUSEL0_WDT_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_WDT_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_WDT_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_WDT_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_WDT_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_RTC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_RTC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_RTC_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_RTC_Msk) >> TAL_INTCPUSEL0_RTC_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_RTC_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_RTC_Msk;
	tmp |= value << TAL_INTCPUSEL0_RTC_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_RTC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_RTC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_RTC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_RTC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_NVMCTRL_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_NVMCTRL_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_NVMCTRL_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_NVMCTRL_Msk) >> TAL_INTCPUSEL0_NVMCTRL_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_NVMCTRL_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_NVMCTRL_Msk;
	tmp |= value << TAL_INTCPUSEL0_NVMCTRL_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_NVMCTRL_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_NVMCTRL_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_NVMCTRL_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_NVMCTRL_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_USB_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_USB_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_USB_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_USB_Msk) >> TAL_INTCPUSEL0_USB_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_USB_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_USB_Msk;
	tmp |= value << TAL_INTCPUSEL0_USB_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_USB_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_USB_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_USB_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_USB_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_SERCOM0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_SERCOM0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_SERCOM0_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_SERCOM0_Msk) >> TAL_INTCPUSEL0_SERCOM0_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_SERCOM0_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_SERCOM0_Msk;
	tmp |= value << TAL_INTCPUSEL0_SERCOM0_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_SERCOM0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_SERCOM0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_SERCOM0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_SERCOM0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_SERCOM1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_SERCOM1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_SERCOM1_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_SERCOM1_Msk) >> TAL_INTCPUSEL0_SERCOM1_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_SERCOM1_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_SERCOM1_Msk;
	tmp |= value << TAL_INTCPUSEL0_SERCOM1_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_SERCOM1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_SERCOM1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_SERCOM1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_SERCOM1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_SERCOM2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_SERCOM2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_SERCOM2_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_SERCOM2_Msk) >> TAL_INTCPUSEL0_SERCOM2_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_SERCOM2_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_SERCOM2_Msk;
	tmp |= value << TAL_INTCPUSEL0_SERCOM2_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_SERCOM2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_SERCOM2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_SERCOM2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_SERCOM2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_SERCOM3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_SERCOM3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_SERCOM3_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_SERCOM3_Msk) >> TAL_INTCPUSEL0_SERCOM3_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_SERCOM3_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_SERCOM3_Msk;
	tmp |= value << TAL_INTCPUSEL0_SERCOM3_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_SERCOM3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_SERCOM3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_SERCOM3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_SERCOM3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_SERCOM4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_SERCOM4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_SERCOM4_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_SERCOM4_Msk) >> TAL_INTCPUSEL0_SERCOM4_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_SERCOM4_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_SERCOM4_Msk;
	tmp |= value << TAL_INTCPUSEL0_SERCOM4_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_SERCOM4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_SERCOM4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_SERCOM4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_SERCOM4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_SERCOM5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_SERCOM5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_SERCOM5_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_SERCOM5_Msk) >> TAL_INTCPUSEL0_SERCOM5_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_SERCOM5_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_SERCOM5_Msk;
	tmp |= value << TAL_INTCPUSEL0_SERCOM5_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_SERCOM5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_SERCOM5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_SERCOM5_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_SERCOM5_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_TCC0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_TCC0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_TCC0_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_TCC0_Msk) >> TAL_INTCPUSEL0_TCC0_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_TCC0_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_TCC0_Msk;
	tmp |= value << TAL_INTCPUSEL0_TCC0_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_TCC0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_TCC0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_TCC0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_TCC0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_TCC1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= TAL_INTCPUSEL0_TCC1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL0_TCC1_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp = (tmp & TAL_INTCPUSEL0_TCC1_Msk) >> TAL_INTCPUSEL0_TCC1_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL0_TCC1_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= ~TAL_INTCPUSEL0_TCC1_Msk;
	tmp |= value << TAL_INTCPUSEL0_TCC1_Pos;
	((Tal *)hw)->INTCPUSEL0.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_TCC1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~TAL_INTCPUSEL0_TCC1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_TCC1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= TAL_INTCPUSEL0_TCC1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL0_reg(const void *const hw, hri_tal_intcpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_intcpusel0_reg_t hri_tal_get_INTCPUSEL0_reg(const void *const hw, hri_tal_intcpusel0_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL0.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_INTCPUSEL0_reg(const void *const hw, hri_tal_intcpusel0_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL0_reg(const void *const hw, hri_tal_intcpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL0_reg(const void *const hw, hri_tal_intcpusel0_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL0.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_intcpusel0_reg_t hri_tal_read_INTCPUSEL0_reg(const void *const hw)
{
	return ((Tal *)hw)->INTCPUSEL0.reg;
}

static inline void hri_tal_set_INTCPUSEL1_TCC2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_TCC2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_TCC2_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_TCC2_Msk) >> TAL_INTCPUSEL1_TCC2_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_TCC2_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_TCC2_Msk;
	tmp |= value << TAL_INTCPUSEL1_TCC2_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_TCC2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_TCC2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_TCC2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_TCC2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_TC0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_TC0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_TC0_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_TC0_Msk) >> TAL_INTCPUSEL1_TC0_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_TC0_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_TC0_Msk;
	tmp |= value << TAL_INTCPUSEL1_TC0_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_TC0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_TC0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_TC0_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_TC0_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_TC1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_TC1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_TC1_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_TC1_Msk) >> TAL_INTCPUSEL1_TC1_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_TC1_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_TC1_Msk;
	tmp |= value << TAL_INTCPUSEL1_TC1_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_TC1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_TC1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_TC1_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_TC1_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_TC2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_TC2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_TC2_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_TC2_Msk) >> TAL_INTCPUSEL1_TC2_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_TC2_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_TC2_Msk;
	tmp |= value << TAL_INTCPUSEL1_TC2_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_TC2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_TC2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_TC2_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_TC2_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_TC3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_TC3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_TC3_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_TC3_Msk) >> TAL_INTCPUSEL1_TC3_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_TC3_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_TC3_Msk;
	tmp |= value << TAL_INTCPUSEL1_TC3_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_TC3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_TC3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_TC3_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_TC3_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_TC4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_TC4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_TC4_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_TC4_Msk) >> TAL_INTCPUSEL1_TC4_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_TC4_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_TC4_Msk;
	tmp |= value << TAL_INTCPUSEL1_TC4_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_TC4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_TC4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_TC4_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_TC4_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_ADC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_ADC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_ADC_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_ADC_Msk) >> TAL_INTCPUSEL1_ADC_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_ADC_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_ADC_Msk;
	tmp |= value << TAL_INTCPUSEL1_ADC_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_ADC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_ADC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_ADC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_ADC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_AC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_AC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_AC_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_AC_Msk) >> TAL_INTCPUSEL1_AC_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_AC_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_AC_Msk;
	tmp |= value << TAL_INTCPUSEL1_AC_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_AC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_AC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_AC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_AC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_DAC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_DAC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_DAC_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_DAC_Msk) >> TAL_INTCPUSEL1_DAC_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_DAC_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_DAC_Msk;
	tmp |= value << TAL_INTCPUSEL1_DAC_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_DAC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_DAC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_DAC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_DAC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_PTC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_PTC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_PTC_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_PTC_Msk) >> TAL_INTCPUSEL1_PTC_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_PTC_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_PTC_Msk;
	tmp |= value << TAL_INTCPUSEL1_PTC_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_PTC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_PTC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_PTC_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_PTC_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_AES_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_AES_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_AES_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_AES_Msk) >> TAL_INTCPUSEL1_AES_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_AES_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_AES_Msk;
	tmp |= value << TAL_INTCPUSEL1_AES_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_AES_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_AES_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_AES_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_AES_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_TRNG_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_TRNG_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_TRNG_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_TRNG_Msk) >> TAL_INTCPUSEL1_TRNG_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_TRNG_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_TRNG_Msk;
	tmp |= value << TAL_INTCPUSEL1_TRNG_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_TRNG_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_TRNG_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_TRNG_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_TRNG_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_PICOP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= TAL_INTCPUSEL1_PICOP_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_INTCPUSEL1_PICOP_bit(const void *const hw)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp = (tmp & TAL_INTCPUSEL1_PICOP_Msk) >> TAL_INTCPUSEL1_PICOP_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_INTCPUSEL1_PICOP_bit(const void *const hw, bool value)
{
	uint32_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= ~TAL_INTCPUSEL1_PICOP_Msk;
	tmp |= value << TAL_INTCPUSEL1_PICOP_Pos;
	((Tal *)hw)->INTCPUSEL1.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_PICOP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~TAL_INTCPUSEL1_PICOP_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_PICOP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= TAL_INTCPUSEL1_PICOP_Msk;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_INTCPUSEL1_reg(const void *const hw, hri_tal_intcpusel1_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_intcpusel1_reg_t hri_tal_get_INTCPUSEL1_reg(const void *const hw, hri_tal_intcpusel1_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->INTCPUSEL1.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_INTCPUSEL1_reg(const void *const hw, hri_tal_intcpusel1_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_INTCPUSEL1_reg(const void *const hw, hri_tal_intcpusel1_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_INTCPUSEL1_reg(const void *const hw, hri_tal_intcpusel1_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->INTCPUSEL1.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_intcpusel1_reg_t hri_tal_read_INTCPUSEL1_reg(const void *const hw)
{
	return ((Tal *)hw)->INTCPUSEL1.reg;
}

static inline void hri_tal_set_IRQTRIG_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg |= TAL_IRQTRIG_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_IRQTRIG_ENABLE_bit(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp = (tmp & TAL_IRQTRIG_ENABLE) >> TAL_IRQTRIG_ENABLE_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_IRQTRIG_ENABLE_bit(const void *const hw, bool value)
{
	uint16_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp &= ~TAL_IRQTRIG_ENABLE;
	tmp |= value << TAL_IRQTRIG_ENABLE_Pos;
	((Tal *)hw)->IRQTRIG.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_IRQTRIG_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg &= ~TAL_IRQTRIG_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_IRQTRIG_ENABLE_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg ^= TAL_IRQTRIG_ENABLE;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_IRQTRIG_IRQNUM_bf(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg |= TAL_IRQTRIG_IRQNUM(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_irqtrig_reg_t hri_tal_get_IRQTRIG_IRQNUM_bf(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp = (tmp & TAL_IRQTRIG_IRQNUM(mask)) >> TAL_IRQTRIG_IRQNUM_Pos;
	return tmp;
}

static inline void hri_tal_write_IRQTRIG_IRQNUM_bf(const void *const hw, hri_tal_irqtrig_reg_t data)
{
	uint16_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp &= ~TAL_IRQTRIG_IRQNUM_Msk;
	tmp |= TAL_IRQTRIG_IRQNUM(data);
	((Tal *)hw)->IRQTRIG.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_IRQTRIG_IRQNUM_bf(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg &= ~TAL_IRQTRIG_IRQNUM(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_IRQTRIG_IRQNUM_bf(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg ^= TAL_IRQTRIG_IRQNUM(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_irqtrig_reg_t hri_tal_read_IRQTRIG_IRQNUM_bf(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp = (tmp & TAL_IRQTRIG_IRQNUM_Msk) >> TAL_IRQTRIG_IRQNUM_Pos;
	return tmp;
}

static inline void hri_tal_set_IRQTRIG_OVERRIDE_bf(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg |= TAL_IRQTRIG_OVERRIDE(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_irqtrig_reg_t hri_tal_get_IRQTRIG_OVERRIDE_bf(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp = (tmp & TAL_IRQTRIG_OVERRIDE(mask)) >> TAL_IRQTRIG_OVERRIDE_Pos;
	return tmp;
}

static inline void hri_tal_write_IRQTRIG_OVERRIDE_bf(const void *const hw, hri_tal_irqtrig_reg_t data)
{
	uint16_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp &= ~TAL_IRQTRIG_OVERRIDE_Msk;
	tmp |= TAL_IRQTRIG_OVERRIDE(data);
	((Tal *)hw)->IRQTRIG.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_IRQTRIG_OVERRIDE_bf(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg &= ~TAL_IRQTRIG_OVERRIDE(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_IRQTRIG_OVERRIDE_bf(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg ^= TAL_IRQTRIG_OVERRIDE(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_irqtrig_reg_t hri_tal_read_IRQTRIG_OVERRIDE_bf(const void *const hw)
{
	uint16_t tmp;
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp = (tmp & TAL_IRQTRIG_OVERRIDE_Msk) >> TAL_IRQTRIG_OVERRIDE_Pos;
	return tmp;
}

static inline void hri_tal_set_IRQTRIG_reg(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_irqtrig_reg_t hri_tal_get_IRQTRIG_reg(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Tal *)hw)->IRQTRIG.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_IRQTRIG_reg(const void *const hw, hri_tal_irqtrig_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_IRQTRIG_reg(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_IRQTRIG_reg(const void *const hw, hri_tal_irqtrig_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->IRQTRIG.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_irqtrig_reg_t hri_tal_read_IRQTRIG_reg(const void *const hw)
{
	return ((Tal *)hw)->IRQTRIG.reg;
}

static inline hri_tal_brkstatus_reg_t hri_tal_get_BRKSTATUS_CM0P_bf(const void *const hw, hri_tal_brkstatus_reg_t mask)
{
	return (((Tal *)hw)->BRKSTATUS.reg & TAL_BRKSTATUS_CM0P(mask)) >> TAL_BRKSTATUS_CM0P_Pos;
}

static inline hri_tal_brkstatus_reg_t hri_tal_read_BRKSTATUS_CM0P_bf(const void *const hw)
{
	return (((Tal *)hw)->BRKSTATUS.reg & TAL_BRKSTATUS_CM0P_Msk) >> TAL_BRKSTATUS_CM0P_Pos;
}

static inline hri_tal_brkstatus_reg_t hri_tal_get_BRKSTATUS_PPP_bf(const void *const hw, hri_tal_brkstatus_reg_t mask)
{
	return (((Tal *)hw)->BRKSTATUS.reg & TAL_BRKSTATUS_PPP(mask)) >> TAL_BRKSTATUS_PPP_Pos;
}

static inline hri_tal_brkstatus_reg_t hri_tal_read_BRKSTATUS_PPP_bf(const void *const hw)
{
	return (((Tal *)hw)->BRKSTATUS.reg & TAL_BRKSTATUS_PPP_Msk) >> TAL_BRKSTATUS_PPP_Pos;
}

static inline hri_tal_brkstatus_reg_t hri_tal_get_BRKSTATUS_EVBRK_bf(const void *const hw, hri_tal_brkstatus_reg_t mask)
{
	return (((Tal *)hw)->BRKSTATUS.reg & TAL_BRKSTATUS_EVBRK(mask)) >> TAL_BRKSTATUS_EVBRK_Pos;
}

static inline hri_tal_brkstatus_reg_t hri_tal_read_BRKSTATUS_EVBRK_bf(const void *const hw)
{
	return (((Tal *)hw)->BRKSTATUS.reg & TAL_BRKSTATUS_EVBRK_Msk) >> TAL_BRKSTATUS_EVBRK_Pos;
}

static inline hri_tal_brkstatus_reg_t hri_tal_get_BRKSTATUS_EXTBRK_bf(const void *const       hw,
                                                                      hri_tal_brkstatus_reg_t mask)
{
	return (((Tal *)hw)->BRKSTATUS.reg & TAL_BRKSTATUS_EXTBRK(mask)) >> TAL_BRKSTATUS_EXTBRK_Pos;
}

static inline hri_tal_brkstatus_reg_t hri_tal_read_BRKSTATUS_EXTBRK_bf(const void *const hw)
{
	return (((Tal *)hw)->BRKSTATUS.reg & TAL_BRKSTATUS_EXTBRK_Msk) >> TAL_BRKSTATUS_EXTBRK_Pos;
}

static inline hri_tal_brkstatus_reg_t hri_tal_get_BRKSTATUS_reg(const void *const hw, hri_tal_brkstatus_reg_t mask)
{
	uint16_t tmp;
	tmp = ((Tal *)hw)->BRKSTATUS.reg;
	tmp &= mask;
	return tmp;
}

static inline hri_tal_brkstatus_reg_t hri_tal_read_BRKSTATUS_reg(const void *const hw)
{
	return ((Tal *)hw)->BRKSTATUS.reg;
}

static inline bool hri_tal_get_INTSTATUS_IRQ0_bit(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->INTSTATUS[index].reg & TAL_INTSTATUS_IRQ0) >> TAL_INTSTATUS_IRQ0_Pos;
}

static inline bool hri_tal_get_INTSTATUS_IRQ1_bit(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->INTSTATUS[index].reg & TAL_INTSTATUS_IRQ1) >> TAL_INTSTATUS_IRQ1_Pos;
}

static inline bool hri_tal_get_INTSTATUS_IRQ2_bit(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->INTSTATUS[index].reg & TAL_INTSTATUS_IRQ2) >> TAL_INTSTATUS_IRQ2_Pos;
}

static inline bool hri_tal_get_INTSTATUS_IRQ3_bit(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->INTSTATUS[index].reg & TAL_INTSTATUS_IRQ3) >> TAL_INTSTATUS_IRQ3_Pos;
}

static inline bool hri_tal_get_INTSTATUS_IRQ4_bit(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->INTSTATUS[index].reg & TAL_INTSTATUS_IRQ4) >> TAL_INTSTATUS_IRQ4_Pos;
}

static inline bool hri_tal_get_INTSTATUS_IRQ5_bit(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->INTSTATUS[index].reg & TAL_INTSTATUS_IRQ5) >> TAL_INTSTATUS_IRQ5_Pos;
}

static inline bool hri_tal_get_INTSTATUS_IRQ6_bit(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->INTSTATUS[index].reg & TAL_INTSTATUS_IRQ6) >> TAL_INTSTATUS_IRQ6_Pos;
}

static inline bool hri_tal_get_INTSTATUS_IRQ7_bit(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->INTSTATUS[index].reg & TAL_INTSTATUS_IRQ7) >> TAL_INTSTATUS_IRQ7_Pos;
}

static inline hri_tal_intstatus_reg_t hri_tal_get_INTSTATUS_reg(const void *const hw, uint8_t index,
                                                                hri_tal_intstatus_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->INTSTATUS[index].reg;
	tmp &= mask;
	return tmp;
}

static inline hri_tal_intstatus_reg_t hri_tal_read_INTSTATUS_reg(const void *const hw, uint8_t index)
{
	return ((Tal *)hw)->INTSTATUS[index].reg;
}

static inline hri_tal_cpuirqs_reg_t hri_tal_get_CPUIRQS_CPUIRQS_bf(const void *const hw, uint8_t index,
                                                                   hri_tal_cpuirqs_reg_t mask)
{
	return (((Tal *)hw)->CPUIRQS[index].reg & TAL_CPUIRQS_CPUIRQS(mask)) >> TAL_CPUIRQS_CPUIRQS_Pos;
}

static inline hri_tal_cpuirqs_reg_t hri_tal_read_CPUIRQS_CPUIRQS_bf(const void *const hw, uint8_t index)
{
	return (((Tal *)hw)->CPUIRQS[index].reg & TAL_CPUIRQS_CPUIRQS_Msk) >> TAL_CPUIRQS_CPUIRQS_Pos;
}

static inline hri_tal_cpuirqs_reg_t hri_tal_get_CPUIRQS_reg(const void *const hw, uint8_t index,
                                                            hri_tal_cpuirqs_reg_t mask)
{
	uint32_t tmp;
	tmp = ((Tal *)hw)->CPUIRQS[index].reg;
	tmp &= mask;
	return tmp;
}

static inline hri_tal_cpuirqs_reg_t hri_tal_read_CPUIRQS_reg(const void *const hw, uint8_t index)
{
	return ((Tal *)hw)->CPUIRQS[index].reg;
}

static inline void hri_talctis_set_CTICTRLA_RESTART_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg |= TAL_CTICTRLA_RESTART;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_talctis_get_CTICTRLA_RESTART_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTICTRLA.reg;
	tmp = (tmp & TAL_CTICTRLA_RESTART) >> TAL_CTICTRLA_RESTART_Pos;
	return (bool)tmp;
}

static inline void hri_talctis_write_CTICTRLA_RESTART_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((TalCtis *)hw)->CTICTRLA.reg;
	tmp &= ~TAL_CTICTRLA_RESTART;
	tmp |= value << TAL_CTICTRLA_RESTART_Pos;
	((TalCtis *)hw)->CTICTRLA.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_clear_CTICTRLA_RESTART_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg &= ~TAL_CTICTRLA_RESTART;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_toggle_CTICTRLA_RESTART_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg ^= TAL_CTICTRLA_RESTART;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_set_CTICTRLA_ACTION_bf(const void *const hw, hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg |= TAL_CTICTRLA_ACTION(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctictrla_reg_t hri_talctis_get_CTICTRLA_ACTION_bf(const void *const      hw,
                                                                        hri_tal_ctictrla_reg_t mask)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTICTRLA.reg;
	tmp = (tmp & TAL_CTICTRLA_ACTION(mask)) >> TAL_CTICTRLA_ACTION_Pos;
	return tmp;
}

static inline void hri_talctis_write_CTICTRLA_ACTION_bf(const void *const hw, hri_tal_ctictrla_reg_t data)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((TalCtis *)hw)->CTICTRLA.reg;
	tmp &= ~TAL_CTICTRLA_ACTION_Msk;
	tmp |= TAL_CTICTRLA_ACTION(data);
	((TalCtis *)hw)->CTICTRLA.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_clear_CTICTRLA_ACTION_bf(const void *const hw, hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg &= ~TAL_CTICTRLA_ACTION(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_toggle_CTICTRLA_ACTION_bf(const void *const hw, hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg ^= TAL_CTICTRLA_ACTION(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctictrla_reg_t hri_talctis_read_CTICTRLA_ACTION_bf(const void *const hw)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTICTRLA.reg;
	tmp = (tmp & TAL_CTICTRLA_ACTION_Msk) >> TAL_CTICTRLA_ACTION_Pos;
	return tmp;
}

static inline void hri_talctis_set_CTICTRLA_reg(const void *const hw, hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctictrla_reg_t hri_talctis_get_CTICTRLA_reg(const void *const hw, hri_tal_ctictrla_reg_t mask)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTICTRLA.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_talctis_write_CTICTRLA_reg(const void *const hw, hri_tal_ctictrla_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_clear_CTICTRLA_reg(const void *const hw, hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_toggle_CTICTRLA_reg(const void *const hw, hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTICTRLA.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctictrla_reg_t hri_talctis_read_CTICTRLA_reg(const void *const hw)
{
	return ((TalCtis *)hw)->CTICTRLA.reg;
}

static inline void hri_talctis_set_CTIMASK_CM0P_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg |= TAL_CTIMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_talctis_get_CTIMASK_CM0P_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp = (tmp & TAL_CTIMASK_CM0P) >> TAL_CTIMASK_CM0P_Pos;
	return (bool)tmp;
}

static inline void hri_talctis_write_CTIMASK_CM0P_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp &= ~TAL_CTIMASK_CM0P;
	tmp |= value << TAL_CTIMASK_CM0P_Pos;
	((TalCtis *)hw)->CTIMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_clear_CTIMASK_CM0P_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg &= ~TAL_CTIMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_toggle_CTIMASK_CM0P_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg ^= TAL_CTIMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_set_CTIMASK_PPP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg |= TAL_CTIMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_talctis_get_CTIMASK_PPP_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp = (tmp & TAL_CTIMASK_PPP) >> TAL_CTIMASK_PPP_Pos;
	return (bool)tmp;
}

static inline void hri_talctis_write_CTIMASK_PPP_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp &= ~TAL_CTIMASK_PPP;
	tmp |= value << TAL_CTIMASK_PPP_Pos;
	((TalCtis *)hw)->CTIMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_clear_CTIMASK_PPP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg &= ~TAL_CTIMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_toggle_CTIMASK_PPP_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg ^= TAL_CTIMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_set_CTIMASK_EVBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg |= TAL_CTIMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_talctis_get_CTIMASK_EVBRK_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp = (tmp & TAL_CTIMASK_EVBRK) >> TAL_CTIMASK_EVBRK_Pos;
	return (bool)tmp;
}

static inline void hri_talctis_write_CTIMASK_EVBRK_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp &= ~TAL_CTIMASK_EVBRK;
	tmp |= value << TAL_CTIMASK_EVBRK_Pos;
	((TalCtis *)hw)->CTIMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_clear_CTIMASK_EVBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg &= ~TAL_CTIMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_toggle_CTIMASK_EVBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg ^= TAL_CTIMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_set_CTIMASK_EXTBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg |= TAL_CTIMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_talctis_get_CTIMASK_EXTBRK_bit(const void *const hw)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp = (tmp & TAL_CTIMASK_EXTBRK) >> TAL_CTIMASK_EXTBRK_Pos;
	return (bool)tmp;
}

static inline void hri_talctis_write_CTIMASK_EXTBRK_bit(const void *const hw, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp &= ~TAL_CTIMASK_EXTBRK;
	tmp |= value << TAL_CTIMASK_EXTBRK_Pos;
	((TalCtis *)hw)->CTIMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_clear_CTIMASK_EXTBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg &= ~TAL_CTIMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_toggle_CTIMASK_EXTBRK_bit(const void *const hw)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg ^= TAL_CTIMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_set_CTIMASK_reg(const void *const hw, hri_tal_ctimask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctimask_reg_t hri_talctis_get_CTIMASK_reg(const void *const hw, hri_tal_ctimask_reg_t mask)
{
	uint8_t tmp;
	tmp = ((TalCtis *)hw)->CTIMASK.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_talctis_write_CTIMASK_reg(const void *const hw, hri_tal_ctimask_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_clear_CTIMASK_reg(const void *const hw, hri_tal_ctimask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_talctis_toggle_CTIMASK_reg(const void *const hw, hri_tal_ctimask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((TalCtis *)hw)->CTIMASK.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctimask_reg_t hri_talctis_read_CTIMASK_reg(const void *const hw)
{
	return ((TalCtis *)hw)->CTIMASK.reg;
}

static inline void hri_tal_set_CTICTRLA_RESTART_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg |= TAL_CTICTRLA_RESTART;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_CTICTRLA_RESTART_bit(const void *const hw, uint8_t submodule_index)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg;
	tmp = (tmp & TAL_CTICTRLA_RESTART) >> TAL_CTICTRLA_RESTART_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_CTICTRLA_RESTART_bit(const void *const hw, uint8_t submodule_index, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg;
	tmp &= ~TAL_CTICTRLA_RESTART;
	tmp |= value << TAL_CTICTRLA_RESTART_Pos;
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTICTRLA_RESTART_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg &= ~TAL_CTICTRLA_RESTART;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTICTRLA_RESTART_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg ^= TAL_CTICTRLA_RESTART;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_CTICTRLA_ACTION_bf(const void *const hw, uint8_t submodule_index,
                                                  hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg |= TAL_CTICTRLA_ACTION(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctictrla_reg_t hri_tal_get_CTICTRLA_ACTION_bf(const void *const hw, uint8_t submodule_index,
                                                                    hri_tal_ctictrla_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg;
	tmp = (tmp & TAL_CTICTRLA_ACTION(mask)) >> TAL_CTICTRLA_ACTION_Pos;
	return tmp;
}

static inline void hri_tal_write_CTICTRLA_ACTION_bf(const void *const hw, uint8_t submodule_index,
                                                    hri_tal_ctictrla_reg_t data)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg;
	tmp &= ~TAL_CTICTRLA_ACTION_Msk;
	tmp |= TAL_CTICTRLA_ACTION(data);
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTICTRLA_ACTION_bf(const void *const hw, uint8_t submodule_index,
                                                    hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg &= ~TAL_CTICTRLA_ACTION(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTICTRLA_ACTION_bf(const void *const hw, uint8_t submodule_index,
                                                     hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg ^= TAL_CTICTRLA_ACTION(mask);
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctictrla_reg_t hri_tal_read_CTICTRLA_ACTION_bf(const void *const hw, uint8_t submodule_index)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg;
	tmp = (tmp & TAL_CTICTRLA_ACTION_Msk) >> TAL_CTICTRLA_ACTION_Pos;
	return tmp;
}

static inline void hri_tal_set_CTICTRLA_reg(const void *const hw, uint8_t submodule_index, hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctictrla_reg_t hri_tal_get_CTICTRLA_reg(const void *const hw, uint8_t submodule_index,
                                                              hri_tal_ctictrla_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_CTICTRLA_reg(const void *const hw, uint8_t submodule_index,
                                              hri_tal_ctictrla_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTICTRLA_reg(const void *const hw, uint8_t submodule_index,
                                              hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTICTRLA_reg(const void *const hw, uint8_t submodule_index,
                                               hri_tal_ctictrla_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctictrla_reg_t hri_tal_read_CTICTRLA_reg(const void *const hw, uint8_t submodule_index)
{
	return ((Tal *)hw)->Ctis[submodule_index].CTICTRLA.reg;
}

static inline void hri_tal_set_CTIMASK_CM0P_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg |= TAL_CTIMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_CTIMASK_CM0P_bit(const void *const hw, uint8_t submodule_index)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp = (tmp & TAL_CTIMASK_CM0P) >> TAL_CTIMASK_CM0P_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_CTIMASK_CM0P_bit(const void *const hw, uint8_t submodule_index, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp &= ~TAL_CTIMASK_CM0P;
	tmp |= value << TAL_CTIMASK_CM0P_Pos;
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTIMASK_CM0P_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg &= ~TAL_CTIMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTIMASK_CM0P_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg ^= TAL_CTIMASK_CM0P;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_CTIMASK_PPP_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg |= TAL_CTIMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_CTIMASK_PPP_bit(const void *const hw, uint8_t submodule_index)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp = (tmp & TAL_CTIMASK_PPP) >> TAL_CTIMASK_PPP_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_CTIMASK_PPP_bit(const void *const hw, uint8_t submodule_index, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp &= ~TAL_CTIMASK_PPP;
	tmp |= value << TAL_CTIMASK_PPP_Pos;
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTIMASK_PPP_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg &= ~TAL_CTIMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTIMASK_PPP_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg ^= TAL_CTIMASK_PPP;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_CTIMASK_EVBRK_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg |= TAL_CTIMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_CTIMASK_EVBRK_bit(const void *const hw, uint8_t submodule_index)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp = (tmp & TAL_CTIMASK_EVBRK) >> TAL_CTIMASK_EVBRK_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_CTIMASK_EVBRK_bit(const void *const hw, uint8_t submodule_index, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp &= ~TAL_CTIMASK_EVBRK;
	tmp |= value << TAL_CTIMASK_EVBRK_Pos;
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTIMASK_EVBRK_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg &= ~TAL_CTIMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTIMASK_EVBRK_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg ^= TAL_CTIMASK_EVBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_CTIMASK_EXTBRK_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg |= TAL_CTIMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline bool hri_tal_get_CTIMASK_EXTBRK_bit(const void *const hw, uint8_t submodule_index)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp = (tmp & TAL_CTIMASK_EXTBRK) >> TAL_CTIMASK_EXTBRK_Pos;
	return (bool)tmp;
}

static inline void hri_tal_write_CTIMASK_EXTBRK_bit(const void *const hw, uint8_t submodule_index, bool value)
{
	uint8_t tmp;
	TAL_CRITICAL_SECTION_ENTER();
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp &= ~TAL_CTIMASK_EXTBRK;
	tmp |= value << TAL_CTIMASK_EXTBRK_Pos;
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg = tmp;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTIMASK_EXTBRK_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg &= ~TAL_CTIMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTIMASK_EXTBRK_bit(const void *const hw, uint8_t submodule_index)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg ^= TAL_CTIMASK_EXTBRK;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_set_CTIMASK_reg(const void *const hw, uint8_t submodule_index, hri_tal_ctimask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg |= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctimask_reg_t hri_tal_get_CTIMASK_reg(const void *const hw, uint8_t submodule_index,
                                                            hri_tal_ctimask_reg_t mask)
{
	uint8_t tmp;
	tmp = ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
	tmp &= mask;
	return tmp;
}

static inline void hri_tal_write_CTIMASK_reg(const void *const hw, uint8_t submodule_index, hri_tal_ctimask_reg_t data)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg = data;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_clear_CTIMASK_reg(const void *const hw, uint8_t submodule_index, hri_tal_ctimask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg &= ~mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline void hri_tal_toggle_CTIMASK_reg(const void *const hw, uint8_t submodule_index, hri_tal_ctimask_reg_t mask)
{
	TAL_CRITICAL_SECTION_ENTER();
	((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg ^= mask;
	TAL_CRITICAL_SECTION_LEAVE();
}

static inline hri_tal_ctimask_reg_t hri_tal_read_CTIMASK_reg(const void *const hw, uint8_t submodule_index)
{
	return ((Tal *)hw)->Ctis[submodule_index].CTIMASK.reg;
}

#ifdef __cplusplus
}
#endif

#endif /* _HRI_TAL_L21_H_INCLUDED */
#endif /* _SAML21_TAL_COMPONENT_ */
