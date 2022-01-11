/*!
 * \file  ral_sx126x_bsp.c
 *
 * \brief Implements the BSP (BoardSpecificPackage) HAL functions for SX126X
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
#include "ral_sx126x_bsp.h"

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

void ral_sx126x_bsp_get_reg_mode( const void* context, sx126x_reg_mod_t* reg_mode )
{
    *reg_mode = SX126X_REG_MODE_DCDC;
}

void ral_sx126x_bsp_get_rf_switch_cfg( const void* context, bool* dio2_is_set_as_rf_switch )
{
    // Antenna switch is controlled by the radio on this board design.
    *dio2_is_set_as_rf_switch = true;
}

void ral_sx126x_bsp_get_tx_cfg( const void* context, const ral_sx126x_bsp_tx_cfg_input_params_t* input_params,
                                ral_sx126x_bsp_tx_cfg_output_params_t* output_params )
{
    int8_t pwr = input_params->system_output_pwr_in_dbm;

    output_params->pa_cfg.pa_lut = 0x01;

#if defined( SX1261MBXBAS )
    // Clamp power if needed
    if( pwr > 15 )
    {
        pwr = 15;
    }
    else if( pwr < -17 )
    {
        pwr = -17;
    }

    // config pa according to power
    if( pwr == 15 )
    {
        output_params->pa_cfg.device_sel                 = 0x01;  // select SX1261 device
        output_params->pa_cfg.hp_max                     = 0x00;  // not used on sx1261
        output_params->pa_cfg.pa_duty_cycle              = 0x06;
        output_params->chip_output_pwr_in_dbm_configured = 14;
        output_params->chip_output_pwr_in_dbm_expected   = 15;
    }
    else if( pwr == 14 )
    {
        output_params->pa_cfg.device_sel                 = 0x01;  // select SX1261 device
        output_params->pa_cfg.hp_max                     = 0x00;  // not used on sx1261
        output_params->pa_cfg.pa_duty_cycle              = 0x04;
        output_params->chip_output_pwr_in_dbm_configured = 14;
        output_params->chip_output_pwr_in_dbm_expected   = 14;
    }
    else
    {
        output_params->pa_cfg.device_sel                 = 0x01;  // select SX1261 device
        output_params->pa_cfg.hp_max                     = 0x00;  // not used on sx1261
        output_params->pa_cfg.pa_duty_cycle              = 0x04;
        output_params->chip_output_pwr_in_dbm_configured = pwr;
        output_params->chip_output_pwr_in_dbm_expected   = pwr;
    }
#elif defined( SX1262MBXCAS ) | defined( SX1262MBXDAS )
    // Clamp power if needed
    if( pwr > 22 )
    {
        pwr = 22;
    }
    else if( pwr < -9 )
    {
        pwr = -9;
    }
    if( pwr == 22 )
    {
        output_params->pa_cfg.device_sel                 = 0x00;  // select SX1262 device
        output_params->pa_cfg.hp_max                     = 0x07;  // to achieve 22dBm
        output_params->pa_cfg.pa_duty_cycle              = 0x04;
        output_params->chip_output_pwr_in_dbm_configured = 22;
        output_params->chip_output_pwr_in_dbm_expected   = 22;
    }
    else
    {
        output_params->pa_cfg.device_sel                 = 0x00;  // select SX1262 device
        output_params->pa_cfg.hp_max                     = 0x07;  // to achieve 22dBm
        output_params->pa_cfg.pa_duty_cycle              = 0x04;
        output_params->chip_output_pwr_in_dbm_configured = pwr;
        output_params->chip_output_pwr_in_dbm_expected   = pwr;
    }
#else
#error "Please define a radio shield to be used"
#endif
    output_params->pa_ramp_time = SX126X_RAMP_40_US;
}

void ral_sx126x_bsp_get_xosc_cfg( const void* context, bool* tcxo_is_radio_controlled,
                                  sx126x_tcxo_ctrl_voltages_t* supply_voltage, uint32_t* startup_time_in_tick )
{
#if defined( SX1261MBXBAS ) || defined( SX1262MBXDAS )
    // No TCXO component available on this board design.
    *tcxo_is_radio_controlled = false;
#elif defined( SX1262MBXCAS )
    // TCXO is controlled by the radio on this board design.
    *tcxo_is_radio_controlled = true;
    *supply_voltage           = SX126X_TCXO_CTRL_1_8V;
    *startup_time_in_tick     = 164;  // 5ms in 30.52µs ticks
#else
#error "Please define a radio shield to be used"
#endif
}

void ral_sx126x_bsp_get_trim_cap( const void* context, uint8_t* trimming_cap_xta, uint8_t* trimming_cap_xtb )
{
    // No trimimming use radio default
    *trimming_cap_xta = 0;
    *trimming_cap_xtb = 0;
}

void ral_sx126x_bsp_get_rx_boost_cfg( const void* context, bool* rx_boost_is_activated )
{
    *rx_boost_is_activated = false;
}

void ral_sx126x_bsp_get_ocp_value( const void* context, uint8_t* ocp_in_step_of_2_5_ma )
{
    // Do nothing, let the driver choose the default values
}

ral_status_t ral_sx126x_bsp_init( const ral_t* ral_context, radio_board_dio_irq_handler dio_irq )
{
    ral_status_t status = RAL_STATUS_ERROR;
        radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    // Attach interrupt handler to radio irq pin
    GpioSetInterrupt( &radio_context->dio_1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY,
                      dio_irq );

    status = ( ral_status_t ) sx126x_reset( radio_context );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ( ral_status_t ) sx126x_set_standby( radio_context, SX126X_STANDBY_CFG_RC );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );

    uint8_t trimming_cap_xta = 0;
    uint8_t trimming_cap_xtb = 0;
    ral_sx126x_bsp_get_trim_cap( radio_context, &trimming_cap_xta, &trimming_cap_xtb );
    status = ( ral_status_t ) sx126x_set_trimming_capacitor_values( radio_context, trimming_cap_xta,
                                                                    trimming_cap_xtb );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ral_init( ral_context );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

/* --- EOF ------------------------------------------------------------------ */
