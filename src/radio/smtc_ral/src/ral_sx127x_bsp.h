/**
 * @file      ral_sx127x_bsp.h
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

#ifndef RAL_SX127X_BSP_H
#define RAL_SX127X_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include "ral_defs.h"
#include "sx127x.h"

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

typedef struct ral_sx127x_bsp_tx_cfg_input_params_s
{
    int8_t   system_output_pwr_in_dbm;
    uint32_t freq_in_hz;
} ral_sx127x_bsp_tx_cfg_input_params_t;

typedef struct ral_sx127x_bsp_tx_cfg_output_params_s
{
    sx127x_pa_cfg_params_t pa_cfg;
    sx127x_ramp_time_t     pa_ramp_time;
    int8_t                 chip_output_pwr_in_dbm_configured;
    int8_t                 chip_output_pwr_in_dbm_expected;
} ral_sx127x_bsp_tx_cfg_output_params_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/**
 * Get the Tx-related configuration (power amplifier configuration, output power and ramp time) to be applied to the
 * chip
 *
 * @param [in] context Chip implementation context
 * @param [in] input_params Parameters used to compute the chip configuration
 * @param [out] output_params Parameters to be configured in the chip
 */
void ral_sx127x_bsp_get_tx_cfg( const void* context, const ral_sx127x_bsp_tx_cfg_input_params_t* input_params,
                                ral_sx127x_bsp_tx_cfg_output_params_t* output_params );

/**
 * Get the OCP (Over Current Protection) value
 *
 * @remark Imax = 45 + 5 * ocp_trim_value   [mA] if ocp_trim_value <= 15 (120 [mA])
           Imax = -30 + 10 * ocp_trim_value [mA] if 15 < ocp_trim_value <= 27 (130 to 240 [mA])
           Imax = 240 [mA] for higher settings
           Default Imax = 100mA (ocp_trim_value = 0x0B)
 * @param [in] context Chip implementation context
 * @param [out] ocp_trim_value OCP trim value
 */
void ral_sx127x_bsp_get_ocp_value( const void* context, uint8_t* ocp_trim_value );

#ifdef __cplusplus
}
#endif

#endif  // RAL_SX127X_BSP_H

/* --- EOF ------------------------------------------------------------------ */
