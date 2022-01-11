/*!
 * \file  radio_board.c
 *
 * \brief Radio board Hardware Abstraction Layer
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
#include <stdlib.h>
#include <peripheral_clk_config.h>
#include <hal_ext_irq.h>
#include <hal_gpio.h>
#include "board-config.h"
#include "delay.h"
#include "radio_board.h"
#include "ral_sx127x.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#define RADIO_SHIELD_HAS_TCXO 1

#if( RADIO_SHIELD_HAS_TCXO == 1 )
#undef RADIO_BOARD_TCXO_WAKEUP_TIME
#define RADIO_BOARD_TCXO_WAKEUP_TIME 5  // 5 milliseconds
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*!
 * Board context
 */
static radio_context_t radio_context_reference;

/*!
 * SX127X context
 */
static sx127x_t sx127x_context_reference = {
    .hal_context = &radio_context_reference,
};

/*!
 * RAL context
 */
static ral_t ral_context_reference = RAL_SX127X_INSTANTIATE( &sx127x_context_reference );

/*!
 * TCXO power control pin
 */
Gpio_t tcxo_pwr;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

radio_context_t* radio_board_get_radio_context_reference( void )
{
    return &radio_context_reference;
}

sx127x_t* radio_board_get_sx127x_context_reference( void )
{
    return &sx127x_context_reference;
}

ral_t* radio_board_get_ral_context_reference( void )
{
    return &ral_context_reference;
}

ral_status_t radio_board_init( const ral_t* ral_context, radio_board_dio_irq_handler dio_irq )
{
    return ral_sx127x_bsp_init( ral_context, dio_irq );
}

void radio_board_init_io( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->dio_0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &tcxo_pwr, RADIO_TCXO_PWR_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    ext_irq_init( );
    gpio_set_pin_function( RADIO_DIO_0, PINMUX_PB16A_EIC_EXTINT0 );
    // Must be setup to be trigged on both edges. See CONF_EIC_SENSE11 under hpl_eic_config.h
    gpio_set_pin_function( RADIO_DIO_1, PINMUX_PA11A_EIC_EXTINT11 );
    gpio_set_pin_function( RADIO_DIO_2, PINMUX_PA12A_EIC_EXTINT12 );
}

void radio_board_deinit_io( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->dio_0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &tcxo_pwr, RADIO_TCXO_PWR_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void radio_board_init_dbg_io( void )
{
#if defined( USE_RADIO_DEBUG )
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    GpioInit( &radio_context->dbg_tx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dbg_rx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void radio_board_set_ant_switch( bool is_tx_on )
{
    // Antenna switch automatically done. Nothing to do.
}

void radio_board_start_radio_tcxo( void )
{
    GpioWrite( &tcxo_pwr, 1 );
    DelayMs( BOARD_TCXO_WAKEUP_TIME );
}

void radio_board_stop_radio_tcxo( void )
{
    GpioWrite( &tcxo_pwr, 0 );
}

uint32_t radio_board_get_tcxo_wakeup_time_in_ms( void )
{
    // TCXO component available on this board design has RADIO_BOARD_TCXO_WAKEUP_TIME ms wake up time.
    // return RADIO_BOARD_TCXO_WAKEUP_TIME ms delay
    return RADIO_BOARD_TCXO_WAKEUP_TIME;
}

radio_board_operating_mode_t radio_board_get_operating_mode( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    return radio_context->op_mode;
}

void radio_board_set_operating_mode( radio_board_operating_mode_t op_mode )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->op_mode = op_mode;
#if defined( USE_RADIO_DEBUG )
    switch( op_mode )
    {
    case RADIO_BOARD_OP_MODE_TX:
        GpioWrite( &radio_context->dbg_tx, 1 );
        GpioWrite( &radio_context->dbg_rx, 0 );
        break;
    case RADIO_BOARD_OP_MODE_RX:
    case RADIO_BOARD_OP_MODE_RX_C:
    case RADIO_BOARD_OP_MODE_RX_DC:
        GpioWrite( &radio_context->dbg_tx, 0 );
        GpioWrite( &radio_context->dbg_rx, 1 );
        break;
    default:
        GpioWrite( &radio_context->dbg_tx, 0 );
        GpioWrite( &radio_context->dbg_rx, 0 );
        break;
    }
#endif
}

uint32_t radio_board_get_dio_1_pin_state( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    return GpioRead( &radio_context->dio_1 );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

/* --- EOF ------------------------------------------------------------------ */
