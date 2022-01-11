/*!
 * \file  sysIrqHandlers.c
 *
 * \brief Default IRQ handlers
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include "stm32l4xx.h"
#include "eeprom_emul.h"

/*!
 * \brief  This function handles NMI exception.
 * \param  None
 * \retval None
 */
void NMI_Handler( void )
{
    uint32_t corruptedflashaddress;

    // Check if NMI is due to flash ECCD (error detection)
    if( __HAL_FLASH_GET_FLAG( FLASH_FLAG_ECCD ) )
    {
        // Compute corrupted flash address
        corruptedflashaddress = FLASH_BASE + ( FLASH->ECCR & FLASH_ECCR_ADDR_ECC );

#if defined(FLASH_OPTR_BFB2)
        // Add bank size to corrupteflashaddress if fail bank is bank 2
        if( READ_BIT( FLASH->ECCR, FLASH_ECCR_BK_ECC ) == FLASH_ECCR_BK_ECC )
        {
            corruptedflashaddress += FLASH_BANK_SIZE;
        }
#endif

        // Check if corrupted flash address is in eeprom emulation pages
        if( ( corruptedflashaddress >= START_PAGE_ADDRESS ) || ( corruptedflashaddress <= END_EEPROM_ADDRESS ) )
        {
            /* Delete the corrupted flash address */
            if( EE_DeleteCorruptedFlashAddress( corruptedflashaddress ) == EE_OK )
            {
                // Resume execution if deletion succeeds
                return;
            }
        }
    }

    /* Go to infinite loop when NMI occurs in case:
    - ECCD is raised in eeprom emulation flash pages but corrupted flash address deletion fails
    - ECCD is raised out of eeprom emulation flash pages
    - no ECCD is raised */
    while( 1 )
    {
    }
}

/*!
 * \brief  This function handles Hard Fault exception.
 * \param  None
 * \retval None
 */
void HardFault_Handler( void )
{
    //*************************************************************************
    // When a power down or external reset occurs during a Flash Write operation,
    // the first line at 0x0 may be corrupted at 0x0 (low occurrence).
    // In this case the Flash content is restored.
    //    address : flash bank1 base address
    //    data : first flash line (64 bits). This variable must be updated after each build.
    //    sram2address : sram2 start address to copy and restore first flash page
    //*************************************************************************
    uint32_t address = FLASH_BASE;
    uint64_t data = 0x08002df120001bc0;
    uint32_t sram2address = 0x20030000;
    uint32_t page = 0;
    uint32_t banks = FLASH_BANK_1;
    uint32_t element = 0U;

    if( ( *( __IO uint32_t* )address == 0x0 ) && ( *( __IO uint32_t* )( address + 4 ) == 0x0 ) )
    {
        // Authorize the FLASH Registers access
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;

        // Save first flash page in SRAM2
        for( element = 2; element < FLASH_PAGE_SIZE; element++ )
        {
            *( __IO uint32_t* )( sram2address + ( element * 4 ) ) = *( __IO uint32_t* )( address + ( element * 4 ) );
        }

        // Restore the first flash line in SRAM2 to its correct value
        *( __IO uint32_t* )sram2address = ( uint32_t )data; 
        *( __IO uint32_t* )( sram2address + 4 ) = ( uint32_t )( data >> 32 ); 

        // Clear FLASH all errors
        __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_ALL_ERRORS );

        // Erase first flash page
#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
        if( READ_BIT( FLASH->OPTR, FLASH_OPTR_DBANK ) == RESET )
        {
            CLEAR_BIT( FLASH->CR, FLASH_CR_BKER );
        }
        else
#endif
        {
            if( ( banks & FLASH_BANK_1 ) != RESET )
            {
                CLEAR_BIT( FLASH->CR, FLASH_CR_BKER );
            }
            else
            {
                SET_BIT( FLASH->CR, FLASH_CR_BKER );
            }
        }
#endif

        // Proceed to erase the page
        MODIFY_REG( FLASH->CR, FLASH_CR_PNB, ( page << POSITION_VAL( FLASH_CR_PNB ) ) );
        SET_BIT( FLASH->CR, FLASH_CR_PER );
        SET_BIT( FLASH->CR, FLASH_CR_STRT );

        // Wait for last operation to be completed
        while( __HAL_FLASH_GET_FLAG( FLASH_FLAG_BSY ) ){ }
        // If the erase operation is completed, disable the PER Bit
        CLEAR_BIT( FLASH->CR, ( FLASH_CR_PER | FLASH_CR_PNB ) );

        // Restore first flash page in flash from SRAM2
        for( element = 0; element < FLASH_PAGE_SIZE; element++ )
        {
            // Wait for last operation to be completed
            while( __HAL_FLASH_GET_FLAG( FLASH_FLAG_BSY ) ){ }

            // Set PG bit
            SET_BIT( FLASH->CR, FLASH_CR_PG );

            // Write in flash
            *( __IO uint32_t* )( address + ( element * 4 ) ) = *( __IO uint32_t* )( sram2address + ( element * 4 ) );
        }

        // System reset
        NVIC_SystemReset( );
    }

    // Go to infinite loop when Hard Fault exception occurs
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

/*!
 * \brief  This function handles Flash interrupt request.
 * \param  None
 * \retval None
 */
void FLASH_IRQHandler( void )
{
    HAL_FLASH_IRQHandler();
}

/*!
 * \brief  This function handles PVD interrupt request.
 * \param  None
 * \retval None
 */
void PVD_PVM_IRQHandler( void )
{
    // Loop inside the handler to prevent the Cortex from using the Flash,
    // allowing the flash interface to finish any ongoing transfer.
    while( __HAL_PWR_GET_FLAG( PWR_FLAG_PVDO ) != RESET )
    {
    }
}

