/*
    FreeRTOS V8.2.1 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/


#ifndef PORTMACRO_CMSIS_H
#define PORTMACRO_CMSIS_H

#include "app_util.h"
#include "nordic_common.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_soc.h"
#include "softdevice_handler.h"
#include "app_error.h"
#include "app_util_platform.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  uint32_t
#define portBASE_TYPE   long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if ( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL

    /* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
    not need to be guarded with a critical section. */
    #define portTICK_TYPE_IS_ATOMIC 1
#endif
/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH            ( -1 )
#define portTICK_PERIOD_MS            ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT            8

/* RTC register */
#define portNRF_RTC_REG        NRF_RTC1
/* IRQn used by the selected RTC */
#define portNRF_RTC_IRQn       RTC1_IRQn
/* Constants required to manipulate the NVIC. */
#define portNRF_RTC_PRESCALER  ( (uint32_t) (ROUNDED_DIV(configSYSTICK_CLOCK_HZ, configTICK_RATE_HZ) - 1) )
/* Maximum RTC ticks */
#define portNRF_RTC_MAXTICKS   ((1U<<24)-1U)
/*-----------------------------------------------------------*/

/* Internal auxiliary macro */
#define portPendSVSchedule() do                                                 \
{                                                                               \
    /* Set a PendSV to request a context switch. */                             \
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;                                         \
    __SEV();                                                                    \
    /* Barriers are normally not required but do ensure the code is completely  \
    within the specified behaviour for the architecture. */                     \
    __DSB();                                                                    \
    __ISB();                                                                    \
}while (0)

/* Scheduler utilities. */
#define portYIELD() vPortTaskYield()

#define portEND_SWITCHING_ISR( xSwitchRequired ) if ( (xSwitchRequired) != pdFALSE ) { portPendSVSchedule(); }
#define portYIELD_FROM_ISR( x ) portEND_SWITCHING_ISR( x )
/*-----------------------------------------------------------*/

/* Critical section management. */
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
extern void vPortTaskYield( void );
#define portSET_INTERRUPT_MASK_FROM_ISR()        ulPortDisableISR()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)     vPortRestoreISR(x)
#define portDISABLE_INTERRUPTS()                 vPortDisableISR()
#define portENABLE_INTERRUPTS()                  vPortEnableISR()
#define portENTER_CRITICAL()                     vPortEnterCritical()
#define portEXIT_CRITICAL()                      vPortExitCritical()

/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
not necessary for to use this port.  They are defined so the common demo files
(which build with all the ports) will build. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/* Tickless idle/low power functionality. */
#ifndef portSUPPRESS_TICKS_AND_SLEEP
    extern void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime );
    #define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime ) vPortSuppressTicksAndSleep( xExpectedIdleTime )
#endif

/*-----------------------------------------------------------*/

__STATIC_INLINE void vPortDisableISR(void)
{
#ifdef SOFTDEVICE_PRESENT
    uint8_t dummy = 0;
    uint32_t err_code = sd_nvic_critical_region_enter(&dummy);
    APP_ERROR_CHECK(err_code);
#else
    __disable_irq();
#endif
}

__STATIC_INLINE void vPortEnableISR(void)
{
#ifdef SOFTDEVICE_PRESENT
    uint32_t err_code = sd_nvic_critical_region_exit(0);
    APP_ERROR_CHECK(err_code);
#else
    __enable_irq();
#endif
}

/* Union used to remember current ISR state.
 * It concentrates information about ISR state and internal SD state into 32 bit variable. */
typedef union
{
    uint32_t u32;        //< 32 bit access to the variable
    uint8_t  sd_nested;  //< Soft device nested element
    bool     irq_nested; //< Global was disabled
}portISRState_t;

__STATIC_INLINE uint32_t ulPortDisableISR(void)
{
    portISRState_t isrs    = {0};
#ifdef SOFTDEVICE_PRESENT
    uint32_t err_code = sd_nvic_critical_region_enter(&isrs.sd_nested);
    APP_ERROR_CHECK(err_code);
#else
    uint32_t primask = __get_PRIMASK();
    if(primask == 0)
    {
        __disable_irq();
    }
    else
    {
        isrs.irq_nested = true;
    }
#endif
    return isrs.u32;
}

__STATIC_INLINE void vPortRestoreISR(uint32_t state)
{
    portISRState_t isrs = {state};
#ifdef SOFTDEVICE_PRESENT
    uint32_t err_code = sd_nvic_critical_region_exit(isrs.sd_nested);
    APP_ERROR_CHECK(err_code);
#else
    if(!isrs.irq_nested)
    {
        __enable_irq();
    }
#endif
}

/*-----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_CMSIS_H */

