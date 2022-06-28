/**
 * @file      ral_lr11xx_bsp.h
 *
 * @brief     Board Support Package for the LR11XX-specific RAL.
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

#ifndef RAL_LR11XX_BSP_H__
#define RAL_LR11XX_BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include "ral_defs.h"
#include "lr11xx_radio_types.h"
#include "lr11xx_system_types.h"

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

typedef struct ral_lr11xx_bsp_tx_cfg_input_params_s
{
    int8_t   system_output_pwr_in_dbm;
    uint32_t freq_in_hz;
} ral_lr11xx_bsp_tx_cfg_input_params_t;

typedef struct ral_lr11xx_bsp_tx_cfg_output_params_s
{
    lr11xx_radio_pa_cfg_t    pa_cfg;
    lr11xx_radio_ramp_time_t pa_ramp_time;
    int8_t                   chip_output_pwr_in_dbm_configured;
    int8_t                   chip_output_pwr_in_dbm_expected;
} ral_lr11xx_bsp_tx_cfg_output_params_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/**
 * Get the internal RF switch configuration to be applied to the chip
 *
 * @param [in] context Chip implementation context
 * @param [out] rf_switch_cfg RF switch configuration to be applied to the chip
 */
void ral_lr11xx_bsp_get_rf_switch_cfg( const void* context, lr11xx_system_rfswitch_cfg_t* rf_switch_cfg );

/**
 * Get the Tx-related configuration (power amplifier configuration, output power and ramp time) to be applied to the
 * chip
 *
 * @param [in] context Chip implementation context
 * @param [in] input_params Parameters used to compute the chip configuration
 * @param [out] output_params Parameters to be configured in the chip
 */
void ral_lr11xx_bsp_get_tx_cfg( const void* context, const ral_lr11xx_bsp_tx_cfg_input_params_t* input_params,
                                ral_lr11xx_bsp_tx_cfg_output_params_t* output_params );

/**
 * Get the regulator mode
 *
 * @param [in] context Chip implementation context
 * @param [out] reg_mode System regulator mode
 */
void ral_lr11xx_bsp_get_reg_mode( const void* context, lr11xx_system_reg_mode_t* reg_mode );

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
void ral_lr11xx_bsp_get_xosc_cfg( const void* context, bool* tcxo_is_radio_controlled,
                                  lr11xx_system_tcxo_supply_voltage_t* supply_voltage, uint32_t* startup_time_in_tick );

/**
 * Get the CRC on SPI state
 *
 * @param [in] context Chip implementation context
 * @param [out] crc_is_activated Let the caller know if the CRC is activated
 */
void ral_lr11xx_bsp_get_crc_state( const void* context, bool* crc_is_activated );

/**
 * Get the RSSI calibration table
 *
 * @param [in] context Chip implementation context
 * @param [in] freq_in_hz RF frequency in Hertz
 * @param [out] rssi_calibration_table Pointer to a structure holding the RSSI calibration table
 */
void ral_lr11xx_bsp_get_rssi_calibration_table( const void* context, const uint32_t freq_in_hz,
                                                lr11xx_radio_rssi_calibration_table_t* rssi_calibration_table );

#ifdef __cplusplus
}
#endif

#endif  // RAL_LR11XX_BSP_H__

/* --- EOF ------------------------------------------------------------------ */
