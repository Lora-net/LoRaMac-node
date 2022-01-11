/**
 * @file      sx127x_regs_gfsk_defs.h
 *
 * @brief     SX1272/3 and SX1276/7/8/9  GFSK modem registers definition
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
#ifndef SX127X_REGS_GFSK_DEFS_H
#define SX127X_REGS_GFSK_DEFS_H

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
 * @brief GFSK/OOK modem registers definition
 *
 * @enum sx127x_reg_gfsk_e
 * @typedef sx127x_reg_gfsk_t
 */
typedef enum sx127x_reg_gfsk_e
{
    SX127X_REG_GFSK_BITRATE_MSB     = 0x02,
    SX127X_REG_GFSK_BITRATE_LSB     = 0x03,
    SX127X_REG_GFSK_FDEV_MSB        = 0x04,
    SX127X_REG_GFSK_FDEV_LSB        = 0x05,
    SX127X_REG_GFSK_RX_CONFIG       = 0x0D,
    SX127X_REG_GFSK_RSSI_CONFIG     = 0x0E,
    SX127X_REG_GFSK_RSSI_COLLISION  = 0x0F,
    SX127X_REG_GFSK_RSSI_THRESH     = 0x10,
    SX127X_REG_GFSK_RSSI_VALUE      = 0x11,
    SX127X_REG_GFSK_RX_BW           = 0x12,
    SX127X_REG_GFSK_AFC_BW          = 0x13,
    SX127X_REG_GFSK_OOK_PEAK        = 0x14,
    SX127X_REG_GFSK_OOK_FIX         = 0x15,
    SX127X_REG_GFSK_OOK_AVG         = 0x16,
    SX127X_REG_GFSK_AFC_FEI         = 0x1A,
    SX127X_REG_GFSK_AFC_MSB         = 0x1B,
    SX127X_REG_GFSK_AFC_LSB         = 0x1C,
    SX127X_REG_GFSK_FEI_MSB         = 0x1D,
    SX127X_REG_GFSK_FEI_LSB         = 0x1E,
    SX127X_REG_GFSK_PREAMBLE_DETECT = 0x1F,
    SX127X_REG_GFSK_RX_TIMEOUT_1    = 0x20,
    SX127X_REG_GFSK_RX_TIMEOUT_2    = 0x21,
    SX127X_REG_GFSK_RX_TIMEOUT_3    = 0x22,
    SX127X_REG_GFSK_RX_DELAY        = 0x23,
    SX127X_REG_GFSK_OSC             = 0x24,
    SX127X_REG_GFSK_PREAMBLE_MSB    = 0x25,
    SX127X_REG_GFSK_PREAMBLE_LSB    = 0x26,
    SX127X_REG_GFSK_SYNC_CONFIG     = 0x27,
    SX127X_REG_GFSK_SYNC_VALUE_1    = 0x28,
    SX127X_REG_GFSK_SYNC_VALUE_2    = 0x29,
    SX127X_REG_GFSK_SYNC_VALUE_3    = 0x2A,
    SX127X_REG_GFSK_SYNC_VALUE_4    = 0x2B,
    SX127X_REG_GFSK_SYNC_VALUE_5    = 0x2C,
    SX127X_REG_GFSK_SYNC_VALUE_6    = 0x2D,
    SX127X_REG_GFSK_SYNC_VALUE_7    = 0x2E,
    SX127X_REG_GFSK_SYNC_VALUE_8    = 0x2F,
    SX127X_REG_GFSK_PACKET_CONFIG_1 = 0x30,
    SX127X_REG_GFSK_PACKET_CONFIG_2 = 0x31,
    SX127X_REG_GFSK_PAYLOAD_LENGTH  = 0x32,
    SX127X_REG_GFSK_NODE_ADDR       = 0x33,
    SX127X_REG_GFSK_BROADCAST_ADDR  = 0x34,
    SX127X_REG_GFSK_FIFO_THRESH     = 0x35,
    SX127X_REG_GFSK_SEQ_CONFIG_1    = 0x36,
    SX127X_REG_GFSK_SEQ_CONFIG_2    = 0x37,
    SX127X_REG_GFSK_TIMER_RESOL     = 0x38,
    SX127X_REG_GFSK_TIMER_1_COEF    = 0x39,
    SX127X_REG_GFSK_TIMER_2_COEF    = 0x3A,
    SX127X_REG_GFSK_IMAGE_CAL       = 0x3B,
    SX127X_REG_GFSK_TEMP            = 0x3C,
    SX127X_REG_GFSK_LOW_BAT         = 0x3D,
    SX127X_REG_GFSK_IRQ_FLAGS_1     = 0x3E,
    SX127X_REG_GFSK_IRQ_FLAGS_2     = 0x3F,
} sx127x_reg_gfsk_t;

/**
 * @brief GFSK/OOK modem registers definition
 *
 * @enum sx1276_reg_gfsk_e
 * @typedef sx1276_reg_gfsk_t
 */
typedef enum sx1276_reg_gfsk_e
{
    SX1276_REG_GFSK_PLL_HOP      = 0x44,
    SX1276_REG_GFSK_BITRATE_FRAC = 0x5D,
} sx1276_reg_gfsk_t;

/**
 * @brief SX127X RestartRxOnCollision - Turns on the mechanism restarting the receiver automatically if it gets
 * saturated or a packet collision is detected
 */
enum sx127x_reg_gfsk_rx_config_restart_rx_on_collision_e
{
    SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_ON_COLLISION_MASK = ~( 1 << 7 ),
    SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_ON_COLLISION_OFF  = ( 0 << 7 ),  // Default
    SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_ON_COLLISION_ON   = ( 1 << 7 ),
};

/**
 * @brief SX127X RestartRxWithoutPllLock - Triggers a manual Restart of the Receiver chain when set to 1.
 * Use this bit when there is no frequency change, RestartRxWithPllLock otherwise.
 */
enum sx127x_reg_gfsk_rx_config_restart_rx_without_pll_lock_e
{
    SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_WITHOUT_PLL_LOCK_MASK = ~( 1 << 6 ),
    SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_WITHOUT_PLL_LOCK_TRIG = ( 1 << 6 ),
};

/**
 * @brief SX127X RestartRxWithoutPllLock - Triggers a manual Restart of the Receiver chain when set to 1.
 * Use this bit when there is no frequency change, RestartRxWithPllLock otherwise.
 */
enum sx127x_reg_gfsk_rx_config_restart_rx_with_pll_lock_e
{
    SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_WITH_PLL_LOCK_MASK = ~( 1 << 5 ),
    SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_WITH_PLL_LOCK_TRIG = ( 1 << 5 ),
};

/**
 * @brief SX127X AfcAutoOn -
 */
enum sx127x_reg_gfsk_rx_config_afc_auto_on_e
{
    SX127X_REG_GFSK_RX_CONFIG_AFC_AUTO_ON_MASK     = ~( 1 << 4 ),
    SX127X_REG_GFSK_RX_CONFIG_AFC_AUTO_ON_DISABLED = ( 0 << 4 ),  // Default
    SX127X_REG_GFSK_RX_CONFIG_AFC_AUTO_ON_ENABLED  = ( 1 << 4 ),
};

/**
 * @brief SX127X AgcAutoOn -
 */
enum sx127x_reg_gfsk_rx_config_agc_auto_on_e
{
    SX127X_REG_GFSK_RX_CONFIG_AGC_AUTO_ON_MASK     = ~( 1 << 3 ),
    SX127X_REG_GFSK_RX_CONFIG_AGC_AUTO_ON_DISABLED = ( 0 << 3 ),
    SX127X_REG_GFSK_RX_CONFIG_AGC_AUTO_ON_ENABLED  = ( 1 << 3 ),  // Default
};

/**
 * @brief SX127X RxTrigger -
 */
enum sx127x_reg_gfsk_rx_config_rx_trigger_e
{
    SX127X_REG_GFSK_RX_CONFIG_RX_TRIG_MASK                  = ~( 7 << 0 ),
    SX127X_REG_GFSK_RX_CONFIG_RX_TRIG_NONE                  = ( 0 << 0 ),
    SX127X_REG_GFSK_RX_CONFIG_RX_TRIG_RSSI_IRQ              = ( 1 << 0 ),
    SX127X_REG_GFSK_RX_CONFIG_RX_TRIG_PREAMBLE_DET_IRQ      = ( 6 << 0 ),  // Default
    SX127X_REG_GFSK_RX_CONFIG_RX_TRIG_RSSI_PREAMBLE_DET_IRQ = ( 7 << 0 ),
};

/**
 * @brief SX127X RssiOffset - Signed RSSI offset, to compensate for the possible losses/gains in the front-end (LNA, SAW
 * filter...) 1dB / LSB, 2’s complement format
 */
enum sx127x_reg_gfsk_rssi_config_rssi_offset_e
{
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_OFFSET_MASK = ~( 31 << 3 ),
};

/**
 * @brief SX127X RssiSmoothing - Defines the number of samples taken to average the RSSI result
 */
enum sx127x_reg_gfsk_rssi_config_rssi_smoothing_e
{
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_MASK = ~( 7 << 0 ),
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_2    = ~( 0 << 0 ),
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_4    = ~( 1 << 0 ),
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_8    = ~( 2 << 0 ),  // Default
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_16   = ~( 3 << 0 ),
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_32   = ~( 4 << 0 ),
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_64   = ~( 5 << 0 ),
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_128  = ~( 6 << 0 ),
    SX127X_REG_GFSK_RSSI_CONFIG_RSSI_SMOOTHING_256  = ~( 7 << 0 ),
};

/**
 * @brief SX127X BitSyncOn - Enables the Bit Synchronizer.
 */
enum sx127x_reg_gfsk_ook_peak_bit_sync_on_e
{
    SX127X_REG_GFSK_OOK_PEAK_BIT_SYNC_ON_MASK     = ~( 1 << 5 ),
    SX127X_REG_GFSK_OOK_PEAK_BIT_SYNC_ON_DISABLED = ( 0 << 5 ),
    SX127X_REG_GFSK_OOK_PEAK_BIT_SYNC_ON_ENABLED  = ( 1 << 5 ),  // Default
};

/**
 * @brief SX127X OokThreshType - Selects the type of threshold in the OOK data slicer
 */
enum sx127x_reg_gfsk_ook_thresh_type_e
{
    SX127X_REG_GFSK_OOK_THRESH_TYPE_MASK            = ~( 3 << 3 ),
    SX127X_REG_GFSK_OOK_THRESH_TYPE_FIXED_THRESHOLD = ( 0 << 3 ),
    SX127X_REG_GFSK_OOK_THRESH_TYPE_PEAK_MODE       = ( 1 << 3 ),  // Default
    SX127X_REG_GFSK_OOK_THRESH_TYPE_AVERAGE_MODE    = ( 2 << 3 ),
};

/**
 * @brief SX127X OokPeakThreshStep - Size of each decrement of the RSSI threshold in the OOK demodulator
 */
enum sx127x_reg_gfsk_ook_peak_thresh_step_e
{
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_MASK   = ~( 7 << 0 ),
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_0_5_DB = ( 0 << 0 ),  // Default
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_1_0_DB = ( 1 << 0 ),
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_1_5_DB = ( 2 << 0 ),
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_2_0_DB = ( 3 << 0 ),
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_3_0_DB = ( 4 << 0 ),
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_4_0_DB = ( 5 << 0 ),
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_5_0_DB = ( 6 << 0 ),
    SX127X_REG_GFSK_OOK_PEAK_THRESH_STEP_6_0_DB = ( 7 << 0 ),
};

/**
 * @brief SX127X OokPeakThreshDec - Period of decrement of the RSSI threshold in the OOK demodulator
 */
enum sx127x_reg_gfsk_ook_avg_peak_thresh_dec_e
{
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_MASK               = ~( 7 << 5 ),
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_ONCE_EVERY_CHIP    = ( 0 << 5 ),  // Default
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_ONCE_EVERY_2_CHIPS = ( 1 << 5 ),
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_ONCE_EVERY_4_CHIPS = ( 2 << 5 ),
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_ONCE_EVERY_8_CHIPS = ( 3 << 5 ),
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_TWICE_EVERY_CHIP   = ( 4 << 5 ),
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_4_TIMES_EACH_CHIP  = ( 5 << 5 ),
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_8_TIMES_EACH_CHIP  = ( 6 << 5 ),
    SX127X_REG_GFSK_OOK_AVG_PEAK_THRESH_DEC_16_TIMES_EACH_CHIP = ( 7 << 5 ),
};

/**
 * @brief SX127X OokAverageOffset - Static offset added to the threshold in average mode in order to reduce glitching
 * activity
 */
enum sx127x_reg_gfsk_ook_average_offset_e
{
    SX127X_REG_GFSK_OOK_AVERAGE_OFFSET_MASK = ~( 3 << 2 ),
    SX127X_REG_GFSK_OOK_AVERAGE_OFFSET_0_DB = ( 0 << 2 ),  // Default
    SX127X_REG_GFSK_OOK_AVERAGE_OFFSET_2_DB = ( 1 << 2 ),
    SX127X_REG_GFSK_OOK_AVERAGE_OFFSET_4_DB = ( 2 << 2 ),
    SX127X_REG_GFSK_OOK_AVERAGE_OFFSET_6_DB = ( 3 << 2 ),
};

/**
 * @brief SX127X OokAverageThreshFilt - Filter coefficients in average mode of the OOK demodulator
 */
enum sx127x_reg_gfsk_ook_average_thresh_filt_e
{
    SX127X_REG_GFSK_OOK_AVERAGE_THRESH_FILT_MASK          = ~( 3 << 0 ),
    SX127X_REG_GFSK_OOK_AVERAGE_THRESH_FILT_CR_OVER_32_PI = ( 0 << 0 ),
    SX127X_REG_GFSK_OOK_AVERAGE_THRESH_FILT_CR_OVER_8_PI  = ( 1 << 0 ),
    SX127X_REG_GFSK_OOK_AVERAGE_THRESH_FILT_CR_OVER_4_PI  = ( 2 << 0 ),  // Default
    SX127X_REG_GFSK_OOK_AVERAGE_THRESH_FILT_CR_OVER_2_PI  = ( 3 << 0 ),
};

/**
 * @brief SX127X AgcStart -Triggers an AGC sequence when set to 1.
 */
enum sx127x_reg_gfsk_afc_fei_agc_start_e
{
    SX127X_REG_GFSK_AFC_FEI_AGC_START_TRIG_MASK = ~( 1 << 4 ),
    SX127X_REG_GFSK_AFC_FEI_AGC_START_TRIG      = ( 1 << 4 ),
};

/**
 * @brief SX127X AfcClear -Clear AFC register set in Rx mode. Always reads 0.
 */
enum sx127x_reg_gfsk_afc_fei_afc_clear_e
{
    SX127X_REG_GFSK_AFC_FEI_AGC_AFC_CLEAR_MASK = ~( 1 << 1 ),
    SX127X_REG_GFSK_AFC_FEI_AGC_AFC_CLEAR      = ( 1 << 1 ),
};

/**
 * @brief SX127X AfcAutoClearOn - Only valid if AfcAutoOn is set
 * 0 -> AFC register is not cleared at the beginning of the automatic AFC phase
 * 1 -> AFC register is cleared at the beginning of the automatic AFC phase
 */
enum sx127x_reg_gfsk_afc_fei_afc_auto_clear_on_e
{
    SX127X_REG_GFSK_AFC_FEI_AFC_AUTO_CLEAR_ON_MASK     = ~( 1 << 0 ),
    SX127X_REG_GFSK_AFC_FEI_AFC_AUTO_CLEAR_ON_DISABLED = ( 0 << 0 ),  // Default
    SX127X_REG_GFSK_AFC_FEI_AFC_AUTO_CLEAR_ON_ENABLED  = ( 1 << 0 ),
};

/**
 * @brief SX127X PreambleDetectorOn - Enables Preamble detector when set to 1. The AGC settings supersede this bit
 * during the startup / AGC phase.
 */
enum sx127x_reg_gfsk_preamble_detect_preamble_detector_on_e
{
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_ON_MASK     = ~( 1 << 7 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_ON_DISABLED = ( 0 << 7 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_ON_ENABLED  = ( 1 << 7 ),  // Recommended
};

/**
 * @brief SX127X PreambleDetectorOn + PreambleDetectorSize -
 * - Enables Preamble detector when set to 1. The AGC settings supersede this bit during the startup / AGC phase.
 * - Number of Preamble bytes to detect to trigger an interrupt
 */
enum sx127x_reg_gfsk_preamble_detect_preamble_detector_e
{
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_MASK    = ~( 7 << 5 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_OFF     = ( 0 << 5 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_1_BYTE  = ( 4 << 5 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_2_BYTES = ( 5 << 5 ),  // Recommended
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_3_BYTES = ( 6 << 5 ),
};

/**
 * @brief SX127X PreambleDetectorTol - Number or chip errors tolerated over PreambleDetectorSize.
 */
enum sx127x_reg_gfsk_preamble_detect_preamble_detector_tol_e
{
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_MASK = ~( 31 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_0    = ( 0 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_1    = ( 1 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_2    = ( 2 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_3    = ( 3 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_4    = ( 4 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_5    = ( 5 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_6    = ( 6 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_7    = ( 7 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_8    = ( 8 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_9    = ( 9 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_10   = ( 10 << 0 ),  // Recommended
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_11   = ( 11 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_12   = ( 12 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_13   = ( 13 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_14   = ( 14 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_15   = ( 15 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_16   = ( 16 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_17   = ( 17 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_18   = ( 18 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_19   = ( 19 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_20   = ( 20 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_21   = ( 21 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_22   = ( 22 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_23   = ( 23 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_24   = ( 24 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_25   = ( 25 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_26   = ( 26 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_27   = ( 27 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_28   = ( 28 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_29   = ( 29 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_30   = ( 30 << 0 ),
    SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_TOL_31   = ( 31 << 0 ),
};

/**
 * @brief SX127X RcCalStart - Triggers the calibration of the RC oscillator when set. Always reads 0. RC calibration
 * must be triggered in Standby mode.
 */
enum sx127x_reg_gfsk_osc_rc_cal_start_e
{
    SX127X_REG_GFSK_OSC_RC_CAL_START_MASK = ~( 1 << 3 ),
    SX127X_REG_GFSK_OSC_RC_CAL_START_TRIG = ( 1 << 3 ),
};

/**
 * @brief SX127X ClkOut - Selects CLKOUT frequency
 */
enum sx127x_reg_gfsk_osc_clk_out_e
{
    SX127X_REG_GFSK_OSC_CLK_OUT_MASK         = ~( 7 << 0 ),
    SX127X_REG_GFSK_OSC_CLK_OUT_FXOSC        = ( 0 << 0 ),
    SX127X_REG_GFSK_OSC_CLK_OUT_FXOSC_DIV_2  = ( 1 << 0 ),
    SX127X_REG_GFSK_OSC_CLK_OUT_FXOSC_DIV_4  = ( 2 << 0 ),
    SX127X_REG_GFSK_OSC_CLK_OUT_FXOSC_DIV_8  = ( 3 << 0 ),
    SX127X_REG_GFSK_OSC_CLK_OUT_FXOSC_DIV_16 = ( 4 << 0 ),
    SX127X_REG_GFSK_OSC_CLK_OUT_FXOSC_DIV_32 = ( 5 << 0 ),
    SX127X_REG_GFSK_OSC_CLK_OUT_RC           = ( 6 << 0 ),
    SX127X_REG_GFSK_OSC_CLK_OUT_OFF          = ( 7 << 0 ),  // Recommended
};

/**
 * @brief SX127X AutoRestartRxMode - Controls the automatic restart of the receiver after the reception of a valid
 * packet (PayloadReady or CrcOk)
 */
enum sx127x_reg_gfsk_sync_config_auto_restart_rx_mode_e
{
    SX127X_REG_GFSK_SYNC_CONFIG_AUTO_RESTART_RX_MODE_MASK         = ~( 3 << 6 ),
    SX127X_REG_GFSK_SYNC_CONFIG_AUTO_RESTART_RX_MODE_OFF          = ( 0 << 6 ),
    SX127X_REG_GFSK_SYNC_CONFIG_AUTO_RESTART_RX_MODE_WAIT_PLL_OFF = ( 1 << 6 ),
    SX127X_REG_GFSK_SYNC_CONFIG_AUTO_RESTART_RX_MODE_WAIT_PLL_ON  = ( 2 << 6 ),  // Default
};

/**
 * @brief SX127X PreamblePolarity - Sets the polarity of the Preamble
 */
enum sx127x_reg_gfsk_sync_config_preamble_polarity_e
{
    SX127X_REG_GFSK_SYNC_CONFIG_PREAMBLE_POLARITY_MASK = ~( 1 << 5 ),
    SX127X_REG_GFSK_SYNC_CONFIG_PREAMBLE_POLARITY_AA   = ( 0 << 5 ),  // Default
    SX127X_REG_GFSK_SYNC_CONFIG_PREAMBLE_POLARITY_55   = ( 1 << 5 ),
};

/**
 * @brief SX127X SyncOn - Enables the Sync word generation and detection
 */
enum sx127x_reg_gfsk_sync_config_sync_on_e
{
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_ON_MASK     = ~( 1 << 4 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_ON_DISABLED = ( 0 << 4 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_ON_ENABLED  = ( 1 << 4 ),  // Default
};

/**
 * @brief SX127X SyncSize - Size of the Sync word - (SyncSize + 1) bytes, (SyncSize) bytes if ioHomeOn=1
 */
enum sx127x_reg_gfsk_sync_config_sync_size_e
{
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_MASK    = ~( 7 << 0 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_1_BYTE  = ( 0 << 0 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_2_BYTES = ( 1 << 0 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_3_BYTES = ( 2 << 0 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_4_BYTES = ( 3 << 0 ),  // Default
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_5_BYTES = ( 4 << 0 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_6_BYTES = ( 5 << 0 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_7_BYTES = ( 6 << 0 ),
    SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_8_BYTES = ( 7 << 0 ),
};

/**
 * @brief SX127X PacketFormat - Defines the packet format used
 * 0 -> Fixed length
 * 1 -> Variable length
 */
enum sx127x_reg_gfsk_packet_config_1_packet_format_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_1_PACKET_FORMAT_MASK     = ~( 1 << 7 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_PACKET_FORMAT_FIXED    = ( 0 << 7 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_PACKET_FORMAT_VARIABLE = ( 1 << 7 ),  // Default
};

/**
 * @brief SX127X DcFree - Defines DC-free encoding/decoding performed
 */
enum sx127x_reg_gfsk_packet_config_1_dc_free_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_1_DC_FREE_MASK       = ~( 3 << 5 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_DC_FREE_OFF        = ( 0 << 5 ),  // Default
    SX127X_REG_GFSK_PACKET_CONFIG_1_DC_FREE_MANCHESTER = ( 1 << 5 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_DC_FREE_WHITENING  = ( 2 << 5 ),
};

/**
 * @brief SX127X CrcOn - Enables CRC calculation/check (Tx/Rx)
 */
enum sx127x_reg_gfsk_packet_config_1_crc_on_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_ON_MASK     = ~( 1 << 4 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_ON_DISABLED = ( 0 << 4 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_ON_ENABLED  = ( 1 << 4 ),  // Default
};

/**
 * @brief SX127X CrcAutoClearOff - Defines the behavior of the packet handler when CRC check fails:
 * 0 -> Clear FIFO and restart new packet reception. No PayloadReady interrupt issued.
 * 1 -> Do not clear FIFO. PayloadReady interrupt issued.
 */
enum sx127x_reg_gfsk_packet_config_1_crc_auto_clear_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_AUTO_CLEAR_MASK = ~( 1 << 4 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_AUTO_CLEAR_ON   = ( 0 << 4 ),  // Default
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_AUTO_CLEAR_OFF  = ( 1 << 4 ),
};

/**
 * @brief SX127X AddressFiltering - Defines address based filtering in Rx
 */
enum sx127x_reg_gfsk_packet_config_1_address_filtering_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_1_ADDRESS_FILTERING_MASK           = ~( 3 << 1 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_ADDRESS_FILTERING_OFF            = ( 0 << 1 ),  // Default
    SX127X_REG_GFSK_PACKET_CONFIG_1_ADDRESS_FILTERING_NODE           = ( 1 << 1 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_ADDRESS_FILTERING_NODE_BROADCAST = ( 2 << 1 ),
};

/**
 * @brief SX127X CrcWhiteningType - Selects the CRC and whitening algorithms:
 * 0 -> CCITT CRC implementation with standard whitening
 * 1 -> IBM CRC implementation with alternate whitening
 */
enum sx127x_reg_gfsk_packet_config_1_crc_whitening_type_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_WHITENING_TYPE_MASK  = ~( 1 << 0 ),
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_WHITENING_TYPE_CCITT = ( 0 << 0 ),  // Default
    SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_WHITENING_TYPE_IBM   = ( 1 << 0 ),
};

/**
 * @brief SX1276 WMBusCrc - WMBus CRC control
 */
enum sx1276_reg_gfsk_packet_config_2_wmbus_crc_e
{
    SX1276_REG_GFSK_PACKET_CONFIG_2_WMBUS_CRC_MASK = ~( 1 << 7 ),
    SX1276_REG_GFSK_PACKET_CONFIG_2_WMBUS_CRC_OFF  = ( 0 << 7 ),  // Default
    SX1276_REG_GFSK_PACKET_CONFIG_2_WMBUS_CRC_ON   = ( 1 << 7 ),
};

/**
 * @brief SX127X DataMode - Data processing mode:
 * 0 -> Continuous mode
 * 1 -> Packet mode
 */
enum sx127x_reg_gfsk_packet_config_2_data_mode_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_2_DATA_MODE_MASK       = ~( 1 << 6 ),
    SX127X_REG_GFSK_PACKET_CONFIG_2_DATA_MODE_CONTINUOUS = ( 0 << 6 ),
    SX127X_REG_GFSK_PACKET_CONFIG_2_DATA_MODE_PACKET     = ( 1 << 6 ),  // Default
};

/**
 * @brief SX127X IoHomeOn -Enables the io-homecontrol® compatibility mode
 */
enum sx127x_reg_gfsk_packet_config_2_io_home_on_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_2_IO_HOME_ON_MASK     = ~( 1 << 5 ),
    SX127X_REG_GFSK_PACKET_CONFIG_2_IO_HOME_ON_DISABLED = ( 0 << 5 ),  // Default
    SX127X_REG_GFSK_PACKET_CONFIG_2_IO_HOME_ON_ENABLED  = ( 1 << 5 ),
};

/**
 * @brief SX127X IoHomePowerFrame - reserved - Linked to io-homecontrol® compatibility mode
 */
enum sx127x_reg_gfsk_packet_config_2_io_home_power_frame_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_2_IO_HOME_POWER_FRAME_MASK = ~( 1 << 4 ),
};

/**
 * @brief SX127X BeaconOn - Enables the Beacon mode in Fixed packet format
 */
enum sx127x_reg_gfsk_packet_config_2_beacon_on_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_2_BEACON_ON_MASK     = ~( 1 << 3 ),
    SX127X_REG_GFSK_PACKET_CONFIG_2_BEACON_ON_DISABLED = ( 0 << 3 ),  // Default
    SX127X_REG_GFSK_PACKET_CONFIG_2_BEACON_ON_ENABLED  = ( 1 << 3 ),
};

/**
 * @brief SX127X PayloadLength(10:8) - Packet Length Most significant bits
 */
enum sx127x_reg_gfsk_packet_config_2_payload_length_msb_e
{
    SX127X_REG_GFSK_PACKET_CONFIG_2_PAYLOAD_LENGTH_MSB_MASK = ~( 7 << 0 ),
};

/**
 * @brief SX127X TxStartCondition - Defines the condition to start packet transmission
 */
enum sx127x_reg_gfsk_fifo_thresh_tx_start_condition_e
{
    SX127X_REG_GFSK_FIFO_THRESH_TX_START_CONDITION_MASK           = ~( 1 << 7 ),
    SX127X_REG_GFSK_FIFO_THRESH_TX_START_CONDITION_FIFO_THRESH    = ( 0 << 7 ),
    SX127X_REG_GFSK_FIFO_THRESH_TX_START_CONDITION_FIFO_NOT_EMPTY = ( 1 << 7 ),  // Recommended
};

/**
 * @brief SX127X FifoThreshold - Used to trigger FifoLevel interrupt, when number of bytes in FIFO >= FifoThreshold + 1
 */
enum sx127x_reg_gfsk_fifo_thresh_fifo_threshold_e
{
    SX127X_REG_GFSK_FIFO_THRESH_FIFO_THRESHOLD_MASK = ~( 0x3F ),
};

/**
 * @brief SX127X SequencerStart - Controls the top level Sequencer
 * When set to ‘1’, executes the “Start” transition.
 * The sequencer can only be enabled when the chip is in Sleep or Standby mode.
 */
enum sx127x_reg_gfsk_seq_config_1_sequencer_start_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_1_SEQUENCER_START_MASK = ~( 1 << 7 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_SEQUENCER_START_TRIG = ( 1 << 7 ),
};

/**
 * @brief SX127X SequencerStop - Forces the Sequencer Off
 */
enum sx127x_reg_gfsk_seq_config_1_sequencer_stop_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_1_SEQUENCER_STOP_MASK = ~( 1 << 6 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_SEQUENCER_STOP_TRIG = ( 1 << 6 ),
};

/**
 * @brief SX127X IdleMode - Selects chip mode during the state
 * 0 -> Standby mode
 * 1 -> Sleep mode
 */
enum sx127x_reg_gfsk_seq_config_1_idle_mode_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_1_IDLE_MODE_MASK    = ~( 1 << 5 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_IDLE_MODE_STANDBY = ( 0 << 5 ),  // Default
    SX127X_REG_GFSK_SEQ_CONFIG_1_IDLE_MODE_SLEEP   = ( 1 << 5 ),
};

/**
 * @brief SX127X FromStart - Controls the Sequencer transition when SequencerStart is set to 1 in Sleep or Standby mode
 */
enum sx127x_reg_gfsk_seq_config_1_from_start_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_START_MASK                    = ~( 3 << 3 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_START_TO_LPS                  = ( 0 << 3 ),  // Default
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_START_TO_RX                   = ( 1 << 3 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_START_TO_TX                   = ( 2 << 3 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_START_TO_TX_ON_FIFO_LEVEL_IRQ = ( 3 << 3 ),
};

/**
 * @brief SX127X LowPowerSelection - Selects the Sequencer LowPower state after a to LowPowerSelection transition
 * @remark Initial mode is the chip LowPower mode at Sequencer Start.
 */
enum sx127x_reg_gfsk_seq_config_1_lps_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_1_LPS_MASK          = ~( 1 << 2 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_LPS_SEQUENCER_OFF = ( 0 << 2 ),  // Default
    SX127X_REG_GFSK_SEQ_CONFIG_1_LPS_IDLE          = ( 1 << 2 ),
};

/**
 * @brief SX127X FromIdle - Controls the Sequencer transition from the Idle state on a T1 interrupt
 */
enum sx127x_reg_gfsk_seq_config_1_from_idle_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_IDLE_MASK  = ~( 1 << 1 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_IDLE_TO_TX = ( 0 << 1 ),  // Default
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_IDLE_TO_RX = ( 1 << 1 ),
};

/**
 * @brief SX127X FromTransmit - Controls the Sequencer transition from the Transmit state
 */
enum sx127x_reg_gfsk_seq_config_1_from_transmit_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_TRANSMIT_MASK                    = ~( 1 << 0 ),
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_TRANSMIT_TO_LPS_ON_TX_PACKET_IRQ = ( 0 << 0 ),  // Default
    SX127X_REG_GFSK_SEQ_CONFIG_1_FROM_TRANSMIT_TO_RX_ON_TX_PACKET_IRQ  = ( 1 << 0 ),
};

/**
 * @brief SX127X FromReceive - Controls the Sequencer transition from the Receive state
 * @remark Irrespective of this setting, transition to LowPowerSelection on a T2 interrupt
 * @remark (1) If the CRC is wrong (corrupted packet, with CRC on but CrcAutoClearOn=0), the PayloadReady interrupt will
 * drive the sequencer to RxTimeout state.
 */
enum sx127x_reg_gfsk_seq_config_2_from_receive_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_MASK                                    = ~( 7 << 5 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_TO_UNUSED_OOO                           = ( 0 << 5 ),  // Default
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_TO_RX_PACKET_ON_PAYLOAD_READY_IRQ       = ( 1 << 5 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_TO_LPS_ON_PAYLOAD_READY_IRQ             = ( 2 << 5 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_TO_RX_PACKET_ON_CRC_OK_IRQ              = ( 3 << 5 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_TO_SEQUENCER_OFF_ON_RSSI_IRQ            = ( 4 << 5 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_TO_SEQUENCER_OFF_ON_SYNC_ADDR_IRQ       = ( 5 << 5 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_TO_SEQUENCER_OFF_ON_PREAMBLE_DETECT_IRQ = ( 6 << 5 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RECEIVE_TO_UNUSED_111                           = ( 7 << 5 ),
};

/**
 * @brief SX127X FromRxTimeout - Controls the state-machine transition from the Receive state on a RxTimeout interrupt
 * (and on PayloadReady if FromReceive = 3)
 * @remark RxTimeout interrupt is a TimeoutRxRssi, TimeoutRxPreamble or TimeoutSignalSync interrupt
 */
enum sx127x_reg_gfsk_seq_config_2_from_rx_timout_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_TIMEOUT_MASK             = ~( 3 << 3 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_TIMEOUT_TO_RX_RESTART    = ( 0 << 3 ),  // Default
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_TIMEOUT_TO_TX            = ( 1 << 3 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_TIMEOUT_TO_LPS           = ( 2 << 3 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_TIMEOUT_TO_SEQUENCER_OFF = ( 3 << 3 ),
};

/**
 * @brief SX127X FromPacketReceived - Controls the state-machine transition from the PacketReceived state
 */
enum sx127x_reg_gfsk_seq_config_2_from_rx_packet_e
{
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_PACKET_MASK                    = ~( 7 << 0 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_PACKET_TO_SEQUENCER_OFF        = ( 0 << 0 ),  // Default
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_PACKET_TO_TX_ON_FIFO_EMPTY_IRQ = ( 1 << 0 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_PACKET_TO_LPS                  = ( 2 << 0 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_PACKET_TO_FS_TO_RX             = ( 3 << 0 ),
    SX127X_REG_GFSK_SEQ_CONFIG_2_FROM_RX_PACKET_TO_RX                   = ( 4 << 0 ),
};

/**
 * @brief SX127X Timer1Resolution - Resolution of Timer 1
 */
enum sx127x_reg_gfsk_timer_resol_timer_1_resolution_e
{
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_1_RESOLUTION_MASK      = ~( 3 << 2 ),
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_1_RESOLUTION_OFF       = ( 0 << 2 ),  // Default
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_1_RESOLUTION_000064_US = ( 1 << 2 ),
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_1_RESOLUTION_004100_US = ( 2 << 2 ),
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_1_RESOLUTION_262000_US = ( 3 << 2 ),
};

/**
 * @brief SX127X Timer2Resolution - Resolution of Timer 2
 */
enum sx127x_reg_gfsk_timer_resol_timer_2_resolution_e
{
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_2_RESOLUTION_MASK      = ~( 3 << 0 ),
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_2_RESOLUTION_OFF       = ( 0 << 0 ),  // Default
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_2_RESOLUTION_000064_US = ( 1 << 0 ),
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_2_RESOLUTION_004100_US = ( 2 << 0 ),
    SX127X_REG_GFSK_TIMER_RESOL_TIMER_2_RESOLUTION_262000_US = ( 3 << 0 ),
};

/**
 * @brief SX127X AutoImageCalOn - Controls the Image calibration mechanism
 * 0 -> Calibration of the receiver depending on the temperature is disabled
 * 1 -> Calibration of the receiver depending on the temperature enabled.
 */
enum sx127x_reg_gfsk_image_cal_auto_image_cal_on_e
{
    SX127X_REG_GFSK_IMAGE_CAL_AUTO_IMAGE_CAL_ON_MASK     = ~( 1 << 7 ),
    SX127X_REG_GFSK_IMAGE_CAL_AUTO_IMAGE_CAL_ON_DISABLED = ( 0 << 7 ),  // Recommended
    SX127X_REG_GFSK_IMAGE_CAL_AUTO_IMAGE_CAL_ON_ENABLED  = ( 1 << 7 ),
};

/**
 * @brief SX127X ImageCalStart - Triggers the IQ and RSSI calibration when set in Standby mode.
 */
enum sx127x_reg_gfsk_image_cal_iamge_cal_start_e
{
    SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_START_MASK = ~( 1 << 6 ),
    SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_START_TRIG = ( 1 << 6 ),
};

/**
 * @brief SX127X ImageCalRunning - Set to 1 while the Image and RSSI calibration are running.
 * Toggles back to 0 when the process is completed
 * @remark read only
 */
enum sx127x_reg_gfsk_image_cal_iamge_cal_running_e
{
    SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_RUNNING_MASK = ~( 1 << 5 ),
    SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_DONE         = ( 0 << 5 ),  // Default
    SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_RUNNING      = ( 1 << 5 ),
};

/**
 * @brief SX127X TempChange - IRQ flag witnessing a temperature change exceeding
 * TempThreshold since the last Image and RSSI calibration
 * @remark read only
 */
enum sx127x_reg_gfsk_image_cal_temp_change_e
{
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_CHANGE_MASK         = ~( 1 << 3 ),
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_CHANGE_BELOW_THRESH = ( 0 << 3 ),  // Default
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_CHANGE_ABOVE_THRESH = ( 1 << 3 ),
};

/**
 * @brief SX127X TempThreshold - Temperature change threshold to trigger a new I/Q calibration
 */
enum sx127x_reg_gfsk_image_cal_temp_threshold_e
{
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_THRESHOLD_MASK   = ~( 3 << 1 ),
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_THRESHOLD_05_DEG = ( 0 << 1 ),
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_THRESHOLD_10_DEG = ( 1 << 1 ),  // Default
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_THRESHOLD_15_DEG = ( 2 << 1 ),
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_THRESHOLD_20_DEG = ( 3 << 1 ),
};

/**
 * @brief SX127X TempMonitorOff - Controls the temperature monitor operation:
 * 0 -> Temperature monitoring done in all modes except Sleep and Standby
 * 1 -> Temperature monitoring stopped.
 */
enum sx127x_reg_gfsk_image_cal_temp_monitor_off_e
{
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_MONITOR_OFF_MASK     = ~( 1 << 0 ),
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_MONITOR_OFF_ENABLED  = ( 0 << 0 ),  // Default
    SX127X_REG_GFSK_IMAGE_CAL_TEMP_MONITOR_OFF_DISABLED = ( 1 << 0 ),
};

/**
 * @brief SX127X LowBatOn - Low Battery detector enable signal
 * 0 -> LowBat detector disabled
 * 1 -> LowBat detector enabled
 */
enum sx127x_reg_gfsk_low_bat_low_bat_on_e
{
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_ON_MASK     = ~( 1 << 3 ),
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_ON_ENABLED  = ( 0 << 3 ),  // Default
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_ON_DISABLED = ( 1 << 3 ),
};

/**
 * @brief SX127X LowBatTrim - Low Battery detector enable signal
 * 0 -> LowBat detector disabled
 * 1 -> LowBat detector enabled
 */
enum sx127x_reg_gfsk_low_bat_low_bat_trim_e
{
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_MASK   = ~( 7 << 0 ),
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_1695_V = ( 0 << 0 ),
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_1764_V = ( 1 << 0 ),
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_1835_V = ( 2 << 0 ),  // Default
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_1905_V = ( 3 << 0 ),
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_1976_V = ( 4 << 0 ),
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_2045_V = ( 5 << 0 ),
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_2116_V = ( 6 << 0 ),
    SX127X_REG_GFSK_LOW_BAT_LOW_BAT_TRIM_2185_V = ( 7 << 0 ),
};

/**
 * @brief SX127X IrqFlags1(MSB) + IrqFlags2(LSB) (registers are merged)
 */
enum sx127x_reg_gfsk_irq_flags_e
{
    SX127X_REG_GFSK_IRQ_FLAGS_NONE            = ( 0 << 0 ),
    SX127X_REG_GFSK_IRQ_FLAGS_MODE_READY      = ( 1 << 15 ),
    SX127X_REG_GFSK_IRQ_FLAGS_RX_READY        = ( 1 << 14 ),
    SX127X_REG_GFSK_IRQ_FLAGS_TX_READY        = ( 1 << 13 ),
    SX127X_REG_GFSK_IRQ_FLAGS_PLL_LOCK        = ( 1 << 12 ),
    SX127X_REG_GFSK_IRQ_FLAGS_RSSI            = ( 1 << 11 ),
    SX127X_REG_GFSK_IRQ_FLAGS_TIMEOUT         = ( 1 << 10 ),
    SX127X_REG_GFSK_IRQ_FLAGS_PREAMBLE_DETECT = ( 1 << 9 ),
    SX127X_REG_GFSK_IRQ_FLAGS_SYNC_ADDR_MATCH = ( 1 << 8 ),
    SX127X_REG_GFSK_IRQ_FLAGS_FIFO_FULL       = ( 1 << 7 ),
    SX127X_REG_GFSK_IRQ_FLAGS_FIFO_EMPTY      = ( 1 << 6 ),
    SX127X_REG_GFSK_IRQ_FLAGS_FIFO_LEVEL      = ( 1 << 5 ),
    SX127X_REG_GFSK_IRQ_FLAGS_FIFO_OVERRUN    = ( 1 << 4 ),
    SX127X_REG_GFSK_IRQ_FLAGS_TX_PACKET       = ( 1 << 3 ),
    SX127X_REG_GFSK_IRQ_FLAGS_PAYLOAD_READY   = ( 1 << 2 ),
    SX127X_REG_GFSK_IRQ_FLAGS_CRC_OK          = ( 1 << 1 ),
    SX127X_REG_GFSK_IRQ_FLAGS_LOW_BAT         = ( 1 << 0 ),
    SX127X_REG_GFSK_IRQ_FLAGS_ALL =
        SX127X_REG_GFSK_IRQ_FLAGS_MODE_READY | SX127X_REG_GFSK_IRQ_FLAGS_RX_READY | SX127X_REG_GFSK_IRQ_FLAGS_TX_READY |
        SX127X_REG_GFSK_IRQ_FLAGS_PLL_LOCK | SX127X_REG_GFSK_IRQ_FLAGS_RSSI | SX127X_REG_GFSK_IRQ_FLAGS_TIMEOUT |
        SX127X_REG_GFSK_IRQ_FLAGS_PREAMBLE_DETECT | SX127X_REG_GFSK_IRQ_FLAGS_SYNC_ADDR_MATCH |
        SX127X_REG_GFSK_IRQ_FLAGS_FIFO_FULL | SX127X_REG_GFSK_IRQ_FLAGS_FIFO_EMPTY |
        SX127X_REG_GFSK_IRQ_FLAGS_FIFO_LEVEL | SX127X_REG_GFSK_IRQ_FLAGS_FIFO_OVERRUN |
        SX127X_REG_GFSK_IRQ_FLAGS_TX_PACKET | SX127X_REG_GFSK_IRQ_FLAGS_PAYLOAD_READY |
        SX127X_REG_GFSK_IRQ_FLAGS_CRC_OK | SX127X_REG_GFSK_IRQ_FLAGS_LOW_BAT,
};

/**
 * @brief SX1276 FastHopOn - Bypasses the main state machine for a quick frequency hop. Writing RegFrfLsb will trigger
 * the frequency change.
 * 0 -> Frf is validated when FS_TO_TX or FS_TO_RX is requested
 * 1 -> Frf is validated triggered when SX127X_REG_COMMON_FRF_LSB is written
 */
enum sx1276_reg_gfsk_pll_hop_fast_hop_on_e
{
    SX1276_REG_GFSK_PLL_HOP_FAST_HOP_ON_MASK     = ~( 1 << 7 ),
    SX1276_REG_GFSK_PLL_HOP_FAST_HOP_ON_ENABLED  = ( 0 << 7 ),  // Default
    SX1276_REG_GFSK_PLL_HOP_FAST_HOP_ON_DISABLED = ( 1 << 7 ),
};

/**
 * @brief SX1276 BitRateFrac - Fractional part of the bit rate divider (Only valid for FSK)
 */
enum sx1276_reg_gfsk_bitrate_frac_bitrate_frac_e
{
    SX1276_REG_GFSK_BITRATE_FRAC_BITRATE_FRAC_MASK = ~( 15 << 0 ),
};

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

#ifdef __cplusplus
}
#endif

#endif  // SX127X_REGS_GFSK_DEFS_H

/* --- EOF ------------------------------------------------------------------ */
