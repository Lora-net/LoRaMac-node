/*!
 * \file  main.c
 *
 * \brief Radio sensitivity test
 *
 * \remark When messages stop being displayed on the uart, RF packets aren't
 *         received any more and the sensitivity level has been reached.
 *         By reading the RF generator output power we can estimate the board
 *         sensitivity
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
#include <stdio.h>
#include "board.h"
#include "timer.h"
#include "../../mac/loramac_radio.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

// clang-format off
#if defined( REGION_AS923 )
#define RF_FREQ_IN_HZ                               923000000
#elif defined( REGION_AU915 )
#define RF_FREQ_IN_HZ                               915000000
#elif defined( REGION_CN470 )
#define RF_FREQ_IN_HZ                               470000000
#elif defined( REGION_CN779 )
#define RF_FREQ_IN_HZ                               779000000
#elif defined( REGION_EU433 )
#define RF_FREQ_IN_HZ                               433000000
#elif defined( REGION_EU868 )
#define RF_FREQ_IN_HZ                               868000000
#elif defined( REGION_KR920 )
#define RF_FREQ_IN_HZ                               920000000
#elif defined( REGION_IN865 )
#define RF_FREQ_IN_HZ                               865000000
#elif defined( REGION_US915 )
#define RF_FREQ_IN_HZ                               915000000
#elif defined( REGION_RU864 )
#define RF_FREQ_IN_HZ                               864000000
#else
#error "Please select a region under compiler options."
#endif

#if defined( USE_MODEM_LORA )

/*!
 * \brief LoRa modulation spreading factor
 */
#define LORA_SF                                     RAL_LORA_SF10

/*!
 * \brief LoRa modulation bandwidth
 */
#define LORA_BW                                     RAL_LORA_BW_125_KHZ

/*!
 * \brief LoRa modulation coding rate
 */
#define LORA_CR                                     RAL_LORA_CR_4_5

/*!
 * \brief LoRa preamble length
 */
#define LORA_PREAMBLE_LEN_IN_SYMB                   8

/*!
 * \brief LoRa is packet length fixed or variable
 */
#define LORA_IS_PKT_LEN_FIXED                       false

/*!
 * \brief LoRa is packet crc on or off
 */
#define LORA_IS_CRC_ON                              true

/*!
 * \brief LoRa is IQ inversion on or off
 */
#define LORA_IS_INVERT_IQ_ON                        false

/*!
 * \brief LoRa rx synchronization timeout
 */
#define LORA_RX_SYNC_TIMEOUT_IN_SYMB                6

#elif defined( USE_MODEM_FSK )

/*!
 * \brief GFSK bitrate
 */
#define GFSK_BR_IN_BPS                              50000

/*!
 * \brief GFSK frequency deviation
 */
#define GFSK_FDEV_IN_HZ                             25000

/*!
 * \brief GFSK bandwidth double sided
 */
#define GFSK_BW_DSB_IN_HZ                           100000

/*!
 * \brief GFSK preable length
 */
#define GFSK_PREABLE_LEN_IN_BITS                    40

/*!
 * \brief GFSK sync word length
 */
#define GFSK_SYNC_WORD_LEN_IN_BITS                  24

/*!
 * \brief GFSK is packet length fixed or variable
 */
#define GFSK_IS_PKT_LEN_FIXED                       false

/*!
 * \brief GFSK is packet crc on or off
 */
#define GFSK_IS_CRC_ON                              true

/*!
 * \brief GFSK rx synchronization timeout
 */
#define GFSK_RX_SYNC_TIMEOUT_IN_SYMB                6

/*!
 * \brief GFSK tx timeout
 */
#define GFSK_TX_TIMEOUT_IN_MS                       4000

#else
#error "Please select a modem under compiler options."
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*!
 * \brief Radio interrupt callbacks
 */
static loramac_radio_irq_t radio_irq_callbacks;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*!
 * \brief Rx done interrupt callback
 *
 * \param [out] params  Pointer to the received parameters
 */
static void irq_rx_done( loramac_radio_irq_rx_done_params_t* params );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

/*!
 * \brief Main application entry point.
 */
int main( void )
{
    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );

    // Radio initialization
    radio_irq_callbacks.loramac_radio_irq_rx_done = irq_rx_done;

    loramac_radio_init( &radio_irq_callbacks );

#if defined( USE_MODEM_LORA )
    loramac_radio_lora_cfg_params_t lora_params = {
        .rf_freq_in_hz           = RF_FREQ_IN_HZ,
        .sf                      = LORA_SF,
        .bw                      = LORA_BW,
        .cr                      = LORA_CR,
        .preamble_len_in_symb    = LORA_PREAMBLE_LEN_IN_SYMB,
        .is_pkt_len_fixed        = LORA_IS_PKT_LEN_FIXED,
        .pld_len_in_bytes        = 255,
        .is_crc_on               = LORA_IS_CRC_ON,
        .invert_iq_is_on         = LORA_IS_INVERT_IQ_ON,
        .rx_sync_timeout_in_symb = LORA_RX_SYNC_TIMEOUT_IN_SYMB,
        .is_rx_continuous        = true,
    };
    loramac_radio_lora_set_cfg( &lora_params );
#elif defined( USE_MODEM_FSK )
    loramac_radio_gfsk_cfg_params_t gfsk_params = {
        .rf_freq_in_hz           = RF_FREQ_IN_HZ,
        .br_in_bps               = GFSK_BR_IN_BPS,
        .fdev_in_hz              = GFSK_FDEV_IN_HZ,
        .bw_dsb_in_hz            = GFSK_BW_DSB_IN_HZ,
        .preamble_len_in_bits    = GFSK_PREABLE_LEN_IN_BITS,
        .sync_word_len_in_bits   = GFSK_SYNC_WORD_LEN_IN_BITS,
        .is_pkt_len_fixed        = GFSK_IS_PKT_LEN_FIXED,
        .pld_len_in_bytes        = 255,
        .is_crc_on               = GFSK_IS_CRC_ON,
        .rx_sync_timeout_in_symb = GFSK_RX_SYNC_TIMEOUT_IN_SYMB,
        .is_rx_continuous        = true,
    };
    loramac_radio_gfsk_set_cfg( &gfsk_params );
#endif

    loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );

    while( 1 )
    {
        // Tick the RTC to execute callback in context of the main loop (in stead of the IRQ)
        TimerProcess( );
        BoardLowPowerHandler( );
        // Process Radio IRQ
        loramac_radio_irq_process( );
    }
}

static void irq_rx_done( loramac_radio_irq_rx_done_params_t* params )
{
#if defined( USE_MODEM_LORA )
    printf( "[IRQ] rx done rssi: %4d dBm, snr: %4d dB\n", params->rssi_in_dbm, params->snr_in_db );
#elif defined( USE_MODEM_FSK )
    printf( "[IRQ] rx done rssi: %4d dBm\n", params->rssi_in_dbm );
#endif
}
