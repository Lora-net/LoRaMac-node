/*!
 * \file  sx127x_hal.c
 *
 * \brief Implements the sx127x radio HAL functions
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
#include "board-config.h"
#include "delay.h"
#include "timer.h"
#include "radio_board.h"
#include "sx127x_hal.h"

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

/*!
 * SX127x HAL timer object
 */
static TimerEvent_t timer_timeout = { 0 };

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

sx127x_radio_id_t sx127x_hal_get_radio_id( const sx127x_t* radio )
{
#if defined( SX1272 )
    return SX127X_RADIO_ID_SX1272;
#elif defined( SX1276 )
    return SX127X_RADIO_ID_SX1276;
#else
#error "Please define the radio to be used"
#endif
}

void sx127x_hal_dio_irq_attach( const sx127x_t* radio )
{
    radio_context_t* radio_context = ( radio_context_t* ) ( radio->hal_context );

    // Attach driver pins interrupt handlers to MCU pins
    GpioSetInterrupt( &radio_context->dio_0, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, radio->dio_0_irq_handler );
    GpioSetContext( &radio_context->dio_0, ( sx127x_t* ) radio );
    GpioSetInterrupt( &radio_context->dio_1, IRQ_RISING_FALLING_EDGE, IRQ_HIGH_PRIORITY, radio->dio_1_irq_handler );
    GpioSetContext( &radio_context->dio_1, ( sx127x_t* ) radio );
    GpioSetInterrupt( &radio_context->dio_2, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, radio->dio_2_irq_handler );
    GpioSetContext( &radio_context->dio_2, ( sx127x_t* ) radio );
}

sx127x_hal_status_t sx127x_hal_write( const sx127x_t* radio, const uint16_t address, const uint8_t* data,
                                      const uint16_t data_length )
{
    radio_context_t* radio_context = ( radio_context_t* ) ( radio->hal_context );

    CRITICAL_SECTION_BEGIN( );
    GpioWrite( &radio_context->spi.Nss, 0 );

    SpiInOut( &radio_context->spi, address | 0x80 );
    for( uint16_t i = 0; i < data_length; i++ )
    {
        SpiInOut( &radio_context->spi, data[i] );
    }

    GpioWrite( &radio_context->spi.Nss, 1 );
    CRITICAL_SECTION_END( );
    return SX127X_HAL_STATUS_OK;
}

sx127x_hal_status_t sx127x_hal_read( const sx127x_t* radio, const uint16_t address, uint8_t* data,
                                     const uint16_t data_length )
{
    radio_context_t* radio_context = ( radio_context_t* ) ( radio->hal_context );

    CRITICAL_SECTION_BEGIN( );
    GpioWrite( &radio_context->spi.Nss, 0 );

    SpiInOut( &radio_context->spi, address & ( ~0x80 ) );
    for( uint16_t i = 0; i < data_length; i++ )
    {
        data[i] = SpiInOut( &radio_context->spi, 0 );
    }

    GpioWrite( &radio_context->spi.Nss, 1 );
    CRITICAL_SECTION_END( );
    return SX127X_HAL_STATUS_OK;
}

void sx127x_hal_reset( const sx127x_t* radio )
{
    radio_context_t* radio_context = ( radio_context_t* ) ( radio->hal_context );

    radio_board_start_radio_tcxo( );

#if defined( SX1272 )
    // Set RESET pin to 1
    GpioInit( &radio_context->reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
#elif defined( SX1276 )
    // Set RESET pin to 0
    GpioInit( &radio_context->reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

    // Wait 1 ms
    DelayMs( 1 );

    // Configure RESET as input
    GpioInit( &radio_context->reset, RADIO_RESET, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    // Wait 6 ms
    DelayMs( 6 );
}

uint32_t sx127x_hal_get_dio_1_pin_state( const sx127x_t* radio )
{
    radio_context_t* radio_context = ( radio_context_t* ) ( radio->hal_context );

    return GpioRead( &radio_context->dio_1 );
}

sx127x_hal_status_t sx127x_hal_timer_start( const sx127x_t* radio, const uint32_t time_in_ms,
                                            void ( *callback )( void* context ) )
{
    TimerInit( &timer_timeout, callback );
    TimerSetContext( &timer_timeout, ( void* ) radio );
    TimerSetValue( &timer_timeout, time_in_ms );
    TimerStart( &timer_timeout );

    return SX127X_HAL_STATUS_OK;
}

sx127x_hal_status_t sx127x_hal_timer_stop( const sx127x_t* radio )
{
    TimerStop( &timer_timeout );

    return SX127X_HAL_STATUS_OK;
}

bool sx127x_hal_timer_is_started( const sx127x_t* radio )
{
    return TimerIsStarted( &timer_timeout );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

/* --- EOF ------------------------------------------------------------------ */
