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

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM4F port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
void vPortStartFirstTask( void ) __attribute__ (( naked ));

/*
 * Exception handlers.
 */
void vPortSVCHandler( void );
void xPortPendSVHandler( void ) __attribute__ (( naked ));


/*-----------------------------------------------------------*/

void vPortStartFirstTask( void )
{
    __asm volatile(
    "   .syntax unified           \n"
    "                             \n"
    "   ldr r0, =__isr_vector     \n" /* Locate the stack using __isr_vector table. */
    "   ldr r0, [r0]              \n"
    "   msr msp, r0               \n" /* Set the msp back to the start of the stack. */
    "                             \n"
    "   ldr r3, =pxCurrentTCB \n" /* Obtain location of pxCurrentTCB. */
    "   ldr r1, [r3]              \n"
    "   ldr r0, [r1]              \n" /* The first item in pxCurrentTCB is the task top of stack. */
    "   adds r0, #32              \n" /* Discard everything up to r0 */
    "   msr psp, r0               \n" /* This is now the new top of stack to use in the task. */
    "   movs r0, #2               \n" /* Switch to the psp stack. */
    "   msr CONTROL, r0           \n"
    "   pop {r0-r5}               \n" /* Pop the registers that are saved automatically. */
    "   mov lr, r5                \n" /* lr is now in r5. */
    "   cpsie i                   \n" /* The first task has its context and interrupts can be enabled. */
    "   pop {pc}                  \n" /* Finally, pop the PC to jump to the user defined task code. */
    "                             \n"
    "   .align 2                  \n"
    );
}

/*-----------------------------------------------------------*/

void vPortSVCHandler( void )
{
    /* This function is no longer used, but retained for backward
    compatibility. */
}

/*-----------------------------------------------------------*/

void xPortPendSVHandler( void )
{
    /* This is a naked function. */

    __asm volatile
    (
    "   .syntax unified                     \n"
    "                                       \n"
    "   mrs r0, psp                         \n"
    "                                       \n"
    "   ldr r3, =pxCurrentTCB               \n" /* Get the location of the current TCB. */
    "   ldr r2, [r3]                        \n"
    "                                       \n"
    "   subs r0, #32                        \n" /* Make space for the remaining low registers. */
    "   str r0, [r2]                        \n" /* Save the new top of stack. */
    "   stmia r0!, {r4-r7}                  \n" /* Store the low registers that are not saved automatically. */
    "   mov r4, r8                          \n" /* Store the high registers. */
    "   mov r5, r9                          \n"
    "   mov r6, r10                         \n"
    "   mov r7, r11                         \n"
    "   stmia r0!, {r4-r7}                  \n"
    "                                       \n"
    "   push {r3, r14}                      \n"
    "   bl vPortSafeTaskSwitchContext       \n"
    "   pop {r2, r3}                        \n" /* lr goes in r3. r2 now holds tcb pointer. */
    "                                       \n"
    "   ldr r1, [r2]                        \n"
    "   ldr r0, [r1]                        \n" /* The first item in pxCurrentTCB is the task top of stack. */
    "   adds r0, #16                        \n" /* Move to the high registers. */
    "   ldmia r0!, {r4-r7}                  \n" /* Pop the high registers. */
    "   mov r8, r4                          \n"
    "   mov r9, r5                          \n"
    "   mov r10, r6                         \n"
    "   mov r11, r7                         \n"
    "                                       \n"
    "   msr psp, r0                         \n" /* Remember the new top of stack for the task. */
    "                                       \n"
    "   subs r0, #32                        \n" /* Go back for the low registers that are not automatically restored. */
    "   ldmia r0!, {r4-r7}                  \n" /* Pop low registers.  */
    "                                       \n"
    "   bx r3                               \n"
    "                                       \n"
    "   .align 2                            \n"
    );
}
