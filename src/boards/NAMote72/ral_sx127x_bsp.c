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
       PD_2=0  PD_2=1
op pab  rfo     rfo
0  4.6  18.5    27.0
1  5.6  21.1    28.1
2  6.7  23.3    29.1
3  7.7  25.3    30.1
4  8.8  26.2    30.7
5  9.8  27.3    31.2
6  10.7 28.1    31.6
7  11.7 28.6    32.2
8  12.8 29.2    32.4
9  13.7 29.9    32.9
10 14.7 30.5    33.1
11 15.6 30.8    33.4
12 16.4 30.9    33.6
13 17.1 31.0    33.7
14 17.8 31.1    33.7
15 18.4 31.1    33.7
*/
//                            txpow:   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14  15  16  17  18  19
static const uint8_t pab_table[20] = { 0, 0, 0, 0, 0, 1, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15 };

//                            txpow:  20 21 22 23 24 25 26 27 28 29 30
static const uint8_t rfo_table[11] = { 1, 1, 1, 2, 2, 3, 4, 5, 6, 8, 9 };

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

    output_params->pa_cfg.is_20_dbm_output_on = false;

    if( pwr > 19 )
    {
        output_params->pa_cfg.pa_select                  = SX127X_PA_SELECT_RFO;
        output_params->chip_output_pwr_in_dbm_configured = rfo_table[pwr - 20];
    }
    else
    {
        output_params->pa_cfg.pa_select                  = SX127X_PA_SELECT_BOOST;
        output_params->chip_output_pwr_in_dbm_configured = pab_table[MAX( pwr, 0 )];
    }

    output_params->chip_output_pwr_in_dbm_expected = pwr;
    output_params->pa_ramp_time                    = SX127X_RAMP_40_US;
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
