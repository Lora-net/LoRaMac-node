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
#ifdef SOFTDEVICE_PRESENT
#include "nrf_soc.h"
#endif

/* Make sure that all elements in isr state fits in uint32_t element*/
#include "portmacro_cmsis.h"
STATIC_ASSERT(sizeof(portISRState_t) == sizeof(uint32_t));

/* Constants used for assertion check of the selected CPU */
#define portCORTEX_M0_r0p0_ID               ( 0x410CC200UL )

/* Constants required to check the validity of an interrupt priority. */
#define portFIRST_USER_INTERRUPT_NUMBER     ( 16 )
#define portMAX_8_BIT_VALUE                 ( ( uint8_t ) 0xff )
#define portTOP_BIT_OF_BYTE                 ( ( uint8_t ) 0x80 )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR                    (((xPSR_Type){.b.T = 1}).w)
#define portINITIAL_EXEC_RETURN             ( 0xfffffffd )

/* Let the user override the pre-loading of the initial LR with the address of
prvTaskExitError() in case is messes up unwinding of the stack in the
debugger. */
#ifdef configTASK_RETURN_ADDRESS
    #define portTASK_RETURN_ADDRESS configTASK_RETURN_ADDRESS
#else
    #define portTASK_RETURN_ADDRESS prvTaskExitError
#endif

/* Each task maintains its own interrupt status in the critical nesting
variable. */
static UBaseType_t uxCriticalNesting = 0;

#ifdef SOFTDEVICE_PRESENT
/* Flag that is set when yielding should be done after critical section exiting */
static UBaseType_t uxYieldFlag = 0;
#endif

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
extern void vPortSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void xPortSysTickHandler( void );

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
extern void vPortStartFirstTask( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );


/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    /* Simulate the stack frame as it would be created by a context switch
    interrupt. */

    /* Offset added to account for the way the MCU uses the stack on entry/exit
    of interrupts, and to ensure alignment. */
    pxTopOfStack--;

    *pxTopOfStack = portINITIAL_XPSR;   /* xPSR */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pxCode; /* PC */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) portTASK_RETURN_ADDRESS;    /* LR */

    /* Save code space by skipping register initialisation. */
    pxTopOfStack -= 5;  /* R12, R3, R2 and R1. */
    *pxTopOfStack = ( StackType_t ) pvParameters;   /* R0 */

    pxTopOfStack -= 8;  /* R11, R10, R9, R8, R7, R6, R5 and R4. */

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
    /* A function that implements a task must not exit or attempt to return to
    its caller as there is nothing to return to.  If a task wants to exit it
    should instead call vTaskDelete( NULL ).

    Artificially force an assert() to be triggered if configASSERT() is
    defined, then stop here so application writers can catch the error. */
    configASSERT( uxCriticalNesting == ~0UL );
    portDISABLE_INTERRUPTS();
    for ( ;; );
}


/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
BaseType_t xPortStartScheduler( void )
{
    /* This port can be used only on Cortex-M0 processor */
    configASSERT( SCB->CPUID == portCORTEX_M0_r0p0_ID );

    /* Make PendSV the lowest priority interrupts. */
    NVIC_SetPriority(PendSV_IRQn, configKERNEL_INTERRUPT_PRIORITY);

    /* Start the timer that generates the tick ISR.  Interrupts are disabled
    here already. */
    vPortSetupTimerInterrupt();

    /* Initialise the critical nesting count ready for the first task. */
    uxCriticalNesting = 0;

    /* Finally this port requires SEVONPEND to be active */
    SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

    /* Start the first task. */
    vPortStartFirstTask();

    /* Should never get here as the tasks will now be executing!  Call the task
    exit error function to prevent compiler warnings about a static function
    not being called in the case that the application writer overrides this
    functionality by defining configTASK_RETURN_ADDRESS. */
    prvTaskExitError();

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented in ports where there is nothing to return to.
    Artificially force an assert. */
    configASSERT( uxCriticalNesting == 1000UL );
}
/*-----------------------------------------------------------*/

void vPortTaskYield( void )
{
#ifdef SOFTDEVICE_PRESENT
    if (uxCriticalNesting > 0)
    {
        uxYieldFlag = 1;
    }
    else
    {
        portPendSVSchedule();
    }
#else
    portPendSVSchedule();
#endif
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;

    /* This is not the interrupt safe version of the enter critical function so
    assert() if it is being called from an interrupt context.  Only API
    functions that end in "FromISR" can be used in an interrupt.  Only assert if
    the critical nesting count is 1 to protect against recursive calls if the
    assert function also uses a critical section. */
    if ( uxCriticalNesting == 1 )
    {
        configASSERT( ( SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk ) == 0 );
    }
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
    configASSERT( uxCriticalNesting );
    uxCriticalNesting--;
    if ( uxCriticalNesting == 0 )
    {
        portENABLE_INTERRUPTS();
#ifdef SOFTDEVICE_PRESENT
        if (uxYieldFlag)
        {
            uxYieldFlag = 0;
            portPendSVSchedule();
        }
#endif
    }
}
/*-----------------------------------------------------------*/

void vPortSafeTaskSwitchContext( void )
{
    uint32_t isrstate = portSET_INTERRUPT_MASK_FROM_ISR();
    vTaskSwitchContext();
    portCLEAR_INTERRUPT_MASK_FROM_ISR(isrstate);
}
