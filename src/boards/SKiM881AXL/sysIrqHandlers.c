/*!
 * \file      sysIrqHandlers.c
 *
 * \brief     Default IRQ handlers
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */

/*!
 * \brief  This function handles NMI exception.
 * \param  None
 * \retval None
 */
void NMI_Handler( void )
{
}

/*!
 * \brief  This function handles Hard Fault exception.
 * \param  None
 * \retval None
 */
#if defined( HARD_FAULT_HANDLER_ENABLED )
void HardFault_Handler_C( unsigned int *args )
{
    volatile unsigned int stacked_r0;
    volatile unsigned int stacked_r1;
    volatile unsigned int stacked_r2;
    volatile unsigned int stacked_r3;
    volatile unsigned int stacked_r12;
    volatile unsigned int stacked_lr;
    volatile unsigned int stacked_pc;
    volatile unsigned int stacked_psr;

    stacked_r0 = ( ( unsigned long) args[0] );
    stacked_r1 = ( ( unsigned long) args[1] );
    stacked_r2 = ( ( unsigned long) args[2] );
    stacked_r3 = ( ( unsigned long) args[3] );

    stacked_r12 = ( ( unsigned long) args[4] );
    stacked_lr = ( ( unsigned long) args[5] );
    stacked_pc = ( ( unsigned long) args[6] );
    stacked_psr = ( ( unsigned long) args[7] );

    ( void )stacked_r0;
    ( void )stacked_r1;
    ( void )stacked_r2;
    ( void )stacked_r3;

    ( void )stacked_r12;
    ( void )stacked_lr ;
    ( void )stacked_pc ;
    ( void )stacked_psr;

    while( 1 );
}

#if defined(__CC_ARM)
#warning "HardFault_Handler: ARMCC does not allow some of the required instructions to be inlined under C code."
// To mimic the behavior provided for IAR and GCC one needs to create a hard_fault_handler.s file and add it to the Keil project.
// Something similar to the below code should be added to hard_fault_handler.s
// Refer to https://www.segger.com/downloads/application-notes/AN00016
// @code
//    AREA OSKERNEL, CODE, READONLY, ALIGN=2
//    PRESERVE8
//
//    EXPORT HardFault_Handler
//    IMPORT HardFault_Handler_C
//
//    THUMB
//
//    HardFault_Handler PROC
//        MOVS R0, #4
//        MOV R1, LR
//        TST R0, R1 // Check EXC_RETURN in Link register bit 2.
//        BNE Uses_PSP
//        MRS R0, MSP // Stacking was using MSP.
//        B Pass_StackPtr
//        Uses_PSP:
//        MRS R0, PSP // Stacking was using PSP
//        Pass_StackPtr:
//        ALIGN
//        LDR R2,=HardFault_Handler_C
//        BX  R2
//        ENDP
//    END
// @code
#elif defined(__ICCARM__)
void HardFault_Handler(void)
{
    // Refer to https://www.segger.com/downloads/application-notes/AN00016
    __asm("MOVS R0, #4");
    __asm("MOV R1, LR");
    __asm("TST R0, R1"); // Check EXC_RETURN in Link register bit 2.
    __asm("BNE Uses_PSP");
    __asm("MRS R0, MSP"); // Stacking was using MSP.
    __asm("B Pass_StackPtr");
    __asm("Uses_PSP:");
    __asm("MRS R0, PSP"); // Stacking was using PSP
    __asm("Pass_StackPtr:");
    __asm("LDR R2,=HardFault_Handler_C");
    __asm("BX  R2");
}
#elif defined(__GNUC__)
void HardFault_Handler(void)
{
    // Refer to https://www.segger.com/downloads/application-notes/AN00016
    __asm volatile("MOVS R0, #4");
    __asm volatile("MOV R1, LR");
    __asm volatile("TST R0, R1"); // Check EXC_RETURN in Link register bit 2.
    __asm volatile("BNE Uses_PSP");
    __asm volatile("MRS R0, MSP");// Stacking was using MSP.
    __asm volatile("B Pass_StackPtr");
    __asm volatile("Uses_PSP:");
    __asm volatile("MRS R0, PSP"); // Stacking was using PSP
    __asm volatile("Pass_StackPtr:");
    __asm volatile("LDR R2,=HardFault_Handler_C");
    __asm volatile("BX  R2");
}
#else
    #warning Not supported compiler type
#endif

#endif

/*!
 * \brief  This function handles Memory Manage exception.
 * \param  None
 * \retval None
 */
void MemManage_Handler( void )
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while ( 1 )
    {
    }
}

/*!
 * \brief  This function handles Bus Fault exception.
 * \param  None
 * \retval None
 */
void BusFault_Handler( void )
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while ( 1 )
    {
    }
}

/*!
 * \brief  This function handles Usage Fault exception.
 * \param  None
 * \retval None
 */
void UsageFault_Handler( void )
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while ( 1 )
    {
    }
}

/*!
 * \brief  This function handles Debug Monitor exception.
 * \param  None
 * \retval None
 */
void DebugMon_Handler( void )
{
}
