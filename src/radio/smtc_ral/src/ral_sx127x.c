/**
 * @file      ral_sx127x.c
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
#include "sx127x.h"
#include "ral_sx127x.h"
#include "ral_sx127x_bsp.h"

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

/**
 * @brief Convert interrupt flags from SX127x context to RAL context
 *
 * @param [in] sx127x_irq  SX127x interrupt status
 *
 * @returns RAL interrupt status
 */
static ral_irq_t ral_sx127x_convert_irq_flags_to_ral( sx127x_irq_mask_t sx127x_irq );

/**
 * @brief Convert interrupt flags from RAL context to SX127x context
 *
 * @param [in] ral_irq RAL interrupt status
 *
 * @returns SX127x interrupt status
 */
static sx127x_irq_mask_t ral_sx127x_convert_irq_flags_from_ral( ral_irq_t ral_irq );

/**
 * @brief Convert GFSK modulation parameters from RAL context to SX127x context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx127x_convert_gfsk_mod_params_from_ral( const ral_gfsk_mod_params_t* ral_mod_params,
                                                                 sx127x_gfsk_mod_params_t*    radio_mod_params );

/**
 * @brief Convert GFSK packet parameters from RAL context to SX127x context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx127x_convert_gfsk_pkt_params_from_ral( const ral_gfsk_pkt_params_t* ral_pkt_params,
                                                                 sx127x_gfsk_pkt_params_t*    radio_pkt_params );

/**
 * @brief Convert LoRa modulation parameters from RAL context to SX127x context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx127x_convert_lora_mod_params_from_ral( const ral_lora_mod_params_t* ral_mod_params,
                                                                 sx127x_lora_mod_params_t*    radio_mod_params );

/**
 * @brief Convert LoRa packet parameters from RAL context to SX127x context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_sx127x_convert_lora_pkt_params_from_ral( const ral_lora_pkt_params_t* ral_pkt_params,
                                                                 sx127x_lora_pkt_params_t*    radio_pkt_params );

/**
 * @brief Convert LoRa coding rate value from SX127x context to RAL context
 *
 * @param [in] radio_cr  Radio LoRa coding rate value
 * @param [out] ral_cr   RAL LoRa coding rate value
 *
 * @returns Operation status
 */
static ral_status_t ral_sx127x_convert_lora_cr_to_ral( const sx127x_lora_cr_t radio_cr, ral_lora_cr_t* ral_cr );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

bool ral_sx127x_handles_part( const char* part_number )
{
    return ( strcmp( "sx1272", part_number ) == 0 ) || ( strcmp( "sx1273", part_number ) == 0 ) ||
           ( strcmp( "sx1276", part_number ) == 0 ) || ( strcmp( "sx1277", part_number ) == 0 ) ||
           ( strcmp( "sx1278", part_number ) == 0 ) || ( strcmp( "sx1279", part_number ) == 0 );
}

ral_status_t ral_sx127x_reset( const void* context )
{
    return ( ral_status_t ) sx127x_reset( ( sx127x_t* ) context );
}

ral_status_t ral_sx127x_init( const void* context )
{
    return ( ral_status_t ) sx127x_init( ( sx127x_t* ) context );
}

ral_status_t ral_sx127x_wakeup( const void* context )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_sleep( const void* context, const bool retain_config )
{
    // retain_config parameter is not supported by SX127x radios
    return ( ral_status_t ) sx127x_set_sleep( ( sx127x_t* ) context );
}

ral_status_t ral_sx127x_set_standby( const void* context, ral_standby_cfg_t standby_cfg )
{
    // standby_cfg parameter is not supported by SX127x radios
    return ( ral_status_t ) sx127x_set_standby( ( sx127x_t* ) context );
}

ral_status_t ral_sx127x_set_fs( const void* context )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_tx( const void* context )
{
    return ( ral_status_t ) sx127x_set_tx( ( sx127x_t* ) context );
}

ral_status_t ral_sx127x_set_rx( const void* context, const uint32_t timeout_in_ms )
{
    if( timeout_in_ms == RAL_RX_TIMEOUT_CONTINUOUS_MODE )
    {
        return ( ral_status_t ) sx127x_set_rx( ( sx127x_t* ) context, 0x00FFFFFF );
    }
    else
    {
        return ( ral_status_t ) sx127x_set_rx( ( sx127x_t* ) context, timeout_in_ms );
    }
}

ral_status_t ral_sx127x_cfg_rx_boosted( const void* context, const bool enable_boost_mode )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_rx_tx_fallback_mode( const void* context, const ral_fallback_modes_t ral_fallback_mode )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_stop_timer_on_preamble( const void* context, const bool enable )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_rx_duty_cycle( const void* context, const uint32_t rx_time_in_ms,
                                           const uint32_t sleep_time_in_ms )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_lora_cad( const void* context )
{
    return ( ral_status_t ) sx127x_set_cad( ( sx127x_t* ) context );
}

ral_status_t ral_sx127x_set_tx_cw( const void* context )
{
    return ( ral_status_t ) sx127x_set_tx_cw( ( sx127x_t* ) context );
}

ral_status_t ral_sx127x_set_tx_infinite_preamble( const void* context )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_cal_img( const void* context, const uint16_t freq1_in_mhz, const uint16_t freq2_in_mhz )
{
    return ( ral_status_t ) sx127x_cal_img( ( sx127x_t* ) context, freq1_in_mhz * 1000000 );
}

ral_status_t ral_sx127x_set_tx_cfg( const void* context, const int8_t output_pwr_in_dbm, const uint32_t rf_freq_in_hz )
{
    ral_status_t                               status               = RAL_STATUS_ERROR;
    ral_sx127x_bsp_tx_cfg_output_params_t      tx_cfg_output_params = { 0 };
    const ral_sx127x_bsp_tx_cfg_input_params_t tx_cfg_input_params  = {
        .freq_in_hz               = rf_freq_in_hz,
        .system_output_pwr_in_dbm = output_pwr_in_dbm,
    };

    ral_sx127x_bsp_get_tx_cfg( context, &tx_cfg_input_params, &tx_cfg_output_params );

    status = ( ral_status_t ) sx127x_set_pa_cfg( ( sx127x_t* ) context, &tx_cfg_output_params.pa_cfg );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    uint8_t ocp_trim_value = SX127X_REG_COMMON_OCP_OCP_TRIM_100_MA;

    ral_sx127x_bsp_get_ocp_value( context, &ocp_trim_value );

    if( ocp_trim_value != SX127X_REG_COMMON_OCP_OCP_TRIM_100_MA )
    {
        status = ( ral_status_t ) sx127x_set_ocp_value( ( sx127x_t* ) context, ocp_trim_value );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
    }

    status = ( ral_status_t ) sx127x_set_tx_params( ( sx127x_t* ) context,
                                                    tx_cfg_output_params.chip_output_pwr_in_dbm_configured,
                                                    tx_cfg_output_params.pa_ramp_time );

    return status;
}

ral_status_t ral_sx127x_set_pkt_payload( const void* context, const uint8_t* buffer, const uint16_t size )
{
    return ( ral_status_t ) sx127x_write_buffer( ( sx127x_t* ) context, 0x00, buffer, size );
}

ral_status_t ral_sx127x_get_pkt_payload( const void* context, uint16_t max_size_in_bytes, uint8_t* buffer,
                                         uint16_t* size_in_bytes )
{
    ral_status_t              status                 = RAL_STATUS_ERROR;
    sx127x_rx_buffer_status_t radio_rx_buffer_status = { 0 };

    status = ( ral_status_t ) sx127x_get_rx_buffer_status( ( sx127x_t* ) context, &radio_rx_buffer_status );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    if( size_in_bytes != 0 )
    {
        *size_in_bytes = radio_rx_buffer_status.pld_len_in_bytes;
    }

    if( radio_rx_buffer_status.pld_len_in_bytes <= max_size_in_bytes )
    {
        status =
            ( ral_status_t ) sx127x_read_buffer( ( sx127x_t* ) context, radio_rx_buffer_status.buffer_start_pointer,
                                                 buffer, radio_rx_buffer_status.pld_len_in_bytes );
    }
    else
    {
        status = RAL_STATUS_ERROR;
    }

    return status;
}

ral_status_t ral_sx127x_get_irq_status( const void* context, ral_irq_t* irq )
{
    ral_status_t      status          = RAL_STATUS_ERROR;
    sx127x_irq_mask_t sx127x_irq_mask = SX127X_IRQ_NONE;

    status = ( ral_status_t ) sx127x_get_irq_status( ( sx127x_t* ) context, &sx127x_irq_mask );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    *irq = ral_sx127x_convert_irq_flags_to_ral( sx127x_irq_mask );

    return status;
}

ral_status_t ral_sx127x_clear_irq_status( const void* context, const ral_irq_t irq )
{
    const sx127x_irq_mask_t sx127x_irq_mask = ral_sx127x_convert_irq_flags_from_ral( irq );

    return ( ral_status_t ) sx127x_clear_irq_status( ( sx127x_t* ) context, sx127x_irq_mask );
}

ral_status_t ral_sx127x_get_and_clear_irq_status( const void* context, ral_irq_t* irq )
{
    ral_status_t      status          = RAL_STATUS_ERROR;
    sx127x_irq_mask_t sx127x_irq_mask = SX127X_IRQ_NONE;

    status = ( ral_status_t ) sx127x_get_and_clear_irq_status( ( sx127x_t* ) context, &sx127x_irq_mask );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    if( irq != 0 )
    {
        *irq = ral_sx127x_convert_irq_flags_to_ral( sx127x_irq_mask );
    }

    return status;
}

ral_status_t ral_sx127x_set_dio_irq_params( const void* context, const ral_irq_t irq )
{
    const uint16_t sx127x_irq = ral_sx127x_convert_irq_flags_from_ral( irq );

    return ( ral_status_t ) sx127x_set_irq_mask( ( sx127x_t* ) context, sx127x_irq );
}

ral_status_t ral_sx127x_set_rf_freq( const void* context, const uint32_t freq_in_hz )
{
    return ( ral_status_t ) sx127x_set_rf_freq( ( sx127x_t* ) context, freq_in_hz );
}

ral_status_t ral_sx127x_set_pkt_type( const void* context, const ral_pkt_type_t pkt_type )
{
    sx127x_pkt_type_t radio_pkt_type = SX127X_PKT_TYPE_GFSK;

    switch( pkt_type )
    {
    case RAL_PKT_TYPE_GFSK:
    {
        radio_pkt_type = SX127X_PKT_TYPE_GFSK;
        break;
    }
    case RAL_PKT_TYPE_LORA:
    {
        radio_pkt_type = SX127X_PKT_TYPE_LORA;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return ( ral_status_t ) sx127x_set_pkt_type( ( sx127x_t* ) context, radio_pkt_type );
}

ral_status_t ral_sx127x_get_pkt_type( const void* context, ral_pkt_type_t* pkt_type )
{
    ral_status_t      status         = RAL_STATUS_ERROR;
    sx127x_pkt_type_t radio_pkt_type = SX127X_PKT_TYPE_GFSK;

    status = ( ral_status_t ) sx127x_get_pkt_type( ( sx127x_t* ) context, &radio_pkt_type );
    if( status == RAL_STATUS_OK )
    {
        switch( radio_pkt_type )
        {
        case SX127X_PKT_TYPE_GFSK:
        {
            *pkt_type = RAL_PKT_TYPE_GFSK;
            break;
        }
        case SX127X_PKT_TYPE_LORA:
        {
            *pkt_type = RAL_PKT_TYPE_LORA;
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

ral_status_t ral_sx127x_set_gfsk_mod_params( const void* context, const ral_gfsk_mod_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx127x_gfsk_mod_params_t radio_mod_params = { 0 };

    status = ral_sx127x_convert_gfsk_mod_params_from_ral( params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx127x_set_gfsk_mod_params( ( sx127x_t* ) context, &radio_mod_params );
}

ral_status_t ral_sx127x_set_gfsk_pkt_params( const void* context, const ral_gfsk_pkt_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx127x_gfsk_pkt_params_t radio_pkt_params = { 0 };

    status = ral_sx127x_convert_gfsk_pkt_params_from_ral( params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx127x_set_gfsk_pkt_params( ( sx127x_t* ) context, &radio_pkt_params );
}

ral_status_t ral_sx127x_set_lora_mod_params( const void* context, const ral_lora_mod_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx127x_lora_mod_params_t radio_mod_params = { 0 };

    status = ral_sx127x_convert_lora_mod_params_from_ral( params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx127x_set_lora_mod_params( ( sx127x_t* ) context, &radio_mod_params );
}

ral_status_t ral_sx127x_set_lora_pkt_params( const void* context, const ral_lora_pkt_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    sx127x_lora_pkt_params_t radio_pkt_params = { 0 };

    status = ral_sx127x_convert_lora_pkt_params_from_ral( params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) sx127x_set_lora_pkt_params( ( sx127x_t* ) context, &radio_pkt_params );
}

ral_status_t ral_sx127x_set_lora_cad_params( const void* context, const ral_lora_cad_params_t* params )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_lora_symb_nb_timeout( const void* context, const uint8_t nb_of_symbs )
{
    return ( ral_status_t ) sx127x_set_lora_sync_timeout( ( sx127x_t* ) context, nb_of_symbs );
}

ral_status_t ral_sx127x_set_flrc_mod_params( const void* context, const ral_flrc_mod_params_t* params )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}
ral_status_t ral_sx127x_set_flrc_pkt_params( const void* context, const ral_flrc_pkt_params_t* params )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_get_gfsk_rx_pkt_status( const void* context, ral_gfsk_rx_pkt_status_t* rx_pkt_status )
{
    ral_status_t             status              = RAL_STATUS_ERROR;
    sx127x_gfsk_pkt_status_t radio_rx_pkt_status = { 0 };

    status = ( ral_status_t ) sx127x_get_gfsk_pkt_status( ( sx127x_t* ) context, &radio_rx_pkt_status );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    rx_pkt_status->rx_status        = radio_rx_pkt_status.rx_status;
    rx_pkt_status->rssi_sync_in_dbm = radio_rx_pkt_status.rssi_sync;
    rx_pkt_status->rssi_avg_in_dbm  = radio_rx_pkt_status.rssi_avg;

    return status;
}

ral_status_t ral_sx127x_get_lora_rx_pkt_status( const void* context, ral_lora_rx_pkt_status_t* rx_pkt_status )
{
    ral_status_t             status              = RAL_STATUS_ERROR;
    sx127x_lora_pkt_status_t radio_rx_pkt_status = { 0 };

    status = ( ral_status_t ) sx127x_get_lora_pkt_status( ( sx127x_t* ) context, &radio_rx_pkt_status );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    rx_pkt_status->rssi_pkt_in_dbm        = radio_rx_pkt_status.rssi_pkt_in_dbm;
    rx_pkt_status->snr_pkt_in_db          = radio_rx_pkt_status.snr_pkt_in_db;
    rx_pkt_status->signal_rssi_pkt_in_dbm = radio_rx_pkt_status.signal_rssi_pkt_in_db;

    return status;
}

ral_status_t ral_sx127x_get_flrc_rx_pkt_status( const void* context, ral_flrc_rx_pkt_status_t* rx_pkt_status )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_get_rssi_inst( const void* context, int16_t* rssi_in_dbm )
{
    return ( ral_status_t ) sx127x_get_rssi_inst( ( sx127x_t* ) context, rssi_in_dbm );
}

uint32_t ral_sx127x_get_lora_time_on_air_in_ms( const ral_lora_pkt_params_t* pkt_p, const ral_lora_mod_params_t* mod_p )
{
    sx127x_lora_mod_params_t radio_mod_params = { 0 };
    sx127x_lora_pkt_params_t radio_pkt_params = { 0 };

    ral_sx127x_convert_lora_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_sx127x_convert_lora_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return sx127x_get_lora_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

uint32_t ral_sx127x_get_gfsk_time_on_air_in_ms( const ral_gfsk_pkt_params_t* pkt_p, const ral_gfsk_mod_params_t* mod_p )
{
    sx127x_gfsk_mod_params_t radio_mod_params = { 0 };
    sx127x_gfsk_pkt_params_t radio_pkt_params = { 0 };

    ral_sx127x_convert_gfsk_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_sx127x_convert_gfsk_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return sx127x_get_gfsk_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

uint32_t ral_sx127x_get_flrc_time_on_air_in_ms( const ral_flrc_pkt_params_t* pkt_p, const ral_flrc_mod_params_t* mod_p )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_gfsk_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    return ( ral_status_t ) sx127x_set_gfsk_sync_word( ( sx127x_t* ) context, sync_word, sync_word_len );
}

ral_status_t ral_sx127x_set_lora_sync_word( const void* context, const uint8_t sync_word )
{
    return ( ral_status_t ) sx127x_set_lora_sync_word( ( sx127x_t* ) context, sync_word );
}

ral_status_t ral_sx127x_set_flrc_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_gfsk_crc_params( const void* context, const uint16_t seed, const uint16_t polynomial )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_flrc_crc_params( const void* context, const uint32_t seed )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_set_gfsk_whitening_seed( const void* context, const uint16_t seed )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_lr_fhss_init( const void* context, const ral_lr_fhss_params_t* lr_fhss_params )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_lr_fhss_build_frame( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
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

ral_status_t ral_sx127x_lr_fhss_handle_hop( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                            ral_lr_fhss_memory_state_t state )
{
    ( void ) context;         // Unused parameter
    ( void ) state;           // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_lr_fhss_handle_tx_done( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                                ral_lr_fhss_memory_state_t state )
{
    ( void ) context;         // Unused parameter
    ( void ) state;           // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_lr_fhss_get_time_on_air_in_ms( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                                       uint16_t payload_length, uint32_t* time_on_air )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    ( void ) payload_length;  // Unused parameter
    ( void ) time_on_air;     // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_lr_fhss_get_hop_sequence_count( const void*                 context,
                                                        const ral_lr_fhss_params_t* lr_fhss_params )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_get_lora_rx_pkt_cr_crc( const void* context, ral_lora_cr_t* cr, bool* is_crc_present )
{
    ral_status_t     status = RAL_STATUS_ERROR;
    sx127x_lora_cr_t radio_cr;

    status = ( ral_status_t ) sx127x_get_lora_params_from_header( ( sx127x_t* ) context, &radio_cr, is_crc_present );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    status = ral_sx127x_convert_lora_cr_to_ral( radio_cr, cr );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return status;
}

ral_status_t ral_sx127x_get_tx_consumption_in_ua( const void* context, const int8_t output_pwr_in_dbm,
                                                  const uint32_t rf_freq_in_hz, uint32_t* pwr_consumption_in_ua )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_get_gfsk_rx_consumption_in_ua( const void* context, const uint32_t br_in_bps,
                                                       const uint32_t bw_dsb_in_hz, const bool rx_boosted,
                                                       uint32_t* pwr_consumption_in_ua )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_get_lora_rx_consumption_in_ua( const void* context, const ral_lora_bw_t bw,
                                                       const bool rx_boosted, uint32_t* pwr_consumption_in_ua )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_sx127x_get_random_numbers( const void* context, uint32_t* numbers, unsigned int n )
{
    return ( ral_status_t ) sx127x_get_random_numbers( ( sx127x_t* ) context, numbers, n );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static ral_irq_t ral_sx127x_convert_irq_flags_to_ral( sx127x_irq_mask_t sx127x_irq )
{
    ral_irq_t ral_irq = RAL_IRQ_NONE;

    if( ( sx127x_irq & SX127X_IRQ_TX_DONE ) == SX127X_IRQ_TX_DONE )
    {
        ral_irq |= RAL_IRQ_TX_DONE;
    }
    if( ( sx127x_irq & SX127X_IRQ_RX_DONE ) == SX127X_IRQ_RX_DONE )
    {
        ral_irq |= RAL_IRQ_RX_DONE;
    }
    if( ( sx127x_irq & SX127X_IRQ_TIMEOUT ) == SX127X_IRQ_TIMEOUT )
    {
        ral_irq |= RAL_IRQ_RX_TIMEOUT;
    }
    if( ( ( sx127x_irq & SX127X_IRQ_SYNC_WORD_VALID ) == SX127X_IRQ_SYNC_WORD_VALID ) ||
        ( ( sx127x_irq & SX127X_IRQ_HEADER_VALID ) == SX127X_IRQ_HEADER_VALID ) )
    {
        ral_irq |= RAL_IRQ_RX_HDR_OK;
    }
    if( ( sx127x_irq & SX127X_IRQ_HEADER_ERROR ) == SX127X_IRQ_HEADER_ERROR )
    {
        ral_irq |= RAL_IRQ_RX_HDR_ERROR;
    }
    if( ( sx127x_irq & SX127X_IRQ_CRC_ERROR ) == SX127X_IRQ_CRC_ERROR )
    {
        ral_irq |= RAL_IRQ_RX_CRC_ERROR;
    }
    if( ( sx127x_irq & SX127X_IRQ_CAD_DONE ) == SX127X_IRQ_CAD_DONE )
    {
        ral_irq |= RAL_IRQ_CAD_DONE;
    }
    if( ( sx127x_irq & SX127X_IRQ_CAD_DETECTED ) == SX127X_IRQ_CAD_DETECTED )
    {
        ral_irq |= RAL_IRQ_CAD_OK;
    }
    if( ( sx127x_irq & SX127X_IRQ_ALL ) == SX127X_IRQ_ALL )
    {
        ral_irq |= RAL_IRQ_ALL;
    }
    return ral_irq;
}

static sx127x_irq_mask_t ral_sx127x_convert_irq_flags_from_ral( ral_irq_t ral_irq )
{
    sx127x_irq_mask_t sx127x_irq_mask = SX127X_IRQ_NONE;

    if( ( ral_irq & RAL_IRQ_TX_DONE ) == RAL_IRQ_TX_DONE )
    {
        sx127x_irq_mask |= SX127X_IRQ_TX_DONE;
    }
    if( ( ral_irq & RAL_IRQ_RX_DONE ) == RAL_IRQ_RX_DONE )
    {
        sx127x_irq_mask |= SX127X_IRQ_RX_DONE;
    }
    if( ( ral_irq & RAL_IRQ_RX_TIMEOUT ) == RAL_IRQ_RX_TIMEOUT )
    {
        sx127x_irq_mask |= SX127X_IRQ_TIMEOUT;
    }
    if( ( ral_irq & RAL_IRQ_RX_HDR_OK ) == RAL_IRQ_RX_HDR_OK )
    {
        sx127x_irq_mask |= SX127X_IRQ_SYNC_WORD_VALID;
        sx127x_irq_mask |= SX127X_IRQ_HEADER_VALID;
    }
    if( ( ral_irq & RAL_IRQ_RX_HDR_ERROR ) == RAL_IRQ_RX_HDR_ERROR )
    {
        sx127x_irq_mask |= SX127X_IRQ_HEADER_ERROR;
    }
    if( ( ral_irq & RAL_IRQ_RX_CRC_ERROR ) == RAL_IRQ_RX_CRC_ERROR )
    {
        sx127x_irq_mask |= SX127X_IRQ_CRC_ERROR;
    }
    if( ( ral_irq & RAL_IRQ_CAD_DONE ) == RAL_IRQ_CAD_DONE )
    {
        sx127x_irq_mask |= SX127X_IRQ_CAD_DONE;
    }
    if( ( ral_irq & RAL_IRQ_CAD_OK ) == RAL_IRQ_CAD_OK )
    {
        sx127x_irq_mask |= SX127X_IRQ_CAD_DETECTED;
    }
    if( ( ral_irq & RAL_IRQ_ALL ) == RAL_IRQ_ALL )
    {
        sx127x_irq_mask |= SX127X_IRQ_ALL;
    }

    return sx127x_irq_mask;
}

static ral_status_t ral_sx127x_convert_gfsk_mod_params_from_ral( const ral_gfsk_mod_params_t* ral_mod_params,
                                                                 sx127x_gfsk_mod_params_t*    radio_mod_params )
{
    radio_mod_params->br_in_bps    = ral_mod_params->br_in_bps;
    radio_mod_params->fdev_in_hz   = ral_mod_params->fdev_in_hz;
    radio_mod_params->bw_ssb_in_hz = ral_mod_params->bw_dsb_in_hz >> 1;

    switch( ral_mod_params->pulse_shape )
    {
    case RAL_GFSK_PULSE_SHAPE_OFF:
    {
        radio_mod_params->pulse_shape.gfsk = SX127X_GFSK_PULSE_SHAPE_OFF;
        return RAL_STATUS_OK;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_03:
    {
        radio_mod_params->pulse_shape.gfsk = SX127X_GFSK_PULSE_SHAPE_BT_03;
        return RAL_STATUS_OK;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_05:
    {
        radio_mod_params->pulse_shape.gfsk = SX127X_GFSK_PULSE_SHAPE_BT_05;
        return RAL_STATUS_OK;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_1:
    {
        radio_mod_params->pulse_shape.gfsk = SX127X_GFSK_PULSE_SHAPE_BT_1;
        return RAL_STATUS_OK;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }
}

static ral_status_t ral_sx127x_convert_gfsk_pkt_params_from_ral( const ral_gfsk_pkt_params_t* ral_pkt_params,
                                                                 sx127x_gfsk_pkt_params_t*    radio_pkt_params )
{
    radio_pkt_params->preamble_len_in_bytes = ral_pkt_params->preamble_len_in_bits >> 3;

    switch( ral_pkt_params->preamble_detector )
    {
    case RAL_GFSK_PREAMBLE_DETECTOR_OFF:
    {
        radio_pkt_params->preamble_detector = SX127X_GFSK_PREAMBLE_DETECTOR_OFF;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS:
    {
        radio_pkt_params->preamble_detector = SX127X_GFSK_PREAMBLE_DETECTOR_1_BYTE;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_16BITS:
    {
        radio_pkt_params->preamble_detector = SX127X_GFSK_PREAMBLE_DETECTOR_2_BYTES;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_24BITS:
    {
        radio_pkt_params->preamble_detector = SX127X_GFSK_PREAMBLE_DETECTOR_3_BYTES;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_pkt_params->sync_word_len_in_bytes = ral_pkt_params->sync_word_len_in_bits >> 3;

    switch( ral_pkt_params->address_filtering )
    {
    case RAL_GFSK_ADDRESS_FILTERING_DISABLE:
    {
        radio_pkt_params->address_filtering = SX127X_GFSK_ADDRESS_FILTERING_DISABLE;
        break;
    }
    case RAL_GFSK_ADDRESS_FILTERING_NODE_ADDRESS:
    {
        radio_pkt_params->address_filtering = SX127X_GFSK_ADDRESS_FILTERING_NODE_ADDRESS;
        break;
    }
    case RAL_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES:
    {
        radio_pkt_params->address_filtering = SX127X_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    switch( ral_pkt_params->header_type )
    {
    case RAL_GFSK_PKT_FIX_LEN:
    {
        radio_pkt_params->header_type = SX127X_GFSK_PKT_FIX_LEN;
        break;
    }
    case RAL_GFSK_PKT_VAR_LEN:
    {
        radio_pkt_params->header_type = SX127X_GFSK_PKT_VAR_LEN;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_pkt_params->pld_len_in_bytes = ral_pkt_params->pld_len_in_bytes;

    switch( ral_pkt_params->crc_type )
    {
    case RAL_GFSK_CRC_OFF:
    {
        radio_pkt_params->crc_type = SX127X_GFSK_CRC_OFF;
        break;
    }
    case RAL_GFSK_CRC_2_BYTES_INV:
    {
        radio_pkt_params->crc_type = SX127X_GFSK_CRC_2_BYTES_INV;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    switch( ral_pkt_params->dc_free )
    {
    case RAL_GFSK_DC_FREE_OFF:
    {
        radio_pkt_params->dc_free = SX127X_GFSK_DC_FREE_OFF;
        break;
    }
    case RAL_GFSK_DC_FREE_WHITENING:
    {
        radio_pkt_params->dc_free = SX127X_GFSK_DC_FREE_WHITENING;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return RAL_STATUS_OK;
}

static ral_status_t ral_sx127x_convert_lora_mod_params_from_ral( const ral_lora_mod_params_t* ral_mod_params,
                                                                 sx127x_lora_mod_params_t*    radio_mod_params )
{
    radio_mod_params->sf = ( sx127x_lora_sf_t ) ral_mod_params->sf;

    switch( ral_mod_params->bw )
    {
    case RAL_LORA_BW_007_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_007;
        break;
    }
    case RAL_LORA_BW_010_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_010;
        break;
    }
    case RAL_LORA_BW_015_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_015;
        break;
    }
    case RAL_LORA_BW_020_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_020;
        break;
    }
    case RAL_LORA_BW_031_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_031;
        break;
    }
    case RAL_LORA_BW_041_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_041;
        break;
    }
    case RAL_LORA_BW_062_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_062;
        break;
    }
    case RAL_LORA_BW_125_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_125;
        break;
    }
    case RAL_LORA_BW_250_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_250;
        break;
    }
    case RAL_LORA_BW_500_KHZ:
    {
        radio_mod_params->bw = SX127X_LORA_BW_500;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_mod_params->cr = ( sx127x_lora_cr_t ) ral_mod_params->cr;

    radio_mod_params->ldro = ral_mod_params->ldro;

    return RAL_STATUS_OK;
}

static ral_status_t ral_sx127x_convert_lora_pkt_params_from_ral( const ral_lora_pkt_params_t* ral_pkt_params,
                                                                 sx127x_lora_pkt_params_t*    radio_pkt_params )
{
    radio_pkt_params->preamble_len_in_symb = ral_pkt_params->preamble_len_in_symb;

    switch( ral_pkt_params->header_type )
    {
    case( RAL_LORA_PKT_EXPLICIT ):
    {
        radio_pkt_params->header_type = SX127X_LORA_PKT_EXPLICIT;
        break;
    }
    case( RAL_LORA_PKT_IMPLICIT ):
    {
        radio_pkt_params->header_type = SX127X_LORA_PKT_IMPLICIT;
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

static ral_status_t ral_sx127x_convert_lora_cr_to_ral( const sx127x_lora_cr_t radio_cr, ral_lora_cr_t* ral_cr )
{
    switch( radio_cr )
    {
    case SX127X_LORA_CR_4_5:
        *ral_cr = RAL_LORA_CR_4_5;
        break;
    case SX127X_LORA_CR_4_6:
        *ral_cr = RAL_LORA_CR_4_6;
        break;
    case SX127X_LORA_CR_4_7:
        *ral_cr = RAL_LORA_CR_4_7;
        break;
    case SX127X_LORA_CR_4_8:
        *ral_cr = RAL_LORA_CR_4_8;
        break;
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    return RAL_STATUS_OK;
}

/* --- EOF ------------------------------------------------------------------ */
