/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Default IRQ handlers

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

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
void HardFault_Handler( void )
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while( 1 )
    {
    }
}

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
