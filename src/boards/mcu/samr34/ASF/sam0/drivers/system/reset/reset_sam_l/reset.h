/**
 * \file
 *
 * \brief SAM Reset functionality
 *
 * Copyright (c) 2014-2020 Microchip Technology Inc. and its subsidiaries.
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
#ifndef RESET_H_INCLUDED
#define RESET_H_INCLUDED

#include <compiler.h>
#include <clock.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup asfdoc_sam0_system_group
 * @{
 */

/**
 * \brief Reset causes of the system.
 *
 * List of possible reset causes of the system.
 */
enum system_reset_cause {
	/** The system was last reset by a backup reset */
	SYSTEM_RESET_CAUSE_BACKUP         = RSTC_RCAUSE_BACKUP,
	/** The system was last reset by a software reset */
	SYSTEM_RESET_CAUSE_SOFTWARE       = RSTC_RCAUSE_SYST,
	/** The system was last reset by the watchdog timer */
	SYSTEM_RESET_CAUSE_WDT            = RSTC_RCAUSE_WDT,
	/** The system was last reset because the external reset line was pulled low */
	SYSTEM_RESET_CAUSE_EXTERNAL_RESET = RSTC_RCAUSE_EXT,
#if SAML21 || SAMR30 || (SAMR34) || (SAMR35) || (WLR089)
	/** The system was last reset by the BOD33. */
	SYSTEM_RESET_CAUSE_BOD33          = RSTC_RCAUSE_BOD33,
	/** The system was last reset by the BOD12 */
	SYSTEM_RESET_CAUSE_BOD12          = RSTC_RCAUSE_BOD12,
#else
	/** The system was last reset by the BOD VDD. */
	SYSTEM_RESET_CAUSE_BOD33          = RSTC_RCAUSE_BODVDD,
	/** The system was last reset by the BOD CORE. */
	SYSTEM_RESET_CAUSE_BOD12          = RSTC_RCAUSE_BODCORE,
#endif
	/** The system was last reset by the POR (Power on reset). */
	SYSTEM_RESET_CAUSE_POR            = RSTC_RCAUSE_POR,
};

/**
 * \brief Backup exit source after a backup reset occurs.
 *
 * List of possible backup exit source.
 */
enum system_reset_backup_exit_source {
#if SAML21 || SAMR30 || (SAMR34) || (SAMR35) || (WLR089)
	/** The backup exit source was external wakeup. */
	SYSTEM_RESET_BACKKUP_EXIT_EXTWAKE    = RSTC_BKUPEXIT_EXTWAKE,
#endif
	/** The backup exit source was RTC interrupt. */
	SYSTEM_RESET_BACKKUP_EXIT_RTC        = RSTC_BKUPEXIT_RTC,
	/** The backup exit source was battery backup power switch */
	SYSTEM_RESET_BACKKUP_EXIT_BBPS       = RSTC_BKUPEXIT_BBPS,
};

#if SAML21 || SAMR30 || (SAMR34) || (SAMR35) || (WLR089)
/**
 * \brief Wakeup debounce counter value.
 *
 * Wakeup debounce counter value when waking up by external wakeup pin from backup mode.
 */
enum system_wakeup_debounce_count {
	/** No debouncing */
	SYSTEM_WAKEUP_DEBOUNCE_OFF         = RSTC_WKDBCONF_WKDBCNT_OFF,
	/** Input pin shall be active for at least two 32KHz clock period. */
	SYSTEM_WAKEUP_DEBOUNCE_2CK32       = RSTC_WKDBCONF_WKDBCNT_2CK32,
	/** Input pin shall be active for at least three 32KHz clock period. */
	SYSTEM_WAKEUP_DEBOUNCE_3CK32       = RSTC_WKDBCONF_WKDBCNT_3CK32,
	/** Input pin shall be active for at least 32 32KHz clock periods */
	SYSTEM_WAKEUP_DEBOUNCE_32CK32      = RSTC_WKDBCONF_WKDBCNT_32CK32,
	/** Input pin shall be active for at least 512 32KHz clock periods */
	SYSTEM_WAKEUP_DEBOUNCE_512CK32     = RSTC_WKDBCONF_WKDBCNT_512CK32,
	/** Input pin shall be active for at least 4096 32KHz clock periods */
	SYSTEM_WAKEUP_DEBOUNCE_4096CK32    = RSTC_WKDBCONF_WKDBCNT_4096CK32,
	/** Input pin shall be active for at least 32768 32KHz clock periods */
	SYSTEM_WAKEUP_DEBOUNCE_32768CK32   = RSTC_WKDBCONF_WKDBCNT_32768CK32,
};
#endif

/**
 * \name Reset Control
 * @{
 */

/**
 * \brief Reset the MCU.
 *
 * Resets the MCU and all associated peripherals and registers, except RTC,
 * OSC32KCTRL, RSTC, GCLK (if WRTLOCK is set), and I/O retention state of PM.
 *
 */
static inline void system_reset(void)
{
	NVIC_SystemReset();
}

/**
 * \brief Get the reset cause.
 *
 * Retrieves the cause of the last system reset.
 *
 * \return An enum value indicating the cause of the last system reset.
 */
static inline enum system_reset_cause system_get_reset_cause(void)
{
	return (enum system_reset_cause)RSTC->RCAUSE.reg;
}

/**
 * @}
 */

/**
 * \name Backup Exit Control
 * @{
 */

/**
 * \brief Get the backup exit source.
 *
 * Get the backup exit source when a backup reset occurs.
 *
 * \return An enum value indicating the latest backup exit source.
 */
static inline enum system_reset_backup_exit_source system_get_backup_exit_source(void)
{
	return (enum system_reset_backup_exit_source)RSTC->BKUPEXIT.reg;
}

#if SAML21 || SAMR30 || (SAMR34) || (SAMR35) || (WLR089)
/**
 * \brief Set wakeup debounce counter.
 *
 * Set the wakeup debounce counter value with the given count.
 *
 * \param[in] wakeup_debounce_count Wakeup debounce counter value
 */
static inline void system_set_pin_wakeup_debounce_counter(
					const enum system_wakeup_debounce_count wakeup_debounce_count)
{
	RSTC->WKDBCONF.reg = wakeup_debounce_count;
}

/**
 * \brief Set low polarity of wakeup input pin.
 *
 * Set low polarity with the given wakeup input pin mask.
 *
 * \param[in] pin_mask Input pin mask
 */
static inline void system_set_pin_wakeup_polarity_low(const uint16_t pin_mask)
{
	RSTC->WKPOL.reg &= ~(RSTC_WKPOL_WKPOL(pin_mask));
}

/**
 * \brief Set high polarity of wakeup input pin.
 *
 * Set high polarity with the given wakeup input pin mask.
 *
 * \param[in] pin_mask Input pin mask
 */
static inline void system_set_pin_wakeup_polarity_high(const uint16_t pin_mask)
{
	RSTC->WKPOL.reg |= RSTC_WKPOL_WKPOL(pin_mask);
}

/**
 * \brief Enable wakeup of input pin from the backup mode.
 *
 * Enable pin wakeup from the backup mode with the given pin mask.
 *
 * \param[in] pin Input pin mask
 */
static inline void system_enable_pin_wakeup(const uint16_t pin_mask)
{
	RSTC->WKEN.reg |= RSTC_WKEN_WKEN(pin_mask);
}

/**
 * \brief Disable wakeup of input pin from the backup mode.
 *
 * Disable pin wakeup from the backup mode with the given pin mask.
 *
 * \param[in] pin Input pin mask
 */
static inline void system_disable_pin_wakeup(const uint16_t pin_mask)
{
	RSTC->WKEN.reg &= ~(RSTC_WKEN_WKEN(pin_mask));
}

/**
 * \brief Check whether any of the enabled wake up pins are active and caused the wakeup.
 *
 * Check whether any of the enabled wake up pins are active and caused the wakeup
 *  from backup sleep mode when exiting backup mode.
 *
 * \return Pin mask, the corresponding pin is active when its pin mask is 1.
 */
static inline uint16_t system_get_pin_wakeup_cause(void)
{
	return (RSTC_WKCAUSE_MASK & (RSTC->WKCAUSE.reg >> RSTC_WKCAUSE_WKCAUSE_Pos));
}
#endif
/**
 * @}
 */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* RESET_H_INCLUDED */
