/*!
 * \file  loramac_radio.c
 *
 * \brief LoRaMac stack radio interface implementation
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
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "radio_board.h"
#include "loramac_radio.h"
#include "ral.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*!
 * \brief LoRaWAN network type configuration
 */
typedef enum network_type_t
{
    NETWORK_PRIVATE = 0x12,  //!< LoRaWAN private network
    NETWORK_PUBLIC  = 0x34,  //!< LoRaWAN public network
} network_type_t;

/*!
 * \brief Radio GFSK setup additional paramters
 */
typedef struct gfsk_extra_params_s
{
    uint8_t* sync_word;
    uint16_t crc_polynomial;
    uint16_t crc_seed;
    uint16_t whitening_seed;
    uint32_t rx_sync_timeout_in_symb;
} gfsk_extra_params_t;

/*!
 * \brief Radio LoRa setup additional paramters
 */
typedef struct lora_extra_params_s
{
    uint8_t  sync_word;
    uint32_t rx_sync_timeout_in_symb;
} lora_extra_params_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*!
 * Radio interrupt callbacks
 */
static const loramac_radio_irq_t* radio_irq_callbacks;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*!
 * Tx and Rx timers
 */
static TimerEvent_t tx_timeout_timer;
static TimerEvent_t rx_timeout_timer;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*!
 * \brief Radio irq callback
 */
static void radio_irq( void* context );

/*!
 * \brief Tx timeout timer callback
 */
static void tx_timeout_irq( void* context );

/*!
 * \brief Rx timeout timer callback
 */
static void rx_timeout_irq( void* context );

/*!
 * \brief Setup the radio GFSK modem parameters
 *
 * \param [in] context      RAL context
 * \param [in] mod_params   GFSK modulation parameters
 * \param [in] pkt_params   GFSK packet parameters
 * \param [in] extra_params GFSK additional parameters
 *
 * \retval status Operation status
 */
static ral_status_t radio_gfsk_setup( const ral_t* context, const ral_gfsk_mod_params_t* mod_params,
                                      const ral_gfsk_pkt_params_t* pkt_params,
                                      const gfsk_extra_params_t*   extra_params );

/*!
 * \brief Setup the radio LoRa modem parameters
 *
 * \param [in] context      RAL context
 * \param [in] mod_params   LoRa modulation parameters
 * \param [in] pkt_params   LoRa packet parameters
 * \param [in] extra_params LoRa additional parameters
 *
 * \retval status Operation status
 */
static ral_status_t radio_lora_setup( const ral_t* context, const ral_lora_mod_params_t* mod_params,
                                      const ral_lora_pkt_params_t* pkt_params,
                                      const lora_extra_params_t*   extra_params );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

loramac_radio_status_t loramac_radio_init( const loramac_radio_irq_t* irq_callbacks )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    // Initialize variables
    radio_irq_callbacks = irq_callbacks;

    radio_context->radio_params.is_image_calibrated = false;  // Force image calibration
    radio_context->radio_params.max_payload_length  = 0xFF;
    radio_context->radio_params.tx_timeout_in_ms    = 0;
    radio_context->radio_params.rx_timeout_in_ms    = 0;
    radio_context->radio_params.is_rx_continuous    = false;
    radio_context->radio_params.is_irq_fired        = false;
#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    radio_context->radio_params.lr_fhss.is_lr_fhss_on = false;
#endif
    // Initialize timeout timers
    TimerInit( &tx_timeout_timer, tx_timeout_irq );
    TimerInit( &rx_timeout_timer, rx_timeout_irq );

    // Initialize radio driver
    return ( loramac_radio_status_t ) radio_board_init( ral_context, radio_irq );
}

bool loramac_radio_is_radio_idle( void )
{
    switch( radio_board_get_operating_mode( ) )
    {
    case RADIO_BOARD_OP_MODE_SLEEP:
    case RADIO_BOARD_OP_MODE_STDBY:
        return true;
    default:
        return false;
    }
}

loramac_radio_status_t loramac_radio_gfsk_set_cfg( const loramac_radio_gfsk_cfg_params_t* cfg_params )
{
    ral_t*                ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t*      radio_context = radio_board_get_radio_context_reference( );
    ral_gfsk_mod_params_t mod_params    = {
        .br_in_bps    = cfg_params->br_in_bps,
        .fdev_in_hz   = cfg_params->fdev_in_hz,
        .bw_dsb_in_hz = cfg_params->bw_dsb_in_hz,
        .pulse_shape  = RAL_GFSK_PULSE_SHAPE_BT_1,
    };
    ral_gfsk_pkt_params_t pkt_params = {
        .preamble_len_in_bits  = cfg_params->preamble_len_in_bits,
        .preamble_detector     = RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS,
        .sync_word_len_in_bits = cfg_params->sync_word_len_in_bits,
        .header_type           = ( cfg_params->is_pkt_len_fixed == true ) ? RAL_GFSK_PKT_FIX_LEN : RAL_GFSK_PKT_VAR_LEN,
        .pld_len_in_bytes      = cfg_params->pld_len_in_bytes,
        .crc_type              = ( cfg_params->is_crc_on == true ) ? RAL_GFSK_CRC_2_BYTES_INV : RAL_GFSK_CRC_OFF,
        .dc_free               = RAL_GFSK_DC_FREE_WHITENING,
    };
    gfsk_extra_params_t extra_params = {
        .sync_word               = ( uint8_t[] ){ 0xC1, 0x94, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00 },
        .crc_polynomial          = 0x1021,
        .crc_seed                = 0x1D0F,
        .whitening_seed          = 0x01FF,
        .rx_sync_timeout_in_symb = 0,
    };

    radio_context->radio_params.rf_freq_in_hz    = cfg_params->rf_freq_in_hz;
    radio_context->radio_params.tx_rf_pwr_in_dbm = cfg_params->tx_rf_pwr_in_dbm;
    radio_context->radio_params.is_rx_continuous = cfg_params->is_rx_continuous;
    if( cfg_params->is_rx_continuous == false )
    {
        extra_params.rx_sync_timeout_in_symb = cfg_params->rx_sync_timeout_in_symb;
    }
    if( cfg_params->is_pkt_len_fixed == true )
    {
        radio_context->radio_params.max_payload_length = cfg_params->pld_len_in_bytes;
    }
    else
    {
        radio_context->radio_params.max_payload_length = 0xFF;
    }
    radio_context->radio_params.rx_timeout_in_ms =
        extra_params.rx_sync_timeout_in_symb * 8000UL / cfg_params->br_in_bps;
    radio_context->radio_params.tx_timeout_in_ms = cfg_params->tx_timeout_in_ms;

    return ( loramac_radio_status_t ) radio_gfsk_setup( ral_context, &mod_params, &pkt_params, &extra_params );
}

loramac_radio_status_t loramac_radio_lora_set_cfg( const loramac_radio_lora_cfg_params_t* cfg_params )
{
    ral_t*                ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t*      radio_context = radio_board_get_radio_context_reference( );
    ral_lora_mod_params_t mod_params    = {
        .sf   = cfg_params->sf,
        .bw   = cfg_params->bw,
        .cr   = cfg_params->cr,
        .ldro = ral_compute_lora_ldro( cfg_params->sf, cfg_params->bw ),
    };
    ral_lora_pkt_params_t pkt_params = {
        .preamble_len_in_symb = cfg_params->preamble_len_in_symb,
        .header_type      = ( cfg_params->is_pkt_len_fixed == true ) ? RAL_LORA_PKT_IMPLICIT : RAL_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes = cfg_params->pld_len_in_bytes,
        .crc_is_on        = cfg_params->is_crc_on,
        .invert_iq_is_on  = cfg_params->invert_iq_is_on,
    };
    lora_extra_params_t extra_params = {
        .sync_word = ( radio_context->radio_params.is_public_network == true ) ? NETWORK_PUBLIC : NETWORK_PRIVATE,
        .rx_sync_timeout_in_symb = 0,
    };

    radio_context->radio_params.rf_freq_in_hz    = cfg_params->rf_freq_in_hz;
    radio_context->radio_params.tx_rf_pwr_in_dbm = cfg_params->tx_rf_pwr_in_dbm;
    radio_context->radio_params.is_rx_continuous = cfg_params->is_rx_continuous;
    if( cfg_params->is_rx_continuous == false )
    {
        extra_params.rx_sync_timeout_in_symb = cfg_params->rx_sync_timeout_in_symb;
    }
    if( cfg_params->is_pkt_len_fixed == true )
    {
        radio_context->radio_params.max_payload_length = cfg_params->pld_len_in_bytes;
    }
    else
    {
        radio_context->radio_params.max_payload_length = 0xFF;
    }
    // Timeout Max, Timeout handled directly in SetRx function
    radio_context->radio_params.rx_timeout_in_ms = 0;  // Rx single mode
    radio_context->radio_params.tx_timeout_in_ms = cfg_params->tx_timeout_in_ms;

    return ( loramac_radio_status_t ) radio_lora_setup( ral_context, &mod_params, &pkt_params, &extra_params );
}

loramac_radio_status_t loramac_radio_lr_fhss_set_cfg( const loramac_radio_lr_fhss_cfg_params_t* cfg_params )
{
    ral_status_t status = RAL_STATUS_UNSUPPORTED_FEATURE;
#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );
    uint32_t hop_sequence_count    = ral_lr_fhss_get_hop_sequence_count( ral_context, &cfg_params->lr_fhss_params );

    radio_context->radio_params.rf_freq_in_hz    = cfg_params->lr_fhss_params.center_frequency_in_hz;
    radio_context->radio_params.tx_rf_pwr_in_dbm = cfg_params->tx_rf_pwr_in_dbm;

    radio_context->radio_params.lr_fhss.lr_fhss_params  = cfg_params->lr_fhss_params;
    radio_context->radio_params.lr_fhss.hop_sequence_id = randr( 0, hop_sequence_count );

    radio_context->radio_params.tx_timeout_in_ms = cfg_params->tx_timeout_in_ms;

    status = ral_set_standby( ral_context, RAL_STANDBY_CFG_RC );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    status = ral_lr_fhss_init( ral_context, &cfg_params->lr_fhss_params );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    radio_context->radio_params.lr_fhss.is_lr_fhss_on = true;
#endif
    return ( loramac_radio_status_t ) status;
}

loramac_radio_status_t loramac_radio_transmit( const uint8_t* buffer, const uint16_t size_in_bytes )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );
#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    ral_lr_fhss_memory_state_t lr_fhss_state = radio_board_get_lr_fhss_state_reference( );

    if( radio_context->radio_params.lr_fhss.is_lr_fhss_on == true )
    {
        status =
            ral_lr_fhss_build_frame( ral_context, &radio_context->radio_params.lr_fhss.lr_fhss_params, lr_fhss_state,
                                     radio_context->radio_params.lr_fhss.hop_sequence_id, buffer, size_in_bytes );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
        // RAL_IRQ_TX_DONE, RAL_IRQ_LR_FHSS_HOP
        status = ral_set_dio_irq_params( ral_context, RAL_IRQ_TX_DONE | RAL_IRQ_LR_FHSS_HOP );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
    }
    else
#endif
    {
        status = ral_set_pkt_payload( ral_context, buffer, size_in_bytes );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
        // RAL_IRQ_TX_DONE
        status = ral_set_dio_irq_params( ral_context, RAL_IRQ_TX_DONE );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
    }
    radio_board_set_ant_switch( true );
    status = ral_set_tx_cfg( ral_context, radio_context->radio_params.tx_rf_pwr_in_dbm,
                             radio_context->radio_params.rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    status = ral_set_tx( ral_context );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_TX );
    TimerSetValue( &tx_timeout_timer, radio_context->radio_params.tx_timeout_in_ms );
    TimerStart( &tx_timeout_timer );
    return ( loramac_radio_status_t ) status;
}

loramac_radio_status_t loramac_radio_set_sleep( void )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    status = ral_set_sleep( ral_context, true );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_SLEEP );
    radio_board_set_ant_switch( false );
    radio_board_stop_radio_tcxo( );
    DelayMs( 2 );
    // Force image calibration
    radio_context->radio_params.is_image_calibrated = false;
    return ( loramac_radio_status_t ) status;
}

loramac_radio_status_t loramac_radio_set_standby( void )
{
    ral_status_t status      = RAL_STATUS_ERROR;
    ral_t*       ral_context = radio_board_get_ral_context_reference( );

    radio_board_start_radio_tcxo( );
    status = ral_set_standby( ral_context, RAL_STANDBY_CFG_RC );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    radio_board_set_ant_switch( false );
    return ( loramac_radio_status_t ) status;
}

loramac_radio_status_t loramac_radio_set_rx( const uint32_t timeout_in_ms )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    if( radio_context->radio_params.lr_fhss.is_lr_fhss_on == true )
    {
        return LORAMAC_RADIO_STATUS_ERROR;
    }
#endif
    radio_board_start_radio_tcxo( );
    radio_board_set_ant_switch( false );
    // RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT, RAL_IRQ_RX_CRC_ERROR
    status = ral_set_dio_irq_params( ral_context, RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT | RAL_IRQ_RX_CRC_ERROR );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    // Note: For SX127x radios this API returns unsupported feature.
    status = ral_cfg_rx_boosted( ral_context, false );
    if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
    {
        return ( loramac_radio_status_t ) status;
    }
    if( timeout_in_ms != 0 )
    {
        TimerSetValue( &rx_timeout_timer, timeout_in_ms );
        TimerStart( &rx_timeout_timer );
    }
    if( radio_context->radio_params.is_rx_continuous == true )
    {
        status = ral_set_rx( ral_context, RAL_RX_TIMEOUT_CONTINUOUS_MODE );
        radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX_C );
    }
    else
    {
        status = ral_set_rx( ral_context, radio_context->radio_params.rx_timeout_in_ms );
        radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX );
    }
    return ( loramac_radio_status_t ) status;
}

loramac_radio_status_t loramac_radio_set_tx_cw( const loramac_radio_tx_cw_cfg_params_t* cfg_params )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );
    uint32_t         timeout       = ( uint32_t ) cfg_params->timeout_in_s * 1000;

#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    radio_context->radio_params.lr_fhss.is_lr_fhss_on = false;
#endif

    radio_board_start_radio_tcxo( );
    radio_board_set_ant_switch( true );
    if( radio_context->radio_params.is_image_calibrated == false )
    {
        uint16_t freq_in_mhz = cfg_params->rf_freq_in_hz / 1000000;

        status = ral_cal_img( ral_context, freq_in_mhz, freq_in_mhz );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
        radio_context->radio_params.is_image_calibrated = true;
    }
    status = ral_set_rf_freq( ral_context, cfg_params->rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    status = ( ral_status_t ) ral_set_tx_cfg( ral_context, cfg_params->tx_rf_pwr_in_dbm, cfg_params->rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    status = ral_set_tx_cw( ral_context );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_TX );

    TimerSetValue( &tx_timeout_timer, timeout );
    TimerStart( &tx_timeout_timer );
    return ( loramac_radio_status_t ) status;
}

loramac_radio_status_t loramac_radio_is_channel_free( const loramac_radio_channel_free_cfg_params_t* cfg_params,
                                                      bool*                                          is_channel_free )
{
    ral_status_t                    status          = RAL_STATUS_ERROR;
    ral_t*                          ral_context     = radio_board_get_ral_context_reference( );
    uint32_t                        start_time      = 0;
    int16_t                         rssi            = 0;
    loramac_radio_gfsk_cfg_params_t gfsk_cfg_params = {
        .rf_freq_in_hz           = cfg_params->rf_freq_in_hz,
        .br_in_bps               = 600,
        .fdev_in_hz              = 0,
        .bw_dsb_in_hz            = cfg_params->rx_bw_in_hz,
        .preamble_len_in_bits    = 40,
        .sync_word_len_in_bits   = 24,
        .is_crc_on               = false,
        .is_pkt_len_fixed        = false,
        .pld_len_in_bytes        = 0,
        .rx_sync_timeout_in_symb = 0,
        .is_rx_continuous        = true,
        .tx_timeout_in_ms        = 0,
        .tx_rf_pwr_in_dbm        = 0,
    };

    status = ( ral_status_t ) loramac_radio_gfsk_set_cfg( &gfsk_cfg_params );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    status = ral_set_dio_irq_params( ral_context, RAL_IRQ_NONE );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    status = ral_set_rx( ral_context, RAL_RX_TIMEOUT_CONTINUOUS_MODE );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX_C );
    DelayMs( 1 );
    *is_channel_free = true;
    start_time       = TimerGetCurrentTime( );
    // Perform carrier sense for max_carrier_sense_time_ms
    while( TimerGetElapsedTime( start_time ) < cfg_params->max_carrier_sense_time_ms )
    {
        status = ral_get_rssi_inst( ral_context, &rssi );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
        if( rssi > cfg_params->rssi_threshold_in_dbm )
        {
            *is_channel_free = false;
            break;
        }
    }
    status = ral_set_sleep( ral_context, true );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_SLEEP );
    return ( loramac_radio_status_t ) status;
}

loramac_radio_status_t loramac_radio_set_network_type( const bool is_public_network )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.is_public_network = is_public_network;

    return LORAMAC_RADIO_STATUS_OK;
}

uint32_t loramac_radio_gfsk_get_time_on_air_in_ms( const loramac_radio_gfsk_time_on_air_params_t* params )
{
    ral_t*                ral_context = radio_board_get_ral_context_reference( );
    ral_gfsk_mod_params_t mod_params  = {
        .br_in_bps = params->br_in_bps,
    };
    ral_gfsk_pkt_params_t pkt_params = {
        .preamble_len_in_bits = params->preamble_len_in_bits,
        .header_type          = ( params->is_pkt_len_fixed == true ) ? RAL_GFSK_PKT_FIX_LEN : RAL_GFSK_PKT_VAR_LEN,
        .pld_len_in_bytes     = params->pld_len_in_bytes,
        .crc_type             = ( params->is_crc_on == true ) ? RAL_GFSK_CRC_2_BYTES_INV : RAL_GFSK_CRC_OFF,
    };
    return ral_get_gfsk_time_on_air_in_ms( ral_context, &pkt_params, &mod_params );
}

uint32_t loramac_radio_lora_get_time_on_air_in_ms( const loramac_radio_lora_time_on_air_params_t* params )
{
    ral_t*                ral_context = radio_board_get_ral_context_reference( );
    ral_lora_mod_params_t mod_params  = {
        .sf   = params->sf,
        .bw   = params->bw,
        .cr   = params->cr,
        .ldro = ral_compute_lora_ldro( params->sf, params->bw ),
    };
    ral_lora_pkt_params_t pkt_params = {
        .preamble_len_in_symb = params->preamble_len_in_symb,
        .header_type          = ( params->is_pkt_len_fixed == true ) ? RAL_LORA_PKT_IMPLICIT : RAL_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes     = params->pld_len_in_bytes,
        .crc_is_on            = params->is_crc_on,
    };
    return ral_get_lora_time_on_air_in_ms( ral_context, &pkt_params, &mod_params );
}

uint32_t loramac_radio_lr_fhss_get_time_on_air_in_ms( const loramac_radio_lr_fhss_time_on_air_params_t* params )
{
#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    ral_status_t status            = RAL_STATUS_ERROR;
    ral_t*       ral_context       = radio_board_get_ral_context_reference( );
    uint32_t     time_on_air_in_ms = 0;
    status = ral_lr_fhss_get_time_on_air_in_ms( ral_context, &params->lr_fhss_params, params->pld_len_in_bytes,
                                                &time_on_air_in_ms );
    if( status != RAL_STATUS_OK )
    {
        // Panic
        while( 1 )
            ;
    }
    return time_on_air_in_ms;
#else
    // Panic
    while( 1 )
        ;
    return 0;  // Never reached. Avoids compiler warning
#endif
}

uint32_t loramac_radio_get_wakeup_time_in_ms( void )
{
    // Adds a 3 ms safety margin to the provided wakeup time
    return radio_board_get_tcxo_wakeup_time_in_ms( ) + 3;
}

loramac_radio_status_t loramac_radio_get_random_number( uint32_t* random_number )
{
    ral_status_t status      = RAL_STATUS_ERROR;
    ral_t*       ral_context = radio_board_get_ral_context_reference( );

    // Set operating mode to standby
    status = ral_set_standby( ral_context, RAL_STANDBY_CFG_RC );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    // Set a valid packet type
    status = ral_set_pkt_type( ral_context, RAL_PKT_TYPE_LORA );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    // Disable all radio interrupts
    status = ral_set_dio_irq_params( ral_context, RAL_IRQ_NONE );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    return ( loramac_radio_status_t ) ral_get_random_numbers( ral_context, random_number, 1 );
}

loramac_radio_status_t loramac_radio_irq_process( void )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );
#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    ral_lr_fhss_memory_state_t lr_fhss_state = radio_board_get_lr_fhss_state_reference( );
#endif
    ral_irq_t                    irq_flags = RAL_IRQ_NONE;
    radio_board_operating_mode_t op_mode   = radio_board_get_operating_mode( );

    // Check if there is an interrupt pending
    if( radio_context->radio_params.is_irq_fired == false )
    {
        return LORAMAC_RADIO_STATUS_OK;
    }

    status = ral_get_and_clear_irq_status( ral_context, &irq_flags );
    if( status != RAL_STATUS_OK )
    {
        return ( loramac_radio_status_t ) status;
    }
    CRITICAL_SECTION_BEGIN( );
    // Clear IRQ flag
    radio_context->radio_params.is_irq_fired = false;
    // Check if DIO1 pin is High. If it is the case revert radio_context->radio_params.is_irq_fired to true
    if( radio_board_get_dio_1_pin_state( ) == 1 )
    {
        radio_context->radio_params.is_irq_fired = true;
    }
    CRITICAL_SECTION_END( );
    // Update operating mode current status
    if( ( ( irq_flags & RAL_IRQ_TX_DONE ) != 0 ) || ( ( irq_flags & RAL_IRQ_CAD_DONE ) != 0 ) ||
        ( ( irq_flags & RAL_IRQ_RX_TIMEOUT ) != 0 ) )
    {
        radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    }
    if( ( ( irq_flags & RAL_IRQ_RX_HDR_ERROR ) != 0 ) || ( ( irq_flags & RAL_IRQ_RX_DONE ) != 0 ) ||
        ( ( irq_flags & RAL_IRQ_RX_CRC_ERROR ) != 0 ) )
    {
        if( op_mode != RADIO_BOARD_OP_MODE_RX_C )
        {
            radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
#if defined( SX126X )
            // WORKAROUND - Implicit Header Mode Timeout Behavior, see DS_SX1261-2_V1.2 datasheet chapter 15.3
            status = ( ral_status_t ) sx126x_stop_rtc( radio_context );
            if( status != RAL_STATUS_OK )
            {
                return ( loramac_radio_status_t ) status;
            }
            // WORKAROUND END
#endif
        }
    }
    // Process radio irq flags
    if( ( irq_flags & RAL_IRQ_TX_DONE ) == RAL_IRQ_TX_DONE )
    {
        TimerStop( &tx_timeout_timer );
#if( LORAMAC_LR_FHSS_IS_ON == 1 )
        if( radio_context->radio_params.lr_fhss.is_lr_fhss_on == true )
        {
            status = ral_lr_fhss_handle_tx_done( ral_context, &radio_context->radio_params.lr_fhss.lr_fhss_params,
                                                 lr_fhss_state );
            if( status != RAL_STATUS_OK )
            {
                return ( loramac_radio_status_t ) status;
            }
        }
#endif
        if( ( radio_irq_callbacks != NULL ) && ( radio_irq_callbacks->loramac_radio_irq_tx_done != NULL ) )
        {
            radio_irq_callbacks->loramac_radio_irq_tx_done( );
        }
    }
    if( ( ( irq_flags & RAL_IRQ_RX_DONE ) == RAL_IRQ_RX_DONE ) &&
        ( ( irq_flags & RAL_IRQ_RX_CRC_ERROR ) != RAL_IRQ_RX_CRC_ERROR ) )
    {
        ral_pkt_type_t                     pkt_type       = RAL_PKT_TYPE_GFSK;
        loramac_radio_irq_rx_done_params_t rx_done_params = {
            .buffer = radio_context->radio_params.buffer,
        };

        TimerStop( &rx_timeout_timer );
        status = ral_get_pkt_payload( ral_context, radio_context->radio_params.max_payload_length,
                                      radio_context->radio_params.buffer, &rx_done_params.size_in_bytes );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
        status = ral_get_pkt_type( ral_context, &pkt_type );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
        if( pkt_type == RAL_PKT_TYPE_LORA )
        {
            ral_lora_rx_pkt_status_t lora_rx_pkt_status;

            status = ral_get_lora_rx_pkt_status( ral_context, &lora_rx_pkt_status );
            if( status != RAL_STATUS_OK )
            {
                return ( loramac_radio_status_t ) status;
            }
            rx_done_params.rssi_in_dbm = lora_rx_pkt_status.rssi_pkt_in_dbm;
            rx_done_params.snr_in_db   = lora_rx_pkt_status.snr_pkt_in_db;
            if( ( radio_irq_callbacks != NULL ) && ( radio_irq_callbacks->loramac_radio_irq_rx_done != NULL ) )
            {
                radio_irq_callbacks->loramac_radio_irq_rx_done( &rx_done_params );
            }
        }
        else
        {
            ral_gfsk_rx_pkt_status_t gfsk_rx_pkt_status;

            status = ral_get_gfsk_rx_pkt_status( ral_context, &gfsk_rx_pkt_status );
            if( status != RAL_STATUS_OK )
            {
                return ( loramac_radio_status_t ) status;
            }
            rx_done_params.rssi_in_dbm = gfsk_rx_pkt_status.rssi_avg_in_dbm;
            rx_done_params.snr_in_db   = 0;
            if( ( radio_irq_callbacks != NULL ) && ( radio_irq_callbacks->loramac_radio_irq_rx_done != NULL ) )
            {
                radio_irq_callbacks->loramac_radio_irq_rx_done( &rx_done_params );
            }
        }
    }
    if( ( irq_flags & RAL_IRQ_RX_CRC_ERROR ) == RAL_IRQ_RX_CRC_ERROR )
    {
        if( ( radio_irq_callbacks != NULL ) && ( radio_irq_callbacks->loramac_radio_irq_rx_error != NULL ) )
        {
            radio_irq_callbacks->loramac_radio_irq_rx_error( );
        }
    }
    if( ( ( irq_flags & RAL_IRQ_RX_TIMEOUT ) == RAL_IRQ_RX_TIMEOUT ) ||
        ( ( irq_flags & RAL_IRQ_RX_HDR_ERROR ) == RAL_IRQ_RX_HDR_ERROR ) )
    {
        if( op_mode == RADIO_BOARD_OP_MODE_TX )
        {
            TimerStop( &tx_timeout_timer );

            if( ( radio_irq_callbacks != NULL ) && ( radio_irq_callbacks->loramac_radio_irq_tx_timeout != NULL ) )
            {
                radio_irq_callbacks->loramac_radio_irq_tx_timeout( );
            }
        }
        else
        {
            TimerStop( &rx_timeout_timer );
            if( ( radio_irq_callbacks != NULL ) && ( radio_irq_callbacks->loramac_radio_irq_rx_timeout != NULL ) )
            {
                radio_irq_callbacks->loramac_radio_irq_rx_timeout( );
            }
        }
    }
#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    if( ( irq_flags & RAL_IRQ_LR_FHSS_HOP ) == RAL_IRQ_LR_FHSS_HOP )
    {
        status =
            ral_lr_fhss_handle_hop( ral_context, &radio_context->radio_params.lr_fhss.lr_fhss_params, lr_fhss_state );
        if( status != RAL_STATUS_OK )
        {
            return ( loramac_radio_status_t ) status;
        }
    }
#endif
    return ( loramac_radio_status_t ) status;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void radio_irq( void* context )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.is_irq_fired = true;
}

static void tx_timeout_irq( void* context )
{
#if( SX127X )
    sx127x_t* sx127x_context = radio_board_get_sx127x_context_reference( );

    sx127x_tx_timeout_irq_workaround( sx127x_context );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_SLEEP );
#endif
    if( ( radio_irq_callbacks != NULL ) && ( radio_irq_callbacks->loramac_radio_irq_tx_timeout != NULL ) )
    {
        radio_irq_callbacks->loramac_radio_irq_tx_timeout( );
    }
}

static void rx_timeout_irq( void* context )
{
    if( ( radio_irq_callbacks != NULL ) && ( radio_irq_callbacks->loramac_radio_irq_rx_timeout != NULL ) )
    {
        radio_irq_callbacks->loramac_radio_irq_rx_timeout( );
    }
}

static ral_status_t radio_gfsk_setup( const ral_t* context, const ral_gfsk_mod_params_t* mod_params,
                                      const ral_gfsk_pkt_params_t* pkt_params, const gfsk_extra_params_t* extra_params )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    radio_context->radio_params.lr_fhss.is_lr_fhss_on = false;
#endif

    status = ral_set_standby( context, RAL_STANDBY_CFG_RC );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    status = ral_set_pkt_type( context, RAL_PKT_TYPE_GFSK );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    // Note: For SX127x radios this API returns unsupported feature.
    status = ral_stop_timer_on_preamble( context, false );
    if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
    {
        return status;
    }
    if( radio_context->radio_params.is_image_calibrated == false )
    {
        uint16_t freq_in_mhz = radio_context->radio_params.rf_freq_in_hz / 1000000;

        status = ral_cal_img( context, freq_in_mhz, freq_in_mhz );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
        radio_context->radio_params.is_image_calibrated = true;
    }
    status = ral_set_rf_freq( context, radio_context->radio_params.rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_tx_cfg( context, radio_context->radio_params.tx_rf_pwr_in_dbm,
                             radio_context->radio_params.rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_gfsk_mod_params( context, mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_gfsk_pkt_params( context, pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    if( pkt_params->crc_type != RAL_GFSK_CRC_OFF )
    {
        // Note: For SX127x radios this API returns unsupported feature.
        status = ral_set_gfsk_crc_params( context, extra_params->crc_seed, extra_params->crc_polynomial );
        if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
        {
            return status;
        }
    }
    status = ral_set_gfsk_sync_word( context, extra_params->sync_word, ( pkt_params->sync_word_len_in_bits + 7 ) / 8 );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    if( pkt_params->dc_free == RAL_GFSK_DC_FREE_WHITENING )
    {
        // Note: For SX127x radios this API returns unsupported feature.
        status = ral_set_gfsk_whitening_seed( context, extra_params->whitening_seed );
        if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
        {
            return status;
        }
    }
    return status;
}

static ral_status_t radio_lora_setup( const ral_t* context, const ral_lora_mod_params_t* mod_params,
                                      const ral_lora_pkt_params_t* pkt_params, const lora_extra_params_t* extra_params )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    radio_context->radio_params.lr_fhss.is_lr_fhss_on = false;
#endif

    status = ral_set_standby( context, RAL_STANDBY_CFG_RC );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    status = ral_set_pkt_type( context, RAL_PKT_TYPE_LORA );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    // Note: For SX127x radios this API returns unsupported feature.
    status = ral_stop_timer_on_preamble( context, false );
    if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
    {
        return status;
    }
    status = ral_set_lora_symb_nb_timeout( context, extra_params->rx_sync_timeout_in_symb );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    if( radio_context->radio_params.is_image_calibrated == false )
    {
        uint16_t freq_in_mhz = radio_context->radio_params.rf_freq_in_hz / 1000000;

        status = ral_cal_img( context, freq_in_mhz, freq_in_mhz );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
        radio_context->radio_params.is_image_calibrated = true;
    }
    status = ral_set_rf_freq( context, radio_context->radio_params.rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_tx_cfg( context, radio_context->radio_params.tx_rf_pwr_in_dbm,
                             radio_context->radio_params.rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_lora_mod_params( context, mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_lora_pkt_params( context, pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_lora_sync_word( context, extra_params->sync_word );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    return status;
}

/* --- EOF ------------------------------------------------------------------ */
