/**
 * @file      ral_sx126x_bsp.h
 *
 * @brief     Board Support Package for the SX126x-specific RAL.
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

#ifndef RAL_SX126X_BSP_H__
#define RAL_SX126X_BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include "ral_defs.h"
#include "sx126x.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

typedef struct ral_sx126x_bsp_tx_cfg_input_params_s
{
    int8_t   system_output_pwr_in_dbm;
    uint32_t freq_in_hz;
} ral_sx126x_bsp_tx_cfg_input_params_t;

typedef struct ral_sx126x_bsp_tx_cfg_output_params_s
{
    sx126x_pa_cfg_params_t pa_cfg;
    sx126x_ramp_time_t     pa_ramp_time;
    int8_t                 chip_output_pwr_in_dbm_configured;
    int8_t                 chip_output_pwr_in_dbm_expected;
} ral_sx126x_bsp_tx_cfg_output_params_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/**
 * Get the regulator mode configuration
 *
 * @param [in] context Chip implementation context
 * @param [out] reg_mode
 */
void ral_sx126x_bsp_get_reg_mode( const void* context, sx126x_reg_mod_t* reg_mode );

/**
 * Get the internal RF switch configuration
 *
 * @param [in] context Chip implementation context
 * @param [out] dio2_is_set_as_rf_switch
 */
void ral_sx126x_bsp_get_rf_switch_cfg( const void* context, bool* dio2_is_set_as_rf_switch );

/**
 * Get the Tx-related configuration (power amplifier configuration, output power and ramp time) to be applied to the
 * chip
 *
 * @param [in] context Chip implementation context
 * @param [in] input_params Parameters used to compute the chip configuration
 * @param [out] output_params Parameters to be configured in the chip
 */
void ral_sx126x_bsp_get_tx_cfg( const void* context, const ral_sx126x_bsp_tx_cfg_input_params_t* input_params,
                                ral_sx126x_bsp_tx_cfg_output_params_t* output_params );

/**
 * Get the XOSC configuration
 *
 * @remark If no TCXO is present, this function should set tcxo_is_radio_controlled to false, and return.
 *
 * @param [in] context Chip implementation context
 * @param [out] tcxo_is_radio_controlled Let the caller know if there is a radio-controlled TCXO
 * @param [out] supply_voltage TCXO supply voltage parameter
 * @param [out] startup_time_in_tick TCXO setup time in clock tick
 */
void ral_sx126x_bsp_get_xosc_cfg( const void* context, bool* tcxo_is_radio_controlled,
                                  sx126x_tcxo_ctrl_voltages_t* supply_voltage, uint32_t* startup_time_in_tick );

/**
 * Get the trimming capacitor values
 *
 * @param [in] context Chip implementation context
 * @param [out] trimming_cap_xta Value for the trimming capacitor connected to XTA pin
 * @param [out] trimming_cap_xtb Value for the trimming capacitor connected to XTB pin
 */
void ral_sx126x_bsp_get_trim_cap( const void* context, uint8_t* trimming_cap_xta, uint8_t* trimming_cap_xtb );

/**
 * Get the trimming capacitor values
 *
 * @param [in] context Chip implementation context
 * @param [out] rx_boost_is_activated Let the caller know if the RX boosted mode is activated
 */
void ral_sx126x_bsp_get_rx_boost_cfg( const void* context, bool* rx_boost_is_activated );

/**
 * Get the OCP (Over Current Protection) value
 *
 * @param [in] context Chip implementation context
 * @param [out] ocp_in_step_of_2_5_ma OCP value given in steps of 2.5 mA
 */
void ral_sx126x_bsp_get_ocp_value( const void* context, uint8_t* ocp_in_step_of_2_5_ma );

#ifdef __cplusplus
}
#endif

#endif  // RAL_SX126X_BSP_H__

/* --- EOF ------------------------------------------------------------------ */
