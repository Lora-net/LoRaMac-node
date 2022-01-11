/*!
 * \file  eeprom-board.c
 *
 * \brief Target board EEPROM driver implementation
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
#include <stdbool.h>
#include "stm32l4xx.h"
#include "eeprom_emul.h"
#include "eeprom-board.h"
#include "utilities.h"

uint16_t EepromVirtualAddress[NB_OF_VARIABLES];
__IO uint32_t ErasingOnGoing = 0;

/*!
 * \brief Initializes the EEPROM emulation module.
 */
void EepromMcuInit( void )
{
    EE_Status eeStatus = EE_OK;

    // Unlock the Flash Program Erase controller
    HAL_FLASH_Unlock( );

    // Set user List of Virtual Address variables: 0x0000 and 0xFFFF values are prohibited
    for( uint16_t varValue = 0; varValue < NB_OF_VARIABLES; varValue++ )
    {
        EepromVirtualAddress[varValue] = varValue + 1;
    }

    // Set EEPROM emulation firmware to erase all potentially incompletely erased
    // pages if the system came from an asynchronous reset. Conditional erase is
    // safe to use if all Flash operations where completed before the system reset
    if( __HAL_PWR_GET_FLAG( PWR_FLAG_SB ) == RESET )
    {
        // System reset comes from a power-on reset: Forced Erase
        // Initialize EEPROM emulation driver (mandatory)
        eeStatus = EE_Init( EepromVirtualAddress, EE_FORCED_ERASE );
        if( eeStatus != EE_OK )
        {
            assert_param( LMN_STATUS_ERROR );
        }
    }
    else
    {
        // Clear the Standby flag
        __HAL_PWR_CLEAR_FLAG( PWR_FLAG_SB );

        // Check and Clear the Wakeup flag
        if( __HAL_PWR_GET_FLAG( PWR_FLAG_WUF1 ) != RESET )
        {
            __HAL_PWR_CLEAR_FLAG( PWR_FLAG_WUF1 );
        }

        // System reset comes from a STANDBY wakeup: Conditional Erase
        // Initialize EEPROM emulation driver (mandatory)
        eeStatus = EE_Init( EepromVirtualAddress, EE_CONDITIONAL_ERASE );
        if( eeStatus != EE_OK )
        {
            assert_param( LMN_STATUS_ERROR );
        }
    }

    // Lock the Flash Program Erase controller
    HAL_FLASH_Lock( );
}

/*!
 * \brief Indicates if an erasing operation is on going.
 *
 * \retval isEradingOnGoing Returns true is an erasing operation is on going.
 */
bool EepromMcuIsErasingOnGoing( void )
{
    return ErasingOnGoing;
}

LmnStatus_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    LmnStatus_t status = LMN_STATUS_OK;
    EE_Status eeStatus = EE_OK;

    // Unlock the Flash Program Erase controller
    HAL_FLASH_Unlock( );

    CRITICAL_SECTION_BEGIN( );
    for( uint32_t i = 0; i < size; i++ )
    {
        eeStatus |= EE_WriteVariable8bits( EepromVirtualAddress[addr + i], buffer[i] );
    }
    CRITICAL_SECTION_END( );

    if( eeStatus != EE_OK )
    {
        status = LMN_STATUS_ERROR;
    }

    if( ( eeStatus & EE_STATUSMASK_CLEANUP ) == EE_STATUSMASK_CLEANUP )
    {
        ErasingOnGoing = 0;
        eeStatus |= EE_CleanUp( );
    }
    if( ( eeStatus & EE_STATUSMASK_ERROR ) == EE_STATUSMASK_ERROR )
    {
        status = LMN_STATUS_ERROR;
    }

    // Lock the Flash Program Erase controller
    HAL_FLASH_Lock( );
    return status;
}

LmnStatus_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    LmnStatus_t status = LMN_STATUS_OK;

    // Unlock the Flash Program Erase controller
    HAL_FLASH_Unlock( );

    for( uint32_t i = 0; i < size; i++ )
    {
        if( EE_ReadVariable8bits( EepromVirtualAddress[addr + i], buffer + i ) != EE_OK )
        {
            status = LMN_STATUS_ERROR;
            break;
        }
    }

    // Lock the Flash Program Erase controller
    HAL_FLASH_Lock( );
    return status;
}

void EepromMcuSetDeviceAddr( uint8_t addr )
{
    assert_param( LMN_STATUS_ERROR );
}

LmnStatus_t EepromMcuGetDeviceAddr( void )
{
    assert_param( LMN_STATUS_ERROR );
    return 0;
}

/*!
 * \brief  FLASH end of operation interrupt callback.
 * \param  ReturnValue: The value saved in this parameter depends on the ongoing procedure
 *                  Mass Erase: Bank number which has been requested to erase
 *                  Page Erase: Page which has been erased
 *                    (if 0xFFFFFFFF, it means that all the selected pages have been erased)
 *                  Program: Address which was selected for data program
 * \retval None
 */
void HAL_FLASH_EndOfOperationCallback( uint32_t ReturnValue )
{
    // Call CleanUp callback when all requested pages have been erased
    if( ReturnValue == 0xFFFFFFFF )
    {
        EE_EndOfCleanup_UserCallback( );
    }
}

/*!
 * \brief  Clean Up end of operation interrupt callback.
 * \param  None
 * \retval None
 */
void EE_EndOfCleanup_UserCallback( void )
{
    ErasingOnGoing = 0;
}
