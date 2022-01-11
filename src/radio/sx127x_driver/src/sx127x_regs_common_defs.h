/**
 * @file      sx127x_regs_common_defs.h
 *
 * @brief     SX1272/3 and SX1276/7/8/9  common registers definition
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
#ifndef SX127X_REGS_COMMON_DEFS_H
#define SX127X_REGS_COMMON_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stddef.h>   // NULL
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type

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

/**
 * @brief Common registers definition
 *
 * @enum sx127x_reg_common_e
 * @typedef sx127x_reg_common_t
 */
typedef enum sx127x_reg_common_e
{
    SX127X_REG_COMMON_FIFO          = 0x00,
    SX127X_REG_COMMON_OP_MODE       = 0x01,
    SX127X_REG_COMMON_FRF_MSB       = 0x06,
    SX127X_REG_COMMON_FRF_MID       = 0x07,
    SX127X_REG_COMMON_FRF_LSB       = 0x08,
    SX127X_REG_COMMON_PA_CONFIG     = 0x09,
    SX127X_REG_COMMON_PA_RAMP       = 0x0A,
    SX127X_REG_COMMON_OCP           = 0x0B,
    SX127X_REG_COMMON_LNA           = 0x0C,
    SX127X_REG_COMMON_DIO_MAPPING_1 = 0x40,
    SX127X_REG_COMMON_DIO_MAPPING_2 = 0x41,
    SX127X_REG_COMMON_VERSION       = 0x42,
} sx127x_reg_common_t;

/**
 * @brief Common registers definition
 *
 * @enum sx1272_reg_common_e
 * @typedef sx1272_reg_common_t
 */
typedef enum sx1272_reg_common_e
{
    SX1272_REG_COMMON_AGC_REF      = 0x43,
    SX1272_REG_COMMON_AGC_THRESH_1 = 0x44,
    SX1272_REG_COMMON_AGC_THRESH_2 = 0x45,
    SX1272_REG_COMMON_AGC_THRESH_3 = 0x46,
    SX1272_REG_COMMON_PLL_HOP      = 0x4B,
    SX1272_REG_COMMON_TCXO         = 0x58,
    SX1272_REG_COMMON_PA_DAC       = 0x5A,
    SX1272_REG_COMMON_PLL          = 0x5C,
    SX1272_REG_COMMON_PLL_LOW_PN   = 0x5E,
    SX1272_REG_COMMON_PA_MANUAL    = 0x63,
    SX1272_REG_COMMON_FORMER_TEMP  = 0x6C,
    SX1272_COMMON_BITRATE_FRAC     = 0x70,
} sx1272_reg_common_t;

/**
 * @brief Common registers definition
 *
 * @enum sx1276_reg_common_e
 * @typedef sx1276_reg_common_t
 */
typedef enum sx1276_reg_common_e
{
    SX1276_REG_COMMON_TCXO         = 0x4B,
    SX1276_REG_COMMON_PA_DAC       = 0x4D,
    SX1276_REG_COMMON_FORMER_TEMP  = 0x5B,
    SX1276_REG_COMMON_AGC_REF      = 0x61,
    SX1276_REG_COMMON_AGC_THRESH_1 = 0x62,
    SX1276_REG_COMMON_AGC_THRESH_2 = 0x63,
    SX1276_REG_COMMON_AGC_THRESH_3 = 0x64,
    SX1276_REG_COMMON_PLL_LF       = 0x70,
} sx1276_reg_common_t;

/**
 * @brief SX127X LongRangeMode - This bit can be modified only in Sleep mode. A write operation on other device modes is
 * ignored.
 */
enum sx127x_reg_common_op_mode_long_range_mode_e
{
    SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_MASK = ~( 1 << 7 ),
    SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_GFSK = ( 0 << 7 ),  // Default
    SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_LORA = ( 1 << 7 ),
};

/**
 * @brief SX127X AccessSharedReg - This bit operates when device is in LoRa mode; if set it allows access to FSK
 * registers page located in address space
 */
enum sx127x_reg_common_op_mode_access_shared_reg_e
{
    SX127X_REG_COMMON_OP_MODE_ACCESS_SHARED_REG_MASK    = ~( 1 << 6 ),
    SX127X_REG_COMMON_OP_MODE_ACCESS_SHARED_REG_LORA    = ( 0 << 6 ),  // Default
    SX127X_REG_COMMON_OP_MODE_ACCESS_SHARED_REG_FSK_OOK = ( 1 << 6 ),
};

/**
 * @brief SX127X ModulationType - Modulation scheme
 */
enum sx127x_reg_common_op_mode_modulation_type_e
{
    SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_MASK = ~( 3 << 5 ),
    SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_FSK  = ( 0 << 5 ),  // Default
    SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_OOK  = ( 1 << 5 ),
};

/**
 * @brief SX1272 ModulationShaping - Data shaping (GFSK only)
 */
enum sx1272_reg_common_op_mode_gfsk_mod_shape_e
{
    SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_MASK  = ~( 3 << 3 ),
    SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_OFF   = ( 0 << 3 ),  // Default
    SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_BT_1  = ( 1 << 3 ),
    SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_BT_05 = ( 2 << 3 ),
    SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_BT_03 = ( 3 << 3 ),
};

/**
 * @brief SX1272 ModulationShaping - Data shaping (OOK only)
 */
enum sx1272_reg_common_op_mode_ook_mod_shape_e
{
    SX1272_REG_COMMON_OP_MODE_OOK_MOD_SHAPE_MASK          = ~( 3 << 3 ),
    SX1272_REG_COMMON_OP_MODE_OOK_MOD_SHAPE_OFF           = ( 0 << 3 ),  // Default
    SX1272_REG_COMMON_OP_MODE_OOK_MOD_SHAPE_F_CUT_OFF_BR  = ( 1 << 3 ),
    SX1272_REG_COMMON_OP_MODE_OOK_MOD_SHAPE_F_CUT_OFF_2BR = ( 2 << 3 ),
};

/**
 * @brief SX1276 LowFrequencyModeOn - Access Low Frequency Mode registers
 */
enum sx1276_reg_common_op_mode_low_frequency_mode_on_e
{
    SX1276_REG_COMMON_OP_MODE_LOW_FREQUENCY_MODE_ON_MASK = ~( 1 << 3 ),
    SX1276_REG_COMMON_OP_MODE_LOW_FREQUENCY_MODE_ON_HF   = ( 0 << 3 ),
    SX1276_REG_COMMON_OP_MODE_LOW_FREQUENCY_MODE_ON_LF   = ( 1 << 3 ),  // Default
};

/**
 * @brief SX127X transceiver modes
 */
enum sx127x_reg_common_op_mode_mode_e
{
    SX127X_REG_COMMON_OP_MODE_MODE_MASK           = ~( 7 << 0 ),
    SX127X_REG_COMMON_OP_MODE_MODE_SLEEP          = ( 0 << 0 ),
    SX127X_REG_COMMON_OP_MODE_MODE_STANDBY        = ( 1 << 0 ),  // Default
    SX127X_REG_COMMON_OP_MODE_MODE_FS_TX          = ( 2 << 0 ),
    SX127X_REG_COMMON_OP_MODE_MODE_TX             = ( 3 << 0 ),
    SX127X_REG_COMMON_OP_MODE_MODE_FS_RX          = ( 4 << 0 ),
    SX127X_REG_COMMON_OP_MODE_MODE_RX             = ( 5 << 0 ),
    SX127X_REG_COMMON_OP_MODE_MODE_LORA_RX_SINGLE = ( 6 << 0 ),
    SX127X_REG_COMMON_OP_MODE_MODE_LORA_CAD       = ( 7 << 0 ),
};

/**
 * @brief SX127X PaSelect - Selects PA output pin
 */
enum sx127x_reg_common_pa_config_pa_select_e
{
    SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_MASK  = ~( 1 << 7 ),
    SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_RFO   = ( 0 << 7 ),  // default
    SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_BOOST = ( 1 << 7 ),
};

/**
 * @brief SX1276 MaxPower [0..7] - Select max output power: Pmax=10.8+0.6*MaxPower [dBm]
 */
enum sx1276_reg_common_pa_config_max_power_e
{
    SX1276_REG_COMMON_PA_CONFIG_MAX_POWER_MASK = ~( 7 << 4 ),
};

/**
 * @brief SX1272 OutputPower [0..15] - Pout=-1+OutputPower if PaSelect = 0 (RFO pins)
 *                                     Pout= 2+OutputPower if PaSelect = 1 (PA_BOOST pin)
 *        SX1276 OutputPower [0..15] - Pout=Pmax-(15-OutputPower) if PaSelect = 0 (RFO pins)
 *                                     Pout=17-(15-OutputPower)   if PaSelect = 1 (PA_BOOST pin)
 */
enum sx127x_reg_common_pa_config_output_power_e
{
    SX127X_REG_COMMON_PA_CONFIG_OUTPUT_POWER_MASK = ~( 15 << 0 ),
};

/**
 * @brief SX1276 ModulationShaping - Data shaping (GFSK only)
 */
enum sx1276_reg_common_pa_ramp_gfsk_mod_shape_e
{
    SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_MASK  = ~( 3 << 5 ),
    SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_OFF   = ( 0 << 5 ),  // Default
    SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_BT_1  = ( 1 << 5 ),
    SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_BT_05 = ( 2 << 5 ),
    SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_BT_03 = ( 3 << 5 ),
};

/**
 * @brief SX1276 ModulationShaping - Data shaping (OOK only)
 */
enum sx1276_reg_common_pa_ramp_ook_mod_shape_e
{
    SX1276_REG_COMMON_PA_RAMP_OOK_MOD_SHAPE_MASK          = ~( 3 << 5 ),
    SX1276_REG_COMMON_PA_RAMP_OOK_MOD_SHAPE_OFF           = ( 0 << 5 ),  // Default
    SX1276_REG_COMMON_PA_RAMP_OOK_MOD_SHAPE_F_CUT_OFF_BR  = ( 1 << 5 ),
    SX1276_REG_COMMON_PA_RAMP_OOK_MOD_SHAPE_F_CUT_OFF_2BR = ( 2 << 5 ),
};

/**
 * @brief SX1272 LowPnTxPllOff - Select a higher power, lower phase noise PLL only when the transmitter is used
 */
enum sx1272_reg_common_pa_ramp_low_pn_tx_pll_off_e
{
    SX1272_REG_COMMON_PA_RAMP_LOW_PN_TX_PLL_MASK = ~( 1 << 4 ),
    SX1272_REG_COMMON_PA_RAMP_LOW_PN_TX_PLL_ON   = ( 0 << 4 ),
    SX1272_REG_COMMON_PA_RAMP_LOW_PN_TX_PLL_OFF  = ( 1 << 4 ),  // default
};

/**
 * @brief SX127X PaRamp - Rise/Fall time of ramp up/down in FSK/OOK/LoRa
 */
enum sx127x_reg_common_pa_ramp_pa_ramp_e
{
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_MASK    = ~( 15 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_3400_US = ( 0 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_2000_US = ( 1 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_1000_US = ( 2 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_500_US  = ( 3 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_250_US  = ( 4 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_125_US  = ( 5 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_100_US  = ( 6 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_62_US   = ( 7 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_50_US   = ( 8 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_40_US   = ( 9 << 0 ),  // Default
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_31_US   = ( 10 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_25_US   = ( 11 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_20_US   = ( 12 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_15_US   = ( 13 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_12_US   = ( 14 << 0 ),
    SX127X_REG_COMMON_PA_RAMP_PA_RAMP_10_US   = ( 15 << 0 ),
};

/**
 * @brief SX127X OcpOn - Enables overload current protection (OCP) for the PA
 */
enum sx127x_reg_common_ocp_ocp_on_e
{
    SX127X_REG_COMMON_OCP_OCP_ON_MASK = ~( 1 << 5 ),
    SX127X_REG_COMMON_OCP_OCP_ON_OFF  = ( 0 << 5 ),
    SX127X_REG_COMMON_OCP_OCP_ON_ON   = ( 1 << 5 ),  // Default
};

/**
 * @brief SX127X OcpTrim - Trimming of OCP current
 *                         Imax =  45+ 5*OcpTrim [mA]   if      OcpTrim <= 15 (120 mA)
 *                         Imax = -30+10*OcpTrim [mA]   if 15 < OcpTrim <= 27 (130 to 240 mA)
 *                         Imax = 240mA for higher settings
 *                         Default Imax = 100mA
 */
enum sx127x_reg_common_ocp_ocp_trim_e
{
    SX127X_REG_COMMON_OCP_OCP_TRIM_MASK   = ~( 31 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_045_MA = ( 0 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_050_MA = ( 1 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_055_MA = ( 2 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_060_MA = ( 3 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_065_MA = ( 4 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_070_MA = ( 5 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_075_MA = ( 6 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_080_MA = ( 7 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_085_MA = ( 8 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_090_MA = ( 9 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_095_MA = ( 10 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_100_MA = ( 11 << 0 ),  // Default
    SX127X_REG_COMMON_OCP_OCP_TRIM_105_MA = ( 12 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_110_MA = ( 13 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_115_MA = ( 14 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_120_MA = ( 15 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_130_MA = ( 16 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_140_MA = ( 17 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_150_MA = ( 18 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_160_MA = ( 19 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_170_MA = ( 20 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_180_MA = ( 21 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_190_MA = ( 22 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_200_MA = ( 23 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_210_MA = ( 24 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_220_MA = ( 25 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_230_MA = ( 26 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_240_MA = ( 27 << 0 ),
    SX127X_REG_COMMON_OCP_OCP_TRIM_MAX    = ( 28 << 0 ),
};

/**
 * @brief SX127X LnaGain - LNA gain setting
 */
enum sx127x_reg_common_lna_lna_gain_e
{
    SX127X_REG_COMMON_LNA_LNA_GAIN_MASK = ~( 7 << 5 ),
    SX127X_REG_COMMON_LNA_LNA_GAIN_G1   = ( 1 << 5 ),  // Default
    SX127X_REG_COMMON_LNA_LNA_GAIN_G2   = ( 2 << 5 ),
    SX127X_REG_COMMON_LNA_LNA_GAIN_G3   = ( 3 << 5 ),
    SX127X_REG_COMMON_LNA_LNA_GAIN_G4   = ( 4 << 5 ),
    SX127X_REG_COMMON_LNA_LNA_GAIN_G5   = ( 5 << 5 ),
    SX127X_REG_COMMON_LNA_LNA_GAIN_G6   = ( 6 << 5 ),
};

/**
 * @brief SX1276 LnaBoostLf - Low Frequency (RFI_LF) LNA current adjustment
 */
enum sx1276_reg_common_lna_lna_boost_lf_e
{
    SX1276_REG_COMMON_LNA_LNA_BOOST_LF_MASK = ~( 3 << 3 ),
    SX1276_REG_COMMON_LNA_LNA_BOOST_LF_OFF  = ( 0 << 3 ),  // Default
};

/**
 * @brief SX1272 LnaBoost - LNA current adjustment
 */
enum sx1272_reg_common_lna_lna_boost_e
{
    SX1272_REG_COMMON_LNA_LNA_BOOST_MASK = ~( 3 << 0 ),
    SX1272_REG_COMMON_LNA_LNA_BOOST_OFF  = ( 0 << 0 ),  // Default
    SX1272_REG_COMMON_LNA_LNA_BOOST_ON   = ( 3 << 0 ),
};

/**
 * @brief SX1276 LnaBoostHf - High Frequency (RFI_HF) LNA current adjustment
 */
enum sx1276_reg_common_lna_lna_boost_hf_e
{
    SX1276_REG_COMMON_LNA_LNA_BOOST_HF_MASK = ~( 3 << 0 ),
    SX1276_REG_COMMON_LNA_LNA_BOOST_HF_OFF  = ( 0 << 0 ),  // Default
    SX1276_REG_COMMON_LNA_LNA_BOOST_HF_ON   = ( 3 << 0 ),
};

/**
 * @brief SX127X Dio0Mapping - Mapping of pin DIO0
 */
enum sx127x_reg_common_dio_mapping_1_dio_0_e
{
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_MASK = ~( 3 << 6 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_00   = ( 0 << 6 ),  // Default
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_01   = ( 1 << 6 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_10   = ( 2 << 6 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_11   = ( 3 << 6 ),
};

/**
 * @brief SX127X Dio1Mapping - Mapping of pin DIO1
 */
enum sx127x_reg_common_dio_mapping_1_dio_1_e
{
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_MASK = ~( 3 << 4 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_00   = ( 0 << 4 ),  // Default
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_01   = ( 1 << 4 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_10   = ( 2 << 4 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_11   = ( 3 << 4 ),
};

/**
 * @brief SX127X Dio2Mapping - Mapping of pin DIO2
 */
enum sx127x_reg_common_dio_mapping_1_dio_2_e
{
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_MASK = ~( 3 << 2 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_00   = ( 0 << 2 ),  // Default
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_01   = ( 1 << 2 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_10   = ( 2 << 2 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_11   = ( 3 << 2 ),
};

/**
 * @brief SX127X Dio3Mapping - Mapping of pin DIO3
 */
enum sx127x_reg_common_dio_mapping_1_dio_3_e
{
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_MASK = ~( 3 << 0 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_00   = ( 0 << 0 ),  // Default
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_01   = ( 1 << 0 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_10   = ( 2 << 0 ),
    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_11   = ( 3 << 0 ),
};

/**
 * @brief SX127X Dio4Mapping - Mapping of pin DIO4
 */
enum sx127x_reg_common_dio_mapping_2_dio_4_e
{
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_MASK = ~( 3 << 6 ),
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_00   = ( 0 << 6 ),  // Default
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_01   = ( 1 << 6 ),
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_10   = ( 2 << 6 ),
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_11   = ( 3 << 6 ),
};

/**
 * @brief SX127X Dio5Mapping - Mapping of pin DIO5
 */
enum sx127x_reg_common_dio_mapping_2_dio_5_e
{
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_MASK = ~( 3 << 4 ),
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_00   = ( 0 << 4 ),  // Default
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_01   = ( 1 << 4 ),
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_10   = ( 2 << 4 ),
    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_11   = ( 3 << 4 ),
};

/**
 * @brief SX127X MapPreambleDetect - Allows the mapping of either Rssi Or PreambleDetect to the DIO pins
 */
enum sx127x_reg_common_dio_mapping_2_map_preable_detect_e
{
    SX127X_REG_COMMON_DIO_MAPPING_2_MAP_MASK            = ~( 1 << 0 ),
    SX127X_REG_COMMON_DIO_MAPPING_2_MAP_RSSI            = ( 0 << 0 ),  // Default
    SX127X_REG_COMMON_DIO_MAPPING_2_MAP_PREAMBLE_DETECT = ( 1 << 0 ),
};

/*
 * -----------------------------------------------------------------------------
 * --- SX1272 specific ---------------------------------------------------------
 */

/**
 * @brief SX1272 TcxoInputOn - Controls the crystal oscillator
 */
enum sx1272_reg_common_tcxo_tcxo_input_on_e
{
    SX1272_REG_COMMON_TCXO_TCXO_INPUT_MASK = ~( 1 << 4 ),
    SX1272_REG_COMMON_TCXO_TCXO_INPUT_OFF  = ( 0 << 4 ),  // Default
    SX1272_REG_COMMON_TCXO_TCXO_INPUT_ON   = ( 1 << 4 ),
};

/**
 * @brief SX1272 PaDac - Enables the +20dBm option on PA_BOOST pin
 */
enum sx1272_reg_common_pa_dac_pa_dac_e
{
    SX1272_REG_COMMON_PA_DAC_PA_DAC_MASK = ~( 7 << 0 ),
    SX1272_REG_COMMON_PA_DAC_PA_DAC_OFF  = ( 4 << 0 ),  // Default
    SX1272_REG_COMMON_PA_DAC_PA_DAC_ON   = ( 7 << 0 ),
};

/**
 * @brief SX1272 PllBandwidth - Controls the PLL bandwidth
 */
enum sx1272_reg_common_pll_pll_bandwidth_e
{
    SX1272_REG_COMMON_PLL_PLL_BANDWIDTH_MASK = ~( 3 << 6 ),
    SX1272_REG_COMMON_PLL_PLL_BANDWIDTH_75   = ( 0 << 6 ),
    SX1272_REG_COMMON_PLL_PLL_BANDWIDTH_150  = ( 1 << 6 ),
    SX1272_REG_COMMON_PLL_PLL_BANDWIDTH_225  = ( 2 << 6 ),
    SX1272_REG_COMMON_PLL_PLL_BANDWIDTH_300  = ( 3 << 6 ),  // Default
};

/**
 * @brief SX1272 PllBandwidth - Controls the Low Phase Noise PLL bandwidth
 */
enum sx1272_reg_common_pll_low_pn_pll_bandwidth_e
{
    SX1272_REG_COMMON_PLL_LOW_PN_PLL_BANDWIDTH_MASK = ~( 3 << 6 ),
    SX1272_REG_COMMON_PLL_LOW_PN_PLL_BANDWIDTH_75   = ( 0 << 6 ),
    SX1272_REG_COMMON_PLL_LOW_PN_PLL_BANDWIDTH_150  = ( 1 << 6 ),
    SX1272_REG_COMMON_PLL_LOW_PN_PLL_BANDWIDTH_225  = ( 2 << 6 ),
    SX1272_REG_COMMON_PLL_LOW_PN_PLL_BANDWIDTH_300  = ( 3 << 6 ),  // Default
};

/**
 * @brief SX1272 ManualPaControl - 0x01 enables manual PA control See AN1200.32
 */
enum sx1272_reg_common_pa_manual_manual_pa_control_e
{
    SX1272_REG_COMMON_PA_MANUAL_MANUAL_PA_CONTROL_MASK = ~( 1 << 4 ),
    SX1272_REG_COMMON_PA_MANUAL_MANUAL_PA_CONTROL_OFF  = ( 0 << 4 ),  // Default
    SX1272_REG_COMMON_PA_MANUAL_MANUAL_PA_CONTROL_ON   = ( 1 << 4 ),
};

/*
 * -----------------------------------------------------------------------------
 * --- SX1276 specific ---------------------------------------------------------
 */

/**
 * @brief SX1276 TcxoInputOn - Controls the crystal oscillator
 */
enum sx1276_reg_common_tcxo_tcxo_input_on_e
{
    SX1276_REG_COMMON_TCXO_TCXO_INPUT_MASK = ~( 1 << 4 ),
    SX1276_REG_COMMON_TCXO_TCXO_INPUT_OFF  = ( 0 << 4 ),  // Default
    SX1276_REG_COMMON_TCXO_TCXO_INPUT_ON   = ( 1 << 4 ),
};

/**
 * @brief SX1276 PaDac - Enables the +20dBm option on PA_BOOST pin
 */
enum sx1276_reg_common_pa_dac_pa_dac_e
{
    SX1276_REG_COMMON_PA_DAC_PA_DAC_MASK = ~( 7 << 0 ),
    SX1276_REG_COMMON_PA_DAC_PA_DAC_OFF  = ( 4 << 0 ),  // Default
    SX1276_REG_COMMON_PA_DAC_PA_DAC_ON   = ( 7 << 0 ),
};

/**
 * @brief SX1276 PllBandwidth - Controls the PLL bandwidth
 */
enum sx1276_reg_common_pll_lf_pll_bandwidth_e
{
    SX1276_REG_COMMON_PLL_LF_PLL_BANDWIDTH_MASK = ~( 3 << 6 ),
    SX1276_REG_COMMON_PLL_LF_PLL_BANDWIDTH_75   = ( 0 << 6 ),
    SX1276_REG_COMMON_PLL_LF_PLL_BANDWIDTH_150  = ( 1 << 6 ),
    SX1276_REG_COMMON_PLL_LF_PLL_BANDWIDTH_225  = ( 2 << 6 ),
    SX1276_REG_COMMON_PLL_LF_PLL_BANDWIDTH_300  = ( 3 << 6 ),  // Default
};

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

#ifdef __cplusplus
}
#endif

#endif  // SX127X_REGS_COMMON_DEFS_H

/* --- EOF ------------------------------------------------------------------ */
