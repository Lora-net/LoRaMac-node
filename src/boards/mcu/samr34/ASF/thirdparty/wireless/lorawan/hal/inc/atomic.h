/**
* \file  atomic.h
*
* \brief Atomic operations APIs
*		
*
* Copyright (c) 2018 Microchip Technology Inc. and its subsidiaries. 
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
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, 
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
/************************************************************************/
/*      DECIDE ON THE LICENSE TEXT                                      */
/************************************************************************/

#ifndef ATOMIC_H
#define ATOMIC_H

/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#ifdef UT
#include "lora_test_main.h"
#endif

/************************************************************************/
/* Function Prototypes                                                  */
/************************************************************************/
/**
 * \brief Enters a critical section.
 *
 * Disables global interrupts. To support nested critical sections, an internal
 * count of the critical section nesting will be kept, so that global interrupts
 * are only re-enabled upon leaving the outermost nested critical section.
 *
 */
void system_enter_critical_section(void);

/**
 * \brief Leaves a critical section.
 *
 * Enables global interrupts. To support nested critical sections, an internal
 * count of the critical section nesting will be kept, so that global interrupts
 * are only re-enabled upon leaving the outermost nested critical section.
 *
 */
void system_leave_critical_section(void);

/************************************************************************/
/* Defines                                                              */
/************************************************************************/
#ifdef UT
#define   ATOMIC_SECTION_ENTER
#define   ATOMIC_SECTION_EXIT  
#else
#define   ATOMIC_SECTION_ENTER  system_enter_critical_section();
#define   ATOMIC_SECTION_EXIT   system_leave_critical_section();
#endif /* UT */
#endif /* ATOMIC_H */

/* eof atomic.h */
