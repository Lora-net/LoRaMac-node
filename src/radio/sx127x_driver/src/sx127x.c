/**
 * @file      sx127x.c
 *
 * @brief     SX1272/3 and SX1276/7/8/9 radio driver definition
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

#include <stdlib.h>  // NULL
#include <string.h>  // memcpy
#include "sx127x_hal.h"
#include "sx127x_regs.h"
#include "sx127x.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * Radio specifc code enable/disable control
 *     - For SX1272 & SX1273 define SX1272
 *     - For SX1276, SX1277, SX1278 & SX1279 define SX1276
 *     - If both radios are to be used define SX1272 & SX1276
 */
#if !defined( SX1272 ) && !defined( SX1276 )
#error "Please define the radio to be used"
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/**
 * @brief Conversion between bandwidth in Hz and register value
 */
typedef struct
{
    uint32_t bw;
    uint8_t  param;
} gfsk_bw_t;

/**
 * @brief Radio registers override reset values
 */
typedef struct radio_register_s
{
    sx127x_pkt_type_t pkt_type;
    uint8_t           address;
    uint8_t           value;
} radio_register_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/**
 * @brief Internal frequency of the radio
 */
#define SX127X_XTAL_FREQ 32000000UL

/**
 * @brief Scaling factor used to perform fixed-point operations
 */
#define SX127X_PLL_STEP_SHIFT_AMOUNT ( 8 )

/**
 * @brief PLL step - scaled with SX127X_PLL_STEP_SHIFT_AMOUNT
 */
#define SX127X_PLL_STEP_SCALED ( SX127X_XTAL_FREQ >> ( 19 - SX127X_PLL_STEP_SHIFT_AMOUNT ) )

/**
 * @brief GFSK bandwidth converion table
 */
static const gfsk_bw_t gfsk_bw[] = {
    { .bw = 2600, .param = SX127X_GFSK_BW_2600 },     { .bw = 3100, .param = SX127X_GFSK_BW_3100 },
    { .bw = 3900, .param = SX127X_GFSK_BW_3900 },     { .bw = 5200, .param = SX127X_GFSK_BW_5200 },
    { .bw = 6300, .param = SX127X_GFSK_BW_6300 },     { .bw = 7800, .param = SX127X_GFSK_BW_7800 },
    { .bw = 10400, .param = SX127X_GFSK_BW_10400 },   { .bw = 12500, .param = SX127X_GFSK_BW_12500 },
    { .bw = 15600, .param = SX127X_GFSK_BW_15600 },   { .bw = 20800, .param = SX127X_GFSK_BW_20800 },
    { .bw = 25000, .param = SX127X_GFSK_BW_25000 },   { .bw = 31300, .param = SX127X_GFSK_BW_31300 },
    { .bw = 41700, .param = SX127X_GFSK_BW_41700 },   { .bw = 50000, .param = SX127X_GFSK_BW_50000 },
    { .bw = 62500, .param = SX127X_GFSK_BW_62500 },   { .bw = 83333, .param = SX127X_GFSK_BW_83333 },
    { .bw = 100000, .param = SX127X_GFSK_BW_100000 }, { .bw = 125000, .param = SX127X_GFSK_BW_125000 },
    { .bw = 166700, .param = SX127X_GFSK_BW_166700 }, { .bw = 200000, .param = SX127X_GFSK_BW_200000 },
    { .bw = 250000, .param = SX127X_GFSK_BW_250000 },
};

static const radio_register_t reg_override_reset_values[] = {
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_COMMON_LNA, .value = 0x23 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_RX_CONFIG, .value = 0x1E },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_RSSI_CONFIG, .value = 0xD2 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_AFC_FEI, .value = 0x01 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_PREAMBLE_DETECT, .value = 0xAA },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_OSC, .value = 0x07 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_SYNC_CONFIG, .value = 0x12 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_SYNC_VALUE_1, .value = 0xC1 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_SYNC_VALUE_2, .value = 0x94 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_SYNC_VALUE_3, .value = 0xC1 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_PACKET_CONFIG_1, .value = 0xD8 },
    /* FIFO threshold set to 32 (31+1) */
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_FIFO_THRESH, .value = 0x9F },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_GFSK_IMAGE_CAL, .value = 0x02 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_COMMON_DIO_MAPPING_1, .value = 0x00 },
    { .pkt_type = SX127X_PKT_TYPE_GFSK, .address = SX127X_REG_COMMON_DIO_MAPPING_2, .value = 0x30 },
};

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/**
 * @brief Initialize radio registers with recommended values
 *
 * @param [in] radio Radio implementation parameters
 *
 * @returns Operation status
 */
static sx127x_status_t sx127x_init_reg_defaults( sx127x_t* radio );

#if defined( SX1272 )
/**
 * @brief Set SX1272 parameters for TX power and power amplifier ramp time
 *
 * @param [in] radio Chip implementation context.
 * @param [in] pwr_in_dbm The Tx output power in dBm
 * @param [in] ramp_time The ramping time configuration for the PA
 *
 * @returns Operation status
 */
static sx127x_status_t sx1272_set_tx_params( sx127x_t* radio, const int8_t pwr_in_dbm,
                                             const sx127x_ramp_time_t pa_ramp );
#endif

#if defined( SX1276 )
/**
 * @brief Set SX1276 parameters for TX power and power amplifier ramp time
 *
 * @param [in] radio Chip implementation context.
 * @param [in] pwr_in_dbm The Tx output power in dBm
 * @param [in] ramp_time The ramping time configuration for the PA
 *
 * @returns Operation status
 */
static sx127x_status_t sx1276_set_tx_params( sx127x_t* radio, const int8_t pwr_in_dbm,
                                             const sx127x_ramp_time_t pa_ramp );
#endif

#if defined( SX1272 )
/**
 * @brief Set SX1272 GFSK pulse shape
 *
 * @param [in] radio Chip implementation context.
 * @param [in] params The structure of GFSK modulation configuration
 *
 * @returns Operation status
 */
static sx127x_status_t sx1272_set_gfsk_pulse_shape( sx127x_t* radio, const sx127x_gfsk_mod_params_t* params );
#endif

#if defined( SX1276 )
/**
 * @brief Set SX1276 GFSK pulse shape
 *
 * @param [in] radio Chip implementation context.
 * @param [in] params The structure of GFSK modulation configuration
 *
 * @returns Operation status
 */
static sx127x_status_t sx1276_set_gfsk_pulse_shape( sx127x_t* radio, const sx127x_gfsk_mod_params_t* params );
#endif

/**
 * @brief Get CRC length in bytes
 *
 * @param [in] radio Radio implementation parameters
 * @param [in] crc_type CRC type to retrieve length
 *
 * @returns crc_len_in_bytes CRC length in bytes
 */
static inline uint32_t sx127x_get_gfsk_crc_len_in_bytes( sx127x_gfsk_crc_type_t crc_type );

/**
 * @brief Get radio operating mode
 *
 * @param [in] radio Radio implementation parameters
 * @param [out] op_mode Radio current operating mode
 *
 * @returns Operation status
 */
static sx127x_status_t sx127x_get_op_mode( sx127x_t* radio, uint8_t* op_mode );

/**
 * @brief Set radio operating mode
 *
 * @param [in] radio Radio implementation parameters
 * @param [in] op_mode Radio operating mode
 *
 * @returns Operation status
 */
static sx127x_status_t sx127x_set_op_mode( sx127x_t* radio, const uint8_t op_mode );

/**
 * @brief Writes the buffer contents to the SX127X FIFO
 *
 * @param [in] radio Radio implementation parameters
 * @param [in] data Pointer to the buffer to be written to the FIFO.
 * @param [in] data_len Buffer size to be written to the FIFO
 *
 * @returns status Operation status
 */
static sx127x_status_t sx127x_write_fifo( sx127x_t* radio, const uint8_t* data, const uint8_t data_len );

/**
 * @brief Reads the contents of the SX127X FIFO to the buffer
 *
 * @param [in] radio Radio implementation parameters
 * @param [out] buffer Pointer to the buffer to be read from the FIFO.
 * @param [in] size Buffer size to be read from the FIFO
 *
 * @returns status Operation status
 */
static sx127x_status_t sx127x_read_fifo( sx127x_t* radio, uint8_t* data, const uint8_t data_len );

/**
 * @brief Stops all started timers
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
static sx127x_status_t sx127x_timer_stop_all( sx127x_t* radio );

#if defined( SX1276 )
/**
 * @brief Apply ERRATA 2.1 Sensitivity Optimization with a 500 kHz Bandwidth
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
static sx127x_status_t sx1276_fix_lora_500_khz_bw_sensitivity( sx127x_t* radio );

/**
 * @brief Apply ERRATA 2.3 recomendations concerning LoRa signal spurious reception
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
static sx127x_status_t sx1276_fix_lora_rx_spurious_signal( sx127x_t* radio );
#endif

/**
 * @brief Update IQ control registers depending on required Tx/Rx state
 *
 * @param [in] radio Chip implementation context.
 */
static sx127x_status_t sx127x_update_lora_iq_inverted_regs( sx127x_t* radio, bool is_tx_on );

/**
 * @brief Callback function called when sx127x_hal_timer expires
 *
 * @param [in] context Chip implementation context.
 */
static void rx_timer_irq_handler( void* context );

/**
 * @brief DIO_0 interrupt handler
 *
 * @remark sx127x_hal shall use these callbacks when a dio IRQ is activated
 *
 * @param [in] context Chip implementation context.
 */
static void dio_0_irq_handler( void* context );

/**
 * @brief DIO_1 interrupt handler
 *
 * @remark sx127x_hal shall use these callbacks when a dio IRQ is activated
 *
 * @param [in] context Chip implementation context.
 */
static void dio_1_irq_handler( void* context );

/**
 * @brief DIO_2 interrupt handler
 *
 * @remark sx127x_hal shall use these callbacks when a dio IRQ is activated
 *
 * @param [in] context Chip implementation context.
 */
static void dio_2_irq_handler( void* context );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

sx127x_status_t sx127x_init( sx127x_t* radio )
{
    // Get HAL radio ID
    radio->radio_id = sx127x_hal_get_radio_id( radio );

    // Initialize Rx timer timeout callback
    radio->rx_timer_irq_handler = rx_timer_irq_handler;

    // Initialize DIO IRQ callbacks
    radio->dio_0_irq_handler = dio_0_irq_handler;
    radio->dio_1_irq_handler = dio_1_irq_handler;
    radio->dio_2_irq_handler = dio_2_irq_handler;

    sx127x_hal_dio_irq_attach( radio );

    if( sx127x_init_reg_defaults( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    return SX127X_STATUS_OK;
}

void sx127x_irq_attach( sx127x_t* radio, void ( *irq_handler )( void* irq_handler_context ), void* irq_handler_context )
{
    radio->irq_handler_context = irq_handler_context;
    radio->irq_handler         = irq_handler;
}

sx127x_status_t sx127x_set_sleep( sx127x_t* radio )
{
    if( sx127x_timer_stop_all( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_SLEEP;
    return sx127x_set_op_mode( radio, radio->op_mode );
}

sx127x_status_t sx127x_set_standby( sx127x_t* radio )
{
    if( sx127x_timer_stop_all( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_STANDBY;
    return sx127x_set_op_mode( radio, radio->op_mode );
}

sx127x_status_t sx127x_set_fs_tx( sx127x_t* radio )
{
    if( sx127x_timer_stop_all( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_FS_TX;
    return sx127x_set_op_mode( radio, radio->op_mode );
}

sx127x_status_t sx127x_set_fs_rx( sx127x_t* radio )
{
    if( sx127x_timer_stop_all( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_FS_RX;
    return sx127x_set_op_mode( radio, radio->op_mode );
}

sx127x_status_t sx127x_set_tx( sx127x_t* radio )
{
    if( sx127x_timer_stop_all( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {  // LoRa
        if( sx127x_update_lora_iq_inverted_regs( radio, true ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }

#if defined( IS_LORA_FREQUENCY_HOPPING_SUPPORTED )
        if( radio->is_lora_freq_hopping_on == false )
        {
            // DIO_0[TxDone], DIO_2[FhssChangeChannel]
            radio->reg_dio_mapping[0] &=
                SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_MASK & SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_MASK;
        }
        else
#endif
        {
            // DIO_0[TxDone]
            radio->reg_dio_mapping[0] &= SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_MASK;
        }
        radio->reg_dio_mapping[0] |= SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_01;
    }
    else
    {  // GFSK/OOK
        // DIO_0[PacketSent], DIO_1[FifoLevel], DIO_2[FifoFull], DIO_3[FifoEmpty], DIO_4[LowBat], DIO_5[ModeReady]
        radio->reg_dio_mapping[0] &= SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_MASK &
                                     SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_MASK &
                                     SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_MASK;

        radio->reg_dio_mapping[1] = SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_11 | SX127X_REG_COMMON_DIO_MAPPING_2_MAP_RSSI;
    }

    if( sx127x_write_register( radio, SX127X_REG_COMMON_DIO_MAPPING_1, radio->reg_dio_mapping, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_TX;
    return sx127x_set_op_mode( radio, radio->op_mode );
}

sx127x_status_t sx127x_set_rx( sx127x_t* radio, const uint32_t timeout_ms )
{
    if( sx127x_timer_stop_all( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {  // LoRa
        if( sx127x_update_lora_iq_inverted_regs( radio, false ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }

#if defined( IS_LORA_FREQUENCY_HOPPING_SUPPORTED )
        if( radio->is_lora_freq_hopping_on == true )
        {
            // DIO_0[RxDone], DIO_2[FhssChangeChannel]
            radio->reg_dio_mapping[0] &=
                SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_MASK & SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_MASK;
        }
        else
#endif
        {
            // DIO_0[RxDone]
            radio->reg_dio_mapping[0] &= SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_MASK;
        }
    }
    else
    {  // GFSK/OOK
        // DIO_0[PayloadReady], DIO_1[FifoLevel], DIO_2[SyncAddr], DIO_3[FifoEmpty], DIO_4[LowBat], DIO_5[ModeReady]
        radio->reg_dio_mapping[0] &= SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_MASK &
                                     SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_MASK &
                                     SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_MASK;
        radio->reg_dio_mapping[0] |= SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_00 |
                                     SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_00 |
                                     SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_11;

        radio->reg_dio_mapping[1] = SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_11 | SX127X_REG_COMMON_DIO_MAPPING_2_MAP_RSSI;

        radio->is_gfsk_sync_word_detected       = false;
        radio->gfsk_pkt_params.pld_len_in_bytes = 0;
        radio->buffer_index                     = 0;
    }

    if( sx127x_write_register( radio, SX127X_REG_COMMON_DIO_MAPPING_1, radio->reg_dio_mapping, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {
#if defined( SX1276 )
        switch( radio->radio_id )
        {
        case SX127X_RADIO_ID_SX1276:
        // Intentional fall through
        case SX127X_RADIO_ID_SX1277:
        // Intentional fall through
        case SX127X_RADIO_ID_SX1278:
        // Intentional fall through
        case SX127X_RADIO_ID_SX1279:
            // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
            if( sx1276_fix_lora_500_khz_bw_sensitivity( radio ) != SX127X_STATUS_OK )
            {
                return SX127X_STATUS_ERROR;
            }
            // ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
            if( sx1276_fix_lora_rx_spurious_signal( radio ) != SX127X_STATUS_OK )
            {
                return SX127X_STATUS_ERROR;
            }
            break;
        default:
            break;
        }
#endif
        uint8_t reg_value = 0;
        if( sx127x_write_register( radio, SX127X_REG_LORA_FIFO_ADDR_PTR, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }

    radio->is_rx_continuous = false;
    if( timeout_ms == 0x00000000 )
    {  // Rx Single
        if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
        {  // LoRa - Rx Single timeout is set by sx127x_set_lora_sync_timeout API
            radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_LORA_RX_SINGLE;
        }
        else
        {  // GFSK/OOK - Rx Single timeout not possible in GFSK
            return SX127X_STATUS_ERROR;
        }
    }
    else if( timeout_ms >= 0x00FFFFFF )
    {  // Rx continuous
        radio->is_rx_continuous = true;
        radio->op_mode          = SX127X_REG_COMMON_OP_MODE_MODE_RX;
    }
    else
    {  // Rx operation for given time period
        radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_RX;
        // Setup and start a timer for the given time perion
        if( sx127x_hal_timer_start( radio, timeout_ms, radio->rx_timer_irq_handler ) != SX127X_HAL_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }

    return sx127x_set_op_mode( radio, radio->op_mode );
}

sx127x_status_t sx127x_set_cad( sx127x_t* radio )
{
    // DIO_0[CADDone]
    radio->reg_dio_mapping[0] &= SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_MASK;
    radio->reg_dio_mapping[0] |= SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_10;

    if( sx127x_write_register( radio, SX127X_REG_COMMON_DIO_MAPPING_1, radio->reg_dio_mapping, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_LORA_CAD;
    return sx127x_set_op_mode( radio, radio->op_mode );
}

sx127x_status_t sx127x_set_tx_cw( sx127x_t* radio )
{
    // Disable GFSK/OOK packet mode. (Continuous mode)
    uint8_t reg_value;
    if( sx127x_read_register( radio, SX127X_REG_GFSK_PACKET_CONFIG_2, &reg_value, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_value = ( reg_value & SX127X_REG_GFSK_PACKET_CONFIG_2_DATA_MODE_MASK ) |
                SX127X_REG_GFSK_PACKET_CONFIG_2_DATA_MODE_CONTINUOUS;

    if( sx127x_write_register( radio, SX127X_REG_GFSK_PACKET_CONFIG_2, &reg_value, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    // Disable radio interrupts
    // DIO_0[-], DIO_1[-], DIO_2[data], DIO_3[-], DIO_4[-], DIO_5[-]
    radio->reg_dio_mapping[0] = SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_11 | SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_11 |
                                SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_00 | SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_00;

    radio->reg_dio_mapping[1] &=
        SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_MASK & SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_MASK;
    radio->reg_dio_mapping[1] |= SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_10 | SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_10;

    if( sx127x_write_register( radio, SX127X_REG_COMMON_DIO_MAPPING_1, radio->reg_dio_mapping, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->op_mode = SX127X_REG_COMMON_OP_MODE_MODE_TX;
    return sx127x_set_op_mode( radio, radio->op_mode );
}

sx127x_status_t sx127x_cal_img( sx127x_t* radio, const uint32_t freq_in_hz )
{
    sx127x_status_t status = SX127X_STATUS_ERROR;
    uint8_t         reg_value;

    // Save context
    status = sx127x_read_register( radio, SX127X_REG_COMMON_PA_CONFIG, &radio->reg_pa_config, 1 );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }

    // Cut the PA just in case, RFO output, power = -1 dBm
    reg_value = 0;
    status    = sx127x_write_register( radio, SX127X_REG_COMMON_PA_CONFIG, &reg_value, 1 );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }

    radio->rf_freq_in_hz = freq_in_hz;
    status               = sx127x_set_rf_freq( radio, freq_in_hz );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }

    // Launch Rx chain calibration
    status = sx127x_read_register( radio, SX127X_REG_GFSK_IMAGE_CAL, &reg_value, 1 );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }

    reg_value =
        ( reg_value & SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_START_MASK ) | SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_START_TRIG;

    status = sx127x_write_register( radio, SX127X_REG_GFSK_IMAGE_CAL, &reg_value, 1 );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }

    do
    {
        status = sx127x_read_register( radio, SX127X_REG_GFSK_IMAGE_CAL, &reg_value, 1 );
        if( status != SX127X_STATUS_OK )
        {
            return status;
        }
    } while( ( reg_value & SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_RUNNING ) ==
             SX127X_REG_GFSK_IMAGE_CAL_IMAGE_CAL_RUNNING );

    // Restore context
    return sx127x_write_register( radio, SX127X_REG_COMMON_PA_CONFIG, &radio->reg_pa_config, 1 );
}

sx127x_status_t sx127x_set_pa_cfg( sx127x_t* radio, const sx127x_pa_cfg_params_t* params )
{
    radio->pa_cfg_params = *params;
    return SX127X_STATUS_OK;
}

//
// Registers and buffer Access
//

sx127x_status_t sx127x_write_register( sx127x_t* radio, const uint16_t addr, const uint8_t* buffer, const uint8_t size )
{
    return ( sx127x_status_t ) sx127x_hal_write( radio, addr, buffer, size );
}

sx127x_status_t sx127x_read_register( sx127x_t* radio, const uint16_t addr, uint8_t* buffer, const uint8_t size )
{
    return ( sx127x_status_t ) sx127x_hal_read( radio, addr, buffer, size );
}

sx127x_status_t sx127x_write_buffer( sx127x_t* radio, const uint8_t offset, const uint8_t* buffer, const uint8_t size )
{
    for( int i = 0; i < size; i++ )
    {
        radio->buffer[offset + i] = buffer[i];
    }

    // Handle radio->buffer writing to the radio
    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {  // LoRa
        // Initialize payload length
        if( sx127x_write_register( radio, SX127X_REG_LORA_PAYLOAD_LENGTH, &radio->lora_pkt_params.pld_len_in_bytes,
                                   1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        // Initialize LoRa FIFO handling registers
        uint8_t reg_value = 0;

        if( sx127x_write_register( radio, SX127X_REG_LORA_FIFO_TX_BASE_ADDR, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        if( sx127x_write_register( radio, SX127X_REG_LORA_FIFO_ADDR_PTR, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        // Write radio->buffer to the radio
        if( sx127x_write_fifo( radio, radio->buffer, radio->lora_pkt_params.pld_len_in_bytes ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }
    else
    {  // GFSK/OOK
        radio->buffer_index = 0;
        if( radio->gfsk_pkt_params.header_type == SX127X_GFSK_PKT_VAR_LEN )
        {
            // Write of buffer length to the fifo
            if( sx127x_write_fifo( radio, &radio->gfsk_pkt_params.pld_len_in_bytes, 1 ) != SX127X_STATUS_OK )
            {
                return SX127X_STATUS_ERROR;
            }
        }
        else
        {
            if( sx127x_write_register( radio, SX127X_REG_GFSK_PAYLOAD_LENGTH, &radio->gfsk_pkt_params.pld_len_in_bytes,
                                       1 ) != SX127X_STATUS_OK )
            {
                return SX127X_STATUS_ERROR;
            }
        }
        if( radio->gfsk_pkt_params.pld_len_in_bytes <= 64 )
        {
            radio->gfsk_pkt_chunk_len_in_bytes = radio->gfsk_pkt_params.pld_len_in_bytes;
        }
        else
        {
            radio->gfsk_pkt_chunk_len_in_bytes = 32;
        }
        // Initial write of buffer to the fifo
        if( sx127x_write_fifo( radio, radio->buffer, radio->gfsk_pkt_chunk_len_in_bytes ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        radio->buffer_index += radio->gfsk_pkt_chunk_len_in_bytes;
    }

    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_read_buffer( sx127x_t* radio, const uint8_t offset, uint8_t* buffer, const uint8_t size )
{
    for( int i = 0; i < size; i++ )
    {
        buffer[i] = radio->buffer[offset + i];
    }
    return SX127X_STATUS_OK;
}

//
// DIO and IRQ Control Functions
//

sx127x_status_t sx127x_set_irq_mask( sx127x_t* radio, const uint16_t irq_mask )
{
    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {  // LoRa
        // When radio register is set to 0 the irq is active
        uint8_t reg_value = SX127X_REG_LORA_IRQ_FLAGS_MASK_ALL;

        if( ( irq_mask & SX127X_IRQ_ALL ) == SX127X_IRQ_ALL )
        {
            reg_value = ( uint8_t ) ( ~SX127X_REG_LORA_IRQ_FLAGS_ALL );
        }
        else
        {
            if( ( irq_mask & SX127X_IRQ_TX_DONE ) == SX127X_IRQ_TX_DONE )
            {
                reg_value &= ( uint8_t ) ( ~SX127X_REG_LORA_IRQ_FLAGS_MASK_TX_DONE );
            }
            if( ( irq_mask & SX127X_IRQ_RX_DONE ) == SX127X_IRQ_RX_DONE )
            {
                reg_value &= ( uint8_t ) ( ~SX127X_REG_LORA_IRQ_FLAGS_MASK_RX_DONE );
            }
            if( ( irq_mask & SX127X_IRQ_CRC_ERROR ) == SX127X_IRQ_CRC_ERROR )
            {
                reg_value &= ( uint8_t ) ( ~SX127X_REG_LORA_IRQ_FLAGS_MASK_PAYLOAD_CRC_ERROR );
            }
            if( ( irq_mask & SX127X_IRQ_HEADER_VALID ) == SX127X_IRQ_HEADER_VALID )
            {
                reg_value &= ( uint8_t ) ( ~SX127X_REG_LORA_IRQ_FLAGS_MASK_VALID_HEADER );
            }
            if( ( irq_mask & SX127X_IRQ_CAD_DONE ) == SX127X_IRQ_CAD_DONE )
            {
                reg_value &= ( uint8_t ) ( ~SX127X_REG_LORA_IRQ_FLAGS_MASK_CAD_DONE );
            }
            if( ( irq_mask & SX127X_IRQ_CAD_DETECTED ) == SX127X_IRQ_CAD_DETECTED )
            {
                reg_value &= ( uint8_t ) ( ~SX127X_REG_LORA_IRQ_FLAGS_MASK_CAD_DETECT );
            }
            if( ( irq_mask & SX127X_IRQ_TIMEOUT ) == SX127X_IRQ_TIMEOUT )
            {
                reg_value &= ( uint8_t ) ( ~SX127X_REG_LORA_IRQ_FLAGS_MASK_RX_TIMEOUT );
            }
        }
        if( sx127x_write_register( radio, SX127X_REG_LORA_IRQ_FLAGS_MASK, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }

    radio->irq_flags_mask = irq_mask;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_get_irq_status( sx127x_t* radio, uint16_t* irq_flags )
{
    if( irq_flags == NULL )
    {
        return SX127X_STATUS_ERROR;
    }

    *irq_flags = radio->irq_flags;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_clear_irq_status( sx127x_t* radio, const uint16_t irq_mask )
{
    // A 1 clears the interrupt
    radio->irq_flags &= ( ~irq_mask & SX127X_IRQ_ALL );
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_get_and_clear_irq_status( sx127x_t* radio, uint16_t* irq_flags )
{
    uint16_t irq_mask = SX127X_IRQ_NONE;

    sx127x_get_irq_status( radio, &irq_mask );

    if( irq_mask != 0 )
    {
        sx127x_clear_irq_status( radio, irq_mask );
    }
    if( irq_flags != NULL )
    {
        *irq_flags = irq_mask;
    }
    return SX127X_STATUS_OK;
}

//
// RF Modulation and Packet-Related Functions
//

sx127x_status_t sx127x_set_rf_freq( sx127x_t* radio, const uint32_t freq_in_hz )
{
    uint32_t freq_in_pll_steps = sx127x_convert_freq_in_hz_to_pll_step( freq_in_hz );
    uint8_t  reg_values[3]     = { 0 };

    reg_values[0] = ( uint8_t ) ( ( freq_in_pll_steps >> 16 ) & 0xFF );
    reg_values[1] = ( uint8_t ) ( ( freq_in_pll_steps >> 8 ) & 0xFF );
    reg_values[2] = ( uint8_t ) ( ( freq_in_pll_steps >> 0 ) & 0xFF );

    if( sx127x_write_register( radio, SX127X_REG_COMMON_FRF_MSB, reg_values, 3 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    radio->rf_freq_in_hz = freq_in_hz;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_set_pkt_type( sx127x_t* radio, const sx127x_pkt_type_t pkt_type )
{
    uint8_t           reg_value = 0;
    sx127x_pkt_type_t pkt_type_local;
    bool              is_ook_on = false;

    if( sx127x_get_pkt_type( radio, &pkt_type_local ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( pkt_type_local == pkt_type )
    {  // Just returns as the new pkt_type is already set
        return SX127X_STATUS_OK;
    }

    if( sx127x_set_sleep( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_COMMON_OP_MODE, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    switch( pkt_type )
    {
    default:
    case SX127X_PKT_TYPE_GFSK:
        reg_value = ( reg_value & SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_MASK ) |
                    SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_GFSK;
        reg_value = ( reg_value & SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_MASK ) |
                    SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_FSK;
        if( sx127x_write_register( radio, SX127X_REG_COMMON_OP_MODE, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }

        // DIO_0,DIO_1,DIO_2,DIO_3 default mapping
        radio->reg_dio_mapping[0] = SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_00 | SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_00;
        // DIO_4 default mapping, DIO_5 ModeReady
        radio->reg_dio_mapping[1] = SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_11 | SX127X_REG_COMMON_DIO_MAPPING_2_MAP_RSSI;
        break;
    case SX127X_PKT_TYPE_OOK:
        reg_value = ( reg_value & SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_MASK ) |
                    SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_GFSK;
        reg_value = ( reg_value & SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_MASK ) |
                    SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_OOK;
        if( sx127x_write_register( radio, SX127X_REG_COMMON_OP_MODE, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }

        is_ook_on = true;

        // DIO_0,DIO_1,DIO_2,DIO_3 default mapping
        radio->reg_dio_mapping[0] = SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_00 | SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_00;
        // DIO_4 default mapping, DIO_5 ModeReady
        radio->reg_dio_mapping[1] = SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_11 | SX127X_REG_COMMON_DIO_MAPPING_2_MAP_RSSI;
        break;
    case SX127X_PKT_TYPE_LORA:
        reg_value = ( reg_value & SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_MASK ) |
                    SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_LORA;

        if( sx127x_write_register( radio, SX127X_REG_COMMON_OP_MODE, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }

        // DIO_0,DIO_1,DIO_2,DIO_3 default mapping
        radio->reg_dio_mapping[0] = SX127X_REG_COMMON_DIO_MAPPING_1_DIO_0_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_1_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_1_DIO_2_00 | SX127X_REG_COMMON_DIO_MAPPING_1_DIO_3_00;
        // DIO_4, DIO_5 default mapping
        radio->reg_dio_mapping[1] = SX127X_REG_COMMON_DIO_MAPPING_2_DIO_4_00 |
                                    SX127X_REG_COMMON_DIO_MAPPING_2_DIO_5_00 | SX127X_REG_COMMON_DIO_MAPPING_2_MAP_RSSI;
        break;
    }

    if( sx127x_write_register( radio, SX127X_REG_COMMON_DIO_MAPPING_1, radio->reg_dio_mapping, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    // Update radio variable
    radio->pkt_type  = pkt_type;
    radio->is_ook_on = is_ook_on;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_get_pkt_type( sx127x_t* radio, sx127x_pkt_type_t* pkt_type )
{
    uint8_t reg_value = 0;

    if( sx127x_read_register( radio, SX127X_REG_COMMON_OP_MODE, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( ( reg_value & ~SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_MASK ) ==
        SX127X_REG_COMMON_OP_MODE_LONG_RANGE_MODE_LORA )
    {
        radio->pkt_type  = SX127X_PKT_TYPE_LORA;
        radio->is_ook_on = false;
    }
    else
    {
        if( ( reg_value & ~SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_MASK ) ==
            SX127X_REG_COMMON_OP_MODE_MODULATION_TYPE_FSK )
        {
            radio->pkt_type  = SX127X_PKT_TYPE_GFSK;
            radio->is_ook_on = false;
        }
        else
        {
            radio->pkt_type  = SX127X_PKT_TYPE_OOK;
            radio->is_ook_on = true;
        }
    }

    // Update output variable
    *pkt_type = radio->pkt_type;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_set_tx_params( sx127x_t* radio, const int8_t pwr_in_dbm, const sx127x_ramp_time_t pa_ramp )
{
    switch( radio->radio_id )
    {
#if defined( SX1272 )
    case SX127X_RADIO_ID_SX1272:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1273:
        if( sx1272_set_tx_params( radio, pwr_in_dbm, pa_ramp ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        break;
#endif
#if defined( SX1276 )
    case SX127X_RADIO_ID_SX1276:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1277:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1278:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1279:
        if( sx1276_set_tx_params( radio, pwr_in_dbm, pa_ramp ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        break;
#endif
    default:
        return SX127X_STATUS_ERROR;
    }

    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_set_gfsk_mod_params( sx127x_t* radio, const sx127x_gfsk_mod_params_t* params )
{
    // [0] bitrate MSB, [1] bitrate LSB, [2] fdev MSB, [3] fdev LSB
    uint8_t reg_br_fdev_values[4];
    // [0] Rx bandwidth, [1] AFC bandwidth
    uint8_t  reg_bw_values[2];
    uint32_t br;
    uint32_t fdev_in_pll_steps;

    switch( radio->radio_id )
    {
#if defined( SX1272 )
    case SX127X_RADIO_ID_SX1272:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1273:
        if( sx1272_set_gfsk_pulse_shape( radio, params ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        break;
#endif
#if defined( SX1276 )
    case SX127X_RADIO_ID_SX1276:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1277:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1278:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1279:
        if( sx1276_set_gfsk_pulse_shape( radio, params ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        break;
#endif
    default:
        return SX127X_STATUS_ERROR;
    }

    br                    = ( uint32_t ) ( SX127X_XTAL_FREQ / params->br_in_bps );
    reg_br_fdev_values[0] = ( uint8_t ) ( ( br >> 8 ) & 0xFF );
    reg_br_fdev_values[1] = ( uint8_t ) ( ( br >> 0 ) & 0xFF );

    fdev_in_pll_steps     = sx127x_convert_freq_in_hz_to_pll_step( params->fdev_in_hz );
    reg_br_fdev_values[2] = ( uint8_t ) ( ( fdev_in_pll_steps >> 8 ) & 0xFF );
    reg_br_fdev_values[3] = ( uint8_t ) ( ( fdev_in_pll_steps >> 0 ) & 0xFF );

    sx127x_get_gfsk_bw_param( params->bw_ssb_in_hz, reg_bw_values + 0 );
    sx127x_get_gfsk_bw_param( params->bw_ssb_in_hz << 1, reg_bw_values + 1 );

    // Write prepared data to the radio
    if( sx127x_write_register( radio, SX127X_REG_GFSK_BITRATE_MSB, reg_br_fdev_values, 4 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_write_register( radio, SX127X_REG_GFSK_RX_BW, reg_bw_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    uint8_t reg_value = SX127X_REG_GFSK_RX_CONFIG_AFC_AUTO_ON_ENABLED | SX127X_REG_GFSK_RX_CONFIG_AGC_AUTO_ON_ENABLED |
                        SX127X_REG_GFSK_RX_CONFIG_RX_TRIG_PREAMBLE_DET_IRQ;
    if( sx127x_write_register( radio, SX127X_REG_GFSK_RX_CONFIG, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->gfsk_mod_params = *params;
    return SX127X_STATUS_OK;
}

#if defined( SX1272 )
static sx127x_status_t sx1272_set_lora_mod_params( sx127x_t* radio, const sx127x_lora_mod_params_t* params )
{
    // [0] RegModemConfig1, [1] RegModemConfig2
    uint8_t reg_values[2];
    uint8_t reg_detect_optimize        = 0;
    uint8_t reg_detect_optimize_thresh = 0;
    uint8_t bw                         = SX1272_REG_LORA_MODEM_CONFIG_1_BW_125;
    uint8_t cr                         = SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_5;

    switch( params->bw )
    {
    case SX127X_LORA_BW_125:
        bw = SX1272_REG_LORA_MODEM_CONFIG_1_BW_125;
        break;
    case SX127X_LORA_BW_250:
        bw = SX1272_REG_LORA_MODEM_CONFIG_1_BW_250;
        break;
    case SX127X_LORA_BW_500:
        bw = SX1272_REG_LORA_MODEM_CONFIG_1_BW_500;
        break;
    default:
        return SX127X_STATUS_UNKNOWN_VALUE;
    }

    switch( params->cr )
    {
    case SX127X_LORA_CR_4_5:
        cr = SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_5;
        break;
    case SX127X_LORA_CR_4_6:
        cr = SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_6;
        break;
    case SX127X_LORA_CR_4_7:
        cr = SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_7;
        break;
    case SX127X_LORA_CR_4_8:
        cr = SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_CR_4_8;
        break;
    default:
        return SX127X_STATUS_UNKNOWN_VALUE;
    }

    if( sx127x_read_register( radio, SX127X_REG_LORA_MODEM_CONFIG_1, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_values[0] = reg_values[0] & SX1272_REG_LORA_MODEM_CONFIG_1_BW_MASK &
                    SX1272_REG_LORA_MODEM_CONFIG_1_CODING_RATE_MASK &
                    SX1272_REG_LORA_MODEM_CONFIG_1_LOW_DATARATE_OPTIMIZE_MASK;
    reg_values[0] = reg_values[0] | bw | cr | params->ldro;

    reg_values[1] = reg_values[1] & SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_MASK;
    reg_values[1] = reg_values[1] | ( params->sf << 4 );

    // Write prepared data to the radio
    if( sx127x_write_register( radio, SX127X_REG_LORA_MODEM_CONFIG_1, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_LORA_DETECT_OPTIMIZE, &reg_detect_optimize, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    reg_detect_optimize &= SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_MASK;
    if( params->sf == SX127X_LORA_SF6 )
    {
        reg_detect_optimize |= SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_SF6;
        reg_detect_optimize_thresh = SX127X_REG_LORA_DETECTION_THRESHOLD_SF6;
    }
    else
    {
        reg_detect_optimize |= SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_SF7_SF12;
        reg_detect_optimize_thresh = SX127X_REG_LORA_DETECTION_THRESHOLD_SF7_SF12;
    }
    if( sx127x_write_register( radio, SX127X_REG_LORA_DETECT_OPTIMIZE, &reg_detect_optimize, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( sx127x_write_register( radio, SX127X_REG_LORA_DETECTION_THRESHOLD, &reg_detect_optimize_thresh, 1 ) !=
        SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->lora_mod_params = *params;
    return SX127X_STATUS_OK;
}
#endif

#if defined( SX1276 )
static sx127x_status_t sx1276_set_lora_mod_params( sx127x_t* radio, const sx127x_lora_mod_params_t* params )
{
    // [0] RegModemConfig1, [1] RegModemConfig2
    uint8_t reg_values[2];
    uint8_t reg_value;
    uint8_t reg_detect_optimize        = 0;
    uint8_t reg_detect_optimize_thresh = 0;

    if( sx127x_read_register( radio, SX127X_REG_LORA_MODEM_CONFIG_1, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX1276_REG_LORA_MODEM_CONFIG_3, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_values[0] =
        reg_values[0] & SX1276_REG_LORA_MODEM_CONFIG_1_BW_MASK & SX1276_REG_LORA_MODEM_CONFIG_1_CODING_RATE_MASK;
    reg_values[0] = reg_values[0] | ( params->bw << 4 ) | ( params->cr << 1 );

    reg_values[1] = reg_values[1] & SX127X_REG_LORA_MODEM_CONFIG_2_SPREADING_FACTOR_MASK;
    reg_values[1] = reg_values[1] | ( params->sf << 4 );

    reg_value = reg_value & SX1276_REG_LORA_MODEM_CONFIG_3_LOW_DATARATE_OPTIMIZE_MASK;
    reg_value = reg_value | ( params->ldro << 3 );

    // Write prepared data to the radio
    if( sx127x_write_register( radio, SX127X_REG_LORA_MODEM_CONFIG_1, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_write_register( radio, SX1276_REG_LORA_MODEM_CONFIG_3, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_LORA_DETECT_OPTIMIZE, &reg_detect_optimize, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    reg_detect_optimize &= SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_MASK;
    if( params->sf == SX127X_LORA_SF6 )
    {
        reg_detect_optimize |= SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_SF6;
        reg_detect_optimize_thresh = SX127X_REG_LORA_DETECTION_THRESHOLD_SF6;
    }
    else
    {
        reg_detect_optimize |= SX127X_REG_LORA_DETECT_OPTIMIZE_DETECTION_OPTIMIZE_SF7_SF12;
        reg_detect_optimize_thresh = SX127X_REG_LORA_DETECTION_THRESHOLD_SF7_SF12;
    }
    if( sx127x_write_register( radio, SX127X_REG_LORA_DETECT_OPTIMIZE, &reg_detect_optimize, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( sx127x_write_register( radio, SX127X_REG_LORA_DETECTION_THRESHOLD, &reg_detect_optimize_thresh, 1 ) !=
        SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->lora_mod_params = *params;
    return SX127X_STATUS_OK;
}
#endif

sx127x_status_t sx127x_set_lora_mod_params( sx127x_t* radio, const sx127x_lora_mod_params_t* params )
{
    switch( radio->radio_id )
    {
#if defined( SX1272 )
    case SX127X_RADIO_ID_SX1272:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1273:
        if( sx1272_set_lora_mod_params( radio, params ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        break;
#endif
#if defined( SX1276 )
    case SX127X_RADIO_ID_SX1276:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1277:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1278:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1279:
    {
        if( sx1276_set_lora_mod_params( radio, params ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        break;
    }
#endif
    default:
        return SX127X_STATUS_ERROR;
    }

    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_set_gfsk_pkt_params( sx127x_t* radio, const sx127x_gfsk_pkt_params_t* params )
{
    uint8_t preamble_len[2];
    uint8_t reg_value;

    preamble_len[0] = ( uint8_t ) ( ( params->preamble_len_in_bytes >> 8 ) & 0xFF );
    preamble_len[1] = ( uint8_t ) ( ( params->preamble_len_in_bytes >> 0 ) & 0xFF );
    if( sx127x_write_register( radio, SX127X_REG_GFSK_PREAMBLE_MSB, preamble_len, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_GFSK_PREAMBLE_DETECT, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    reg_value =
        ( reg_value & SX127X_REG_GFSK_PREAMBLE_DETECT_PREAMBLE_DETECTOR_MASK ) | ( params->preamble_detector << 5 );
    if( sx127x_write_register( radio, SX127X_REG_GFSK_PREAMBLE_DETECT, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_GFSK_SYNC_CONFIG, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    reg_value = reg_value & SX127X_REG_GFSK_SYNC_CONFIG_SYNC_ON_MASK & SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_MASK;
    if( params->sync_word_len_in_bytes != 0 )
    {
        reg_value = reg_value | SX127X_REG_GFSK_SYNC_CONFIG_SYNC_ON_ENABLED |
                    ( ( params->sync_word_len_in_bytes - 1 ) & ~SX127X_REG_GFSK_SYNC_CONFIG_SYNC_SIZE_MASK );
    }
    if( sx127x_write_register( radio, SX127X_REG_GFSK_SYNC_CONFIG, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_GFSK_PACKET_CONFIG_1, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    reg_value = reg_value & SX127X_REG_GFSK_PACKET_CONFIG_1_PACKET_FORMAT_MASK &
                SX127X_REG_GFSK_PACKET_CONFIG_1_DC_FREE_MASK & SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_ON_MASK &
                SX127X_REG_GFSK_PACKET_CONFIG_1_ADDRESS_FILTERING_MASK;
    reg_value = reg_value | ( ( params->header_type << 7 ) & ~SX127X_REG_GFSK_PACKET_CONFIG_1_PACKET_FORMAT_MASK );
    reg_value = reg_value | ( ( params->dc_free << 5 ) & ~SX127X_REG_GFSK_PACKET_CONFIG_1_DC_FREE_MASK );
    reg_value = reg_value | ( ( params->crc_type << 4 ) & ~SX127X_REG_GFSK_PACKET_CONFIG_1_CRC_ON_MASK );
    reg_value =
        reg_value | ( ( params->address_filtering << 1 ) & ~SX127X_REG_GFSK_PACKET_CONFIG_1_ADDRESS_FILTERING_MASK );

    if( sx127x_write_register( radio, SX127X_REG_GFSK_PACKET_CONFIG_1, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_GFSK_PACKET_CONFIG_2, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_value = reg_value & SX127X_REG_GFSK_PACKET_CONFIG_2_DATA_MODE_MASK;
    reg_value = reg_value | SX127X_REG_GFSK_PACKET_CONFIG_2_DATA_MODE_PACKET;

    if( sx127x_write_register( radio, SX127X_REG_GFSK_PACKET_CONFIG_2, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( params->header_type == SX127X_GFSK_PKT_FIX_LEN )
    {
        reg_value = params->pld_len_in_bytes;
    }
    else
    {
        // Set payload length to the maximum
        reg_value = 0xFF;
    }
    if( sx127x_write_register( radio, SX127X_REG_GFSK_PAYLOAD_LENGTH, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_GFSK_FIFO_THRESH, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    radio->gfsk_fifo_threshold_in_bytes = reg_value & ~SX127X_REG_GFSK_FIFO_THRESH_FIFO_THRESHOLD_MASK;

    radio->gfsk_pkt_params = *params;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_set_lora_pkt_params( sx127x_t* radio, const sx127x_lora_pkt_params_t* params )
{
    // [0] RegModemConfig1, [1] RegModemConfig2
    uint8_t reg_values[2] = { 0 };

    // FIFO writing in LoRa mode must not be performed in sleep mode
    if( sx127x_set_standby( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    // Always use Tx and Rx buffer base address equal to 0
    if( sx127x_write_register( radio, SX127X_REG_LORA_FIFO_TX_BASE_ADDR, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_LORA_MODEM_CONFIG_1, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    switch( radio->radio_id )
    {
#if defined( SX1272 )
    case SX127X_RADIO_ID_SX1272:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1273:
        reg_values[0] = reg_values[0] & SX1272_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_MASK &
                        SX1272_REG_LORA_MODEM_CONFIG_1_RX_PAYLOAD_CRC_MASK;
        reg_values[0] = reg_values[0] |
                        ( ( params->header_type == SX127X_LORA_PKT_EXPLICIT )
                              ? SX1272_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_EXPLICIT
                              : SX1272_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_IMPLICIT ) |
                        ( ( params->crc_is_on == true ) ? SX1272_REG_LORA_MODEM_CONFIG_1_RX_PAYLOAD_CRC_ON
                                                        : SX1272_REG_LORA_MODEM_CONFIG_1_RX_PAYLOAD_CRC_OFF );
        break;
#endif
#if defined( SX1276 )
    case SX127X_RADIO_ID_SX1276:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1277:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1278:
    // Intentional fall through
    case SX127X_RADIO_ID_SX1279:
    {
        reg_values[0] = reg_values[0] & SX1276_REG_LORA_MODEM_CONFIG_1_HEADER_TYPE_MASK;
        reg_values[0] = reg_values[0] | params->header_type;

        reg_values[1] = reg_values[1] & SX1276_REG_LORA_MODEM_CONFIG_2_RX_PAYLOAD_CRC_MASK;
        reg_values[1] =
            reg_values[1] | ( ( params->crc_is_on == true ) ? SX1276_REG_LORA_MODEM_CONFIG_2_RX_PAYLOAD_CRC_ON
                                                            : SX1276_REG_LORA_MODEM_CONFIG_2_RX_PAYLOAD_CRC_OFF );
        break;
    }
#endif
    default:
        return SX127X_STATUS_ERROR;
    }

    // Write prepared data to the radio
    if( sx127x_write_register( radio, SX127X_REG_LORA_MODEM_CONFIG_1, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_values[0] = ( uint8_t ) ( ( params->preamble_len_in_symb >> 8 ) & 0xFF );
    reg_values[1] = ( uint8_t ) ( ( params->preamble_len_in_symb >> 0 ) & 0xFF );
    if( sx127x_write_register( radio, SX127X_REG_LORA_PREAMBLE_MSB, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    // [0] RegPayloadLength, [1] RegMaxPayloadLength
    reg_values[0] = params->pld_len_in_bytes;
    reg_values[1] = params->pld_len_in_bytes;
    if( sx127x_write_register( radio, SX127X_REG_LORA_PAYLOAD_LENGTH, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->invert_iq_is_on = params->invert_iq_is_on;
    radio->lora_pkt_params = *params;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_set_lora_sync_timeout( sx127x_t* radio, const uint16_t nb_of_symbs )
{
    // [0] RegModemConfig2, [1] RegSymbTimeoutLsb
    uint8_t reg_values[2];

    if( sx127x_read_register( radio, SX127X_REG_LORA_MODEM_CONFIG_2, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_values[0] = ( reg_values[0] & SX127X_REG_LORA_MODEM_CONFIG_2_SYMB_TIMEOUT_MASK ) |
                    ( ( nb_of_symbs >> 8 ) & ~SX127X_REG_LORA_MODEM_CONFIG_2_SYMB_TIMEOUT_MASK );

    reg_values[1] = ( uint8_t ) nb_of_symbs;

    if( sx127x_write_register( radio, SX127X_REG_LORA_MODEM_CONFIG_2, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    return SX127X_STATUS_OK;
}

//
// Communication Status Information
//

sx127x_status_t sx127x_get_rx_buffer_status( sx127x_t* radio, sx127x_rx_buffer_status_t* rx_buffer_status )
{
    // Buffer start index is always 0 as it is a virtual buffer in this driver
    rx_buffer_status->buffer_start_pointer = 0;

    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {  // LoRa
        rx_buffer_status->pld_len_in_bytes = radio->lora_pkt_params.pld_len_in_bytes;
    }
    else
    {  // GFSK/OOK
        rx_buffer_status->pld_len_in_bytes = radio->gfsk_pkt_params.pld_len_in_bytes;
    }

    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_get_gfsk_pkt_status( sx127x_t* radio, sx127x_gfsk_pkt_status_t* pkt_status )
{
    // Unsupported feature - Always set to 0
    pkt_status->rx_status = 0;
    pkt_status->rssi_sync = radio->gfsk_rssi_in_dbm;
    // Unsupported feature - copy rssi_sync
    pkt_status->rssi_avg = pkt_status->rssi_sync;

    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_get_lora_pkt_status( sx127x_t* radio, sx127x_lora_pkt_status_t* pkt_status )
{
    // 0: SX127X_REG_LORA_PACKET_SNR_VALUE, 1: SX127X_REG_LORA_PACKET_RSSI_VALUE
    uint8_t reg_values[2];

    if( sx127x_read_register( radio, SX127X_REG_LORA_PACKET_SNR_VALUE, reg_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    // Returns SNR value [dB] rounded to the nearest integer value
    pkt_status->snr_pkt_in_db = ( ( ( int8_t ) reg_values[0] ) + 2 ) >> 2;

    if( pkt_status->snr_pkt_in_db < 0 )
    {
        if( radio->rf_freq_in_hz > RF_FREQUENCY_MID_BAND_THRESHOLD )
        {
            pkt_status->rssi_pkt_in_dbm =
                RSSI_OFFSET_HF + reg_values[1] + ( reg_values[1] >> 4 ) + pkt_status->snr_pkt_in_db;
        }
        else
        {
            pkt_status->rssi_pkt_in_dbm =
                RSSI_OFFSET_LF + reg_values[1] + ( reg_values[1] >> 4 ) + pkt_status->snr_pkt_in_db;
        }
    }
    else
    {
        if( radio->rf_freq_in_hz > RF_FREQUENCY_MID_BAND_THRESHOLD )
        {
            pkt_status->rssi_pkt_in_dbm = RSSI_OFFSET_HF + reg_values[1] + ( reg_values[1] >> 4 );
        }
        else
        {
            pkt_status->rssi_pkt_in_dbm = RSSI_OFFSET_LF + reg_values[1] + ( reg_values[1] >> 4 );
        }
    }
    // Unsupported feature - copy rssi_pkt_in_dbm
    pkt_status->signal_rssi_pkt_in_db = pkt_status->rssi_pkt_in_dbm;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_get_rssi_inst( sx127x_t* radio, int16_t* rssi_in_dbm )
{
    uint8_t reg_value;

    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {  // LoRa
        if( sx127x_read_register( radio, SX127X_REG_LORA_RSSI_VALUE, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        if( radio->rf_freq_in_hz > RF_FREQUENCY_MID_BAND_THRESHOLD )
        {
            *rssi_in_dbm = RSSI_OFFSET_HF + reg_value;
        }
        else
        {
            *rssi_in_dbm = RSSI_OFFSET_LF + reg_value;
        }
    }
    else
    {  // GFSK/OOK
        if( sx127x_read_register( radio, SX127X_REG_GFSK_RSSI_VALUE, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        *rssi_in_dbm = -( reg_value >> 1 );
    }
    return SX127X_STATUS_OK;
}

//
// Miscellaneous
//

sx127x_status_t sx127x_reset( sx127x_t* radio )
{
    sx127x_hal_reset( radio );

    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_get_gfsk_bw_param( const uint32_t bw, uint8_t* param )
{
    sx127x_status_t status = SX127X_STATUS_ERROR;

    if( bw != 0 )
    {
        status = SX127X_STATUS_UNKNOWN_VALUE;
        for( uint8_t i = 0; i < ( sizeof( gfsk_bw ) / sizeof( gfsk_bw_t ) ); i++ )
        {
            if( bw <= gfsk_bw[i].bw )
            {
                *param = gfsk_bw[i].param;
                status = SX127X_STATUS_OK;
                break;
            }
        }
    }

    return status;
}

uint32_t sx127x_get_lora_bw_in_hz( sx127x_lora_bw_t bw )
{
    switch( bw )
    {
    case SX127X_LORA_BW_007:
        return 7812UL;
    case SX127X_LORA_BW_010:
        return 10417UL;
    case SX127X_LORA_BW_015:
        return 15625UL;
    case SX127X_LORA_BW_020:
        return 20833UL;
    case SX127X_LORA_BW_031:
        return 31250UL;
    case SX127X_LORA_BW_041:
        return 41667UL;
    case SX127X_LORA_BW_062:
        return 62500UL;
    case SX127X_LORA_BW_125:
        return 125000UL;
    case SX127X_LORA_BW_250:
        return 250000UL;
    case SX127X_LORA_BW_500:
        return 500000UL;
    default:
        return 0;
    }
}

uint32_t sx127x_get_lora_time_on_air_numerator( const sx127x_lora_pkt_params_t* pkt_params,
                                                const sx127x_lora_mod_params_t* mod_params )
{
    const int32_t pld_len_in_bytes = pkt_params->pld_len_in_bytes;
    const int32_t sf               = mod_params->sf;
    const bool    pld_is_fix       = pkt_params->header_type == SX127X_LORA_PKT_IMPLICIT;
    const int32_t cr_denom         = mod_params->cr + 4;

    int32_t ceil_denominator;
    int32_t ceil_numerator =
        ( pld_len_in_bytes << 3 ) + ( pkt_params->crc_is_on ? 16 : 0 ) - ( 4 * sf ) + ( pld_is_fix ? 0 : 20 );

    if( sf <= 6 )
    {
        ceil_denominator = 4 * sf;
    }
    else
    {
        ceil_numerator += 8;

        if( mod_params->ldro )
        {
            ceil_denominator = 4 * ( sf - 2 );
        }
        else
        {
            ceil_denominator = 4 * sf;
        }
    }

    if( ceil_numerator < 0 )
    {
        ceil_numerator = 0;
    }

    // Perform integral ceil()
    int32_t intermed = ( ( ceil_numerator + ceil_denominator - 1 ) / ceil_denominator ) * cr_denom +
                       pkt_params->preamble_len_in_symb + 12;

    if( sf <= 6 )
    {
        intermed += 2;
    }

    return ( uint32_t ) ( ( 4 * intermed + 1 ) * ( 1 << ( sf - 2 ) ) );
}

uint32_t sx127x_get_lora_time_on_air_in_ms( const sx127x_lora_pkt_params_t* pkt_params,
                                            const sx127x_lora_mod_params_t* mod_params )
{
    uint32_t numerator   = 1000U * sx127x_get_lora_time_on_air_numerator( pkt_params, mod_params );
    uint32_t denominator = sx127x_get_lora_bw_in_hz( mod_params->bw );
    // Perform integral ceil()
    return ( numerator + denominator - 1 ) / denominator;
}

uint32_t sx127x_get_gfsk_time_on_air_numerator( const sx127x_gfsk_pkt_params_t* pkt_params )
{
    return ( pkt_params->preamble_len_in_bytes << 3 ) + ( pkt_params->header_type == SX127X_GFSK_PKT_VAR_LEN ? 8 : 0 ) +
           ( pkt_params->sync_word_len_in_bytes << 3 ) +
           ( ( pkt_params->pld_len_in_bytes +
               ( pkt_params->address_filtering == SX127X_GFSK_ADDRESS_FILTERING_DISABLE ? 0 : 1 ) +
               sx127x_get_gfsk_crc_len_in_bytes( pkt_params->crc_type ) )
             << 3 );
}

uint32_t sx127x_get_gfsk_time_on_air_in_ms( const sx127x_gfsk_pkt_params_t* pkt_params,
                                            const sx127x_gfsk_mod_params_t* mod_params )
{
    uint32_t numerator   = 1000U * sx127x_get_gfsk_time_on_air_numerator( pkt_params );
    uint32_t denominator = mod_params->br_in_bps;

    // Perform integral ceil()
    return ( numerator + denominator - 1 ) / denominator;
}

sx127x_status_t sx127x_get_random_numbers( sx127x_t* radio, uint32_t* numbers, unsigned int n )
{
    // Check that LoRa modem is in use
    if( radio->pkt_type != SX127X_PKT_TYPE_LORA )
    {
        return SX127X_STATUS_ERROR;
    }

    // Start RX continuous
    if( sx127x_set_rx( radio, 0xFFFFFF ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    // Store values
    for( unsigned int i = 0; i < n; i++ )
    {
        for( unsigned int j = 0; j < 32; j++ )
        {
            uint8_t reg_value = 0;

            // Unfiltered RSSI value reading. Only takes the LSB value
            if( sx127x_read_register( radio, SX127X_REG_LORA_RSSI_WIDE_BAND, &reg_value, 1 ) != SX127X_STATUS_OK )
            {
                return SX127X_STATUS_ERROR;
            }
            numbers[i] |= ( ( uint32_t ) reg_value & 0x01 ) << j;
        }
    }

    if( sx127x_set_sleep( radio ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    return SX127X_STATUS_OK;
}

uint32_t sx127x_convert_freq_in_hz_to_pll_step( uint32_t freq_in_hz )
{
    uint32_t steps_int;
    uint32_t steps_frac;

    // Get integer and fractional parts of the frequency computed with a PLL step scaled value
    steps_int  = freq_in_hz / SX127X_PLL_STEP_SCALED;
    steps_frac = freq_in_hz - ( steps_int * SX127X_PLL_STEP_SCALED );

    // Apply the scaling factor to retrieve a frequency in Hz (+ ceiling)
    return ( steps_int << SX127X_PLL_STEP_SHIFT_AMOUNT ) +
           ( ( ( steps_frac << SX127X_PLL_STEP_SHIFT_AMOUNT ) + ( SX127X_PLL_STEP_SCALED >> 1 ) ) /
             SX127X_PLL_STEP_SCALED );
}

sx127x_status_t sx127x_tx_timeout_irq_workaround( sx127x_t* radio )
{
    sx127x_status_t status = SX127X_STATUS_ERROR;

    // Reset the radio
    sx127x_reset( radio );

    // Calibrate Rx chain
    status = sx127x_cal_img( radio, radio->rf_freq_in_hz );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }
    // Initialize radio default values
    status = sx127x_set_sleep( radio );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }
    status = sx127x_init_reg_defaults( radio );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }
    status = sx127x_set_pkt_type( radio, SX127X_PKT_TYPE_LORA );
    if( status != SX127X_STATUS_OK )
    {
        return status;
    }
    // Restore previous network type setting.
    status = sx127x_set_lora_sync_word( radio, radio->reg_lora_sync_word );
    return status;
}

//
// Registers access
//

sx127x_status_t sx127x_set_gfsk_sync_word( sx127x_t* radio, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    if( sync_word_len > 8 )
    {
        return SX127X_STATUS_ERROR;
    }
    if( sx127x_write_register( radio, SX127X_REG_GFSK_SYNC_VALUE_1, sync_word, sync_word_len ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_set_lora_sync_word( sx127x_t* radio, const uint8_t sync_word )
{
    if( sx127x_write_register( radio, SX127X_REG_LORA_SYNC_WORD, &sync_word, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->reg_lora_sync_word = sync_word;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_set_ocp_value( sx127x_t* radio, const uint8_t ocp_trim_value )
{
    uint8_t reg_value = SX127X_REG_COMMON_OCP_OCP_ON_ON | ( ocp_trim_value & SX127X_REG_COMMON_OCP_OCP_TRIM_MASK );

    if( sx127x_write_register( radio, SX127X_REG_COMMON_OCP, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    radio->reg_ocp = reg_value;
    return SX127X_STATUS_OK;
}

sx127x_status_t sx127x_get_lora_params_from_header( sx127x_t* radio, sx127x_lora_cr_t* rx_cr, bool* is_crc_on )
{
    sx127x_status_t status = SX127X_STATUS_ERROR;
    // 0: SX127X_REG_LORA_MODEM_STAT, 1: SX127X_REG_LORA_HOP_CHANNEL
    uint8_t reg_value[] = { 0, 0 };

    status = sx127x_read_register( radio, SX127X_REG_LORA_MODEM_STAT, reg_value, 1 );
    if( status == SX127X_STATUS_OK )
    {
        status = sx127x_read_register( radio, SX127X_REG_LORA_HOP_CHANNEL, reg_value + 1, 1 );
        if( status == SX127X_STATUS_OK )
        {
            *rx_cr = ( sx127x_lora_cr_t ) ( ( reg_value[0] & ~SX127X_REG_LORA_MODEM_STAT_RX_CR_MASK ) >> 5 );
            *is_crc_on =
                ( ( ( reg_value[1] & ~SX127X_REG_LORA_HOP_CHANNEL_CRC_ON_PAYLOAD_MASK ) >> 6 ) != 0 ) ? true : false;
        }
    }
    return status;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static sx127x_status_t sx127x_init_reg_defaults( sx127x_t* radio )
{
    for( size_t i = 0; i < sizeof( reg_override_reset_values ) / sizeof( radio_register_t ); i++ )
    {
        if( sx127x_set_pkt_type( radio, reg_override_reset_values[i].pkt_type ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        if( sx127x_write_register( radio, reg_override_reset_values[i].address, &reg_override_reset_values[i].value,
                                   1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }
    return SX127X_STATUS_OK;
}

#if defined( SX1272 )
static sx127x_status_t sx1272_set_tx_params( sx127x_t* radio, const int8_t pwr_in_dbm,
                                             const sx127x_ramp_time_t pa_ramp )
{
    // [0] PaConfig, [1] PaRamp
    uint8_t reg_pa_values[2] = { 0 };
    uint8_t reg_pa_dac       = 0;
    int8_t  pwr              = pwr_in_dbm;

    // Reads SX127X_REG_COMMON_PA_CONFIG and SX127X_REG_COMMON_PA_RAMP registers
    if( sx127x_read_register( radio, SX127X_REG_COMMON_PA_CONFIG, reg_pa_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( sx127x_read_register( radio, SX1272_REG_COMMON_PA_DAC, &reg_pa_dac, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_pa_values[0] =
        ( reg_pa_values[0] & SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_MASK ) |
        ( ( radio->pa_cfg_params.pa_select == SX127X_PA_SELECT_BOOST ) ? SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_BOOST
                                                                       : SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_RFO );

    reg_pa_dac = ( reg_pa_dac & SX1272_REG_COMMON_PA_DAC_PA_DAC_MASK ) |
                 ( ( radio->pa_cfg_params.is_20_dbm_output_on == true ) ? SX1272_REG_COMMON_PA_DAC_PA_DAC_ON
                                                                        : SX1272_REG_COMMON_PA_DAC_PA_DAC_OFF );

    if( ( radio->pa_cfg_params.pa_select == SX127X_PA_SELECT_BOOST ) )
    {  // PA_BOOST
        if( radio->pa_cfg_params.is_20_dbm_output_on == true )
        {
            reg_pa_values[0] = ( reg_pa_values[0] & SX127X_REG_COMMON_PA_CONFIG_OUTPUT_POWER_MASK ) |
                               ( ( uint8_t ) ( pwr - 5 ) & 0x0F );
        }
        else
        {
            reg_pa_values[0] = ( reg_pa_values[0] & SX127X_REG_COMMON_PA_CONFIG_OUTPUT_POWER_MASK ) |
                               ( ( uint8_t ) ( pwr - 2 ) & 0x0F );
        }
    }
    else
    {  // RFO
        reg_pa_values[0] =
            ( reg_pa_values[0] & SX127X_REG_COMMON_PA_CONFIG_OUTPUT_POWER_MASK ) | ( ( uint8_t ) ( pwr + 1 ) & 0x0F );
    }

    reg_pa_values[1] = ( reg_pa_values[1] & SX127X_REG_COMMON_PA_RAMP_PA_RAMP_MASK ) | pa_ramp;

    // Writes SX127X_REG_COMMON_PA_CONFIG and SX127X_REG_COMMON_PA_RAMP registers
    if( sx127x_write_register( radio, SX127X_REG_COMMON_PA_CONFIG, reg_pa_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( sx127x_write_register( radio, SX1272_REG_COMMON_PA_DAC, &reg_pa_dac, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    radio->reg_pa_config = reg_pa_values[0];
    radio->reg_pa_ramp   = reg_pa_values[1];
    radio->reg_pa_dac    = reg_pa_dac;
    return SX127X_STATUS_OK;
}
#endif

#if defined( SX1276 )
static sx127x_status_t sx1276_set_tx_params( sx127x_t* radio, const int8_t pwr_in_dbm,
                                             const sx127x_ramp_time_t pa_ramp )
{
    // [0] PaConfig, [1] PaRamp
    uint8_t reg_pa_values[2] = { 0 };
    uint8_t reg_pa_dac       = 0;
    int8_t  pwr              = pwr_in_dbm;
    uint8_t max_pwr          = 0;

    // Reads SX127X_REG_COMMON_PA_CONFIG and SX127X_REG_COMMON_PA_RAMP registers
    if( sx127x_read_register( radio, SX127X_REG_COMMON_PA_CONFIG, reg_pa_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( sx127x_read_register( radio, SX1276_REG_COMMON_PA_DAC, &reg_pa_dac, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_pa_values[0] =
        ( reg_pa_values[0] & SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_MASK ) |
        ( ( radio->pa_cfg_params.pa_select == SX127X_PA_SELECT_BOOST ) ? SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_BOOST
                                                                       : SX127X_REG_COMMON_PA_CONFIG_PA_SELECT_RFO );

    reg_pa_dac = ( reg_pa_dac & SX1276_REG_COMMON_PA_DAC_PA_DAC_MASK ) |
                 ( ( radio->pa_cfg_params.is_20_dbm_output_on == true ) ? SX1276_REG_COMMON_PA_DAC_PA_DAC_ON
                                                                        : SX1276_REG_COMMON_PA_DAC_PA_DAC_OFF );

    if( ( radio->pa_cfg_params.pa_select == SX127X_PA_SELECT_BOOST ) )
    {  // PA_BOOST
        if( radio->pa_cfg_params.is_20_dbm_output_on == true )
        {
            reg_pa_values[0] = ( reg_pa_values[0] & SX127X_REG_COMMON_PA_CONFIG_OUTPUT_POWER_MASK ) |
                               ( ( uint8_t ) ( pwr - 5 ) & 0x0F );
        }
        else
        {
            reg_pa_values[0] = ( reg_pa_values[0] & SX127X_REG_COMMON_PA_CONFIG_OUTPUT_POWER_MASK ) |
                               ( ( uint8_t ) ( pwr - 2 ) & 0x0F );
        }
    }
    else
    {  // RFO
        if( pwr > 0 )
        {
            // Use max_pwr = 7 for rf output power bigger than 0. (best linearity)
            max_pwr = 7;
        }
        else
        {
            // Use max_pwr = 0 for rf output power smaller or equal than 0. (best linearity)
            // pwr value must be compensated by the minimal value (4)
            max_pwr = 0;
            pwr += 4;
        }
        reg_pa_values[0] = ( reg_pa_values[0] & SX1276_REG_COMMON_PA_CONFIG_MAX_POWER_MASK &
                             SX127X_REG_COMMON_PA_CONFIG_OUTPUT_POWER_MASK ) |
                           ( max_pwr << 4 ) | ( ( uint8_t ) ( pwr ) &0x0F );
    }

    reg_pa_values[1] = ( reg_pa_values[1] & SX127X_REG_COMMON_PA_RAMP_PA_RAMP_MASK ) | pa_ramp;

    // Writes SX127X_REG_COMMON_PA_CONFIG and SX127X_REG_COMMON_PA_RAMP registers
    if( sx127x_write_register( radio, SX127X_REG_COMMON_PA_CONFIG, reg_pa_values, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( sx127x_write_register( radio, SX1276_REG_COMMON_PA_DAC, &reg_pa_dac, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    radio->reg_pa_config = reg_pa_values[0];
    radio->reg_pa_ramp   = reg_pa_values[1];
    radio->reg_pa_dac    = reg_pa_dac;
    return SX127X_STATUS_OK;
}
#endif

#if defined( SX1272 )
static sx127x_status_t sx1272_set_gfsk_pulse_shape( sx127x_t* radio, const sx127x_gfsk_mod_params_t* params )
{
    uint8_t reg_op_mode = 0;

    if( sx127x_read_register( radio, SX127X_REG_COMMON_OP_MODE, &reg_op_mode, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    switch( params->pulse_shape.gfsk )
    {
    case SX127X_GFSK_PULSE_SHAPE_OFF:
        reg_op_mode = ( reg_op_mode & SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_MASK ) |
                      SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_OFF;
        break;
    case SX127X_GFSK_PULSE_SHAPE_BT_1:
        reg_op_mode = ( reg_op_mode & SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_MASK ) |
                      SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_BT_1;
        break;
    case SX127X_GFSK_PULSE_SHAPE_BT_05:
        reg_op_mode = ( reg_op_mode & SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_MASK ) |
                      SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_BT_05;
        break;
    case SX127X_GFSK_PULSE_SHAPE_BT_03:
        reg_op_mode = ( reg_op_mode & SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_MASK ) |
                      SX1272_REG_COMMON_OP_MODE_GFSK_MOD_SHAPE_BT_03;
        break;
    default:
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_write_register( radio, SX127X_REG_COMMON_OP_MODE, &reg_op_mode, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    return SX127X_STATUS_OK;
}
#endif

#if defined( SX1276 )
static sx127x_status_t sx1276_set_gfsk_pulse_shape( sx127x_t* radio, const sx127x_gfsk_mod_params_t* params )
{
    uint8_t reg_pa_ramp = 0;

    if( sx127x_read_register( radio, SX127X_REG_COMMON_PA_RAMP, &reg_pa_ramp, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    switch( params->pulse_shape.gfsk )
    {
    case SX127X_GFSK_PULSE_SHAPE_OFF:
        reg_pa_ramp = ( reg_pa_ramp & SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_MASK ) |
                      SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_OFF;
        break;
    case SX127X_GFSK_PULSE_SHAPE_BT_1:
        reg_pa_ramp = ( reg_pa_ramp & SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_MASK ) |
                      SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_BT_1;
        break;
    case SX127X_GFSK_PULSE_SHAPE_BT_05:
        reg_pa_ramp = ( reg_pa_ramp & SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_MASK ) |
                      SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_BT_05;
        break;
    case SX127X_GFSK_PULSE_SHAPE_BT_03:
        reg_pa_ramp = ( reg_pa_ramp & SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_MASK ) |
                      SX1276_REG_COMMON_PA_RAMP_GFSK_MOD_SHAPE_BT_03;
        break;
    default:
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_write_register( radio, SX127X_REG_COMMON_PA_RAMP, &reg_pa_ramp, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    radio->reg_pa_ramp = reg_pa_ramp;

    return SX127X_STATUS_OK;
}
#endif

static inline uint32_t sx127x_get_gfsk_crc_len_in_bytes( sx127x_gfsk_crc_type_t crc_type )
{
    switch( crc_type )
    {
    case SX127X_GFSK_CRC_OFF:
        return 0;
    case SX127X_GFSK_CRC_2_BYTES_INV:
        return 2;
    default:
        return 0;
    }
}

static sx127x_status_t sx127x_get_op_mode( sx127x_t* radio, uint8_t* op_mode )
{
    if( sx127x_read_register( radio, SX127X_REG_COMMON_OP_MODE, op_mode, 1 ) == SX127X_STATUS_OK )
    {
        *op_mode = *op_mode & ~SX127X_REG_COMMON_OP_MODE_MODE_MASK;
        return SX127X_STATUS_OK;
    }
    return SX127X_STATUS_ERROR;
}

static sx127x_status_t sx127x_set_op_mode( sx127x_t* radio, const uint8_t op_mode )
{
    sx127x_status_t status        = SX127X_STATUS_ERROR;
    uint8_t         local_op_mode = SX127X_REG_COMMON_OP_MODE_MODE_SLEEP;

    status = sx127x_get_op_mode( radio, &local_op_mode );
    if( status == SX127X_STATUS_OK )
    {
        local_op_mode = ( local_op_mode & SX127X_REG_COMMON_OP_MODE_MODE_MASK ) | op_mode;
        status        = sx127x_write_register( radio, SX127X_REG_COMMON_OP_MODE, &local_op_mode, 1 );
    }
    return status;
}

static sx127x_status_t sx127x_write_fifo( sx127x_t* radio, const uint8_t* data, const uint8_t data_len )
{
    return ( sx127x_status_t ) sx127x_hal_write( radio, 0, data, data_len );
}

static sx127x_status_t sx127x_read_fifo( sx127x_t* radio, uint8_t* data, const uint8_t data_len )
{
    return ( sx127x_status_t ) sx127x_hal_read( radio, 0, data, data_len );
}

static sx127x_status_t sx127x_timer_stop_all( sx127x_t* radio )
{
    if( sx127x_hal_timer_is_started( radio ) == true )
    {
        if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }
    return SX127X_STATUS_OK;
}

#if defined( SX1276 )
static sx127x_status_t sx1276_fix_lora_500_khz_bw_sensitivity( sx127x_t* radio )
{
    // [0] RegHighBwOptimize1, [1] RegHighBwOptimize2
    uint8_t reg_value[2];

    // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
    if( ( radio->lora_mod_params.bw == SX127X_LORA_BW_500 ) &&
        ( radio->rf_freq_in_hz > RF_FREQUENCY_MID_BAND_THRESHOLD ) )
    {
        reg_value[0] = 0x02;
        reg_value[1] = 0x64;
    }
    else if( radio->lora_mod_params.bw == SX127X_LORA_BW_500 )
    {
        reg_value[0] = 0x02;
        reg_value[1] = 0x7F;
    }
    else
    {
        reg_value[0] = 0x03;
    }

    if( sx127x_write_register( radio, SX1276_REG_LORA_HIGH_BW_OPTIMIZE_1, reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( radio->lora_mod_params.bw == SX127X_LORA_BW_500 )
    {
        // Only write RegHighBwOptimize2 when a bandwidth of 500 kHz is being used
        // For all other bandwidths this parameter is automatically selected by the radio
        if( sx127x_write_register( radio, SX1276_REG_LORA_HIGH_BW_OPTIMIZE_2, reg_value + 1, 1 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }

    return SX127X_STATUS_OK;
}

static sx127x_status_t sx1276_fix_lora_rx_spurious_signal( sx127x_t* radio )
{
    uint8_t  reg_value     = 0;
    uint32_t rf_freq_in_hz = radio->rf_freq_in_hz;
    bool     is_auto_if_on = false;
    // [0] RegIfFreq1, [1] RegIfFreq2
    uint8_t if_freq[2] = { 0 };

    switch( radio->lora_mod_params.bw )
    {
    case SX127X_LORA_BW_007:  // 7.8 kHz
        if_freq[0]    = 0x48;
        rf_freq_in_hz = radio->rf_freq_in_hz + sx127x_get_lora_bw_in_hz( radio->lora_mod_params.bw );
        break;
    case SX127X_LORA_BW_010:  // 10.4 kHz
                              // Intentional fallthrough
    case SX127X_LORA_BW_015:  // 15.6 kHz
                              // Intentional fallthrough
    case SX127X_LORA_BW_020:  // 20.8 kHz
                              // Intentional fallthrough
    case SX127X_LORA_BW_031:  // 31.2 kHz
                              // Intentional fallthrough
    case SX127X_LORA_BW_041:  // 41.4 kHz
        if_freq[0]    = 0x44;
        rf_freq_in_hz = radio->rf_freq_in_hz + sx127x_get_lora_bw_in_hz( radio->lora_mod_params.bw );
        break;
    case SX127X_LORA_BW_062:  // 62.5 kHz
                              // Intentional fallthrough
    case SX127X_LORA_BW_125:  // 125 kHz
                              // Intentional fallthrough
    case SX127X_LORA_BW_250:  // 250 kHz
        if_freq[0] = 0x40;
        break;
    case SX127X_LORA_BW_500:  // 500 kHz
        is_auto_if_on = true;
        break;
    default:
        return SX127X_STATUS_ERROR;
    }

    if( sx127x_read_register( radio, SX127X_REG_LORA_DETECT_OPTIMIZE, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    reg_value &= SX127X_REG_LORA_DETECT_OPTIMIZE_AUTOMATIC_IF_MASK;
    reg_value |= ( is_auto_if_on == true ) ? SX127X_REG_LORA_DETECT_OPTIMIZE_AUTOMATIC_IF_ON
                                           : SX127X_REG_LORA_DETECT_OPTIMIZE_AUTOMATIC_IF_OFF;
    if( sx127x_write_register( radio, SX127X_REG_LORA_DETECT_OPTIMIZE, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    if( is_auto_if_on == false )
    {
        if( sx127x_write_register( radio, SX1276_REG_LORA_IF_FREQ_1, if_freq, 2 ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
        if( sx127x_set_rf_freq( radio, rf_freq_in_hz ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }

    return SX127X_STATUS_OK;
}
#endif

static sx127x_status_t sx127x_update_lora_iq_inverted_regs( sx127x_t* radio, bool is_tx_on )
{
    uint8_t reg_iq_1;
    uint8_t reg_iq_2;

    if( sx127x_read_register( radio, SX127X_REG_LORA_INVERT_IQ, &reg_iq_1, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_iq_1 = reg_iq_1 & SX127X_REG_LORA_INVERT_IQ_RX_MASK & SX127X_REG_LORA_INVERT_IQ_TX_MASK;
    if( radio->invert_iq_is_on == true )
    {
        if( is_tx_on == true )
        {
            reg_iq_1 |= SX127X_REG_LORA_INVERT_IQ_RX_NORMAL | SX127X_REG_LORA_INVERT_IQ_TX_INVERTED;
        }
        else
        {
            reg_iq_1 |= SX127X_REG_LORA_INVERT_IQ_RX_INVERTED | SX127X_REG_LORA_INVERT_IQ_TX_NORMAL;
        }
        reg_iq_2 = SX127X_REG_LORA_INVERT_IQ_2_INVERTED;
    }
    else
    {
        if( is_tx_on == true )
        {
            reg_iq_1 |= SX127X_REG_LORA_INVERT_IQ_RX_NORMAL | SX127X_REG_LORA_INVERT_IQ_TX_NORMAL;
        }
        else
        {
            reg_iq_1 |= SX127X_REG_LORA_INVERT_IQ_RX_NORMAL | SX127X_REG_LORA_INVERT_IQ_TX_NORMAL;
        }
        reg_iq_2 = SX127X_REG_LORA_INVERT_IQ_2_NORMAL;
    }

    if( sx127x_write_register( radio, SX127X_REG_LORA_INVERT_IQ, &reg_iq_1, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( sx127x_write_register( radio, SX127X_REG_LORA_INVERT_IQ_2, &reg_iq_2, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    return SX127X_STATUS_OK;
}

static sx127x_status_t sx127x_re_start_rx_chain( sx127x_t* radio )
{
    uint8_t reg_value;
    if( sx127x_read_register( radio, SX127X_REG_GFSK_RX_CONFIG, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    reg_value |= SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_WITH_PLL_LOCK_TRIG;

    if( sx127x_write_register( radio, SX127X_REG_GFSK_RX_CONFIG, &reg_value, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    return SX127X_STATUS_OK;
}

static sx127x_status_t sx127x_get_lora_irq_status( sx127x_t* radio, uint8_t* irq_flags )
{
    if( sx127x_read_register( radio, SX127X_REG_LORA_IRQ_FLAGS, irq_flags, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    return SX127X_STATUS_OK;
}

static sx127x_status_t sx127x_get_gfsk_irq_status( sx127x_t* radio, uint16_t* irq_flags )
{
    uint8_t reg_value[2];

    if( sx127x_read_register( radio, SX127X_REG_GFSK_IRQ_FLAGS_1, reg_value, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    *irq_flags = ( uint16_t ) ( ( reg_value[0] << 8 ) | reg_value[1] );
    return SX127X_STATUS_OK;
}

static sx127x_status_t sx127x_clear_lora_irq_status( sx127x_t* radio, const uint8_t irq_mask )
{
    if( sx127x_write_register( radio, SX127X_REG_LORA_IRQ_FLAGS, &irq_mask, 1 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    return SX127X_STATUS_OK;
}

static sx127x_status_t sx127x_clear_gfsk_irq_status( sx127x_t* radio, const uint16_t irq_mask )
{
    uint8_t reg_value[2];

    reg_value[0] = ( uint8_t ) ( irq_mask >> 8 );
    reg_value[1] = ( uint8_t ) ( irq_mask >> 0 );
    if( sx127x_write_register( radio, SX127X_REG_GFSK_IRQ_FLAGS_1, reg_value, 2 ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }

    return SX127X_STATUS_OK;
}

static sx127x_status_t sx127x_get_and_clear_lora_irq_status( sx127x_t* radio, uint8_t* irq_flags )
{
    uint8_t irq_mask;

    if( sx127x_get_lora_irq_status( radio, &irq_mask ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( irq_mask != 0 )
    {
        if( sx127x_clear_lora_irq_status( radio, irq_mask ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }
    if( irq_flags != NULL )
    {
        *irq_flags = irq_mask;
    }
    return SX127X_STATUS_OK;
}

static sx127x_status_t sx127x_get_and_clear_gfsk_irq_status( sx127x_t* radio, uint16_t* irq_flags )
{
    uint16_t irq_mask;

    if( sx127x_get_gfsk_irq_status( radio, &irq_mask ) != SX127X_STATUS_OK )
    {
        return SX127X_STATUS_ERROR;
    }
    if( irq_mask != 0 )
    {
        if( sx127x_clear_gfsk_irq_status( radio, irq_mask ) != SX127X_STATUS_OK )
        {
            return SX127X_STATUS_ERROR;
        }
    }
    if( irq_flags != NULL )
    {
        *irq_flags = irq_mask;
    }
    return SX127X_STATUS_OK;
}

static void sx127x_notify_irq( sx127x_t* radio, uint16_t irq_flags )
{
    radio->irq_flags = irq_flags;

    if( ( irq_flags & radio->irq_flags_mask ) == irq_flags )
    {
        if( radio->irq_handler != NULL )
        {
            radio->irq_handler( radio->irq_handler_context );
        }
    }
}

static void rx_timer_irq_handler( void* context )
{
    sx127x_t* radio = ( sx127x_t* ) context;

    if( ( radio->pkt_type == SX127X_PKT_TYPE_GFSK ) || ( radio->pkt_type == SX127X_PKT_TYPE_OOK ) )
    {
        radio->is_gfsk_sync_word_detected       = false;
        radio->gfsk_pkt_params.pld_len_in_bytes = 0;
        radio->buffer_index                     = 0;

        if( sx127x_clear_gfsk_irq_status( radio, SX127X_REG_GFSK_IRQ_FLAGS_ALL ) != SX127X_STATUS_OK )
        {
            return;
        }

        if( radio->is_rx_continuous == true )
        {
            uint8_t reg_value;
            if( sx127x_read_register( radio, SX127X_REG_GFSK_RX_CONFIG, &reg_value, 1 ) != SX127X_STATUS_OK )
            {
                return;
            }

            reg_value |= SX127X_REG_GFSK_RX_CONFIG_RESTART_RX_WITH_PLL_LOCK_TRIG;

            if( sx127x_write_register( radio, SX127X_REG_GFSK_RX_CONFIG, &reg_value, 1 ) != SX127X_STATUS_OK )
            {
                return;
            }
        }
        else
        {
            if( sx127x_set_sleep( radio ) != SX127X_STATUS_OK )
            {
                return;
            }
            if( sx127x_hal_timer_is_started( radio ) == true )
            {
                if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
                {
                    return;
                }
            }
        }
    }
    // Notify upper layers that an SX127X_IRQ_TIMEOUT interrupt occurred
    sx127x_notify_irq( radio, SX127X_IRQ_TIMEOUT );
}

static void dio_0_irq_lora_handler( sx127x_t* radio )
{
    if( ( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_RX ) ||
        ( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_LORA_RX_SINGLE ) )
    {
        // RxDone interrupt
        if( sx127x_clear_lora_irq_status( radio, SX127X_REG_LORA_IRQ_FLAGS_RX_DONE ) != SX127X_STATUS_OK )
        {
            return;
        }

        uint8_t irq_flags;
        if( sx127x_get_lora_irq_status( radio, &irq_flags ) != SX127X_STATUS_OK )
        {
            return;
        }

        if( ( irq_flags & SX127X_REG_LORA_IRQ_FLAGS_PAYLOAD_CRC_ERROR ) == SX127X_REG_LORA_IRQ_FLAGS_PAYLOAD_CRC_ERROR )
        {
            if( sx127x_clear_lora_irq_status( radio, SX127X_REG_LORA_IRQ_FLAGS_PAYLOAD_CRC_ERROR ) != SX127X_STATUS_OK )
            {
                return;
            }

            if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
            {
                return;
            }

            // Notify upper layers that an SX127X_IRQ_CRC_ERROR interrupt occurred
            sx127x_notify_irq( radio, SX127X_IRQ_CRC_ERROR );
            return;
        }

        if( sx127x_read_register( radio, SX127X_REG_LORA_RX_NB_BYTES, &radio->lora_pkt_params.pld_len_in_bytes, 1 ) !=
            SX127X_STATUS_OK )
        {
            return;
        }

        uint8_t reg_value;
        if( sx127x_read_register( radio, SX127X_REG_LORA_FIFO_RX_CURRENT_ADDR, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return;
        }
        if( sx127x_write_register( radio, SX127X_REG_LORA_FIFO_ADDR_PTR, &reg_value, 1 ) != SX127X_STATUS_OK )
        {
            return;
        }

        if( sx127x_read_fifo( radio, radio->buffer, radio->lora_pkt_params.pld_len_in_bytes ) != SX127X_STATUS_OK )
        {
            return;
        }

        if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
        {
            return;
        }

        // Notify upper layers that an SX127X_IRQ_RX_DONE interrupt occurred
        sx127x_notify_irq( radio, SX127X_IRQ_RX_DONE );
    }
    else if( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_TX )
    {
        // TxDone interrupt
        if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
        {
            return;
        }

        if( sx127x_clear_lora_irq_status( radio, SX127X_REG_LORA_IRQ_FLAGS_TX_DONE ) != SX127X_STATUS_OK )
        {
            return;
        }

        // Notify upper layers that an SX127X_IRQ_TX_DONE interrupt occurred
        sx127x_notify_irq( radio, SX127X_IRQ_TX_DONE );
    }
    else if( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_LORA_CAD )
    {
        uint8_t irq_flags;
        uint8_t irq_flags_notify = SX127X_IRQ_NONE;

        if( sx127x_get_and_clear_lora_irq_status( radio, &irq_flags ) != SX127X_STATUS_OK )
        {
            return;
        }

        if( ( irq_flags & SX127X_REG_LORA_IRQ_FLAGS_CAD_DETECT ) == SX127X_REG_LORA_IRQ_FLAGS_CAD_DETECT )
        {
            // Notify upper layers that an SX127X_IRQ_CAD_DETECTED interrupt occurred
            irq_flags_notify |= SX127X_IRQ_CAD_DETECTED;
        }
        if( ( irq_flags & SX127X_REG_LORA_IRQ_FLAGS_CAD_DONE ) == SX127X_REG_LORA_IRQ_FLAGS_CAD_DONE )
        {
            // Notify upper layers that an SX127X_IRQ_CAD_DONE interrupt occurred
            irq_flags_notify |= SX127X_IRQ_CAD_DONE;
        }

        // Notify upper layers that an SX127X_IRQ_CAD_DETECTED or SX127X_REG_LORA_IRQ_FLAGS_CAD_DONE interrupt occurred
        sx127x_notify_irq( radio, irq_flags_notify );
    }
}

static void dio_0_irq_gfsk_handler( sx127x_t* radio )
{
    if( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_RX )
    {
        // RxDone interrupt
        if( radio->gfsk_pkt_params.crc_type != SX127X_GFSK_CRC_OFF )
        {
            uint16_t irq_flags;
            if( sx127x_get_and_clear_gfsk_irq_status( radio, &irq_flags ) != SX127X_STATUS_OK )
            {
                return;
            }

            if( ( irq_flags & SX127X_REG_GFSK_IRQ_FLAGS_CRC_OK ) != SX127X_REG_GFSK_IRQ_FLAGS_CRC_OK )
            {
                if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
                {
                    return;
                }

                if( radio->is_rx_continuous == true )
                {
                    // Continuous mode restart Rx chain
                    if( sx127x_re_start_rx_chain( radio ) != SX127X_STATUS_OK )
                    {
                        return;
                    }
                }

                radio->is_gfsk_sync_word_detected = false;

                // Notify upper layers that an SX127X_IRQ_CRC_ERROR interrupt occurred
                sx127x_notify_irq( radio, SX127X_IRQ_CRC_ERROR );
                return;
            }
        }

        // Read received packet size
        if( ( radio->gfsk_pkt_params.pld_len_in_bytes == 0 ) && ( radio->buffer_index == 0 ) )
        {
            if( radio->gfsk_pkt_params.header_type == SX127X_GFSK_PKT_VAR_LEN )
            {
                if( sx127x_read_fifo( radio, &radio->gfsk_pkt_params.pld_len_in_bytes, 1 ) != SX127X_STATUS_OK )
                {
                    return;
                }
            }
            else
            {
                if( sx127x_read_register( radio, SX127X_REG_GFSK_PAYLOAD_LENGTH,
                                          &radio->gfsk_pkt_params.pld_len_in_bytes, 1 ) != SX127X_STATUS_OK )
                {
                    return;
                }
            }
            if( sx127x_read_fifo( radio, radio->buffer + radio->buffer_index,
                                  radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) != SX127X_STATUS_OK )
            {
                return;
            }
            radio->buffer_index += ( radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index );
        }
        else
        {
            if( ( int8_t ) ( radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) < 0 )
            {
                return;
            }
            if( sx127x_read_fifo( radio, radio->buffer + radio->buffer_index,
                                  radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) != SX127X_STATUS_OK )
            {
                return;
            }
            radio->buffer_index += ( radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index );
        }

        if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
        {
            return;
        }

        if( radio->is_rx_continuous == true )
        {
            // Continuous mode restart Rx chain
            if( sx127x_re_start_rx_chain( radio ) != SX127X_STATUS_OK )
            {
                return;
            }
        }

        radio->is_gfsk_sync_word_detected = false;

        // Notify upper layers that an SX127X_IRQ_RX_DONE interrupt occurred
        sx127x_notify_irq( radio, SX127X_IRQ_RX_DONE );
    }
    else if( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_TX )
    {
        // TxDone interrupt
        if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
        {
            return;
        }

        // Notify upper layers that an SX127X_IRQ_TX_DONE interrupt occurred
        sx127x_notify_irq( radio, SX127X_IRQ_TX_DONE );
    }
}

static void dio_1_irq_lora_handler( sx127x_t* radio )
{
    // Check RxTimeout DIO1 pin state
    //
    // DIO1 irq is setup to be triggered on rising and falling edges
    // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
    // verify DIO1 pin state in order to decide if something has to be done.
    // When radio is operating in LoRa reception mode a rising edge must be detected in order to handle the
    // IRQ.
    if( sx127x_hal_get_dio_1_pin_state( radio ) == 0 )
    {
        return;
    }
    if( ( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_RX ) ||
        ( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_LORA_RX_SINGLE ) )
    {
        // Sync time out
        if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
        {
            return;
        }

        if( sx127x_clear_lora_irq_status( radio, SX127X_REG_LORA_IRQ_FLAGS_RX_TIMEOUT ) != SX127X_STATUS_OK )
        {
            return;
        }

        // Notify upper layers that an SX127X_IRQ_TIMEOUT interrupt occurred
        sx127x_notify_irq( radio, SX127X_IRQ_TIMEOUT );
    }
}

static void dio_1_irq_gfsk_handler( sx127x_t* radio )
{
    if( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_RX )
    {
        // Check FIFO level DIO1 pin state
        //
        // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
        // verify DIO1 pin state in order to decide if something has to be done.
        // When radio is operating in FSK reception mode a rising edge must be detected in order to handle the
        // IRQ.
        if( sx127x_hal_get_dio_1_pin_state( radio ) == 0 )
        {
            return;
        }

        // FifoLevel interrupt
        if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
        {
            return;
        }

        // Read received packet size
        if( ( radio->gfsk_pkt_params.pld_len_in_bytes == 0 ) && ( radio->buffer_index == 0 ) )
        {
            if( radio->gfsk_pkt_params.header_type == SX127X_GFSK_PKT_VAR_LEN )
            {
                if( sx127x_read_fifo( radio, &radio->gfsk_pkt_params.pld_len_in_bytes, 1 ) != SX127X_STATUS_OK )
                {
                    return;
                }
            }
            else
            {
                if( sx127x_read_register( radio, SX127X_REG_GFSK_PAYLOAD_LENGTH,
                                          &radio->gfsk_pkt_params.pld_len_in_bytes, 1 ) != SX127X_STATUS_OK )
                {
                    return;
                }
            }
        }

        // ERRATA 3.1 - PayloadReady Set for 31.25ns if FIFO is Empty
        //
        //              When FifoLevel interrupt is used to offload the
        //              FIFO, the microcontroller should  monitor  both
        //              PayloadReady  and FifoLevel interrupts, and
        //              read only (FifoThreshold-1) bytes off the FIFO
        //              when FifoLevel fires
        if( ( radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) >= radio->gfsk_fifo_threshold_in_bytes )
        {
            if( sx127x_read_fifo( radio, radio->buffer + radio->buffer_index,
                                  radio->gfsk_fifo_threshold_in_bytes - 1 ) != SX127X_STATUS_OK )
            {
                return;
            }
            radio->buffer_index += radio->gfsk_fifo_threshold_in_bytes - 1;
        }
        else
        {
            if( ( int8_t ) ( radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) < 0 )
            {
                return;
            }
            if( sx127x_read_fifo( radio, radio->buffer + radio->buffer_index,
                                  radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) != SX127X_STATUS_OK )
            {
                return;
            }
            radio->buffer_index += ( radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index );
        }
    }
    else if( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_TX )
    {
        // Check FIFO level DIO1 pin state
        //
        // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
        // verify DIO1 pin state in order to decide if something has to be done.
        // When radio is operating in FSK transmission mode a falling edge must be detected in order to handle
        // the IRQ.
        if( sx127x_hal_get_dio_1_pin_state( radio ) == 1 )
        {
            return;
        }

        // FifoLevel interrupt
        if( ( radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) > radio->gfsk_pkt_chunk_len_in_bytes )
        {
            if( sx127x_write_fifo( radio, radio->buffer + radio->buffer_index, radio->gfsk_pkt_chunk_len_in_bytes ) !=
                SX127X_STATUS_OK )
            {
                return;
            }
            radio->buffer_index += radio->gfsk_pkt_chunk_len_in_bytes;
        }
        else
        {
            if( ( radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) > 0 )
            {
                // Write the last chunk of data
                if( sx127x_write_fifo( radio, radio->buffer + radio->buffer_index,
                                       radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index ) !=
                    SX127X_STATUS_OK )
                {
                    return;
                }
                radio->buffer_index += radio->gfsk_pkt_params.pld_len_in_bytes - radio->buffer_index;
            }
        }
    }
}

static void dio_2_irq_lora_handler( sx127x_t* radio )
{
#if defined( IS_LORA_FREQUENCY_HOPPING_SUPPORTED )

    if( ( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_RX ) ||
        ( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_LORA_RX_SINGLE ) ||
        ( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_TX ) )
    {
        if( radio->is_lora_freq_hopping_on == true )
        {
            if( sx127x_clear_lora_irq_status( radio, SX127X_REG_LORA_IRQ_FLAGS_FHSS_CHANGE_CHANNEL ) !=
                SX127X_STATUS_OK )
            {
                return;
            }

            // Notify upper layers that an SX127X_IRQ_FHSS_CHANNEL_CHANGE interrupt occurred
            sx127x_notify_irq( radio, SX127X_IRQ_FHSS_CHANNEL_CHANGE );
        }
    }
#else
    // Unused parameter
    ( void ) radio;
#endif
}

static void dio_2_irq_gfsk_handler( sx127x_t* radio )
{
    if( radio->op_mode_irq == SX127X_REG_COMMON_OP_MODE_MODE_RX )
    {
        if( radio->is_gfsk_sync_word_detected == false )
        {
            radio->is_gfsk_sync_word_detected = true;

            if( sx127x_hal_timer_stop( radio ) != SX127X_HAL_STATUS_OK )
            {
                return;
            }

            if( sx127x_get_rssi_inst( radio, &radio->gfsk_rssi_in_dbm ) != SX127X_STATUS_OK )
            {
                return;
            }

            // Notify upper layers that an SX127X_IRQ_SYNC_WORD_VALID interrupt occurred
            sx127x_notify_irq( radio, SX127X_IRQ_SYNC_WORD_VALID );
        }
    }
}

static void dio_0_irq_handler( void* context )
{
    sx127x_t* radio = ( sx127x_t* ) context;

    // Backup current op_mode to op_mode_irq. op_mode_irq has to be used while handling the interrupt
    radio->op_mode_irq = radio->op_mode;
    // Update op_mode with current radio status
    if( sx127x_get_op_mode( radio, &radio->op_mode ) != SX127X_STATUS_OK )
    {
        return;
    }

    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {
        dio_0_irq_lora_handler( radio );
    }
    else
    {
        dio_0_irq_gfsk_handler( radio );
    }
}

static void dio_1_irq_handler( void* context )
{
    sx127x_t* radio = ( sx127x_t* ) context;

    // Backup current op_mode to op_mode_irq. op_mode_irq has to be used while handling the interrupt
    radio->op_mode_irq = radio->op_mode;
    // Update op_mode with current radio status
    if( sx127x_get_op_mode( radio, &radio->op_mode ) != SX127X_STATUS_OK )
    {
        return;
    }

    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {
        dio_1_irq_lora_handler( radio );
    }
    else
    {
        dio_1_irq_gfsk_handler( radio );
    }
}

static void dio_2_irq_handler( void* context )
{
    sx127x_t* radio = ( sx127x_t* ) context;

    // Backup current op_mode to op_mode_irq. op_mode_irq has to be used while handling the interrupt
    radio->op_mode_irq = radio->op_mode;
    // Update op_mode with current radio status
    if( sx127x_get_op_mode( radio, &radio->op_mode ) != SX127X_STATUS_OK )
    {
        return;
    }

    if( radio->pkt_type == SX127X_PKT_TYPE_LORA )
    {
        dio_2_irq_lora_handler( radio );
    }
    else
    {
        dio_2_irq_gfsk_handler( radio );
    }
}

/* --- EOF ------------------------------------------------------------------ */
