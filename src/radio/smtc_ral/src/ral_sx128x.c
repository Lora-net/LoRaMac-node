/**
 * @file      ral_sx128x.c
 *
 * @brief     Radio abstraction layer definition
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

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "sx128x.h"
#include "ral_sx128x.h"
#include "ral_sx128x_bsp.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */
#define SX128X_CONVERT_TABLE_INDEX_OFFSET 18

static const uint32_t ral_sx128x_convert_tx_dbm_to_ua_reg_mode_dcdc[] = {
    6200,   // -18 dBm
    6300,   // -17 dBm
    6400,   // -16 dBm
    6500,   // -15 dBm
    6600,   // -14 dBm
    6700,   // -13 dBm
    6800,   // -12 dBm
    7000,   // -11 dBm
    7100,   // -10 dBm
    7300,   //  -9 dBm
    7400,   //  -8 dBm
    7700,   //  -7 dBm
    7900,   //  -6 dBm
    8100,   //  -5 dBm
    8500,   //  -4 dBm
    8800,   //  -3 dBm
    9200,   //  -2 dBm
    9700,   //  -1 dBm
    10100,  //   0 dBm
    10700,  //   1 dBm
    11300,  //   2 dBm
    12000,  //   3 dBm
    12700,  //   4 dBm
    13600,  //   5 dBm
    14500,  //   6 dBm
    15500,  //   7 dBm
    16800,  //   8 dBm
    17700,  //   9 dBm
    18600,  //  10 dBm
    20300,  //  11 dBm
    22000,  //  12 dBm
    24000,  //  13 dBm
};

static const uint32_t ral_sx128x_convert_tx_dbm_to_ua_reg_mode_ldo[] = {
    11800,  // -18 dBm
    12000,  // -17 dBm
    12200,  // -16 dBm
    12400,  // -15 dBm
    12600,  // -14 dBm
    12800,  // -13 dBm
    13000,  // -12 dBm
    13300,  // -11 dBm
    13500,  // -10 dBm
    14000,  //  -9 dBm
    14200,  //  -8 dBm
    14700,  //  -7 dBm
    15200,  //  -6 dBm
    15600,  //  -5 dBm
    16300,  //  -4 dBm
    17000,  //  -3 dBm
    17700,  //  -2 dBm
    18600,  //  -1 dBm
    19600,  //   0 dBm
    20700,  //   1 dBm
    21900,  //   2 dBm
    23200,  //   3 dBm
    24600,  //   4 dBm
    26300,  //   5 dBm
    28000,  //   6 dBm
    30000,  //   7 dBm
    32200,  //   8 dBm
    34500,  //   9 dBm
    36800,  //  10 dBm
    39200,  //  11 dBm
    41900,  //  12 dBm
    45500,  //  13 dBm
};

#define SX128X_LORA_RX_CONSUMPTION_BW_200_DCDC 5500
#define SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_200_DCDC 6200

#define SX128X_LORA_RX_CONSUMPTION_BW_400_DCDC 6000
#define SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_400_DCDC 6700

#define SX128X_LORA_RX_CONSUMPTION_BW_800_DCDC 7000
#define SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_800_DCDC 7700

#define SX128X_LORA_RX_CONSUMPTION_BW_1600_DCDC 7500
#define SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_1600_DCDC 8200

#define SX128X_LORA_RX_CONSUMPTION_BW_200_LDO 10800
#define SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_200_LDO 12200

#define SX128X_LORA_RX_CONSUMPTION_BW_400_LDO 11800
#define SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_400_LDO 13200

#define SX128X_LORA_RX_CONSUMPTION_BW_800_LDO 13700
#define SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_800_LDO 15200

#define SX128X_LORA_RX_CONSUMPTION_BW_1600_LDO 14800
#define SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_1600_LDO 16300

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

/*!
 * @brief Convert preamble length to sx128x_lora_ranging_preamble_len_t (i.e. an integer into an exponent/mantissa pair)
 *
 * @param [in] preamble_len_in_symb Number of symbols
 *
 * @returns sx128x_lora_ranging_preamble_len_t, containing exponent and mantissa
 *
 * @warning This function rounds the preamble length up to the next valid mantissa/exponent pair, and clamps the value
 * to the maximum value 61440
 */
static sx128x_lora_ranging_preamble_len_t ral_sx128x_convert_lora_pbl_len_from_ral(
    const uint16_t preamble_len_in_symb );

/**
 * @brief Convert interrupt flags from SX128x context to RAL context
 *
 * @param [in] sx128x_irq  SX128x interrupt status
 *
 * @returns RAL interrupt status
 */
static ral_irq_t ral_sx128x_convert_irq_flags_to_ral( sx128x_irq_mask_t sx128x_irq );

/**
 * @brief Convert interrupt flags from RAL context to SX128x context
 *
 * @param [in] ral_irq RAL interrupt status
 *
 * @returns sx128x_irq SX128x interrupt status
 */
static sx128x_irq_mask_t ral_sx128x_convert_irq_flags_from_ral( ral_irq_t ral_irq );

/**
 * @brief Convert GFSK modulation parameters from RAL context to SX128x context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx128x_convert_gfsk_mod_params_from_ral( const ral_gfsk_mod_params_t* ral_mod_params,
                                                                 sx128x_mod_params_gfsk_t*    radio_mod_params );

/**
 * @brief Convert GFSK packet parameters from RAL context to SX128x context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx128x_convert_gfsk_pkt_params_from_ral( const ral_gfsk_pkt_params_t* ral_pkt_params,
                                                                 sx128x_pkt_params_gfsk_t*    radio_pkt_params );

/**
 * @brief Convert LoRa modulation parameters from RAL context to SX128x context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx128x_convert_lora_mod_params_from_ral( const ral_lora_mod_params_t* ral_mod_params,
                                                                 sx128x_mod_params_lora_t*    radio_mod_params );

/**
 * @brief Convert LoRa packet parameters from RAL context to SX128x context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx128x_convert_lora_pkt_params_from_ral( const ral_lora_pkt_params_t* ral_pkt_params,
                                                                 sx128x_pkt_params_lora_t*    radio_pkt_params );

/**
 * @brief Convert LoRa coding rate value from RAL context to SX128x context
 *
 * @param [in] ral_bw     RAL LoRa bandwidth value
 * @param [out] radio_bw  Radio LoRa bandwidth value
 *
 * @returns Operation status
 */
static ral_status_t ral_sx128x_convert_lora_bw_from_radio( const ral_lora_bw_t ral_bw, sx128x_lora_bw_t* radio_bw );

/**
 * @brief Convert LoRa coding rate value from SX128x context to RAL context
 *
 * @param [in] radio_cr  Radio LoRa coding rate value
 * @param [out] ral_cr   RAL LoRa coding rate value
 *
 * @returns Operation status
 */
static ral_status_t ral_sx128x_convert_lora_cr_to_ral( const sx128x_lora_ranging_cr_t radio_cr, ral_lora_cr_t* ral_cr );

/**
 * @brief Convert FLRC modulation parameters from RAL context to SX128x context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx128x_convert_flrc_mod_params_from_ral( const ral_flrc_mod_params_t* ral_mod_params,
                                                                 sx128x_mod_params_flrc_t*    radio_mod_params );

/**
 * @brief Convert FLRC packet parameters from RAL context to SX128x context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx128x_convert_flrc_pkt_params_from_ral( const ral_flrc_pkt_params_t* ral_pkt_params,
                                                                 sx128x_pkt_params_flrc_t*    radio_pkt_params );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

bool ral_sx128x_handles_part( const char* part_number )
{
    return ( strcmp( "sx1280", part_number ) == 0 ) || ( strcmp( "sx1281", part_number ) == 0 );
}

ral_status_t ral_sx128x_reset( const void* context )
{
    return ( ral_status_t ) sx128x_reset( context );
}

ral_status_t ral_sx128x_init( const void* context )
{
    sx128x_reg_mod_t reg_mode;

    ral_sx128x_bsp_get_reg_mode( context, &reg_mode );

    return ( ral_status_t ) sx128x_set_reg_mode( context, reg_mode );
}

ral_status_t ral_sx128x_wakeup( const void* context )
{
    return ( ral_status_t ) sx128x_wakeup( context );
}

ral_status_t ral_sx128x_set_sleep( const void* context, const bool retain_config )
{
    if( retain_config == true )
    {
        ral_status_t status = ( ral_status_t ) sx128x_save_context( context );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
    }

    return ( ral_status_t ) sx128x_set_sleep( context, false, retain_config );
}

ral_status_t ral_sx128x_set_standby( const void* context, ral_standby_cfg_t standby_cfg )
{
    sx128x_standby_cfg_t radio_standby_cfg;

    switch( standby_cfg )
    {
    case RAL_STANDBY_CFG_RC:
    {
        radio_standby_cfg = SX128X_STANDBY_CFG_RC;
        break;
    }
    case RAL_STANDBY_CFG_XOSC:
    {
        radio_standby_cfg = SX128X_STANDBY_CFG_XOSC;
        break;
    }
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    return ( ral_status_t ) sx128x_set_standby( context, radio_standby_cfg );
}

ral_status_t ral_sx128x_set_fs( const void* context )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_set_tx( const void* context )
{
    return ( ral_status_t ) sx128x_set_tx( context, SX128X_TICK_SIZE_1000_US, 0 );
}

ral_status_t ral_sx128x_set_rx( const void* context, const uint32_t timeout_in_ms )
{
    if( timeout_in_ms == RAL_RX_TIMEOUT_CONTINUOUS_MODE )
    {
        return ( ral_status_t ) sx128x_set_rx( context, SX128X_TICK_SIZE_1000_US, 0xFFFF );
    }
    else if( timeout_in_ms == 0 )
    {
        return ( ral_status_t ) sx128x_set_rx( context, SX128X_TICK_SIZE_1000_US, 0 );
    }
    else
    {
        if( timeout_in_ms <= UINT16_MAX )
        {
            return ( ral_status_t ) sx128x_set_rx( context, SX128X_TICK_SIZE_1000_US, timeout_in_ms );
        }
        else
        {
            return ( ral_status_t ) sx128x_set_rx( context, SX128X_TICK_SIZE_4000_US, timeout_in_ms >> 2 );
        }
    }
}

ral_status_t ral_sx128x_cfg_rx_boosted( const void* context, const bool enable_boost_mode )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_set_rx_tx_fallback_mode( const void* context, const ral_fallback_modes_t ral_fallback_mode )
{
    bool fallback_mode_is_fs;

    switch( ral_fallback_mode )
    {
    case RAL_FALLBACK_STDBY_RC:
    {
        fallback_mode_is_fs = false;
        break;
    }
    case RAL_FALLBACK_STDBY_XOSC:
    {
        return RAL_STATUS_UNSUPPORTED_FEATURE;
    }
    case RAL_FALLBACK_FS:
    {
        fallback_mode_is_fs = true;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return ( ral_status_t ) sx128x_set_auto_fs( context, fallback_mode_is_fs );
}

ral_status_t ral_sx128x_stop_timer_on_preamble( const void* context, const bool enable )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_set_rx_duty_cycle( const void* context, const uint32_t rx_time_in_ms,
                                           const uint32_t sleep_time_in_ms )
{
    if( ( rx_time_in_ms <= UINT16_MAX ) && ( sleep_time_in_ms <= UINT16_MAX ) )
    {
        return ( ral_status_t ) sx128x_set_rx_duty_cycle( context, SX128X_TICK_SIZE_1000_US, rx_time_in_ms,
                                                          sleep_time_in_ms );
    }
    else
    {
        return ( ral_status_t ) sx128x_set_rx_duty_cycle( context, SX128X_TICK_SIZE_4000_US, rx_time_in_ms >> 2,
                                                          sleep_time_in_ms >> 2 );
    }
}

ral_status_t ral_sx128x_set_lora_cad( const void* context )
{
    return ( ral_status_t ) sx128x_set_cad( context );
}

ral_status_t ral_sx128x_set_tx_cw( const void* context )
{
    return ( ral_status_t ) sx128x_set_tx_cw( context );
}

ral_status_t ral_sx128x_set_tx_infinite_preamble( const void* context )
{
    return ( ral_status_t ) sx128x_set_tx_infinite_preamble( context );
}

ral_status_t ral_sx128x_cal_img( const void* context, const uint16_t freq1_in_mhz, const uint16_t freq2_in_mhz )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_set_tx_cfg( const void* context, const int8_t output_pwr_in_dbm, const uint32_t rf_freq_in_hz )
{
    ral_sx128x_bsp_tx_cfg_output_params_t      tx_cfg_output_params;
    const ral_sx128x_bsp_tx_cfg_input_params_t tx_cfg_input_params = {
        .freq_in_hz               = rf_freq_in_hz,
        .system_output_pwr_in_dbm = output_pwr_in_dbm,
    };

    ral_sx128x_bsp_get_tx_cfg( context, &tx_cfg_input_params, &tx_cfg_output_params );

    return ( ral_status_t ) sx128x_set_tx_params( context, tx_cfg_output_params.chip_output_pwr_in_dbm_configured,
                                                  tx_cfg_output_params.pa_ramp_time );
}

ral_status_t ral_sx128x_set_pkt_payload( const void* context, const uint8_t* buffer, const uint16_t size )
{
    ral_status_t status = RAL_STATUS_ERROR;

    status = ( ral_status_t ) sx128x_set_buffer_base_address( context, 0x00, 0x00 );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    status = ( ral_status_t ) sx128x_write_buffer( context, 0x00, buffer, size );

    return status;
}

ral_status_t ral_sx128x_get_pkt_payload( const void* context, uint16_t max_size_in_bytes, uint8_t* buffer,
                                         uint16_t* size_in_bytes )
{
    uint8_t pkt_len;

    sx128x_rx_buffer_status_t sx_buf_status;
    ral_status_t              status = ( ral_status_t ) sx128x_get_rx_buffer_status( context, &sx_buf_status );

    if( status == RAL_STATUS_OK )
    {
        sx128x_pkt_type_t           pkt_type     = SX128X_PKT_TYPE_GFSK;
        sx128x_lora_pkt_len_modes_t pkt_len_mode = SX128X_LORA_RANGING_PKT_EXPLICIT;

        status = ( ral_status_t ) sx128x_get_pkt_type( context, &pkt_type );

        if( ( status == RAL_STATUS_OK ) && ( pkt_type == SX128X_PKT_TYPE_LORA ) )
        {
            status = ( ral_status_t ) sx128x_get_lora_pkt_len_mode( context, &pkt_len_mode );
        }

        if( status == RAL_STATUS_OK )
        {
            if( ( pkt_type == SX128X_PKT_TYPE_LORA ) && ( pkt_len_mode == SX128X_LORA_RANGING_PKT_IMPLICIT ) )
            {
                status = ( ral_status_t ) sx128x_get_lora_pkt_len( context, &pkt_len );
            }
            else
            {
                pkt_len = sx_buf_status.pld_len_in_bytes;
            }

            if( size_in_bytes != 0 )
            {
                *size_in_bytes = pkt_len;
            }

            if( status == RAL_STATUS_OK )
            {
                if( pkt_len <= max_size_in_bytes )
                {
                    status = ( ral_status_t ) sx128x_read_buffer( context, sx_buf_status.buffer_start_pointer, buffer,
                                                                  pkt_len );
                }
                else
                {
                    status = RAL_STATUS_ERROR;
                }
            }
        }
    }
    return status;
}

ral_status_t ral_sx128x_get_irq_status( const void* context, ral_irq_t* irq )
{
    ral_status_t      status          = RAL_STATUS_ERROR;
    sx128x_irq_mask_t sx128x_irq_mask = SX128X_IRQ_NONE;

    status = ( ral_status_t ) sx128x_get_irq_status( context, &sx128x_irq_mask );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    *irq = ral_sx128x_convert_irq_flags_to_ral( sx128x_irq_mask );

    return status;
}

ral_status_t ral_sx128x_clear_irq_status( const void* context, const ral_irq_t irq )
{
    const sx128x_irq_mask_t sx128x_irq_mask = ral_sx128x_convert_irq_flags_from_ral( irq );

    return ( ral_status_t ) sx128x_clear_irq_status( context, sx128x_irq_mask );
}

ral_status_t ral_sx128x_get_and_clear_irq_status( const void* context, ral_irq_t* irq )
{
    ral_status_t      status          = RAL_STATUS_ERROR;
    sx128x_irq_mask_t sx128x_irq_mask = SX128X_IRQ_NONE;

    status = ( ral_status_t ) sx128x_get_and_clear_irq_status( context, &sx128x_irq_mask );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    if( irq != 0 )
    {
        *irq = ral_sx128x_convert_irq_flags_to_ral( sx128x_irq_mask );
    }

    return status;
}

ral_status_t ral_sx128x_set_dio_irq_params( const void* context, const ral_irq_t irq )
{
    const uint16_t sx128x_irq = ral_sx128x_convert_irq_flags_from_ral( irq );

    return ( ral_status_t ) sx128x_set_dio_irq_params( context, SX128X_IRQ_ALL, sx128x_irq, SX128X_IRQ_NONE,
                                                       SX128X_IRQ_NONE );
}

ral_status_t ral_sx128x_set_rf_freq( const void* context, const uint32_t freq_in_hz )
{
    return ( ral_status_t ) sx128x_set_rf_freq( context, freq_in_hz );
}

ral_status_t ral_sx128x_set_pkt_type( const void* context, const ral_pkt_type_t pkt_type )
{
    sx128x_pkt_type_t radio_pkt_type;

    switch( pkt_type )
    {
    case RAL_PKT_TYPE_GFSK:
    {
        radio_pkt_type = SX128X_PKT_TYPE_GFSK;
        break;
    }
    case RAL_PKT_TYPE_LORA:
    {
        radio_pkt_type = SX128X_PKT_TYPE_LORA;
        break;
    }
    case RAL_PKT_TYPE_FLRC:
    {
        radio_pkt_type = SX128X_PKT_TYPE_FLRC;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return ( ral_status_t ) sx128x_set_pkt_type( context, radio_pkt_type );
}

ral_status_t ral_sx128x_get_pkt_type( const void* context, ral_pkt_type_t* pkt_type )
{
    ral_status_t      status = RAL_STATUS_ERROR;
    sx128x_pkt_type_t radio_pkt_type;

    status = ( ral_status_t ) sx128x_get_pkt_type( context, &radio_pkt_type );
    if( status == RAL_STATUS_OK )
    {
        switch( radio_pkt_type )
        {
        case SX128X_PKT_TYPE_GFSK:
        {
            *pkt_type = RAL_PKT_TYPE_GFSK;
            break;
        }
        case SX128X_PKT_TYPE_LORA:
        {
            *pkt_type = RAL_PKT_TYPE_LORA;
            break;
        }
        case SX128X_PKT_TYPE_FLRC:
        {
            *pkt_type = RAL_PKT_TYPE_FLRC;
            break;
        }
        default:
        {
            return RAL_STATUS_UNKNOWN_VALUE;
        }
        }
    }

    return status;
}

ral_status_t ral_sx128x_set_gfsk_mod_params( const void* context, const ral_gfsk_mod_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx128x_mod_params_gfsk_t radio_mod_params = { 0 };

    status = ral_sx128x_convert_gfsk_mod_params_from_ral( params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx128x_set_gfsk_mod_params( context, &radio_mod_params );
}

ral_status_t ral_sx128x_set_gfsk_pkt_params( const void* context, const ral_gfsk_pkt_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx128x_pkt_params_gfsk_t radio_pkt_params = { 0 };

    status = ral_sx128x_convert_gfsk_pkt_params_from_ral( params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx128x_set_gfsk_pkt_params( context, &radio_pkt_params );
}

ral_status_t ral_sx128x_set_lora_mod_params( const void* context, const ral_lora_mod_params_t* params )
{
    ral_status_t             status = RAL_STATUS_ERROR;
    sx128x_mod_params_lora_t radio_mod_params;

    status = ral_sx128x_convert_lora_mod_params_from_ral( params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx128x_set_lora_mod_params( context, &radio_mod_params );
}

ral_status_t ral_sx128x_set_lora_pkt_params( const void* context, const ral_lora_pkt_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx128x_pkt_params_lora_t radio_pkt_params = { 0 };

    status = ral_sx128x_convert_lora_pkt_params_from_ral( params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx128x_set_lora_pkt_params( context, &radio_pkt_params );
}

ral_status_t ral_sx128x_set_lora_cad_params( const void* context, const ral_lora_cad_params_t* params )
{
    // TODO: Implement this
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_set_lora_symb_nb_timeout( const void* context, const uint8_t nb_of_symbs )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_set_flrc_mod_params( const void* context, const ral_flrc_mod_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx128x_mod_params_flrc_t radio_mod_params = { 0 };

    status = ral_sx128x_convert_flrc_mod_params_from_ral( params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx128x_set_flrc_mod_params( context, &radio_mod_params );
}

ral_status_t ral_sx128x_set_flrc_pkt_params( const void* context, const ral_flrc_pkt_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx128x_pkt_params_flrc_t radio_pkt_params = { 0 };

    status = ral_sx128x_convert_flrc_pkt_params_from_ral( params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx128x_set_flrc_pkt_params( context, &radio_pkt_params );
}

ral_status_t ral_sx128x_get_gfsk_rx_pkt_status( const void* context, ral_gfsk_rx_pkt_status_t* rx_pkt_status )
{
    ral_status_t             status        = RAL_STATUS_ERROR;
    sx128x_pkt_status_gfsk_t sx_pkt_status = { 0 };

    status = ( ral_status_t ) sx128x_get_gfsk_pkt_status( context, &sx_pkt_status );

    if( status == RAL_STATUS_OK )
    {
        uint8_t rx_status = 0;
        rx_status |= ( ( sx_pkt_status.status & SX128X_PKT_STATUS_PKT_SENT ) == SX128X_PKT_STATUS_PKT_SENT )
                         ? RAL_RX_STATUS_PKT_SENT
                         : 0x00;
        rx_status |= ( ( sx_pkt_status.errors & SX128X_PKT_STATUS_ERROR_PKT_RX ) == SX128X_PKT_STATUS_ERROR_PKT_RX )
                         ? RAL_RX_STATUS_PKT_RECEIVED
                         : 0x00;
        rx_status |= ( ( sx_pkt_status.errors & SX128X_PKT_STATUS_ERROR_TX_RX_ABORTED ) ==
                       SX128X_PKT_STATUS_ERROR_TX_RX_ABORTED )
                         ? RAL_RX_STATUS_ABORT_ERROR
                         : 0x00;
        rx_status |= ( ( sx_pkt_status.errors & SX128X_PKT_STATUS_ERROR_PKT_LEN ) == SX128X_PKT_STATUS_ERROR_PKT_LEN )
                         ? RAL_RX_STATUS_LENGTH_ERROR
                         : 0x00;
        rx_status |= ( ( sx_pkt_status.errors & SX128X_PKT_STATUS_ERROR_PKT_CRC ) == SX128X_PKT_STATUS_ERROR_PKT_CRC )
                         ? RAL_RX_STATUS_CRC_ERROR
                         : 0x00;
        rx_pkt_status->rx_status       = rx_status;
        rx_pkt_status->rssi_avg_in_dbm = sx_pkt_status.rssi;
    }

    return status;
}

ral_status_t ral_sx128x_get_lora_rx_pkt_status( const void* context, ral_lora_rx_pkt_status_t* rx_pkt_status )
{
    ral_status_t             status        = RAL_STATUS_ERROR;
    sx128x_pkt_status_lora_t sx_pkt_status = { 0 };

    status = ( ral_status_t ) sx128x_get_lora_pkt_status( context, &sx_pkt_status );

    if( status == RAL_STATUS_OK )
    {
        rx_pkt_status->rssi_pkt_in_dbm        = sx_pkt_status.rssi;
        rx_pkt_status->snr_pkt_in_db          = sx_pkt_status.snr;
        rx_pkt_status->signal_rssi_pkt_in_dbm = sx_pkt_status.rssi;
    }
    return status;
}

ral_status_t ral_sx128x_get_flrc_rx_pkt_status( const void* context, ral_flrc_rx_pkt_status_t* rx_pkt_status )
{
    ral_status_t             status;
    sx128x_pkt_status_flrc_t sx_pkt_status;

    status = ( ral_status_t ) sx128x_get_flrc_pkt_status( context, &sx_pkt_status );

    if( status == RAL_STATUS_OK )
    {
        rx_pkt_status->rssi_sync_in_dbm = sx_pkt_status.rssi;
    }
    return status;
}

ral_status_t ral_sx128x_get_rssi_inst( const void* context, int16_t* rssi_in_dbm )
{
    return ( ral_status_t ) sx128x_get_rssi_inst( context, rssi_in_dbm );
}

uint32_t ral_sx128x_get_lora_time_on_air_in_ms( const ral_lora_pkt_params_t* pkt_p, const ral_lora_mod_params_t* mod_p )
{
    sx128x_mod_params_lora_t radio_mod_params;
    sx128x_pkt_params_lora_t radio_pkt_params;

    ral_sx128x_convert_lora_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_sx128x_convert_lora_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return sx128x_get_lora_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

uint32_t ral_sx128x_get_gfsk_time_on_air_in_ms( const ral_gfsk_pkt_params_t* pkt_p, const ral_gfsk_mod_params_t* mod_p )
{
    sx128x_mod_params_gfsk_t radio_mod_params;
    sx128x_pkt_params_gfsk_t radio_pkt_params;

    ral_sx128x_convert_gfsk_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_sx128x_convert_gfsk_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return sx128x_get_gfsk_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

uint32_t ral_sx128x_get_flrc_time_on_air_in_ms( const ral_flrc_pkt_params_t* pkt_p, const ral_flrc_mod_params_t* mod_p )
{
    sx128x_mod_params_flrc_t radio_mod_params;
    sx128x_pkt_params_flrc_t radio_pkt_params;

    ral_sx128x_convert_flrc_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_sx128x_convert_flrc_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return sx128x_get_flrc_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

ral_status_t ral_sx128x_set_gfsk_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    return ( ral_status_t ) sx128x_set_gfsk_sync_word( context, 1, sync_word, sync_word_len );
}

ral_status_t ral_sx128x_set_lora_sync_word( const void* context, const uint8_t sync_word )
{
    return ( ral_status_t ) sx128x_set_lora_sync_word( context, sync_word );
}

ral_status_t ral_sx128x_set_flrc_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    if( sync_word_len != 4 )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    return ( ral_status_t ) sx128x_set_flrc_sync_word( context, 1, sync_word );
}

ral_status_t ral_sx128x_set_gfsk_crc_params( const void* context, const uint16_t seed, const uint16_t polynomial )
{
    ral_status_t status = RAL_STATUS_ERROR;

    status = ( ral_status_t ) sx128x_set_gfsk_crc_seed( context, seed );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    status = ( ral_status_t ) sx128x_set_gfsk_crc_polynomial( context, polynomial );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return status;
}

ral_status_t ral_sx128x_set_flrc_crc_params( const void* context, const uint32_t seed )
{
    return ( ral_status_t ) sx128x_set_flrc_crc_seed( context, seed );
}

ral_status_t ral_sx128x_set_gfsk_whitening_seed( const void* context, const uint16_t seed )
{
    if( seed > UINT8_MAX )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    else
    {
        return ( ral_status_t ) sx128x_set_gfsk_ble_whitening_seed( context, ( uint8_t ) seed );
    }
}

ral_status_t ral_sx128x_lr_fhss_init( const void* context, const ral_lr_fhss_params_t* lr_fhss_params )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_lr_fhss_build_frame( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                             ral_lr_fhss_memory_state_t state, uint16_t hop_sequence_id,
                                             const uint8_t* payload, uint16_t payload_length )
{
    ( void ) context;          // Unused parameter
    ( void ) state;            // Unused parameter
    ( void ) lr_fhss_params;   // Unused parameter
    ( void ) hop_sequence_id;  // Unused parameter
    ( void ) payload;          // Unused parameter
    ( void ) payload_length;   // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_lr_fhss_handle_hop( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                            ral_lr_fhss_memory_state_t state )
{
    ( void ) context;         // Unused parameter
    ( void ) state;           // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_lr_fhss_handle_tx_done( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                                ral_lr_fhss_memory_state_t state )
{
    ( void ) context;         // Unused parameter
    ( void ) state;           // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_lr_fhss_get_time_on_air_in_ms( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                                       uint16_t payload_length, uint32_t* time_on_air )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    ( void ) payload_length;  // Unused parameter
    ( void ) time_on_air;     // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_lr_fhss_get_hop_sequence_count( const void*                 context,
                                                        const ral_lr_fhss_params_t* lr_fhss_params )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_get_lora_rx_pkt_cr_crc( const void* context, ral_lora_cr_t* cr, bool* is_crc_present )
{
    ral_status_t             status = RAL_STATUS_ERROR;
    sx128x_lora_ranging_cr_t radio_cr;

    status = ( ral_status_t ) sx128x_get_lora_rx_pkt_cr( context, &radio_cr );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    status = ral_sx128x_convert_lora_cr_to_ral( radio_cr, cr );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    status = ( ral_status_t ) sx128x_get_lora_rx_pkt_crc_present( context, is_crc_present );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return status;
}

ral_status_t ral_sx128x_get_tx_consumption_in_ua( const void* context, const int8_t output_pwr_in_dbm,
                                                  const uint32_t rf_freq_in_hz, uint32_t* pwr_consumption_in_ua )
{
    sx128x_reg_mod_t                           reg_mode;
    uint8_t                                    index;
    ral_sx128x_bsp_tx_cfg_output_params_t      tx_cfg_output_params;
    const ral_sx128x_bsp_tx_cfg_input_params_t tx_cfg_input_params = {
        .freq_in_hz               = rf_freq_in_hz,
        .system_output_pwr_in_dbm = output_pwr_in_dbm,
    };

    ral_sx128x_bsp_get_reg_mode( context, &reg_mode );
    ral_sx128x_bsp_get_tx_cfg( context, &tx_cfg_input_params, &tx_cfg_output_params );

    if( tx_cfg_output_params.chip_output_pwr_in_dbm_expected > SX128X_PWR_MAX )
    {
        index = SX128X_PWR_MAX + SX128X_CONVERT_TABLE_INDEX_OFFSET;
    }
    else if( tx_cfg_output_params.chip_output_pwr_in_dbm_expected < SX128X_PWR_MIN )
    {
        index = SX128X_PWR_MIN + SX128X_CONVERT_TABLE_INDEX_OFFSET;
    }
    else
    {
        index = tx_cfg_output_params.chip_output_pwr_in_dbm_expected + SX128X_CONVERT_TABLE_INDEX_OFFSET;
    }

    if( reg_mode == SX128X_REG_MODE_DCDC )
    {
        *pwr_consumption_in_ua = ral_sx128x_convert_tx_dbm_to_ua_reg_mode_dcdc[index];
    }
    else if( reg_mode == SX128X_REG_MODE_LDO )
    {
        *pwr_consumption_in_ua = ral_sx128x_convert_tx_dbm_to_ua_reg_mode_ldo[index];
    }
    else
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    return RAL_STATUS_OK;
}

ral_status_t ral_sx128x_get_gfsk_rx_consumption_in_ua( const void* context, const uint32_t br_in_bps,
                                                       const uint32_t bw_dsb_in_hz, const bool rx_boosted,
                                                       uint32_t* pwr_consumption_in_ua )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx128x_get_lora_rx_consumption_in_ua( const void* context, const ral_lora_bw_t bw,
                                                       const bool rx_boosted, uint32_t* pwr_consumption_in_ua )
{
    sx128x_reg_mod_t reg_mode;

    ral_sx128x_bsp_get_reg_mode( context, &reg_mode );

    switch( reg_mode )
    {
    case SX128X_REG_MODE_DCDC:
    {
        switch( bw )
        {
        case RAL_LORA_BW_200_KHZ:
        {
            *pwr_consumption_in_ua = ( rx_boosted ) ? SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_200_DCDC
                                                    : SX128X_LORA_RX_CONSUMPTION_BW_200_DCDC;
            return RAL_STATUS_OK;
        }
        case RAL_LORA_BW_400_KHZ:
        {
            *pwr_consumption_in_ua = ( rx_boosted ) ? SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_400_DCDC
                                                    : SX128X_LORA_RX_CONSUMPTION_BW_400_DCDC;
            return RAL_STATUS_OK;
        }
        case RAL_LORA_BW_800_KHZ:
        {
            *pwr_consumption_in_ua = ( rx_boosted ) ? SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_800_DCDC
                                                    : SX128X_LORA_RX_CONSUMPTION_BW_800_DCDC;
            return RAL_STATUS_OK;
        }
        case RAL_LORA_BW_1600_KHZ:
        {
            *pwr_consumption_in_ua = ( rx_boosted ) ? SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_1600_DCDC
                                                    : SX128X_LORA_RX_CONSUMPTION_BW_1600_DCDC;
            return RAL_STATUS_OK;
        }
        default:
            return RAL_STATUS_UNKNOWN_VALUE;
        }
        break;
    }
    case SX128X_REG_MODE_LDO:
    {
        switch( bw )
        {
        case RAL_LORA_BW_200_KHZ:
        {
            *pwr_consumption_in_ua =
                ( rx_boosted ) ? SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_200_LDO : SX128X_LORA_RX_CONSUMPTION_BW_200_LDO;
            return RAL_STATUS_OK;
        }
        case RAL_LORA_BW_400_KHZ:
        {
            *pwr_consumption_in_ua =
                ( rx_boosted ) ? SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_400_LDO : SX128X_LORA_RX_CONSUMPTION_BW_400_LDO;
            return RAL_STATUS_OK;
        }
        case RAL_LORA_BW_800_KHZ:
        {
            *pwr_consumption_in_ua =
                ( rx_boosted ) ? SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_800_LDO : SX128X_LORA_RX_CONSUMPTION_BW_800_LDO;
            return RAL_STATUS_OK;
        }
        case RAL_LORA_BW_1600_KHZ:
        {
            *pwr_consumption_in_ua = ( rx_boosted ) ? SX128X_LORA_RX_BOOSTED_CONSUMPTION_BW_1600_LDO
                                                    : SX128X_LORA_RX_CONSUMPTION_BW_1600_LDO;
            return RAL_STATUS_OK;
        }
        default:
        {
            return RAL_STATUS_UNKNOWN_VALUE;
        }
        }
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }
}

ral_status_t ral_sx128x_get_random_numbers( const void* context, uint32_t* numbers, unsigned int n )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static uint8_t shift_and_count_trailing_zeros( uint16_t* x )
{
    uint8_t exponent = 0;

    if( ( *x & 0x00FF ) == 0 )
    {
        exponent += 8;
        *x >>= 8;
    }
    if( ( *x & 0x000F ) == 0 )
    {
        exponent += 4;
        *x >>= 4;
    }
    if( ( *x & 0x0003 ) == 0 )
    {
        exponent += 2;
        *x >>= 2;
    }
    if( ( *x & 0x0001 ) == 0 )
    {
        exponent += 1;
        *x >>= 1;
    }

    return exponent;
}

static sx128x_lora_ranging_preamble_len_t ral_sx128x_convert_lora_pbl_len_from_ral(
    const uint16_t preamble_len_in_symb )
{
    if( preamble_len_in_symb >= 61440 )
    {
        // 61440 is the maximum preamble size of the SX128X
        return ( sx128x_lora_ranging_preamble_len_t ){
            .mant = 15,
            .exp  = 12,
        };
    }

    uint16_t mant = preamble_len_in_symb;
    int      exp  = shift_and_count_trailing_zeros( &mant );

    // This will loop at most 5 times
    while( mant > 15 )
    {
        // Divide by two, rounding up
        mant = ( mant + 1 ) >> 1;
        exp++;
    }

    return ( sx128x_lora_ranging_preamble_len_t ){
        .mant = mant,
        .exp  = exp,
    };
}

static ral_status_t ral_sx128x_convert_lora_bw_from_radio( const ral_lora_bw_t bw, sx128x_lora_bw_t* sx128x_bw )
{
    switch( bw )
    {
    case RAL_LORA_BW_200_KHZ:
        *sx128x_bw = SX128X_LORA_RANGING_BW_200;
        break;
    case RAL_LORA_BW_400_KHZ:
        *sx128x_bw = SX128X_LORA_RANGING_BW_400;
        break;
    case RAL_LORA_BW_800_KHZ:
        *sx128x_bw = SX128X_LORA_RANGING_BW_800;
        break;
    case RAL_LORA_BW_1600_KHZ:
        *sx128x_bw = SX128X_LORA_RANGING_BW_1600;
        break;
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    return RAL_STATUS_OK;
}

static ral_status_t ral_sx128x_convert_lora_cr_to_ral( const sx128x_lora_ranging_cr_t radio_cr, ral_lora_cr_t* ral_cr )
{
    switch( radio_cr )
    {
    case SX128X_LORA_RANGING_CR_4_5:
        *ral_cr = RAL_LORA_CR_4_5;
        break;
    case SX128X_LORA_RANGING_CR_4_6:
        *ral_cr = RAL_LORA_CR_4_6;
        break;
    case SX128X_LORA_RANGING_CR_4_7:
        *ral_cr = RAL_LORA_CR_4_7;
        break;
    case SX128X_LORA_RANGING_CR_4_8:
        *ral_cr = RAL_LORA_CR_4_8;
        break;
    case SX128X_LORA_RANGING_CR_LI_4_5:
        *ral_cr = RAL_LORA_CR_LI_4_5;
        break;
    case SX128X_LORA_RANGING_CR_LI_4_6:
        *ral_cr = RAL_LORA_CR_LI_4_6;
        break;
    case SX128X_LORA_RANGING_CR_LI_4_8:
        *ral_cr = RAL_LORA_CR_LI_4_8;
        break;
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    return RAL_STATUS_OK;
}

static ral_irq_t ral_sx128x_convert_irq_flags_to_ral( sx128x_irq_mask_t sx128x_irq )
{
    ral_irq_t ral_irq = RAL_IRQ_NONE;

    if( ( sx128x_irq & SX128X_IRQ_TX_DONE ) == SX128X_IRQ_TX_DONE )
    {
        ral_irq |= RAL_IRQ_TX_DONE;
    }
    if( ( sx128x_irq & SX128X_IRQ_RX_DONE ) == SX128X_IRQ_RX_DONE )
    {
        ral_irq |= RAL_IRQ_RX_DONE;
    }
    if( ( sx128x_irq & SX128X_IRQ_TIMEOUT ) == SX128X_IRQ_TIMEOUT )
    {
        ral_irq |= RAL_IRQ_RX_TIMEOUT;
    }
    if( ( ( sx128x_irq & SX128X_IRQ_SYNC_WORD_VALID ) == SX128X_IRQ_SYNC_WORD_VALID ) ||
        ( ( sx128x_irq & SX128X_IRQ_HEADER_VALID ) == SX128X_IRQ_HEADER_VALID ) )
    {
        ral_irq |= RAL_IRQ_RX_HDR_OK;
    }
    if( ( ( sx128x_irq & SX128X_IRQ_SYNC_WORD_ERROR ) == SX128X_IRQ_SYNC_WORD_ERROR ) ||
        ( ( sx128x_irq & SX128X_IRQ_HEADER_ERROR ) == SX128X_IRQ_HEADER_ERROR ) )
    {
        ral_irq |= RAL_IRQ_RX_HDR_ERROR;
    }
    if( ( sx128x_irq & SX128X_IRQ_CRC_ERROR ) == SX128X_IRQ_CRC_ERROR )
    {
        ral_irq |= RAL_IRQ_RX_CRC_ERROR;
    }
    if( ( sx128x_irq & SX128X_IRQ_CAD_DONE ) == SX128X_IRQ_CAD_DONE )
    {
        ral_irq |= RAL_IRQ_CAD_DONE;
    }
    if( ( sx128x_irq & SX128X_IRQ_CAD_DETECTED ) == SX128X_IRQ_CAD_DETECTED )
    {
        ral_irq |= RAL_IRQ_CAD_OK;
    }
    if( ( sx128x_irq & SX128X_IRQ_PREAMBLE_DETECTED ) == SX128X_IRQ_PREAMBLE_DETECTED )
    {
        ral_irq |= RAL_IRQ_RX_PREAMBLE_DETECTED;
    }
    return ral_irq;
}

static sx128x_irq_mask_t ral_sx128x_convert_irq_flags_from_ral( ral_irq_t ral_irq )
{
    sx128x_irq_mask_t sx128x_irq_mask = SX128X_IRQ_NONE;

    if( ( ral_irq & RAL_IRQ_TX_DONE ) == RAL_IRQ_TX_DONE )
    {
        sx128x_irq_mask |= SX128X_IRQ_TX_DONE;
    }
    if( ( ral_irq & RAL_IRQ_RX_DONE ) == RAL_IRQ_RX_DONE )
    {
        sx128x_irq_mask |= SX128X_IRQ_RX_DONE;
    }
    if( ( ral_irq & RAL_IRQ_RX_TIMEOUT ) == RAL_IRQ_RX_TIMEOUT )
    {
        sx128x_irq_mask |= SX128X_IRQ_TIMEOUT;
    }
    if( ( ral_irq & RAL_IRQ_RX_HDR_OK ) == RAL_IRQ_RX_HDR_OK )
    {
        sx128x_irq_mask |= SX128X_IRQ_SYNC_WORD_VALID;
        sx128x_irq_mask |= SX128X_IRQ_HEADER_VALID;
    }
    if( ( ral_irq & RAL_IRQ_RX_HDR_ERROR ) == RAL_IRQ_RX_HDR_ERROR )
    {
        sx128x_irq_mask |= SX128X_IRQ_SYNC_WORD_ERROR;
        sx128x_irq_mask |= SX128X_IRQ_HEADER_ERROR;
    }
    if( ( ral_irq & RAL_IRQ_RX_CRC_ERROR ) == RAL_IRQ_RX_CRC_ERROR )
    {
        sx128x_irq_mask |= SX128X_IRQ_CRC_ERROR;
    }
    if( ( ral_irq & RAL_IRQ_CAD_DONE ) == RAL_IRQ_CAD_DONE )
    {
        sx128x_irq_mask |= SX128X_IRQ_CAD_DONE;
    }
    if( ( ral_irq & RAL_IRQ_CAD_OK ) == RAL_IRQ_CAD_OK )
    {
        sx128x_irq_mask |= SX128X_IRQ_CAD_DETECTED;
    }
    if( ( ral_irq & RAL_IRQ_RX_PREAMBLE_DETECTED ) == RAL_IRQ_RX_PREAMBLE_DETECTED )
    {
        sx128x_irq_mask |= SX128X_IRQ_PREAMBLE_DETECTED;
    }

    return sx128x_irq_mask;
}

static ral_status_t ral_sx128x_convert_gfsk_mod_params_from_ral( const ral_gfsk_mod_params_t* ral_mod_params,
                                                                 sx128x_mod_params_gfsk_t*    radio_mod_params )
{
    ral_status_t status = RAL_STATUS_ERROR;

    sx128x_gfsk_br_bw_t                br_bw_dsb_param;
    uint32_t                           bw_dsb_in_hz = ral_mod_params->bw_dsb_in_hz;
    sx128x_gfsk_flrc_ble_pulse_shape_t pulse_shape;

    status = ( ral_status_t ) sx128x_get_gfsk_br_bw_param( ral_mod_params->br_in_bps, bw_dsb_in_hz, &br_bw_dsb_param );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    switch( ral_mod_params->pulse_shape )
    {
    case RAL_GFSK_PULSE_SHAPE_OFF:
        pulse_shape = SX128X_GFSK_FLRC_BLE_PULSE_SHAPE_OFF;
        break;
    case RAL_GFSK_PULSE_SHAPE_BT_05:
        pulse_shape = SX128X_GFSK_FLRC_BLE_PULSE_SHAPE_BT_05;
        break;
    case RAL_GFSK_PULSE_SHAPE_BT_1:
        pulse_shape = SX128X_GFSK_FLRC_BLE_PULSE_SHAPE_BT_1;
        break;
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    sx128x_gfsk_ble_mod_ind_t mod_ind = 0;
    uint32_t                  br      = sx128x_get_gfsk_br_in_bps( br_bw_dsb_param );
    status = ( ral_status_t ) sx128x_get_gfsk_mod_ind_param( br, ral_mod_params->fdev_in_hz, &mod_ind );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    *radio_mod_params = ( sx128x_mod_params_gfsk_t ){
        .br_bw       = br_bw_dsb_param,
        .mod_ind     = mod_ind,
        .pulse_shape = pulse_shape,
    };

    return RAL_STATUS_OK;
}

static ral_status_t ral_sx128x_convert_gfsk_pkt_params_from_ral( const ral_gfsk_pkt_params_t* ral_pkt_params,
                                                                 sx128x_pkt_params_gfsk_t*    radio_pkt_params )
{
    unsigned int sync_word_len_in_bytes = ral_pkt_params->sync_word_len_in_bits / 8;

    if( ral_pkt_params->sync_word_len_in_bits % 8 != 0 )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    if( sync_word_len_in_bytes > 5 )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    radio_pkt_params->sync_word_len   = ( sx128x_gfsk_sync_word_len_t ) ( ( sync_word_len_in_bytes - 1 ) << 1 );
    radio_pkt_params->match_sync_word = SX128X_GFSK_FLRC_RX_MATCH_SYNCWORD_1;

    unsigned int preamble_len_in_nibbles = ral_pkt_params->preamble_len_in_bits / 4;
    if( ral_pkt_params->preamble_len_in_bits % 4 != 0 )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    if( preamble_len_in_nibbles > 8 || preamble_len_in_nibbles == 0 )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    radio_pkt_params->preamble_len = ( sx128x_gfsk_preamble_len_t ) ( ( preamble_len_in_nibbles - 1 ) << 4 );

    radio_pkt_params->header_type = ( ral_pkt_params->header_type == RAL_GFSK_PKT_FIX_LEN )
                                        ? SX128X_GFSK_FLRC_PKT_FIX_LEN
                                        : SX128X_GFSK_FLRC_PKT_VAR_LEN;

    radio_pkt_params->pld_len_in_bytes = ral_pkt_params->pld_len_in_bytes;

    switch( ral_pkt_params->crc_type )
    {
    case RAL_GFSK_CRC_OFF:
        radio_pkt_params->crc_type = SX128X_GFSK_CRC_OFF;
        break;
    case RAL_GFSK_CRC_1_BYTE:
        radio_pkt_params->crc_type = SX128X_GFSK_CRC_1_BYTES;
        break;
    case RAL_GFSK_CRC_2_BYTES:
        radio_pkt_params->crc_type = SX128X_GFSK_CRC_2_BYTES;
        break;
    case RAL_GFSK_CRC_3_BYTES:
        radio_pkt_params->crc_type = SX128X_GFSK_CRC_3_BYTES;
        break;
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    radio_pkt_params->dc_free = ( ral_pkt_params->dc_free == RAL_GFSK_DC_FREE_WHITENING )
                                    ? SX128X_GFSK_FLRC_BLE_DC_FREE_ON
                                    : SX128X_GFSK_FLRC_BLE_DC_FREE_OFF;

    return RAL_STATUS_OK;
}

static ral_status_t ral_sx128x_convert_lora_mod_params_from_ral( const ral_lora_mod_params_t* ral_mod_params,
                                                                 sx128x_mod_params_lora_t*    radio_mod_params )
{
    radio_mod_params->sf = ( sx128x_lora_sf_t ) ( ral_mod_params->sf << 4 );

    ral_status_t status = ral_sx128x_convert_lora_bw_from_radio( ral_mod_params->bw, &radio_mod_params->bw );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    switch( ral_mod_params->cr )
    {
    case RAL_LORA_CR_4_5:
        radio_mod_params->cr = SX128X_LORA_RANGING_CR_4_5;
        break;
    case RAL_LORA_CR_4_6:
        radio_mod_params->cr = SX128X_LORA_RANGING_CR_4_6;
        break;
    case RAL_LORA_CR_4_7:
        radio_mod_params->cr = SX128X_LORA_RANGING_CR_4_7;
        break;
    case RAL_LORA_CR_4_8:
        radio_mod_params->cr = SX128X_LORA_RANGING_CR_4_8;
        break;
    case RAL_LORA_CR_LI_4_5:
        radio_mod_params->cr = SX128X_LORA_RANGING_CR_LI_4_5;
        break;
    case RAL_LORA_CR_LI_4_6:
        radio_mod_params->cr = SX128X_LORA_RANGING_CR_LI_4_6;
        break;
    case RAL_LORA_CR_LI_4_8:
        radio_mod_params->cr = SX128X_LORA_RANGING_CR_LI_4_8;
        break;
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    return RAL_STATUS_OK;
}

static ral_status_t ral_sx128x_convert_lora_pkt_params_from_ral( const ral_lora_pkt_params_t* ral_pkt_params,
                                                                 sx128x_pkt_params_lora_t*    radio_pkt_params )
{
    radio_pkt_params->preamble_len = ral_sx128x_convert_lora_pbl_len_from_ral( ral_pkt_params->preamble_len_in_symb );

    switch( ral_pkt_params->header_type )
    {
    case( RAL_LORA_PKT_EXPLICIT ):
    {
        radio_pkt_params->header_type = SX128X_LORA_RANGING_PKT_EXPLICIT;
        break;
    }
    case( RAL_LORA_PKT_IMPLICIT ):
    {
        radio_pkt_params->header_type = SX128X_LORA_RANGING_PKT_IMPLICIT;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_pkt_params->pld_len_in_bytes = ral_pkt_params->pld_len_in_bytes;
    radio_pkt_params->crc_is_on        = ral_pkt_params->crc_is_on;
    radio_pkt_params->invert_iq_is_on  = ral_pkt_params->invert_iq_is_on;

    return RAL_STATUS_OK;
}

static ral_status_t ral_sx128x_convert_flrc_mod_params_from_ral( const ral_flrc_mod_params_t* ral_mod_params,
                                                                 sx128x_mod_params_flrc_t*    radio_mod_params )
{
    ral_status_t status = RAL_STATUS_ERROR;

    sx128x_flrc_br_bw_t                br_bw_dsb_param;
    sx128x_gfsk_flrc_ble_pulse_shape_t pulse_shape;

    status = ( ral_status_t ) sx128x_get_flrc_br_bw_param( ral_mod_params->br_in_bps, ral_mod_params->bw_dsb_in_hz,
                                                           &br_bw_dsb_param );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    switch( ral_mod_params->pulse_shape )
    {
    case RAL_FLRC_PULSE_SHAPE_OFF:
        pulse_shape = SX128X_GFSK_FLRC_BLE_PULSE_SHAPE_OFF;
        break;
    case RAL_FLRC_PULSE_SHAPE_BT_05:
        pulse_shape = SX128X_GFSK_FLRC_BLE_PULSE_SHAPE_BT_05;
        break;
    case RAL_FLRC_PULSE_SHAPE_BT_1:
        pulse_shape = SX128X_GFSK_FLRC_BLE_PULSE_SHAPE_BT_1;
        break;
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    *radio_mod_params = ( sx128x_mod_params_flrc_t ){
        .br_bw       = br_bw_dsb_param,
        .cr          = ( sx128x_flrc_cr_t ) ( ral_mod_params->cr << 1 ),
        .pulse_shape = pulse_shape,
    };

    return RAL_STATUS_OK;
}

static ral_status_t ral_sx128x_convert_flrc_pkt_params_from_ral( const ral_flrc_pkt_params_t* ral_pkt_params,
                                                                 sx128x_pkt_params_flrc_t*    radio_pkt_params )
{
    unsigned int preamble_len_in_nibbles = ral_pkt_params->preamble_len_in_bits / 4;
    if( ral_pkt_params->preamble_len_in_bits % 4 != 0 )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    if( preamble_len_in_nibbles > 8 || preamble_len_in_nibbles == 0 )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    *radio_pkt_params = ( sx128x_pkt_params_flrc_t ){
        .preamble_len = ( sx128x_gfsk_preamble_len_t ) ( ( preamble_len_in_nibbles - 1 ) << 4 ),
        .sync_word_len =
            ( ral_pkt_params->sync_word_is_on == true ) ? SX128X_FLRC_SYNC_WORD_ON : SX128X_FLRC_SYNC_WORD_OFF,
        .match_sync_word  = SX128X_GFSK_FLRC_RX_MATCH_SYNCWORD_1,
        .header_type      = ral_pkt_params->pld_is_fix ? SX128X_GFSK_FLRC_PKT_FIX_LEN : SX128X_GFSK_FLRC_PKT_VAR_LEN,
        .pld_len_in_bytes = ral_pkt_params->pld_len_in_bytes,
        .crc_type         = SX128X_FLRC_CRC_OFF,
    };

    switch( ral_pkt_params->crc_type )
    {
    case RAL_FLRC_CRC_OFF:
        radio_pkt_params->crc_type = SX128X_FLRC_CRC_OFF;
        break;
    case RAL_FLRC_CRC_2_BYTES:
        radio_pkt_params->crc_type = SX128X_FLRC_CRC_2_BYTES;
        break;
    case RAL_FLRC_CRC_3_BYTES:
        radio_pkt_params->crc_type = SX128X_FLRC_CRC_3_BYTES;
        break;
    case RAL_FLRC_CRC_4_BYTES:
        radio_pkt_params->crc_type = SX128X_FLRC_CRC_4_BYTES;
        break;
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    return RAL_STATUS_OK;
}

/* --- EOF ------------------------------------------------------------------ */
