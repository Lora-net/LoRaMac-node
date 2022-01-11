/*!
 * \file  sx126x_hal.c
 *
 * \brief Implements the sx126x radio HAL functions
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

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type
#include "board.h"
#include "delay.h"
#include "radio_board.h"
#include "sx126x_hal.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*!
 * \brief Wait until radio busy pin returns to 0
 */
static sx126x_hal_status_t sx126x_hal_wait_on_busy( const void* context );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

sx126x_hal_status_t sx126x_hal_write( const void* context, const uint8_t* command, const uint16_t command_length,
                                      const uint8_t* data, const uint16_t data_length )

{
    radio_context_t* radio_context = ( radio_context_t* ) context;

    if( sx126x_hal_wakeup( context ) == SX126X_HAL_STATUS_OK )
    {
        GpioWrite( &radio_context->spi.Nss, 0 );
        for( uint16_t i = 0; i < command_length; i++ )
        {
            SpiInOut( &radio_context->spi, command[i] );
        }
        for( uint16_t i = 0; i < data_length; i++ )
        {
            SpiInOut( &radio_context->spi, data[i] );
        }
        GpioWrite( &radio_context->spi.Nss, 1 );

        // 0x84 - SX126X_SET_SLEEP
        if( command[0] != 0x84 )
        {
            return sx126x_hal_wait_on_busy( context );
        }
        else
        {
            return SX126X_HAL_STATUS_OK;
        }
    }
    return SX126X_HAL_STATUS_ERROR;
}

sx126x_hal_status_t sx126x_hal_read( const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length )
{
    radio_context_t* radio_context = ( radio_context_t* ) context;

    if( sx126x_hal_wakeup( context ) == SX126X_HAL_STATUS_OK )
    {
        GpioWrite( &radio_context->spi.Nss, 0 );
        for( uint16_t i = 0; i < command_length; i++ )
        {
            SpiInOut( &radio_context->spi, command[i] );
        }
        for( uint16_t i = 0; i < data_length; i++ )
        {
            data[i] = SpiInOut( &radio_context->spi, 0 );
        }
        GpioWrite( &radio_context->spi.Nss, 1 );

        return sx126x_hal_wait_on_busy( context );
    }
    return SX126X_HAL_STATUS_ERROR;
}

sx126x_hal_status_t sx126x_hal_write_read( const void* context, const uint8_t* command, uint8_t* data,
                                           const uint16_t data_length )
{
    radio_context_t* radio_context = ( radio_context_t* ) context;

    if( sx126x_hal_wakeup( context ) == SX126X_HAL_STATUS_OK )
    {
        GpioWrite( &radio_context->spi.Nss, 0 );

        for( uint16_t i = 0; i < data_length; i++ )
        {
            data[i] = SpiInOut( &radio_context->spi, command[i] );
        }

        GpioWrite( &radio_context->spi.Nss, 1 );

        // 0x084 - SX126X_SET_SLEEP
        if( command[0] != 0x84 )
        {
            return sx126x_hal_wait_on_busy( context );
        }
        else
        {
            return SX126X_HAL_STATUS_OK;
        }
    }
    return SX126X_HAL_STATUS_ERROR;
}

sx126x_hal_status_t sx126x_hal_reset( const void* context )
{
    radio_context_t* radio_context = ( radio_context_t* ) context;

    radio_board_start_radio_tcxo( );

    GpioWrite( &radio_context->reset, 0 );
    DelayMs( 1 );
    GpioWrite( &radio_context->reset, 1 );

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup( const void* context )
{
    radio_context_t* radio_context = ( radio_context_t* ) context;

    if( ( radio_board_get_operating_mode( ) == RADIO_BOARD_OP_MODE_SLEEP ) ||
        ( radio_board_get_operating_mode( ) == RADIO_BOARD_OP_MODE_RX_DC ) )
    {
        // Wakeup radio
        GpioWrite( &radio_context->spi.Nss, 0 );
        GpioWrite( &radio_context->spi.Nss, 1 );

        // Radio is awake in STDBY_RC mode
        radio_context->op_mode = RADIO_BOARD_OP_MODE_STDBY;
    }

    // Wait on busy pin for 100 ms
    return sx126x_hal_wait_on_busy( radio_context );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static sx126x_hal_status_t sx126x_hal_wait_on_busy( const void* context )
{
    radio_context_t* radio_context = ( radio_context_t* ) context;

    while( GpioRead( &radio_context->busy ) == 1 )
    {
        ;
    }
    return SX126X_HAL_STATUS_OK;
}

/* --- EOF ------------------------------------------------------------------ */
