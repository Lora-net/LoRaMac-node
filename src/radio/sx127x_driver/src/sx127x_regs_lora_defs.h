/**
 * @file      sx127x_regs_lora_defs.h
 *
 * @brief     SX1272/3 and SX1276/7/8/9  LoRa modem registers definition
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
#ifndef SX127X_REGS_LORA_DEFS_H
#define SX127X_REGS_LORA_DEFS_H

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
 * @brief LoRa modem registers definition
 *
 * @enum sx127x_reg_lora_e
 * @typedef sx127x_reg_lora_t
 */
typedef enum sx127x_reg_lora_e
{
    SX127X_REG_LORA_FIFO_ADDR_PTR           = 0x0D,
    SX127X_REG_LORA_FIFO_TX_BASE_ADDR       = 0x0E,
    SX127X_REG_LORA_FIFO_RX_BASE_ADDR       = 0x0F,
    SX127X_REG_LORA_FIFO_RX_CURRENT_ADDR    = 0x10,
    SX127X_REG_LORA_IRQ_FLAGS_MASK          = 0x11,
    SX127X_REG_LORA_IRQ_FLAGS               = 0x12,
    SX127X_REG_LORA_RX_NB_BYTES             = 0x13,
    SX127X_REG_LORA_RX_HEADER_CNT_VALUE_MSB = 0x14,
    SX127X_REG_LORA_RX_HEADER_CNT_VALUE_LSB = 0x15,
    SX127X_REG_LORA_RX_PACKET_CNT_VALUE_MSB = 0x16,
    SX127X_REG_LORA_RX_PACKET_CNT_VALUE_LSB = 0x17,
    SX127X_REG_LORA_MODEM_STAT              = 0x18,
    SX127X_REG_LORA_PACKET_SNR_VALUE        = 0x19,
    SX127X_REG_LORA_PACKET_RSSI_VALUE       = 0x1A,
    SX127X_REG_LORA_RSSI_VALUE              = 0x1B,
    SX127X_REG_LORA_HOP_CHANNEL             = 0x1C,
    SX127X_REG_LORA_MODEM_CONFIG_1          = 0x1D,
    SX127X_REG_LORA_MODEM_CONFIG_2          = 0x1E,
    SX127X_REG_LORA_SYMB_TIMEOUT_LSB        = 0x1F,
    SX127X_REG_LORA_PREAMBLE_MSB            = 0x20,
    SX127X_REG_LORA_PREAMBLE_LSB            = 0x21,
    SX127X_REG_LORA_PAYLOAD_LENGTH          = 0x22,
    SX127X_REG_LORA_MAX_PAYLOAD_LENGTH      = 0x23,
    SX127X_REG_LORA_HOP_PERIOD              = 0x24,
    SX127X_REG_LORA_FIFO_RX_BYTE_ADDR       = 0x25,
    SX127X_REG_LORA_FEI_MSB                 = 0x28,
    SX127X_REG_LORA_FEI_MID                 = 0x29,
    SX127X_REG_LORA_FEI_LSB                 = 0x2A,
    SX127X_REG_LORA_RSSI_WIDE_BAND          = 0x2C,
    SX127X_REG_LORA_DETECT_OPTIMIZE         = 0x31,
    SX127X_REG_LORA_INVERT_IQ               = 0x33,
    SX127X_REG_LORA_DETECTION_THRESHOLD     = 0x37,
    SX127X_REG_LORA_SYNC_WORD               = 0x39,
    SX127X_REG_LORA_INVERT_IQ_2             = 0x3B,
} sx127x_reg_lora_t;

/**
 * @brief LoRa modem registers definition
 *
 * @enum sx1276_reg_lora_e
 * @typedef sx1276_reg_lora_t
 */
typedef enum sx1276_reg_lora_e
{
    SX1276_REG_LORA_MODEM_CONFIG_3     = 0x26,
    SX1276_REG_LORA_IF_FREQ_1          = 0x2F,
    SX1276_REG_LORA_IF_FREQ_2          = 0x30,
    SX1276_REG_LORA_HIGH_BW_OPTIMIZE_1 = 0x36,
    SX1276_REG_LORA_HIGH_BW_OPTIMIZE_2 = 0x3A,
} sx1276_reg_lora_t;

/**
 * @brief SX127X IrqFlagsMask
 */
enum sx127x_reg_lora_irq_flags_mask_e
{
    SX127X_REG_LORA_IRQ_FLAGS_MASK_NONE                = ( 0 << 0 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_RX_TIMEOUT          = ( 1 << 7 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_RX_DONE             = ( 1 << 6 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_PAYLOAD_CRC_ERROR   = ( 1 << 5 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_VALID_HEADER        = ( 1 << 4 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_TX_DONE             = ( 1 << 3 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_CAD_DONE            = ( 1 << 2 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_FHSS_CHANGE_CHANNEL = ( 1 << 1 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_CAD_DETECT          = ( 1 << 0 ),
    SX127X_REG_LORA_IRQ_FLAGS_MASK_ALL =
        SX127X_REG_LORA_IRQ_FLAGS_MASK_RX_TIMEOUT | SX127X_REG_LORA_IRQ_FLAGS_MASK_RX_DONE |
        SX127X_REG_LORA_IRQ_FLAGS_MASK_PAYLOAD_CRC_ERROR | SX127X_REG_LORA_IRQ_FLAGS_MASK_VALID_HEADER |
        SX127X_REG_LORA_IRQ_FLAGS_MASK_TX_DONE | SX127X_REG_LORA_IRQ_FLAGS_MASK_CAD_DONE |
        SX127X_REG_LORA_IRQ_FLAGS_MASK_FHSS_CHANGE_CHANNEL | SX127X_REG_LORA_IRQ_FLAGS_MASK_CAD_DETECT,
};

/**
 * @brief SX127X IrqFlag
 */
enum sx127x_reg_lora_irq_flags_e
{
    SX127X_REG_LORA_IRQ_FLAGS_NONE                = ( 0 << 0 ),
    SX127X_REG_LORA_IRQ_FLAGS_RX_TIMEOUT          = ( 1 << 7 ),
    SX127X_REG_LORA_IRQ_FLAGS_RX_DONE             = ( 1 << 6 ),
    SX127X_REG_LORA_IRQ_FLAGS_PAYLOAD_CRC_ERROR   = ( 1 << 5 ),
    SX127X_REG_LORA_IRQ_FLAGS_VALID_HEADER        = ( 1 << 4 ),
    SX127X_REG_LORA_IRQ_FLAGS_TX_DONE             = ( 1 << 3 ),
    SX127X_REG_LORA_IRQ_FLAGS_CAD_DONE            = ( 1 << 2 ),
    SX127X_REG_LORA_IRQ_FLAGS_FHSS_CHANGE_CHANNEL = ( 1 << 1 ),
    SX127X_REG_LORA_IRQ_FLAGS_CAD_DETECT          = ( 1 << 0 ),
    SX127X_REG_LORA_IRQ_FLAGS_ALL = SX127X_REG_LORA_IRQ_FLAGS_RX_TIMEOUT | SX127X_REG_LORA_IRQ_FLAGS_RX_DONE |
                                    SX127X_REG_LORA_IRQ_FLAGS_PAYLOAD_CRC_ERROR |
                                    SX127X_REG_LORA_IRQ_FLAGS_VALID_HEADER | SX127X_REG_LORA_IRQ_FLAGS_TX_DONE |
                                    SX127X_REG_LORA_IRQ_FLAGS_CAD_DONE | SX127X_REG_LORA_IRQ_FLAGS_FHSS_CHANGE_CHANNEL |
                                    SX127X_REG_LORA_IRQ_FLAGS_CAD_DETECT,
};

/**
 * @brief SX127X RxCodingRate - Coding rate of last header received
 */
enum sx127x_reg_lora_modem_stat_rx_cr_e
{
    SX127X_REG_LORA_MODEM_STAT_RX_CR_MASK = ~( 7 << 5 ),
    SX127X_REG_LORA_MODEM_STAT_RX_CR_0    = ( 0 << 5 ),
    SX127X_REG_LORA_MODEM_STAT_RX_CR_1    = ( 1 << 5 ),
    SX127X_REG_LORA_MODEM_STAT_RX_CR_2    = ( 2 << 5 ),
    SX127X_REG_LORA_MODEM_STAT_RX_CR_3    = ( 3 << 5 ),
    SX127X_REG_LORA_MODEM_STAT_RX_CR_4    = ( 4 << 5 ),
    SX127X_REG_LORA_MODEM_STAT_RX_CR_5    = ( 5 << 5 ),
    SX127X_REG_LORA_MODEM_STAT_RX_CR_6    = ( 6 << 5 ),
    SX127X_REG_LORA_MODEM_STAT_RX_CR_7    = ( 7 << 5 ),
};

/**
 * @brief SX127X ModemStatus -
 * @remark read only
 */
enum sx127x_reg_lora_modem_stat_modem_status_e
{
    SX127X_REG_LORA_MODEM_STAT_MODEM_STATUS_MASK                = ~( 31 << 0 ),
    SX127X_REG_LORA_MODEM_STAT_MODEM_STATUS_CLEAR               = ( 1 << 4 ),
    SX127X_REG_LORA_MODEM_STAT_MODEM_STATUS_HEADER_INFO_VALID   = ( 1 << 3 ),
    SX127X_REG_LORA_MODEM_STAT_MODEM_STATUS_RX_ON_GOING         = ( 1 << 2 ),
    SX127X_REG_LORA_MODEM_STAT_MODEM_STATUS_SIGNAL_SYNCHRONIZED = ( 1 << 1 ),
    SX127X_REG_LORA_MODEM_STAT_MODEM_STATUS_SIGNAL_DETECTED     = ( 1 << 0 ),
};

/**
 * @brief SX127X PllTimeout - PLL failed to lock while attempting a TX/RX/CAD operation
 * @remark read only
 */
enum sx127x_reg_lora_hop_channel_pll_timeout_e
{
    SX127X_REG_LORA_HOP_CHANNEL_PLL_TIMEOUT_MASK     = ~( 1 << 7 ),
    SX127X_REG_LORA_HOP_CHANNEL_PLL_TIMEOUT_UNLOCKED = ( 0 << 7 ),
    SX127X_REG_LORA_HOP_CHANNEL_PLL_TIMEOUT_LOCKED   = ( 1 << 7 ),
};

/**
 * @brief SX127X CrcOnPayload - CRC Information extracted from the received packet header (Explicit header mode only)
 * @remark read only
 */
enum sx127x_reg_lora_hop_channel_crc_on_payload_e
{
    SX127X_REG_LORA_HOP_CHANNEL_CRC_ON_PAYLOAD_MASK     = ~( 1 << 6 ),
    SX127X_REG_LORA_HOP_CHANNEL_CRC_ON_PAYLOAD_DISABLED = ( 0 << 6 ),
    SX127X_REG_LORA_HOP_CHANNEL_CRC_ON_PAYLOAD_ENABLED  = ( 1 << 6 ),
};

/**
 * @brief SX127X FhssPresentChannel - Current value of frequency hopping channel in use.
 * @remark read only
 */
enum sx127x_reg_lora_hop_channel_fhss_present_channel_e
{
    SX127X_REG_LORA_HOP_CHANNEL_FHSS_PRESENT_CHANNEL_MASK = ~( 63 << 0 ),
};

/**
 * @brief SX1272 Bw - Signal bandwidth.
 */
enum sx1272_reg_lora_modem_config_1_bw_e
{
    SX1272_REG_LORA_MODEM_CONFIG_1_BW_MASK = ~( 3 << 6 ),
    SX1272_REG_LORA_MODEM_CONFIG_1_BW_125  = ( 0 << 6 ),  // Default
    SX1272_REG_LORA_MODEM_CONFIG_1_BW_250  = ( 1 << 6 ),
    SX1272_REG_LORA_MODEM_CONFIG_1_BW_500  = ( 2 << 6 ),
};

/**
 * @brief SX1276 Bw - Signal bandwidth.
 */
enum sx1276_reg_lora_modem_config_1_bw_e
{
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_MASK = ~( 15 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_007  = ( 0 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_010  = ( 1 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_015  = ( 2 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_020  = ( 3 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_031  = ( 4 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_041  = ( 5 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_062  = ( 6 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_125  = ( 7 << 4 ),  // Default
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_250  = ( 8 << 4 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_BW_500  = ( 9 << 4 ),
};

/**
 * @brief SX1272 CodingRate - Error coding rate.
 */
enum sx1272_reg_lora_modem_config_1_coding_rate_e
{
    SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_MASK   = ~( 7 << 3 ),
    SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_5 = ( 1 << 3 ),  // Default
    SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_6 = ( 2 << 3 ),
    SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_7 = ( 3 << 3 ),
    SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_8 = ( 4 << 3 ),
};

/**
 * @brief SX1276 CodingRate - Error coding rate.
 */
enum sx1276_reg_lora_modem_config_1_coding_rate_e
{
    SX1276_REG_LORA_MODEM_CONFIG_1_CODING_RATE_MASK   = ~( 7 << 1 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_5 = ( 1 << 1 ),  // Default
    SX1276_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_6 = ( 2 << 1 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_7 = ( 3 << 1 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_8 = ( 4 << 1 ),
};

/**
 * @brief SX1272 ImplicitHeaderModeOn - Defines the LoRa header type
 * Explicit -> Variable length
 * Implicit -> Fixed length
 */
enum sx1272_reg_lora_modem_config_1_header_type_e
{
    SX1272_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_MASK     = ~( 1 << 2 ),
    SX1272_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_EXPLICIT = ( 0 << 2 ),  // Default
    SX1272_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_IMPLICIT = ( 1 << 2 ),
};

/**
 * @brief SX1276 ImplicitHeaderModeOn - Defines the LoRa header type
 * Explicit -> Variable length
 * Implicit -> Fixed length
 */
enum sx1276_reg_lora_modem_config_1_header_type_e
{
    SX1276_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_MASK     = ~( 1 << 0 ),
    SX1276_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_EXPLICIT = ( 0 << 0 ),  // Default
    SX1276_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_IMPLICIT = ( 1 << 0 ),
};

/**
 * @brief SX1272 RxPayloadCrcOn - Enable CRC generation and check on payload.
 * 0 -> CRC disable
 * 1 -> CRC enable
 * If CRC is needed, RxPayloadCrcOn should be set:
 * - in Implicit header mode: on Tx and Rx side
 * - in Explicit header mode: on the Tx side alone (recovered from the header in Rx side)
 */
enum sx1272_reg_lora_modem_config_1_rx_payload_crc_e
{
    SX1272_REG_LORA_MODEM_CONFIG_1_RX_PAYLOAD_CRC_MASK = ~( 1 << 1 ),
    SX1272_REG_LORA_MODEM_CONFIG_1_RX_PAYLOAD_CRC_OFF  = ( 0 << 1 ),  // Default
    SX1272_REG_LORA_MODEM_CONFIG_1_RX_PAYLOAD_CRC_ON   = ( 1 << 1 ),
};

/**
 * @brief SX1272 LowDataRateOptimize -
 * 0 -> Disabled
 * 1 -> Enabled; mandated for when the symbol length exceeds 16ms
 */
enum sx1272_reg_lora_modem_config_1_low_datarate_optimize_e
{
    SX1272_REG_LORA_MODEM_CONFIG_1_LOW_DATARATE_OPTIMIZE_MASK = ~( 1 << 0 ),
    SX1272_REG_LORA_MODEM_CONFIG_1_LOW_DATARATE_OPTIMIZE_OFF  = ( 0 << 0 ),  // Default
    SX1272_REG_LORA_MODEM_CONFIG_1_LOW_DATARATE_OPTIMIZE_ON   = ( 1 << 0 ),
};

/**
 * @brief SX127X SpreadingFactor - SF rate (expressed as a base-2 logarithm)
 */
enum sx127x_reg_lora_modem_config_2_spreading_factor_e
{
    SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_MASK = ~( 15 << 4 ),
    SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_6    = ( 6 << 4 ),
    SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_7    = ( 7 << 4 ),  // Default
    SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_8    = ( 8 << 4 ),
    SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_9    = ( 9 << 4 ),
    SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_10   = ( 10 << 4 ),
    SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_11   = ( 11 << 4 ),
    SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_12   = ( 12 << 4 ),
};

/**
 * @brief SX127X TxContinuousMode -
 * 0 -> normal mode, a single packet is sent
 * 1 -> Implicit -> continuous mode, send multiple packets across the FIFO (used for spectral analysis)
 */
enum sx127x_reg_lora_modem_config_2_tx_continuous_mode_e
{
    SX1276_REG_LORA_MODEM_CONFIG_2_TX_CONTINUOUS_MODE_MASK = ~( 1 << 3 ),
    SX1276_REG_LORA_MODEM_CONFIG_2_TX_CONTINUOUS_MODE_OFF  = ( 0 << 3 ),  // Default
    SX1276_REG_LORA_MODEM_CONFIG_2_TX_CONTINUOUS_MODE_ON   = ( 1 << 3 ),
};

/**
 * @brief SX1272 AgcAutoOn -
 * 0 -> LNA gain set by register LnaGain
 * 1 -> LNA gain set by the internal AGC loop
 */
enum sx1272_reg_lora_modem_config_2_agc_auto_e
{
    SX1272_REG_LORA_MODEM_CONFIG_2_AGC_AUTO_MASK = ~( 1 << 2 ),
    SX1272_REG_LORA_MODEM_CONFIG_2_AGC_AUTO_OFF  = ( 0 << 2 ),  // Default
    SX1272_REG_LORA_MODEM_CONFIG_2_AGC_AUTO_ON   = ( 1 << 2 ),
};

/**
 * @brief SX1276 RxPayloadCrcOn - Enable CRC generation and check on payload.
 * 0 -> CRC disable
 * 1 -> CRC enable
 * If CRC is needed, RxPayloadCrcOn should be set:
 * - in Implicit header mode: on Tx and Rx side
 * - in Explicit header mode: on the Tx side alone (recovered from the header in Rx side)
 */
enum sx1276_reg_lora_modem_config_2_rx_payload_crc_e
{
    SX1276_REG_LORA_MODEM_CONFIG_2_RX_PAYLOAD_CRC_MASK = ~( 1 << 2 ),
    SX1276_REG_LORA_MODEM_CONFIG_2_RX_PAYLOAD_CRC_OFF  = ( 0 << 2 ),  // Default
    SX1276_REG_LORA_MODEM_CONFIG_2_RX_PAYLOAD_CRC_ON   = ( 1 << 2 ),
};

/**
 * @brief SX127X SymbTimeout(9:8) - RX Time-Out MSB.
 * @remark read only
 */
enum sx127x_reg_lora_modem_config_2_symb_timeout_e
{
    SX127X_REG_LORA_MODEM_CONFIG_2_SYMB_TIMEOUT_MASK = ~( 3 << 0 ),
};

/**
 * @brief SX127x AutomaticIFOn -
 * Should be set to 0x0 after each reset (POR on manual)
 * See errata note for more information
 */
enum sx127x_reg_lora_detect_optimize_automatic_if_e
{
    SX127X_REG_LORA_DETECT_OPTIMIZE_AUTOMATIC_IF_MASK = ~( 1 << 7 ),
    SX127X_REG_LORA_DETECT_OPTIMIZE_AUTOMATIC_IF_OFF  = ( 0 << 7 ),
    SX127X_REG_LORA_DETECT_OPTIMIZE_AUTOMATIC_IF_ON   = ( 1 << 7 ),  // Default
};

/**
 * @brief SX127X DetectionOptimize - LoRa Detection Optimize
 * 0x03 -> SF7 to SF12
 * 0x05 -> SF6
 */
enum sx127x_reg_lora_detect_optimize_detection_optimize_e
{
    SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_MASK     = ~( 7 << 0 ),  // Default
    SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_SF7_SF12 = ( 3 << 0 ),   // Default
    SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_SF6      = ( 5 << 0 ),
};

/**
 * @brief SX127X InvertIQ RX - Invert the LoRa I and Q signals in RX path
 * 0 -> normal mode
 * 1 -> I and Q signals are inverted
 */
enum sx127x_reg_lora_invert_iq_rx_e
{
    SX127X_REG_LORA_INVERT_IQ_RX_MASK     = ~( 1 << 6 ),
    SX127X_REG_LORA_INVERT_IQ_RX_NORMAL   = ( 0 << 6 ),  // Default
    SX127X_REG_LORA_INVERT_IQ_RX_INVERTED = ( 1 << 6 ),
};

/**
 * @brief SX127X InvertIQ TX - Invert the LoRa I and Q signals in TX path
 * 0 -> normal mode
 * 1 -> I and Q signals are inverted
 */
enum sx127x_reg_lora_invert_iq_tx_e
{
    SX127X_REG_LORA_INVERT_IQ_TX_MASK     = ~( 1 << 0 ),
    SX127X_REG_LORA_INVERT_IQ_TX_NORMAL   = ( 1 << 0 ),
    SX127X_REG_LORA_INVERT_IQ_TX_INVERTED = ( 0 << 0 ),
};

/**
 * @brief SX127X DetectionThreshold - LoRa detection threshold
 * 0x0A -> SF7 to SF12
 * 0x0C -> SF6
 */
enum sx127x_reg_lora_detection_threshold_e
{
    SX127X_REG_LORA_DETECTION_THRESHOLD_SF7_SF12 = 0x0A,  // Default
    SX127X_REG_LORA_DETECTION_THRESHOLD_SF6      = 0x0C,
};

/**
 * @brief SX127X InvertIQ TX - Invert the LoRa I and Q signals in TX path
 * 0 -> normal mode
 * 1 -> I and Q signals are inverted
 */
enum sx127x_reg_lora_invert_iq_2_e
{
    SX127X_REG_LORA_INVERT_IQ_2_NORMAL   = 0x1D,  // Default
    SX127X_REG_LORA_INVERT_IQ_2_INVERTED = 0x19,
};

/**
 * @brief SX1276 LowDataRateOptimize -
 * 0 -> Disabled
 * 1 -> Enabled; mandated for when the symbol length exceeds 16ms
 */
enum sx1276_reg_lora_modem_config_3_low_datarate_optimize_e
{
    SX1276_REG_LORA_MODEM_CONFIG_3_LOW_DATARATE_OPTIMIZE_MASK = ~( 1 << 3 ),
    SX1276_REG_LORA_MODEM_CONFIG_3_LOW_DATARATE_OPTIMIZE_OFF  = ( 0 << 3 ),  // Default
    SX1276_REG_LORA_MODEM_CONFIG_3_LOW_DATARATE_OPTIMIZE_ON   = ( 1 << 3 ),
};

/**
 * @brief SX1276 AgcAutoOn -
 * 0 -> LNA gain set by register LnaGain
 * 1 -> LNA gain set by the internal AGC loop
 */
enum sx1276_reg_lora_modem_config_3_agc_auto_e
{
    SX1276_REG_LORA_MODEM_CONFIG_3_AGC_AUTO_MASK = ~( 1 << 2 ),
    SX1276_REG_LORA_MODEM_CONFIG_3_AGC_AUTO_OFF  = ( 0 << 2 ),  // Default
    SX1276_REG_LORA_MODEM_CONFIG_3_AGC_AUTO_ON   = ( 1 << 2 ),
};

/**
 * @brief SX1276 HighBWOptimize1 - Optimization for 500 kHz bandwidth. See errata note.
 */
enum sx1276_reg_lora_high_bw_optimize_1_e
{
    SX1276_REG_LORA_HIGH_BW_OPTIMIZE_1_02 = 0x02,
    SX1276_REG_LORA_HIGH_BW_OPTIMIZE_1_03 = 0x03,
};

/**
 * @brief SX1276 HighBWOptimize2 - Optimization for 500 kHz bandwidth. See errata note.
 */
enum sx1276_reg_lora_high_bw_optimize_2_e
{
    SX1276_REG_LORA_HIGH_BW_OPTIMIZE_2_64 = 0x64,
    SX1276_REG_LORA_HIGH_BW_OPTIMIZE_2_7F = 0x7F,
};

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

#ifdef __cplusplus
}
#endif

#endif  // SX127X_REGS_LORA_DEFS_H

/* --- EOF ------------------------------------------------------------------ */
