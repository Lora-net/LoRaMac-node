/*!
 * \file  main.c
 *
 * \brief Ping-Pong implementation
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
#include <stdio.h>
#include "board.h"
#include "delay.h"
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

/*!
 * \brief Application states definition
 */
typedef enum app_states_e
{
    APP_STATE_LOW_POWER,
    APP_STATE_RX,
    APP_STATE_RX_TIMEOUT,
    APP_STATE_RX_ERROR,
    APP_STATE_TX,
    APP_STATE_TX_TIMEOUT,
} app_states_t;

/*!
 * \brief Application context definition
 */
typedef struct app_context_s
{
    app_states_t state;
    bool         is_master;
    uint16_t     buffer_size_in_bytes;
    uint8_t*     buffer;
} app_context_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

// clang-format off
/*!
 * \brief RF frequency
 */
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

/*!
 * \brief RF transmission output power
 */
#define TX_RF_PWR_IN_DBM                            14

#if defined( USE_MODEM_LORA )

/*!
 * \brief LoRa modulation spreading factor
 */
#define LORA_SF                                     RAL_LORA_SF7

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

/*!
 * \brief LoRa tx timeout
 */
#define LORA_TX_TIMEOUT_IN_MS                       4000

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

/*!
 * \brief Maximum application data buffer size
 *
 * \remark Please do not change this value
 */
#define PING_PONG_APP_DATA_MAX_SIZE                 255

/*!
 * \brief Application payload length
 *
 * \remark Please change this value in order to test different payload lengths
 */
#define APP_PLD_LEN_IN_BYTES                        64
// clang-format on

/*!
 * \brief Ping message string
 */
const uint8_t app_ping_msg[] = "PING";

/*!
 * \brief Pong message string
 */
const uint8_t app_pong_msg[] = "PONG";

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*!
 * \brief Application data buffer
 */
static uint8_t app_data_buffer[PING_PONG_APP_DATA_MAX_SIZE];

/*!
 * \brief Application context
 */
static app_context_t app_context = {
    .state                = APP_STATE_LOW_POWER,
    .is_master            = true,
    .buffer_size_in_bytes = APP_PLD_LEN_IN_BYTES,
    .buffer               = app_data_buffer,
};

/*!
 * \brief Radio interrupt callbacks
 */
static loramac_radio_irq_t radio_irq_callbacks;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*!
 * \brief Tx done interrupt callback
 */
static void irq_tx_done( void );

/*!
 * \brief Rx done interrupt callback
 *
 * \param [out] params  Pointer to the received parameters
 */
static void irq_rx_done( loramac_radio_irq_rx_done_params_t* params );

/*!
 * \brief Rx error interrupt callback
 */
static void irq_rx_error( void );

/*!
 * \brief Tx timeout interrupt callback
 */
static void irq_tx_timeout( void );

/*!
 * \brief Rx timeout interrupt callback
 */
static void irq_rx_timeout( void );

/*!
 * \brief Print the provided buffer in HEX
 *
 * \param [in] buffer Buffer to be printed
 * \param [in] size   Buffer size to be printed
 */
static void print_hex_buffer( uint8_t* buffer, uint8_t size );

/*!
 * \brief Build message to be transmitted
 *
 * \param [in]  is_ping_msg   Indicate if it is a PING or PONG message
 * \param [out] buffer        Buffer to be filled
 * \param [in]  size_in_bytes Buffer size to be filled
 */
static void app_build_message( bool is_ping_msg, uint8_t* buffer, uint8_t size_in_bytes );

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
    radio_irq_callbacks.loramac_radio_irq_tx_done    = irq_tx_done;
    radio_irq_callbacks.loramac_radio_irq_rx_done    = irq_rx_done;
    radio_irq_callbacks.loramac_radio_irq_rx_error   = irq_rx_error;
    radio_irq_callbacks.loramac_radio_irq_tx_timeout = irq_tx_timeout;
    radio_irq_callbacks.loramac_radio_irq_rx_timeout = irq_rx_timeout;

    loramac_radio_init( &radio_irq_callbacks );

#if defined( USE_MODEM_LORA )
    loramac_radio_lora_cfg_params_t lora_params = {
        .rf_freq_in_hz           = RF_FREQ_IN_HZ,
        .tx_rf_pwr_in_dbm        = TX_RF_PWR_IN_DBM,
        .sf                      = LORA_SF,
        .bw                      = LORA_BW,
        .cr                      = LORA_CR,
        .preamble_len_in_symb    = LORA_PREAMBLE_LEN_IN_SYMB,
        .is_pkt_len_fixed        = LORA_IS_PKT_LEN_FIXED,
        .pld_len_in_bytes        = APP_PLD_LEN_IN_BYTES,
        .is_crc_on               = LORA_IS_CRC_ON,
        .invert_iq_is_on         = LORA_IS_INVERT_IQ_ON,
        .rx_sync_timeout_in_symb = LORA_RX_SYNC_TIMEOUT_IN_SYMB,
        .is_rx_continuous        = true,
        .tx_timeout_in_ms        = LORA_TX_TIMEOUT_IN_MS,
    };
    loramac_radio_lora_set_cfg( &lora_params );
#elif defined( USE_MODEM_FSK )
    loramac_radio_gfsk_cfg_params_t gfsk_params = {
        .rf_freq_in_hz           = RF_FREQ_IN_HZ,
        .tx_rf_pwr_in_dbm        = TX_RF_PWR_IN_DBM,
        .br_in_bps               = GFSK_BR_IN_BPS,
        .fdev_in_hz              = GFSK_FDEV_IN_HZ,
        .bw_dsb_in_hz            = GFSK_BW_DSB_IN_HZ,
        .preamble_len_in_bits    = GFSK_PREABLE_LEN_IN_BITS,
        .sync_word_len_in_bits   = GFSK_SYNC_WORD_LEN_IN_BITS,
        .is_pkt_len_fixed        = GFSK_IS_PKT_LEN_FIXED,
        .pld_len_in_bytes        = APP_PLD_LEN_IN_BYTES,
        .is_crc_on               = GFSK_IS_CRC_ON,
        .rx_sync_timeout_in_symb = GFSK_RX_SYNC_TIMEOUT_IN_SYMB,
        .is_rx_continuous        = true,
        .tx_timeout_in_ms        = GFSK_TX_TIMEOUT_IN_MS,
    };
    loramac_radio_gfsk_set_cfg( &gfsk_params );
#endif

    loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );

    while( 1 )
    {
        // Tick the RTC to execute callback in context of the main loop (in stead of the IRQ)
        TimerProcess( );

        switch( app_context.state )
        {
        case APP_STATE_RX:
            if( app_context.is_master == true )
            {
                if( app_context.buffer_size_in_bytes > 0 )
                {
                    if( strncmp( ( const char* ) app_context.buffer, ( const char* ) app_pong_msg, 4 ) == 0 )
                    {
                        printf( "[APP] pong message received\n" );
                        // Send the next PING frame
                        app_build_message( true, app_context.buffer, app_context.buffer_size_in_bytes );
                        DelayMs( 1 );
                        printf( "[APP] ping message transmission\n" );
                        loramac_radio_transmit( app_context.buffer, app_context.buffer_size_in_bytes );
                    }
                    else if( strncmp( ( const char* ) app_context.buffer, ( const char* ) app_ping_msg, 4 ) == 0 )
                    {  // A master already exists then become a slave
                        app_context.is_master = false;
                        printf( "[APP] ping-pong slave mode\n" );
                        loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );
                    }
                    else  // valid reception but neither a PING or a PONG message
                    {     // Set device as master ans start again
                        app_context.is_master = true;
                        printf( "[APP] ping-pong master mode\n" );
                        loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );
                    }
                }
            }
            else
            {
                if( app_context.buffer_size_in_bytes > 0 )
                {
                    if( strncmp( ( const char* ) app_context.buffer, ( const char* ) app_ping_msg, 4 ) == 0 )
                    {
                        printf( "[APP] ping message received\n" );
                        // Send the reply to the PONG string
                        app_build_message( false, app_context.buffer, app_context.buffer_size_in_bytes );
                        DelayMs( 1 );
                        printf( "[APP] pong message transmission\n" );
                        loramac_radio_transmit( app_context.buffer, app_context.buffer_size_in_bytes );
                    }
                    else  // valid reception but not a PING as expected
                    {     // Set device as master and start again
                        app_context.is_master = true;
                        printf( "[APP] ping-pong master mode\n" );
                        loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );
                    }
                }
            }
            app_context.state = APP_STATE_LOW_POWER;
            break;
        case APP_STATE_TX:
            loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );
            if( app_context.is_master == true )
            {
                printf( "[APP] ping message transmitted\n" );
            }
            else
            {
                printf( "[APP] pong message transmitted\n" );
            }
            app_context.state = APP_STATE_LOW_POWER;
            break;
        case APP_STATE_RX_TIMEOUT:
        case APP_STATE_RX_ERROR:
            if( app_context.is_master == true )
            {
                // Send the next PING frame
                app_build_message( true, app_context.buffer, app_context.buffer_size_in_bytes );
                DelayMs( 1 );
                printf( "[APP] ping message transmission\n" );
                loramac_radio_transmit( app_context.buffer, app_context.buffer_size_in_bytes );
            }
            else
            {
                loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );
            }
            app_context.state = APP_STATE_LOW_POWER;
            break;
        case APP_STATE_TX_TIMEOUT:
            loramac_radio_set_rx( RAL_RX_TIMEOUT_CONTINUOUS_MODE );
            app_context.state = APP_STATE_LOW_POWER;
            break;
        case APP_STATE_LOW_POWER:
        default:
            // Set low power
            break;
        }
        BoardLowPowerHandler( );
        // Process Radio IRQ
        loramac_radio_irq_process( );
    }
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void irq_tx_done( void )
{
    loramac_radio_set_sleep( );
    app_context.state = APP_STATE_TX;
}

static void irq_rx_done( loramac_radio_irq_rx_done_params_t* params )
{
    loramac_radio_set_sleep( );

    memcpy( app_context.buffer, params->buffer, params->size_in_bytes );

    app_context.buffer_size_in_bytes = params->size_in_bytes;

#if defined( USE_MODEM_LORA )
    printf( "[IRQ] rx done rssi: %4d dBm, snr: %4d dB\n", params->rssi_in_dbm, params->snr_in_db );
#elif defined( USE_MODEM_FSK )
    printf( "[IRQ] rx done rssi: %4d dBm\n", params->rssi_in_dbm );
#endif
    if( app_context.buffer[1] == 'I' )
    {
        printf( "P I N G " );
        print_hex_buffer( app_context.buffer + 4, params->size_in_bytes );
    }
    else if( app_context.buffer[1] == 'O' )
    {
        printf( "P O N G " );
        print_hex_buffer( app_context.buffer + 4, params->size_in_bytes );
    }
    else
    {
        print_hex_buffer( app_context.buffer, params->size_in_bytes );
    }
    app_context.state = APP_STATE_RX;
}

static void irq_rx_error( void )
{
    loramac_radio_set_sleep( );
    app_context.state = APP_STATE_RX_ERROR;
}

static void irq_tx_timeout( void )
{
    loramac_radio_set_sleep( );
    app_context.state = APP_STATE_TX_TIMEOUT;
}

static void irq_rx_timeout( void )
{
    loramac_radio_set_sleep( );
    app_context.state = APP_STATE_RX_TIMEOUT;
}

static void print_hex_buffer( uint8_t* buffer, uint8_t size )
{
    uint8_t newline = 0;

    for( uint8_t i = 0; i < size; i++ )
    {
        if( newline != 0 )
        {
            printf( "\n" );
            newline = 0;
        }
        printf( "%02X ", buffer[i] );
        if( ( ( i + 1 ) % 16 ) == 0 )
        {
            newline = 1;
        }
    }
    printf( "\n" );
}

static void app_build_message( bool is_ping_msg, uint8_t* buffer, uint8_t size_in_bytes )
{
    uint8_t app_msg_size;

    if( is_ping_msg == true )
    {
        app_msg_size = sizeof( app_ping_msg );
        memcpy( buffer, app_ping_msg, app_msg_size );
    }
    else
    {
        app_msg_size = sizeof( app_pong_msg );
        memcpy( buffer, app_pong_msg, app_msg_size );
    }
    // Fill remaining buffer bytes
    for( uint8_t i = app_msg_size; i < size_in_bytes; i++ )
    {
        buffer[i] = i - app_msg_size;
    }
}

/* --- EOF ------------------------------------------------------------------ */
