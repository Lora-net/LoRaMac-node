/*!
 * \file  main.c
 *
 * \brief Tx Continuous Wave implementation
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

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               923000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#elif defined( REGION_AU915 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               915000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#elif defined( REGION_CN470 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               470000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            20

#elif defined( REGION_CN779 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               779000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#elif defined( REGION_EU433 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               433000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            20

#elif defined( REGION_EU868 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               868000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#elif defined( REGION_KR920 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               920000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#elif defined( REGION_IN865 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               865000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#elif defined( REGION_US915 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               915000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#elif defined( REGION_RU864 )

/*!
 * \brief RF frequency
 */
#define RF_FREQ_IN_HZ                               864000000

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#else
#error "Please select a region under compiler options."
#endif

/*!
 * \brief Tx timeout
 */
#define TX_TIMEOUT_IN_MS                                  10        // seconds (MAX value)
// clang-format on

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*!
 * \brief Radio interrupt callbacks
 */
static loramac_radio_irq_t radio_irq_callbacks;

/*!
 * \brief Tx timeout interrupt callback
 */
static void irq_tx_timeout( void )
{
    // Restarts continuous wave transmission when timeout expires
    loramac_radio_tx_cw_cfg_params_t cfg_params = {
        .rf_freq_in_hz    = RF_FREQ_IN_HZ,
        .tx_rf_pwr_in_dbm = TX_RF_PWR_IN_DBM,
        .timeout_in_s     = TX_TIMEOUT_IN_MS,
    };
    loramac_radio_set_tx_cw( &cfg_params );
}

/*!
 * \brief Main application entry point.
 */
int main( void )
{
    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );

    // Radio initialization
    radio_irq_callbacks.loramac_radio_irq_tx_timeout = irq_tx_timeout;

    loramac_radio_init( &radio_irq_callbacks );

    loramac_radio_tx_cw_cfg_params_t cfg_params = {
        .rf_freq_in_hz    = RF_FREQ_IN_HZ,
        .tx_rf_pwr_in_dbm = TX_RF_PWR_IN_DBM,
        .timeout_in_s     = TX_TIMEOUT_IN_MS,
    };
    loramac_radio_set_tx_cw( &cfg_params );

    // Blink LEDs just to show some activity
    while( 1 )
    {
        // Tick the RTC to execute callback in context of the main loop (in stead of the IRQ)
        TimerProcess( );
        BoardLowPowerHandler( );
        // Process Radio IRQ
        loramac_radio_irq_process( );
    }
}
