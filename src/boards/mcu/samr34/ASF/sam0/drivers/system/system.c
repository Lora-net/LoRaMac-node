/**
 * \file
 *
 * \brief SAM System related functionality
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

#include <system.h>

/**
 * \internal
 * Dummy initialization function, used as a weak alias target for the various
 * init functions called by \ref system_init().
 */
void _system_dummy_init(void);
void _system_dummy_init(void)
{
	return;
}

#if !defined(__DOXYGEN__)
#  if defined(__GNUC__)
void system_clock_init(void) WEAK __attribute__((alias("_system_dummy_init")));
void system_board_init(void) WEAK __attribute__((alias("_system_dummy_init")));
void _system_events_init(void) WEAK __attribute__((alias("_system_dummy_init")));
void _system_extint_init(void) WEAK __attribute__((alias("_system_dummy_init")));
void _system_divas_init(void) WEAK __attribute__((alias("_system_dummy_init")));
#  elif defined(__ICCARM__)
void system_clock_init(void);
void system_board_init(void);
void _system_events_init(void);
void _system_extint_init(void);
void _system_divas_init(void);
#    pragma weak system_clock_init=_system_dummy_init
#    pragma weak system_board_init=_system_dummy_init
#    pragma weak _system_events_init=_system_dummy_init
#    pragma weak _system_extint_init=_system_dummy_init
#    pragma weak _system_divas_init=_system_dummy_init
#  endif
#endif

/**
 * \brief Initialize system
 *
 * This function will call the various initialization functions within the
 * system namespace. If a given optional system module is not available, the
 * associated call will effectively be a NOP (No Operation).
 *
 * Currently the following initialization functions are supported:
 *  - System clock initialization (via the SYSTEM CLOCK sub-module)
 *  - Board hardware initialization (via the Board module)
 *  - Event system driver initialization (via the EVSYS module)
 *  - External Interrupt driver initialization (via the EXTINT module)
 */
void system_init(void)
{
	/* Configure GCLK and clock sources according to conf_clocks.h */
	system_clock_init();

	/* Initialize board hardware */
	system_board_init();

	/* Initialize EVSYS hardware */
	_system_events_init();

	/* Initialize External hardware */
	_system_extint_init();
	
	/* Initialize DIVAS hardware */
	_system_divas_init();
}

