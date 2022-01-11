/**
 * @file      ral_llcc68.c
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
#include "llcc68.h"
#include "ral_llcc68.h"
#include "ral_llcc68_bsp.h"

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
 * @brief Convert interrupt flags from LLCC68 context to RAL context
 *
 * @param [in] llcc68_irq  LLCC68 interrupt status
 *
 * @returns RAL interrupt status
 */
static ral_irq_t ral_llcc68_convert_irq_flags_to_ral( llcc68_irq_mask_t llcc68_irq );

/**
 * @brief Convert interrupt flags from RAL context to LLCC68 context
 *
 * @param [in] ral_irq RAL interrupt status
 *
 * @returns LLCC68 interrupt status
 */
static llcc68_irq_mask_t ral_llcc68_convert_irq_flags_from_ral( ral_irq_t ral_irq );

/**
 * @brief Convert GFSK modulation parameters from RAL context to LLCC68 context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_llcc68_convert_gfsk_mod_params_from_ral( const ral_gfsk_mod_params_t* ral_mod_params,
                                                                 llcc68_mod_params_gfsk_t*    radio_mod_params );

/**
 * @brief Convert GFSK packet parameters from RAL context to LLCC68 context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_llcc68_convert_gfsk_pkt_params_from_ral( const ral_gfsk_pkt_params_t* ral_pkt_params,
                                                                 llcc68_pkt_params_gfsk_t*    radio_pkt_params );

/**
 * @brief Convert LoRa modulation parameters from RAL context to LLCC68 context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_llcc68_convert_lora_mod_params_from_ral( const ral_lora_mod_params_t* ral_mod_params,
                                                                 llcc68_mod_params_lora_t*    radio_mod_params );

/**
 * @brief Convert LoRa packet parameters from RAL context to LLCC68 context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_llcc68_convert_lora_pkt_params_from_ral( const ral_lora_pkt_params_t* ral_pkt_params,
                                                                 llcc68_pkt_params_lora_t*    radio_pkt_params );

/**
 * @brief Convert LoRa CAD parameters from RAL context to LLCC68 context
 *
 * @param [in] ral_lora_cad_params     RAL LoRa CAD parameters
 * @param [out] radio_lora_cad_params  Radio LoRa CAD parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_llcc68_convert_lora_cad_params_from_ral( const ral_lora_cad_params_t* ral_lora_cad_params,
                                                                 llcc68_cad_params_t*         radio_lora_cad_params );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

bool ral_llcc68_handles_part( const char* part_number )
{
    return ( strcmp( "llcc68", part_number ) == 0 );
}

ral_status_t ral_llcc68_reset( const void* context )
{
    return ( ral_status_t ) llcc68_reset( context );
}

ral_status_t ral_llcc68_init( const void* context )
{
    ral_status_t                status = RAL_STATUS_ERROR;
    llcc68_tcxo_ctrl_voltages_t tcxo_supply_voltage;
    llcc68_reg_mod_t            reg_mode;
    bool                        dio2_is_set_as_rf_switch = false;
    bool                        tcxo_is_radio_controlled = false;
    uint32_t                    startup_time_in_tick     = 0;

    ral_llcc68_bsp_get_reg_mode( context, &reg_mode );
    status = ( ral_status_t ) llcc68_set_reg_mode( context, reg_mode );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    ral_llcc68_bsp_get_rf_switch_cfg( context, &dio2_is_set_as_rf_switch );
    status = ( ral_status_t ) llcc68_set_dio2_as_rf_sw_ctrl( context, dio2_is_set_as_rf_switch );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    ral_llcc68_bsp_get_xosc_cfg( context, &tcxo_is_radio_controlled, &tcxo_supply_voltage, &startup_time_in_tick );
    if( tcxo_is_radio_controlled == true )
    {
        status = ( ral_status_t ) llcc68_set_dio3_as_tcxo_ctrl( context, tcxo_supply_voltage, startup_time_in_tick );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }

        status = ( ral_status_t ) llcc68_cal( context, LLCC68_CAL_ALL );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
    }

    return status;
}

ral_status_t ral_llcc68_wakeup( const void* context )
{
    return ( ral_status_t ) llcc68_wakeup( context );
}

ral_status_t ral_llcc68_set_sleep( const void* context, const bool retain_config )
{
    const llcc68_sleep_cfgs_t radio_sleep_cfg =
        ( retain_config == true ) ? LLCC68_SLEEP_CFG_WARM_START : LLCC68_SLEEP_CFG_COLD_START;

    return ( ral_status_t ) llcc68_set_sleep( context, radio_sleep_cfg );
}

ral_status_t ral_llcc68_set_standby( const void* context, ral_standby_cfg_t standby_cfg )
{
    llcc68_standby_cfg_t radio_standby_cfg;

    switch( standby_cfg )
    {
    case RAL_STANDBY_CFG_RC:
    {
        radio_standby_cfg = LLCC68_STANDBY_CFG_RC;
        break;
    }
    case RAL_STANDBY_CFG_XOSC:
    {
        radio_standby_cfg = LLCC68_STANDBY_CFG_XOSC;
        break;
    }
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    return ( ral_status_t ) llcc68_set_standby( context, radio_standby_cfg );
}

ral_status_t ral_llcc68_set_fs( const void* context )
{
    return ( ral_status_t ) llcc68_set_fs( context );
}

ral_status_t ral_llcc68_set_tx( const void* context )
{
    return ( ral_status_t ) llcc68_set_tx( context, 0 );
}

ral_status_t ral_llcc68_set_rx( const void* context, const uint32_t timeout_in_ms )
{
    if( timeout_in_ms == RAL_RX_TIMEOUT_CONTINUOUS_MODE )
    {
        return ( ral_status_t ) llcc68_set_rx_with_timeout_in_rtc_step( context, 0x00FFFFFF );
    }
    else
    {  // max timeout is 0xFFFFFE -> 262143 ms (0xFFFFFE / 64000 * 1000) - Single reception mode set if timeout_ms is 0
        if( timeout_in_ms < 262144 )
        {
            return ( ral_status_t ) llcc68_set_rx( context, timeout_in_ms );
        }
        else
        {
            return RAL_STATUS_ERROR;
        }
    }

    return RAL_STATUS_ERROR;
}

ral_status_t ral_llcc68_cfg_rx_boosted( const void* context, const bool enable_boost_mode )
{
    return ( ral_status_t ) llcc68_cfg_rx_boosted( context, enable_boost_mode );
}

ral_status_t ral_llcc68_stop_timer_on_preamble( const void* context, const bool enable )
{
    return ( ral_status_t ) llcc68_stop_timer_on_preamble( context, enable );
}

ral_status_t ral_llcc68_set_rx_duty_cycle( const void* context, const uint32_t rx_time_in_ms,
                                           const uint32_t sleep_time_in_ms )
{
    return ( ral_status_t ) llcc68_set_rx_duty_cycle( context, rx_time_in_ms, sleep_time_in_ms );
}

ral_status_t ral_llcc68_set_lora_cad( const void* context )
{
    return ( ral_status_t ) llcc68_set_cad( context );
}

ral_status_t ral_llcc68_set_tx_cw( const void* context )
{
    return ( ral_status_t ) llcc68_set_tx_cw( context );
}

ral_status_t ral_llcc68_set_tx_infinite_preamble( const void* context )
{
    return ( ral_status_t ) llcc68_set_tx_infinite_preamble( context );
}

ral_status_t ral_llcc68_cal_img( const void* context, const uint16_t freq1_in_mhz, const uint16_t freq2_in_mhz )
{
    return ( ral_status_t ) llcc68_cal_img_in_mhz( context, freq1_in_mhz, freq2_in_mhz );
}

ral_status_t ral_llcc68_set_tx_cfg( const void* context, const int8_t output_pwr_in_dbm, const uint32_t rf_freq_in_hz )
{
    ral_status_t                               status = RAL_STATUS_ERROR;
    ral_llcc68_bsp_tx_cfg_output_params_t      tx_cfg_output_params;
    const ral_llcc68_bsp_tx_cfg_input_params_t tx_cfg_input_params = {
        .freq_in_hz               = rf_freq_in_hz,
        .system_output_pwr_in_dbm = output_pwr_in_dbm,
    };

    ral_llcc68_bsp_get_tx_cfg( context, &tx_cfg_input_params, &tx_cfg_output_params );

    if( tx_cfg_output_params.pa_cfg.device_sel == 0x00 )
    {
        status = ( ral_status_t ) llcc68_cfg_tx_clamp( context );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
    }

    status = ( ral_status_t ) llcc68_set_pa_cfg( context, &tx_cfg_output_params.pa_cfg );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    uint8_t ocp_in_step_of_2_5_ma = ( tx_cfg_output_params.pa_cfg.device_sel == 0x00 ) ? 0x38 : 0x18;

    ral_llcc68_bsp_get_ocp_value( context, &ocp_in_step_of_2_5_ma );

    if( ( ( tx_cfg_output_params.pa_cfg.device_sel == 0x00 ) && ( ocp_in_step_of_2_5_ma != 0x38 ) ) ||
        ( ( tx_cfg_output_params.pa_cfg.device_sel == 0x01 ) && ( ocp_in_step_of_2_5_ma != 0x18 ) ) )
    {
        status = ( ral_status_t ) llcc68_set_ocp_value( context, ocp_in_step_of_2_5_ma );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
    }

    status = ( ral_status_t ) llcc68_set_tx_params( context, tx_cfg_output_params.chip_output_pwr_in_dbm_configured,
                                                    tx_cfg_output_params.pa_ramp_time );

    return status;
}

ral_status_t ral_llcc68_set_pkt_payload( const void* context, const uint8_t* buffer, const uint16_t size )
{
    ral_status_t status = RAL_STATUS_ERROR;

    status = ( ral_status_t ) llcc68_set_buffer_base_address( context, 0x00, 0x00 );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    status = ( ral_status_t ) llcc68_write_buffer( context, 0x00, buffer, size );

    return status;
}

ral_status_t ral_llcc68_get_pkt_payload( const void* context, uint16_t max_size_in_bytes, uint8_t* buffer,
                                         uint16_t* size_in_bytes )
{
    ral_status_t              status = RAL_STATUS_ERROR;
    llcc68_rx_buffer_status_t radio_rx_buffer_status;

    status = ( ral_status_t ) llcc68_get_rx_buffer_status( context, &radio_rx_buffer_status );
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
        status = ( ral_status_t ) llcc68_read_buffer( context, radio_rx_buffer_status.buffer_start_pointer, buffer,
                                                      radio_rx_buffer_status.pld_len_in_bytes );
    }
    else
    {
        status = RAL_STATUS_ERROR;
    }

    return status;
}

ral_status_t ral_llcc68_get_irq_status( const void* context, ral_irq_t* irq )
{
    ral_status_t      status          = RAL_STATUS_ERROR;
    llcc68_irq_mask_t llcc68_irq_mask = LLCC68_IRQ_NONE;

    status = ( ral_status_t ) llcc68_get_irq_status( context, &llcc68_irq_mask );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    *irq = ral_llcc68_convert_irq_flags_to_ral( llcc68_irq_mask );

    return status;
}

ral_status_t ral_llcc68_clear_irq_status( const void* context, const ral_irq_t irq )
{
    const llcc68_irq_mask_t llcc68_irq_mask = ral_llcc68_convert_irq_flags_from_ral( irq );

    return ( ral_status_t ) llcc68_clear_irq_status( context, llcc68_irq_mask );
}

ral_status_t ral_llcc68_get_and_clear_irq_status( const void* context, ral_irq_t* irq )
{
    ral_status_t      status          = RAL_STATUS_ERROR;
    llcc68_irq_mask_t llcc68_irq_mask = LLCC68_IRQ_NONE;

    status = ( ral_status_t ) llcc68_get_and_clear_irq_status( context, &llcc68_irq_mask );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    if( irq != 0 )
    {
        *irq = ral_llcc68_convert_irq_flags_to_ral( llcc68_irq_mask );
    }

    return status;
}

ral_status_t ral_llcc68_set_dio_irq_params( const void* context, const ral_irq_t irq )
{
    const uint16_t llcc68_irq = ral_llcc68_convert_irq_flags_from_ral( irq );

    return ( ral_status_t ) llcc68_set_dio_irq_params( context, LLCC68_IRQ_ALL, llcc68_irq, LLCC68_IRQ_NONE,
                                                       LLCC68_IRQ_NONE );
}

ral_status_t ral_llcc68_set_rf_freq( const void* context, const uint32_t freq_in_hz )
{
    return ( ral_status_t ) llcc68_set_rf_freq( context, freq_in_hz );
}

ral_status_t ral_llcc68_set_pkt_type( const void* context, const ral_pkt_type_t pkt_type )
{
    llcc68_pkt_type_t radio_pkt_type;

    switch( pkt_type )
    {
    case RAL_PKT_TYPE_GFSK:
    {
        radio_pkt_type = LLCC68_PKT_TYPE_GFSK;
        break;
    }
    case RAL_PKT_TYPE_LORA:
    {
        radio_pkt_type = LLCC68_PKT_TYPE_LORA;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return ( ral_status_t ) llcc68_set_pkt_type( context, radio_pkt_type );
}

ral_status_t ral_llcc68_get_pkt_type( const void* context, ral_pkt_type_t* pkt_type )
{
    ral_status_t      status = RAL_STATUS_ERROR;
    llcc68_pkt_type_t radio_pkt_type;

    status = ( ral_status_t ) llcc68_get_pkt_type( context, &radio_pkt_type );
    if( status == RAL_STATUS_OK )
    {
        switch( radio_pkt_type )
        {
        case LLCC68_PKT_TYPE_GFSK:
        {
            *pkt_type = RAL_PKT_TYPE_GFSK;
            break;
        }
        case LLCC68_PKT_TYPE_LORA:
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

ral_status_t ral_llcc68_set_gfsk_mod_params( const void* context, const ral_gfsk_mod_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    llcc68_mod_params_gfsk_t radio_mod_params = { 0 };

    status = ral_llcc68_convert_gfsk_mod_params_from_ral( params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) llcc68_set_gfsk_mod_params( context, &radio_mod_params );
}

ral_status_t ral_llcc68_set_gfsk_pkt_params( const void* context, const ral_gfsk_pkt_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    llcc68_pkt_params_gfsk_t radio_pkt_params = { 0 };

    status = ral_llcc68_convert_gfsk_pkt_params_from_ral( params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) llcc68_set_gfsk_pkt_params( context, &radio_pkt_params );
}

ral_status_t ral_llcc68_set_lora_mod_params( const void* context, const ral_lora_mod_params_t* params )
{
    ral_status_t             status = RAL_STATUS_ERROR;
    llcc68_mod_params_lora_t radio_mod_params;

    status = ral_llcc68_convert_lora_mod_params_from_ral( params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) llcc68_set_lora_mod_params( context, &radio_mod_params );
}

ral_status_t ral_llcc68_set_lora_pkt_params( const void* context, const ral_lora_pkt_params_t* params )
{
    ral_status_t             status           = RAL_STATUS_ERROR;
    llcc68_pkt_params_lora_t radio_pkt_params = { 0 };

    status = ral_llcc68_convert_lora_pkt_params_from_ral( params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) llcc68_set_lora_pkt_params( context, &radio_pkt_params );
}

ral_status_t ral_llcc68_set_lora_cad_params( const void* context, const ral_lora_cad_params_t* params )
{
    ral_status_t        status = RAL_STATUS_ERROR;
    llcc68_cad_params_t radio_lora_cad_params;

    status = ral_llcc68_convert_lora_cad_params_from_ral( params, &radio_lora_cad_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) llcc68_set_cad_params( context, &radio_lora_cad_params );
}

ral_status_t ral_llcc68_set_lora_symb_nb_timeout( const void* context, const uint8_t nb_of_symbs )
{
    return ( ral_status_t ) llcc68_set_lora_symb_nb_timeout( context, nb_of_symbs );
}

ral_status_t ral_llcc68_set_flrc_mod_params( const void* context, const ral_flrc_mod_params_t* params )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_set_flrc_pkt_params( const void* context, const ral_flrc_pkt_params_t* params )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_get_gfsk_rx_pkt_status( const void* context, ral_gfsk_rx_pkt_status_t* rx_pkt_status )
{
    ral_status_t             status = RAL_STATUS_ERROR;
    llcc68_pkt_status_gfsk_t radio_rx_pkt_status;

    status = ( ral_status_t ) llcc68_get_gfsk_pkt_status( context, &radio_rx_pkt_status );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    uint8_t rx_status = 0;
    rx_status |= ( radio_rx_pkt_status.rx_status.pkt_sent == true ) ? RAL_RX_STATUS_PKT_SENT : 0x00;
    rx_status |= ( radio_rx_pkt_status.rx_status.pkt_received == true ) ? RAL_RX_STATUS_PKT_RECEIVED : 0x00;
    rx_status |= ( radio_rx_pkt_status.rx_status.abort_error == true ) ? RAL_RX_STATUS_ABORT_ERROR : 0x00;
    rx_status |= ( radio_rx_pkt_status.rx_status.length_error == true ) ? RAL_RX_STATUS_LENGTH_ERROR : 0x00;
    rx_status |= ( radio_rx_pkt_status.rx_status.crc_error == true ) ? RAL_RX_STATUS_CRC_ERROR : 0x00;
    rx_status |= ( radio_rx_pkt_status.rx_status.adrs_error == true ) ? RAL_RX_STATUS_ADDR_ERROR : 0x00;

    rx_pkt_status->rx_status = rx_status;

    rx_pkt_status->rssi_sync_in_dbm = radio_rx_pkt_status.rssi_sync;
    rx_pkt_status->rssi_avg_in_dbm  = radio_rx_pkt_status.rssi_avg;

    return status;
}

ral_status_t ral_llcc68_get_lora_rx_pkt_status( const void* context, ral_lora_rx_pkt_status_t* rx_pkt_status )
{
    ral_status_t             status = RAL_STATUS_ERROR;
    llcc68_pkt_status_lora_t radio_rx_pkt_status;

    status = ( ral_status_t ) llcc68_get_lora_pkt_status( context, &radio_rx_pkt_status );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    rx_pkt_status->rssi_pkt_in_dbm        = radio_rx_pkt_status.rssi_pkt_in_dbm;
    rx_pkt_status->snr_pkt_in_db          = radio_rx_pkt_status.snr_pkt_in_db;
    rx_pkt_status->signal_rssi_pkt_in_dbm = radio_rx_pkt_status.signal_rssi_pkt_in_dbm;

    return status;
}

ral_status_t ral_llcc68_get_flrc_rx_pkt_status( const void* context, ral_flrc_rx_pkt_status_t* rx_pkt_status )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_get_rssi_inst( const void* context, int16_t* rssi_in_dbm )
{
    return ( ral_status_t ) llcc68_get_rssi_inst( context, rssi_in_dbm );
}

uint32_t ral_llcc68_get_lora_time_on_air_in_ms( const ral_lora_pkt_params_t* pkt_p, const ral_lora_mod_params_t* mod_p )
{
    llcc68_mod_params_lora_t radio_mod_params;
    llcc68_pkt_params_lora_t radio_pkt_params;

    ral_llcc68_convert_lora_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_llcc68_convert_lora_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return llcc68_get_lora_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

uint32_t ral_llcc68_get_gfsk_time_on_air_in_ms( const ral_gfsk_pkt_params_t* pkt_p, const ral_gfsk_mod_params_t* mod_p )
{
    llcc68_mod_params_gfsk_t radio_mod_params;
    llcc68_pkt_params_gfsk_t radio_pkt_params;

    ral_llcc68_convert_gfsk_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_llcc68_convert_gfsk_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return llcc68_get_gfsk_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

uint32_t ral_llcc68_get_flrc_time_on_air_in_ms( const ral_flrc_pkt_params_t* pkt_p, const ral_flrc_mod_params_t* mod_p )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_set_gfsk_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    return ( ral_status_t ) llcc68_set_gfsk_sync_word( context, sync_word, sync_word_len );
}

ral_status_t ral_llcc68_set_lora_sync_word( const void* context, const uint8_t sync_word )
{
    return ( ral_status_t ) llcc68_set_lora_sync_word( context, sync_word );
}

ral_status_t ral_llcc68_set_flrc_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_set_gfsk_crc_params( const void* context, const uint16_t seed, const uint16_t polynomial )
{
    ral_status_t status = RAL_STATUS_ERROR;

    status = ( ral_status_t ) llcc68_set_gfsk_crc_seed( context, seed );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    status = ( ral_status_t ) llcc68_set_gfsk_crc_polynomial( context, polynomial );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return status;
}

ral_status_t ral_llcc68_set_flrc_crc_params( const void* context, const uint32_t seed )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_set_gfsk_whitening_seed( const void* context, const uint16_t seed )
{
    return ( ral_status_t ) llcc68_set_gfsk_whitening_seed( context, seed );
}

ral_status_t ral_llcc68_lr_fhss_init( const void* context, const ral_lr_fhss_params_t* lr_fhss_params )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_lr_fhss_build_frame( const void* context, ral_lr_fhss_memory_state_t state,
                                             const ral_lr_fhss_params_t* lr_fhss_params, uint16_t hop_sequence_id,
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

ral_status_t ral_llcc68_lr_fhss_handle_hop( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                            ral_lr_fhss_memory_state_t state )
{
    ( void ) context;         // Unused parameter
    ( void ) state;           // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_lr_fhss_handle_tx_done( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                                ral_lr_fhss_memory_state_t state )
{
    ( void ) context;         // Unused parameter
    ( void ) state;           // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_lr_fhss_get_time_on_air_in_ms( const void* context, const ral_lr_fhss_params_t* lr_fhss_params,
                                                       uint16_t payload_length, uint32_t* time_on_air )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    ( void ) payload_length;  // Unused parameter
    ( void ) time_on_air;     // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_lr_fhss_get_hop_sequence_count( const void*                 context,
                                                        const ral_lr_fhss_params_t* lr_fhss_params )
{
    ( void ) context;         // Unused parameter
    ( void ) lr_fhss_params;  // Unused parameter
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_get_lora_rx_pkt_cr_crc( const void* context, ral_lora_cr_t* cr, bool* is_crc_present )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_get_tx_consumption_in_ua( const void* context, const int8_t output_pwr_in_dbm,
                                                  const uint32_t rf_freq_in_hz, uint32_t* pwr_consumption_in_ua )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_get_gfsk_rx_consumption_in_ua( const void* context, const uint32_t br_in_bps,
                                                       const uint32_t bw_dsb_in_hz, const bool rx_boosted,
                                                       uint32_t* pwr_consumption_in_ua )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_get_lora_rx_consumption_in_ua( const void* context, const ral_lora_bw_t bw,
                                                       const bool rx_boosted, uint32_t* pwr_consumption_in_ua )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_llcc68_get_random_numbers( const void* context, uint32_t* numbers, unsigned int n )
{
    return ( ral_status_t ) llcc68_get_random_numbers( context, numbers, n );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static ral_irq_t ral_llcc68_convert_irq_flags_to_ral( llcc68_irq_mask_t llcc68_irq )
{
    ral_irq_t ral_irq = RAL_IRQ_NONE;

    if( ( llcc68_irq & LLCC68_IRQ_TX_DONE ) == LLCC68_IRQ_TX_DONE )
    {
        ral_irq |= RAL_IRQ_TX_DONE;
    }
    if( ( llcc68_irq & LLCC68_IRQ_RX_DONE ) == LLCC68_IRQ_RX_DONE )
    {
        ral_irq |= RAL_IRQ_RX_DONE;
    }
    if( ( llcc68_irq & LLCC68_IRQ_TIMEOUT ) == LLCC68_IRQ_TIMEOUT )
    {
        ral_irq |= RAL_IRQ_RX_TIMEOUT;
    }
    if( ( ( llcc68_irq & LLCC68_IRQ_SYNC_WORD_VALID ) == LLCC68_IRQ_SYNC_WORD_VALID ) ||
        ( ( llcc68_irq & LLCC68_IRQ_HEADER_VALID ) == LLCC68_IRQ_HEADER_VALID ) )
    {
        ral_irq |= RAL_IRQ_RX_HDR_OK;
    }
    if( ( llcc68_irq & LLCC68_IRQ_HEADER_ERROR ) == LLCC68_IRQ_HEADER_ERROR )
    {
        ral_irq |= RAL_IRQ_RX_HDR_ERROR;
    }
    if( ( llcc68_irq & LLCC68_IRQ_CRC_ERROR ) == LLCC68_IRQ_CRC_ERROR )
    {
        ral_irq |= RAL_IRQ_RX_CRC_ERROR;
    }
    if( ( llcc68_irq & LLCC68_IRQ_CAD_DONE ) == LLCC68_IRQ_CAD_DONE )
    {
        ral_irq |= RAL_IRQ_CAD_DONE;
    }
    if( ( llcc68_irq & LLCC68_IRQ_CAD_DETECTED ) == LLCC68_IRQ_CAD_DETECTED )
    {
        ral_irq |= RAL_IRQ_CAD_OK;
    }
    if( ( llcc68_irq & LLCC68_IRQ_PREAMBLE_DETECTED ) == LLCC68_IRQ_PREAMBLE_DETECTED )
    {
        ral_irq |= RAL_IRQ_RX_PREAMBLE_DETECTED;
    }
    return ral_irq;
}

static llcc68_irq_mask_t ral_llcc68_convert_irq_flags_from_ral( ral_irq_t ral_irq )
{
    llcc68_irq_mask_t llcc68_irq_mask = LLCC68_IRQ_NONE;

    if( ( ral_irq & RAL_IRQ_TX_DONE ) == RAL_IRQ_TX_DONE )
    {
        llcc68_irq_mask |= LLCC68_IRQ_TX_DONE;
    }
    if( ( ral_irq & RAL_IRQ_RX_DONE ) == RAL_IRQ_RX_DONE )
    {
        llcc68_irq_mask |= LLCC68_IRQ_RX_DONE;
    }
    if( ( ral_irq & RAL_IRQ_RX_TIMEOUT ) == RAL_IRQ_RX_TIMEOUT )
    {
        llcc68_irq_mask |= LLCC68_IRQ_TIMEOUT;
    }
    if( ( ral_irq & RAL_IRQ_RX_HDR_OK ) == RAL_IRQ_RX_HDR_OK )
    {
        llcc68_irq_mask |= LLCC68_IRQ_SYNC_WORD_VALID;
        llcc68_irq_mask |= LLCC68_IRQ_HEADER_VALID;
    }
    if( ( ral_irq & RAL_IRQ_RX_HDR_ERROR ) == RAL_IRQ_RX_HDR_ERROR )
    {
        llcc68_irq_mask |= LLCC68_IRQ_HEADER_ERROR;
    }
    if( ( ral_irq & RAL_IRQ_RX_CRC_ERROR ) == RAL_IRQ_RX_CRC_ERROR )
    {
        llcc68_irq_mask |= LLCC68_IRQ_CRC_ERROR;
    }
    if( ( ral_irq & RAL_IRQ_CAD_DONE ) == RAL_IRQ_CAD_DONE )
    {
        llcc68_irq_mask |= LLCC68_IRQ_CAD_DONE;
    }
    if( ( ral_irq & RAL_IRQ_CAD_OK ) == RAL_IRQ_CAD_OK )
    {
        llcc68_irq_mask |= LLCC68_IRQ_CAD_DETECTED;
    }
    if( ( ral_irq & RAL_IRQ_RX_PREAMBLE_DETECTED ) == RAL_IRQ_RX_PREAMBLE_DETECTED )
    {
        llcc68_irq_mask |= LLCC68_IRQ_PREAMBLE_DETECTED;
    }

    return llcc68_irq_mask;
}

static ral_status_t ral_llcc68_convert_gfsk_mod_params_from_ral( const ral_gfsk_mod_params_t* ral_mod_params,
                                                                 llcc68_mod_params_gfsk_t*    radio_mod_params )
{
    ral_status_t status       = RAL_STATUS_ERROR;
    uint8_t      bw_dsb_param = 0;

    status = ( ral_status_t ) llcc68_get_gfsk_bw_param( ral_mod_params->bw_dsb_in_hz, &bw_dsb_param );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    radio_mod_params->br_in_bps    = ral_mod_params->br_in_bps;
    radio_mod_params->fdev_in_hz   = ral_mod_params->fdev_in_hz;
    radio_mod_params->bw_dsb_param = bw_dsb_param;

    switch( ral_mod_params->pulse_shape )
    {
    case RAL_GFSK_PULSE_SHAPE_OFF:
    {
        radio_mod_params->pulse_shape = LLCC68_GFSK_PULSE_SHAPE_OFF;
        break;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_03:
    {
        radio_mod_params->pulse_shape = LLCC68_GFSK_PULSE_SHAPE_BT_03;
        break;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_05:
    {
        radio_mod_params->pulse_shape = LLCC68_GFSK_PULSE_SHAPE_BT_05;
        break;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_07:
    {
        radio_mod_params->pulse_shape = LLCC68_GFSK_PULSE_SHAPE_BT_07;
        break;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_1:
    {
        radio_mod_params->pulse_shape = LLCC68_GFSK_PULSE_SHAPE_BT_1;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return status;
}

static ral_status_t ral_llcc68_convert_gfsk_pkt_params_from_ral( const ral_gfsk_pkt_params_t* ral_pkt_params,
                                                                 llcc68_pkt_params_gfsk_t*    radio_pkt_params )
{
    radio_pkt_params->preamble_len_in_bits = ral_pkt_params->preamble_len_in_bits;

    switch( ral_pkt_params->preamble_detector )
    {
    case RAL_GFSK_PREAMBLE_DETECTOR_OFF:
    {
        radio_pkt_params->preamble_detector = LLCC68_GFSK_PREAMBLE_DETECTOR_OFF;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS:
    {
        radio_pkt_params->preamble_detector = LLCC68_GFSK_PREAMBLE_DETECTOR_MIN_8BITS;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_16BITS:
    {
        radio_pkt_params->preamble_detector = LLCC68_GFSK_PREAMBLE_DETECTOR_MIN_16BITS;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_24BITS:
    {
        radio_pkt_params->preamble_detector = LLCC68_GFSK_PREAMBLE_DETECTOR_MIN_24BITS;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_32BITS:
    {
        radio_pkt_params->preamble_detector = LLCC68_GFSK_PREAMBLE_DETECTOR_MIN_32BITS;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_pkt_params->sync_word_len_in_bits = ral_pkt_params->sync_word_len_in_bits;

    switch( ral_pkt_params->address_filtering )
    {
    case RAL_GFSK_ADDRESS_FILTERING_DISABLE:
    {
        radio_pkt_params->address_filtering = LLCC68_GFSK_ADDRESS_FILTERING_DISABLE;
        break;
    }
    case RAL_GFSK_ADDRESS_FILTERING_NODE_ADDRESS:
    {
        radio_pkt_params->address_filtering = LLCC68_GFSK_ADDRESS_FILTERING_NODE_ADDRESS;
        break;
    }
    case RAL_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES:
    {
        radio_pkt_params->address_filtering = LLCC68_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES;
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
        radio_pkt_params->header_type = LLCC68_GFSK_PKT_FIX_LEN;
        break;
    }
    case RAL_GFSK_PKT_VAR_LEN:
    {
        radio_pkt_params->header_type = LLCC68_GFSK_PKT_VAR_LEN;
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
        radio_pkt_params->crc_type = LLCC68_GFSK_CRC_OFF;
        break;
    }
    case RAL_GFSK_CRC_1_BYTE:
    {
        radio_pkt_params->crc_type = LLCC68_GFSK_CRC_1_BYTE;
        break;
    }
    case RAL_GFSK_CRC_2_BYTES:
    {
        radio_pkt_params->crc_type = LLCC68_GFSK_CRC_2_BYTES;
        break;
    }
    case RAL_GFSK_CRC_1_BYTE_INV:
    {
        radio_pkt_params->crc_type = LLCC68_GFSK_CRC_1_BYTE_INV;
        break;
    }
    case RAL_GFSK_CRC_2_BYTES_INV:
    {
        radio_pkt_params->crc_type = LLCC68_GFSK_CRC_2_BYTES_INV;
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
        radio_pkt_params->dc_free = LLCC68_GFSK_DC_FREE_OFF;
        break;
    }
    case RAL_GFSK_DC_FREE_WHITENING:
    {
        radio_pkt_params->dc_free = LLCC68_GFSK_DC_FREE_WHITENING;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return RAL_STATUS_OK;
}

static ral_status_t ral_llcc68_convert_lora_mod_params_from_ral( const ral_lora_mod_params_t* ral_mod_params,
                                                                 llcc68_mod_params_lora_t*    radio_mod_params )
{
    if( ral_mod_params->sf == RAL_LORA_SF12 )
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    radio_mod_params->sf = ( llcc68_lora_sf_t ) ral_mod_params->sf;

    switch( ral_mod_params->bw )
    {
    case RAL_LORA_BW_007_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_007;
        break;
    }
    case RAL_LORA_BW_010_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_010;
        break;
    }
    case RAL_LORA_BW_015_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_015;
        break;
    }
    case RAL_LORA_BW_020_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_020;
        break;
    }
    case RAL_LORA_BW_031_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_031;
        break;
    }
    case RAL_LORA_BW_041_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_041;
        break;
    }
    case RAL_LORA_BW_062_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_062;
        break;
    }
    case RAL_LORA_BW_125_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_125;
        break;
    }
    case RAL_LORA_BW_250_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_250;
        break;
    }
    case RAL_LORA_BW_500_KHZ:
    {
        radio_mod_params->bw = LLCC68_LORA_BW_500;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_mod_params->cr = ( llcc68_lora_cr_t ) ral_mod_params->cr;

    radio_mod_params->ldro = ral_mod_params->ldro;

    return RAL_STATUS_OK;
}

static ral_status_t ral_llcc68_convert_lora_pkt_params_from_ral( const ral_lora_pkt_params_t* ral_pkt_params,
                                                                 llcc68_pkt_params_lora_t*    radio_pkt_params )
{
    radio_pkt_params->preamble_len_in_symb = ral_pkt_params->preamble_len_in_symb;

    switch( ral_pkt_params->header_type )
    {
    case( RAL_LORA_PKT_EXPLICIT ):
    {
        radio_pkt_params->header_type = LLCC68_LORA_PKT_EXPLICIT;
        break;
    }
    case( RAL_LORA_PKT_IMPLICIT ):
    {
        radio_pkt_params->header_type = LLCC68_LORA_PKT_IMPLICIT;
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

static ral_status_t ral_llcc68_convert_lora_cad_params_from_ral( const ral_lora_cad_params_t* ral_lora_cad_params,
                                                                 llcc68_cad_params_t*         radio_lora_cad_params )
{
    switch( ral_lora_cad_params->cad_symb_nb )
    {
    case RAL_LORA_CAD_01_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = LLCC68_CAD_01_SYMB;
        break;
    }
    case RAL_LORA_CAD_02_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = LLCC68_CAD_02_SYMB;
        break;
    }
    case RAL_LORA_CAD_04_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = LLCC68_CAD_04_SYMB;
        break;
    }
    case RAL_LORA_CAD_08_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = LLCC68_CAD_08_SYMB;
        break;
    }
    case RAL_LORA_CAD_16_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = LLCC68_CAD_16_SYMB;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_lora_cad_params->cad_detect_peak = ral_lora_cad_params->cad_det_peak_in_symb;
    radio_lora_cad_params->cad_detect_min  = ral_lora_cad_params->cad_det_min_in_symb;

    switch( ral_lora_cad_params->cad_exit_mode )
    {
    case RAL_LORA_CAD_ONLY:
    {
        radio_lora_cad_params->cad_exit_mode = LLCC68_CAD_ONLY;
        break;
    }
    case RAL_LORA_CAD_RX:
    {
        radio_lora_cad_params->cad_exit_mode = LLCC68_CAD_RX;
        break;
    }
    case RAL_LORA_CAD_LBT:
    {
        radio_lora_cad_params->cad_exit_mode = LLCC68_CAD_LBT;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_lora_cad_params->cad_timeout =
        llcc68_convert_timeout_in_ms_to_rtc_step( ral_lora_cad_params->cad_timeout_in_ms );

    return RAL_STATUS_OK;
}

/* --- EOF ------------------------------------------------------------------ */
