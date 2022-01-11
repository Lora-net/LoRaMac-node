/*!
 * \file  ral_sx127x_bsp.c
 *
 * \brief Implements the BSP (BoardSpecificPackage) HAL functions for SX127X
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
#include "radio_board.h"
#include "ral_sx127x_bsp.h"
#include "sx127x.h"

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

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void ral_sx127x_bsp_get_tx_cfg( const void* context, const ral_sx127x_bsp_tx_cfg_input_params_t* input_params,
                                ral_sx127x_bsp_tx_cfg_output_params_t* output_params )
{
    int8_t pwr = input_params->system_output_pwr_in_dbm;

#if defined( SX1272MB2DAS )
    output_params->pa_cfg.pa_select           = SX127X_PA_SELECT_RFO;
    output_params->pa_cfg.is_20_dbm_output_on = false;
#elif defined( SX1276MB1LAS )
    if( input_params->freq_in_hz > RF_FREQUENCY_MID_BAND_THRESHOLD )
    {
        output_params->pa_cfg.pa_select           = SX127X_PA_SELECT_BOOST;
        output_params->pa_cfg.is_20_dbm_output_on = true;
    }
    else
    {
        output_params->pa_cfg.pa_select           = SX127X_PA_SELECT_RFO;
        output_params->pa_cfg.is_20_dbm_output_on = false;
    }
#elif defined( SX1276MB1MAS )
    output_params->pa_cfg.pa_select           = SX127X_PA_SELECT_RFO;
    output_params->pa_cfg.is_20_dbm_output_on = false;
#else
#error "Please define a radio shield to be used"
#endif

    if( ( output_params->pa_cfg.pa_select == SX127X_PA_SELECT_BOOST ) )
    {  // PA_BOOST
        if( output_params->pa_cfg.is_20_dbm_output_on == true )
        {
            if( pwr < 5 )
            {
                pwr = 5;
            }
            if( pwr > 20 )
            {
                pwr = 20;
            }
        }
        else
        {
            if( pwr < 2 )
            {
                pwr = 2;
            }
            if( pwr > 17 )
            {
                pwr = 17;
            }
        }
    }
    else
    {  // RFO
        if( pwr < -4 )
        {
            pwr = -4;
        }
        if( pwr > 15 )
        {
            pwr = 15;
        }
    }

    output_params->chip_output_pwr_in_dbm_configured = pwr;
    output_params->chip_output_pwr_in_dbm_expected   = pwr;
    output_params->pa_ramp_time                      = SX127X_RAMP_40_US;
}

void ral_sx127x_bsp_get_ocp_value( const void* context, uint8_t* ocp_trim_value )
{
    // Do nothing, let the driver choose the default values
}

ral_status_t ral_sx127x_bsp_init( const ral_t* ral_context, radio_board_dio_irq_handler dio_irq )
{
    ral_status_t status         = RAL_STATUS_ERROR;
    sx127x_t*    sx127x_context = radio_board_get_sx127x_context_reference( );

    // Attach interrupt handler to SX126x virtual irq pin
    sx127x_irq_attach( sx127x_context, dio_irq, ( void* ) sx127x_context );

    status = ( ral_status_t ) sx127x_reset( sx127x_context );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ( ral_status_t ) sx127x_set_standby( sx127x_context );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );

    return ral_init( ral_context );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

/* --- EOF ------------------------------------------------------------------ */
