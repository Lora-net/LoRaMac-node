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
#include "board-config.h"
#include "delay.h"
#include "radio_board.h"
#include "ral_sx127x.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

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
 * Antenna switch GPIO pins objects
 */
Gpio_t radio_sw_ctrl_1;
Gpio_t radio_sw_ctrl_2;
Gpio_t radio_pwr_amp_ctrl;

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
    GpioInit( &radio_sw_ctrl_1, RADIO_SWITCH_CTRL1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_sw_ctrl_2, RADIO_SWITCH_CTRL2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_pwr_amp_ctrl, RADIO_PWRAMP_CTRL, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void radio_board_deinit_io( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->dio_0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_sw_ctrl_1, RADIO_SWITCH_CTRL1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioInit( &radio_sw_ctrl_2, RADIO_SWITCH_CTRL2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioInit( &radio_pwr_amp_ctrl, RADIO_PWRAMP_CTRL, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
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
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    if( is_tx_on == true )
    {
        ral_sx127x_bsp_tx_cfg_input_params_t tx_cfg_in_params = {
            .freq_in_hz               = radio_context->radio_params.rf_freq_in_hz,
            .system_output_pwr_in_dbm = radio_context->radio_params.tx_rf_pwr_in_dbm,
        };
        ral_sx127x_bsp_tx_cfg_output_params_t tx_cfg_out_params;

        ral_sx127x_bsp_get_tx_cfg( radio_context, &tx_cfg_in_params, &tx_cfg_out_params );
        if( tx_cfg_out_params.pa_cfg.pa_select == SX127X_PA_SELECT_BOOST )
        {
            GpioWrite( &radio_sw_ctrl_1, 1 );
            GpioWrite( &radio_sw_ctrl_2, 0 );
        }
        else
        {
            GpioWrite( &radio_sw_ctrl_1, 0 );
            GpioWrite( &radio_sw_ctrl_2, 1 );
        }
    }
    else
    {
        GpioWrite( &radio_sw_ctrl_1, 1 );
        GpioWrite( &radio_sw_ctrl_2, 1 );
    }
}

void radio_board_start_radio_tcxo( void )
{
    // No TCXO component available on this board design. Nothing to do.
}

void radio_board_stop_radio_tcxo( void )
{
    // No TCXO component available on this board design. Nothing to do.
}

uint32_t radio_board_get_tcxo_wakeup_time_in_ms( void )
{
    // No TCXO component available on this board design.
    // return 0 ms delay
    return 0;
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
