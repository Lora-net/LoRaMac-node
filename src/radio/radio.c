/*!
 * \file      radio.c
 *
 * \brief     LoRaMac stack radio interface implementation
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
#include "delay.h"
#include "radio_board.h"
#include "radio.h"
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
 * \brief Type describing the GFSK radio parameters
 */
typedef struct radio_gfsk_params_s
{
    uint32_t              rf_freq_in_hz;
    int8_t                tx_rf_pwr_in_dbm;
    ral_gfsk_mod_params_t mod_params;
    ral_gfsk_pkt_params_t pkt_params;
    const uint8_t*        sync_word;
    uint16_t              crc_seed;
    uint16_t              crc_polynomial;
    uint16_t              whitening_seed;
} radio_gfsk_params_t;

/*!
 * \brief Type describing the LoRa radio parameters
 */
typedef struct radio_lora_params_s
{
    uint32_t              rf_freq_in_hz;
    int8_t                tx_rf_pwr_in_dbm;
    ral_lora_mod_params_t mod_params;
    ral_lora_pkt_params_t pkt_params;
    uint8_t               sync_word;
    uint8_t               symb_nb_timeout;  //! Rx only parameters
} radio_lora_params_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*!
 * LoRa bandwidths conversion table
 */
const ral_lora_bw_t Bandwidths[] = { RAL_LORA_BW_125_KHZ, RAL_LORA_BW_250_KHZ, RAL_LORA_BW_500_KHZ };

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*!
 * Radio interrupt callbacks
 */
static RadioEvents_t* RadioEvents;

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
 * \brief Initializes the radio
 *
 * \param [IN] events Structure containing the driver callback functions
 */
static void RadioInit( RadioEvents_t* events );

/*!
 * Return current radio status
 *
 * \param status Radio status.[RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
 */
static RadioState_t RadioGetStatus( void );

/*!
 * \brief Configures the radio with the given modem
 *
 * \param [IN] modem Modem to be used [0: FSK, 1: LoRa]
 */
static void RadioSetModem( RadioModems_t modem );

/*!
 * \brief Sets the channel frequency
 *
 * \param [IN] freq         Channel RF frequency
 */
static void RadioSetChannel( uint32_t freq );

/*!
 * \brief Checks if the channel is free for the given time
 *
 * \remark The FSK modem is always used for this task as we can select the Rx bandwidth at will.
 *
 * \param [IN] freq                Channel RF frequency in Hertz
 * \param [IN] rxBandwidth         Rx bandwidth in Hertz
 * \param [IN] rssiThresh          RSSI threshold in dBm
 * \param [IN] maxCarrierSenseTime Max time in milliseconds while the RSSI is measured
 *
 * \retval isFree         [true: Channel is free, false: Channel is not free]
 */
static bool RadioIsChannelFree( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime );

/*!
 * \brief Generates a 32 bits random value based on the RSSI readings
 *
 * \remark This function sets the radio in LoRa modem mode and disables
 *         all interrupts.
 *         After calling this function either Radio.SetRxConfig or
 *         Radio.SetTxConfig functions must be called.
 *
 * \retval randomValue    32 bits random value
 */
static uint32_t RadioRandom( void );

/*!
 * \brief Sets the reception parameters
 *
 * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
 * \param [IN] bandwidth    Sets the bandwidth
 *                          FSK : >= 2600 and <= 250000 Hz
 *                          LoRa: [0: 125 kHz, 1: 250 kHz,
 *                                 2: 500 kHz, 3: Reserved]
 * \param [IN] datarate     Sets the Datarate
 *                          FSK : 600..300000 bits/s
 *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
 *                                10: 1024, 11: 2048, 12: 4096  chips]
 * \param [IN] coderate     Sets the coding rate (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * \param [IN] bandwidthAfc Sets the AFC Bandwidth (FSK only)
 *                          FSK : >= 2600 and <= 250000 Hz
 *                          LoRa: N/A ( set to 0 )
 * \param [IN] preambleLen  Sets the Preamble length
 *                          FSK : Number of bytes
 *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
 * \param [IN] symbTimeout  Sets the RxSingle timeout value
 *                          FSK : timeout in number of bytes
 *                          LoRa: timeout in symbols
 * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * \param [IN] payloadLen   Sets payload length when fixed length is used
 * \param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
 * \param [IN] FreqHopOn    Enables disables the intra-packet frequency hopping
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: OFF, 1: ON]
 * \param [IN] HopPeriod    Number of symbols between each hop
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: Number of symbols
 * \param [IN] iqInverted   Inverts IQ signals (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: not inverted, 1: inverted]
 * \param [IN] rxContinuous Sets the reception in continuous mode
 *                          [false: single mode, true: continuous mode]
 */
static void RadioSetRxConfig( RadioModems_t modem, uint32_t bandwidth, uint32_t datarate, uint8_t coderate,
                              uint32_t bandwidthAfc, uint16_t preambleLen, uint16_t symbTimeout, bool fixLen,
                              uint8_t payloadLen, bool crcOn, bool FreqHopOn, uint8_t HopPeriod, bool iqInverted,
                              bool rxContinuous );

/*!
 * \brief Sets the transmission parameters
 *
 * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
 * \param [IN] power        Sets the output power [dBm]
 * \param [IN] fdev         Sets the frequency deviation (FSK only)
 *                          FSK : [Hz]
 *                          LoRa: 0
 * \param [IN] bandwidth    Sets the bandwidth (LoRa only)
 *                          FSK : 0
 *                          LoRa: [0: 125 kHz, 1: 250 kHz,
 *                                 2: 500 kHz, 3: Reserved]
 * \param [IN] datarate     Sets the Datarate
 *                          FSK : 600..300000 bits/s
 *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
 *                                10: 1024, 11: 2048, 12: 4096  chips]
 * \param [IN] coderate     Sets the coding rate (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * \param [IN] preambleLen  Sets the preamble length
 *                          FSK : Number of bytes
 *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
 * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * \param [IN] crcOn        Enables disables the CRC [0: OFF, 1: ON]
 * \param [IN] FreqHopOn    Enables disables the intra-packet frequency hopping
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: OFF, 1: ON]
 * \param [IN] HopPeriod    Number of symbols between each hop
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: Number of symbols
 * \param [IN] iqInverted   Inverts IQ signals (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: not inverted, 1: inverted]
 * \param [IN] timeout      Transmission timeout [ms]
 */
static void RadioSetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev, uint32_t bandwidth, uint32_t datarate,
                              uint8_t coderate, uint16_t preambleLen, bool fixLen, bool crcOn, bool FreqHopOn,
                              uint8_t HopPeriod, bool iqInverted, uint32_t timeout );

/*!
 * \brief Checks if the given RF frequency is supported by the hardware
 *
 * \param [IN] frequency RF frequency to be checked
 * \retval isSupported [true: supported, false: unsupported]
 */
static bool RadioCheckRfFrequency( uint32_t frequency );

/*!
 * \brief Computes the packet time on air in ms for the given payload
 *
 * \Remark Can only be called once SetRxConfig or SetTxConfig have been called
 *
 * \param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
 * \param [IN] bandwidth    Sets the bandwidth
 *                          FSK : >= 2600 and <= 250000 Hz
 *                          LoRa: [0: 125 kHz, 1: 250 kHz,
 *                                 2: 500 kHz, 3: Reserved]
 * \param [IN] datarate     Sets the Datarate
 *                          FSK : 600..300000 bits/s
 *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
 *                                10: 1024, 11: 2048, 12: 4096  chips]
 * \param [IN] coderate     Sets the coding rate (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * \param [IN] preambleLen  Sets the Preamble length
 *                          FSK : Number of bytes
 *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
 * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * \param [IN] payloadLen   Sets payload length when fixed length is used
 * \param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
 *
 * \retval airTime        Computed airTime (ms) for the given packet payload length
 */
static uint32_t RadioTimeOnAir( RadioModems_t modem, uint32_t bandwidth, uint32_t datarate, uint8_t coderate,
                                uint16_t preambleLen, bool fixLen, uint8_t payloadLen, bool crcOn );

/*!
 * \brief Sends the buffer of size. Prepares the packet to be sent and sets
 *        the radio in transmission
 *
 * \param [IN]: buffer     Buffer pointer
 * \param [IN]: size       Buffer size
 */
static void RadioSend( uint8_t* buffer, uint8_t size );

/*!
 * \brief Sets the radio in sleep mode
 */
static void RadioSleep( void );

/*!
 * \brief Sets the radio in standby mode
 */
static void RadioStandby( void );

/*!
 * \brief Sets the radio in reception mode for the given time
 * \param [IN] timeout Reception timeout [ms]
 *                     [0: continuous, others timeout]
 */
static void RadioRx( uint32_t timeout );

/*!
 * \brief Start a Channel Activity Detection
 */
static void RadioStartCad( void );

/*!
 * \brief Sets the radio in continuous wave transmission mode
 *
 * \param [IN]: freq       Channel RF frequency
 * \param [IN]: power      Sets the output power [dBm]
 * \param [IN]: time       Transmission mode timeout [s]
 */
static void RadioSetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time );

/*!
 * \brief Reads the current RSSI value
 *
 * \retval rssiValue Current RSSI value in [dBm]
 */
static int16_t RadioRssi( RadioModems_t modem );

/*!
 * \brief Writes the radio register at the specified address
 *
 * \param [IN]: addr Register address
 * \param [IN]: data New register value
 */
static void RadioWrite( uint32_t addr, uint8_t data );

/*!
 * \brief Reads the radio register at the specified address
 *
 * \param [IN]: addr Register address
 * \retval data Register value
 */
static uint8_t RadioRead( uint32_t addr );

/*!
 * \brief Writes multiple radio registers starting at address
 *
 * \param [IN] addr   First Radio register address
 * \param [IN] buffer Buffer containing the new register's values
 * \param [IN] size   Number of registers to be written
 */
static void RadioWriteBuffer( uint32_t addr, uint8_t* buffer, uint8_t size );

/*!
 * \brief Reads multiple radio registers starting at address
 *
 * \param [IN] addr First Radio register address
 * \param [OUT] buffer Buffer where to copy the registers data
 * \param [IN] size Number of registers to be read
 */
static void RadioReadBuffer( uint32_t addr, uint8_t* buffer, uint8_t size );

/*!
 * \brief Sets the maximum payload length.
 *
 * \param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
 * \param [IN] max        Maximum payload length in bytes
 */
static void RadioSetMaxPayloadLength( RadioModems_t modem, uint8_t max );

/*!
 * \brief Sets the network to public or private. Updates the sync byte.
 *
 * \remark Applies to LoRa modem only
 *
 * \param [IN] enable if true, it enables a public network
 */
static void RadioSetPublicNetwork( bool enable );

/*!
 * \brief Gets the time required for the board plus radio to get out of sleep.[ms]
 *
 * \retval time Radio plus board wakeup time in ms.
 */
static uint32_t RadioGetWakeupTime( void );

/*!
 * \brief Setup the radio GFSK modem parameters
 *
 * \param [in] context Chip implementation context
 * \param [in] params  GFSK parameters to be applied
 *
 * \retval status Operation status
 */
static ral_status_t radio_setup_gfsk( ral_t* context, radio_gfsk_params_t* params );

/*!
 * \brief Setup the radio LoRa modem parameters
 *
 * \param [in] context Chip implementation context
 * \param [in] params  LoRa parameters to be applied
 *
 * \retval status Operation status
 */
static ral_status_t radio_setup_lora( ral_t* context, radio_lora_params_t* params );

/*!
 * \brief Process radio irq
 */
static void RadioIrqProcess( void );

#if !defined( SX127X )
/*!
 * \brief Sets the radio in reception mode with Max LNA gain for the given time
 * \param [IN] timeout Reception timeout [ms]
 *                     [0: continuous, others timeout]
 */
static void RadioRxBoosted( uint32_t timeout );

/*!
 * \brief Sets the Rx duty cycle management parameters
 *
 * \param [in]  rxTime        Structure describing reception timeout value
 * \param [in]  sleepTime     Structure describing sleep timeout value
 */
static void RadioSetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime );
#endif

/*!
 * \brief IRQ callback
 */
static void RadioOnDioIrq( void* context );

/*!
 * \brief Tx timeout timer callback
 */
static void RadioOnTxTimeoutIrq( void* context );

/*!
 * \brief Rx timeout timer callback
 */
static void RadioOnRxTimeoutIrq( void* context );

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio = {
    RadioInit,
    RadioGetStatus,
    RadioSetModem,
    RadioSetChannel,
    RadioIsChannelFree,
    RadioRandom,
    RadioSetRxConfig,
    RadioSetTxConfig,
    RadioCheckRfFrequency,
    RadioTimeOnAir,
    RadioSend,
    RadioSleep,
    RadioStandby,
    RadioRx,
    RadioStartCad,
    RadioSetTxContinuousWave,
    RadioRssi,
    RadioWrite,
    RadioRead,
    RadioWriteBuffer,
    RadioReadBuffer,
    RadioSetMaxPayloadLength,
    RadioSetPublicNetwork,
    RadioGetWakeupTime,
    RadioIrqProcess,
#if defined( SX126X ) || defined( LR1110 )
    // Available on LR1110 and SX126x
    RadioRxBoosted,
    RadioSetRxDutyCycle,
#elif defined( SX127X )
    // Not available on SX127x
    NULL,
    NULL,
#endif
};

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

static void RadioInit( RadioEvents_t* events )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    // Initialize variables
    RadioEvents = events;

    radio_context->radio_params.is_image_calibrated = false;  // Force image calibration
    radio_context->radio_params.max_payload_length  = 0xFF;
    radio_context->radio_params.tx_timeout_in_ms    = 0;
    radio_context->radio_params.rx_timeout_in_ms    = 0;
    radio_context->radio_params.is_rx_continuous    = false;
    radio_context->radio_params.is_irq_fired        = false;

    // Initialize driver timeout timers
    TimerInit( &tx_timeout_timer, RadioOnTxTimeoutIrq );
    TimerInit( &rx_timeout_timer, RadioOnRxTimeoutIrq );

    // Initialize radio driver
    radio_board_init( ral_context, RadioOnDioIrq );
}

static RadioState_t RadioGetStatus( void )
{
    switch( radio_board_get_operating_mode( ) )
    {
    case RADIO_BOARD_OP_MODE_TX:
        return RF_TX_RUNNING;
    case RADIO_BOARD_OP_MODE_RX:
    case RADIO_BOARD_OP_MODE_RX_C:
    case RADIO_BOARD_OP_MODE_RX_DC:
        return RF_RX_RUNNING;
    case RADIO_BOARD_OP_MODE_CAD:
        return RF_CAD;
    default:
        return RF_IDLE;
    }
}

static void RadioSetModem( RadioModems_t modem )
{
    ral_t* ral_context = radio_board_get_ral_context_reference( );

    switch( modem )
    {
    default:
    case MODEM_FSK:
        ral_set_pkt_type( ral_context, RAL_PKT_TYPE_GFSK );
        break;
    case MODEM_LORA:
        ral_set_pkt_type( ral_context, RAL_PKT_TYPE_LORA );
        break;
    }
}

static void RadioSetChannel( uint32_t freq )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.rf_freq_in_hz = freq;
}

static bool RadioIsChannelFree( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime )
{
    ral_t*              ral_context  = radio_board_get_ral_context_reference( );
    bool                status       = true;
    uint32_t            start_time   = 0;
    int16_t             rssi         = 0;
    radio_gfsk_params_t radio_params = {
        .rf_freq_in_hz           = freq,
        .mod_params.br_in_bps    = 600,
        .mod_params.fdev_in_hz   = 0,
        .mod_params.bw_dsb_in_hz = rxBandwidth << 1,
        .mod_params.pulse_shape  = RAL_GFSK_PULSE_SHAPE_BT_1,
    };

    radio_setup_gfsk( ral_context, &radio_params );
    ral_set_dio_irq_params( ral_context, RAL_IRQ_NONE );
    ral_set_rx( ral_context, RAL_RX_TIMEOUT_CONTINUOUS_MODE );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX_C );
    DelayMs( 1 );
    start_time = TimerGetCurrentTime( );
    // Perform carrier sense for maxCarrierSenseTime
    while( TimerGetElapsedTime( start_time ) < maxCarrierSenseTime )
    {
        ral_get_rssi_inst( ral_context, &rssi );
        if( rssi > rssiThresh )
        {
            status = false;
            break;
        }
    }
    status = ral_set_sleep( ral_context, true );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_SLEEP );
    return status;
}

static uint32_t RadioRandom( void )
{
    ral_t*   ral_context = radio_board_get_ral_context_reference( );
    uint32_t rnd         = 0;

    // Set operating mode to standby
    ral_set_standby( ral_context, RAL_STANDBY_CFG_RC );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    // Set a valid packet type
    ral_set_pkt_type( ral_context, RAL_PKT_TYPE_LORA );
    // Disable all radio interrupts
    ral_set_dio_irq_params( ral_context, RAL_IRQ_NONE );
    ral_get_random_numbers( ral_context, &rnd, 1 );
    return rnd;
}

static void RadioSetRxConfig( RadioModems_t modem, uint32_t bandwidth, uint32_t datarate, uint8_t coderate,
                              uint32_t bandwidthAfc, uint16_t preambleLen, uint16_t symbTimeout, bool fixLen,
                              uint8_t payloadLen, bool crcOn, bool freqHopOn, uint8_t hopPeriod, bool iqInverted,
                              bool rxContinuous )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.is_rx_continuous = rxContinuous;
    if( rxContinuous == true )
    {
        symbTimeout = 0;
    }
    if( fixLen == true )
    {
        radio_context->radio_params.max_payload_length = payloadLen;
    }
    else
    {
        radio_context->radio_params.max_payload_length = 0xFF;
    }

    switch( modem )
    {
    case MODEM_FSK:
    {
        radio_context->radio_params.rx_timeout_in_ms = ( uint32_t ) symbTimeout * 8000UL / datarate;

        radio_gfsk_params_t radio_params = { 
            .rf_freq_in_hz = radio_context->radio_params.rf_freq_in_hz,
            .tx_rf_pwr_in_dbm = 0,
            .mod_params ={
                .br_in_bps    = datarate,
                .fdev_in_hz   = 0,
                .bw_dsb_in_hz = bandwidth << 1,
                .pulse_shape  = RAL_GFSK_PULSE_SHAPE_BT_1,
            },
            .pkt_params ={
                .preamble_len_in_bits  = ( preambleLen << 3 ),
                .preamble_detector     = RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS,
                .address_filtering     = RAL_GFSK_ADDRESS_FILTERING_DISABLE,
                .sync_word_len_in_bits = 3 << 3,
                .header_type           = ( fixLen == true ) ? RAL_GFSK_PKT_FIX_LEN : RAL_GFSK_PKT_VAR_LEN,
                .pld_len_in_bytes      = radio_context->radio_params.max_payload_length,
                .crc_type              = ( crcOn ) ? RAL_GFSK_CRC_2_BYTES_INV : RAL_GFSK_CRC_OFF,
                .dc_free               = RAL_GFSK_DC_FREE_WHITENING,
            },
            .sync_word      = (uint8_t[] ){ 0xC1, 0x94, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00 },
            .crc_polynomial = 0x1021,
            .crc_seed       = 0x1D0F,
            .whitening_seed = 0x01FF,
        };
        radio_setup_gfsk( ral_context, &radio_params );
        break;
    }
    case MODEM_LORA:
    {
        radio_lora_params_t radio_params = {
            .rf_freq_in_hz = radio_context->radio_params.rf_freq_in_hz,
            .tx_rf_pwr_in_dbm = 0,
            .mod_params = {
                .sf = ( ral_lora_sf_t ) datarate,
                .bw = ( ral_lora_bw_t ) Bandwidths[bandwidth],
                .cr = ( ral_lora_cr_t ) coderate,
                .ldro = ral_compute_lora_ldro( ( ral_lora_sf_t ) datarate,  ( ral_lora_bw_t )Bandwidths[bandwidth] ),
            },
            .pkt_params = {
                .preamble_len_in_symb = preambleLen,
                .header_type      = ( fixLen == true ) ? RAL_LORA_PKT_IMPLICIT : RAL_LORA_PKT_EXPLICIT,
                .pld_len_in_bytes = radio_context->radio_params.max_payload_length,
                .crc_is_on        = crcOn,
                .invert_iq_is_on  = iqInverted,
            },
            .symb_nb_timeout = symbTimeout,
            .sync_word       = radio_context->radio_params.is_public_network ? 0x34 : 0x12,
        };
        radio_setup_lora( ral_context, &radio_params );

        // Timeout Max, Timeout handled directly in SetRx function
        radio_context->radio_params.rx_timeout_in_ms = 0;  // Rx single mode
        break;
    }
    }
}

static void RadioSetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev, uint32_t bandwidth, uint32_t datarate,
                              uint8_t coderate, uint16_t preambleLen, bool fixLen, bool crcOn, bool freqHopOn,
                              uint8_t hopPeriod, bool iqInverted, uint32_t timeout )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.tx_timeout_in_ms = timeout;
    switch( modem )
    {
    case MODEM_FSK:
    {
        radio_gfsk_params_t radio_params = {
            .rf_freq_in_hz = radio_context->radio_params.rf_freq_in_hz,
            .tx_rf_pwr_in_dbm = radio_context->radio_params.tx_rf_pwr_in_dbm = power,
            .mod_params ={
                .br_in_bps    = datarate,
                .fdev_in_hz   = fdev,
                .bw_dsb_in_hz = bandwidth << 1,
                .pulse_shape  = RAL_GFSK_PULSE_SHAPE_BT_1,
            },
            .pkt_params ={
                .preamble_len_in_bits  = ( preambleLen << 3 ),
                .preamble_detector     = RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS,
                .sync_word_len_in_bits = 3 << 3,
                .address_filtering     = RAL_GFSK_ADDRESS_FILTERING_DISABLE,
                .header_type           = ( fixLen == true ) ? RAL_GFSK_PKT_FIX_LEN : RAL_GFSK_PKT_VAR_LEN,
                .pld_len_in_bytes      =  radio_context->radio_params.max_payload_length,
                .crc_type              = ( crcOn ) ? RAL_GFSK_CRC_2_BYTES_INV : RAL_GFSK_CRC_OFF,
                .dc_free               = RAL_GFSK_DC_FREE_WHITENING,
            },
            .sync_word      = (uint8_t[] ){ 0xC1, 0x94, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00 },
            .crc_polynomial = 0x1021,
            .crc_seed       = 0x1D0F,
            .whitening_seed = 0x01FF,
        };
        radio_setup_gfsk( ral_context, &radio_params );
        break;
    }
    case MODEM_LORA:
    {
        radio_lora_params_t radio_params = {
            .rf_freq_in_hz = radio_context->radio_params.rf_freq_in_hz,
            .tx_rf_pwr_in_dbm = radio_context->radio_params.tx_rf_pwr_in_dbm = power,
            .mod_params = {
                .sf = ( ral_lora_sf_t ) datarate,
                .bw = ( ral_lora_bw_t ) Bandwidths[bandwidth],
                .cr = ( ral_lora_cr_t ) coderate,
                .ldro = ral_compute_lora_ldro( ( ral_lora_sf_t ) datarate,  ( ral_lora_bw_t )Bandwidths[bandwidth] ),
            },
            .pkt_params = {
                .preamble_len_in_symb = preambleLen,
                .header_type      = ( fixLen == true ) ? RAL_LORA_PKT_IMPLICIT : RAL_LORA_PKT_EXPLICIT,
                .pld_len_in_bytes = radio_context->radio_params.max_payload_length,
                .crc_is_on        = crcOn,
                .invert_iq_is_on  = iqInverted,
            },
            .symb_nb_timeout = 0,
            .sync_word       = radio_context->radio_params.is_public_network ? 0x34 : 0x12,
        };
        radio_setup_lora( ral_context, &radio_params );
        break;
    }
    }
}

static bool RadioCheckRfFrequency( uint32_t frequency )
{
    return true;
}

static uint32_t RadioTimeOnAir( RadioModems_t modem, uint32_t bandwidth, uint32_t datarate, uint8_t coderate,
                                uint16_t preambleLen, bool fixLen, uint8_t payloadLen, bool crcOn )
{
    ral_t* ral_context = radio_board_get_ral_context_reference( );

    if( modem == MODEM_FSK )
    {  // MODEM_FSK
        ral_gfsk_mod_params_t mod_params = {
            .br_in_bps = datarate,
        };
        ral_gfsk_pkt_params_t pkt_params = {
            .preamble_len_in_bits = preambleLen << 3,
            .header_type          = ( fixLen == true ) ? RAL_GFSK_PKT_FIX_LEN : RAL_GFSK_PKT_VAR_LEN,
            .pld_len_in_bytes     = payloadLen,
            .crc_type             = ( crcOn == true ) ? RAL_GFSK_CRC_2_BYTES_INV : RAL_GFSK_CRC_OFF,
        };
        return ral_get_gfsk_time_on_air_in_ms( ral_context, &pkt_params, &mod_params );
    }
    else
    {  // MODEM_LORA
        ral_lora_mod_params_t mod_params = {
            .sf   = ( ral_lora_sf_t ) datarate,
            .bw   = ( ral_lora_bw_t ) Bandwidths[bandwidth],
            .cr   = ( ral_lora_cr_t ) coderate,
            .ldro = ral_compute_lora_ldro( ( ral_lora_sf_t ) datarate, ( ral_lora_bw_t ) Bandwidths[bandwidth] ),
        };
        ral_lora_pkt_params_t pkt_params = {
            .preamble_len_in_symb = preambleLen,
            .header_type          = ( fixLen == true ) ? RAL_LORA_PKT_IMPLICIT : RAL_LORA_PKT_EXPLICIT,
            .pld_len_in_bytes     = payloadLen,
            .crc_is_on            = crcOn,
        };
        return ral_get_lora_time_on_air_in_ms( ral_context, &pkt_params, &mod_params );
    }
}

static void RadioSend( uint8_t* buffer, uint8_t size )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.max_payload_length = size;
    ral_set_pkt_payload( ral_context, buffer, size );
    // RAL_IRQ_TX_DONE | RAL_IRQ_RX_TIMEOUT
    ral_set_dio_irq_params( ral_context, RAL_IRQ_TX_DONE );
    radio_board_set_ant_switch( true );
    ral_set_tx_cfg( ral_context, radio_context->radio_params.tx_rf_pwr_in_dbm,
                    radio_context->radio_params.rf_freq_in_hz );
    ral_set_tx( ral_context );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_TX );
    TimerSetValue( &tx_timeout_timer, radio_context->radio_params.tx_timeout_in_ms );
    TimerStart( &tx_timeout_timer );
}

static void RadioSleep( void )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    ral_set_sleep( ral_context, true );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_SLEEP );
    radio_board_set_ant_switch( false );
    radio_board_stop_radio_tcxo( );
    DelayMs( 2 );
    // Force image calibration
    radio_context->radio_params.is_image_calibrated = false;
}

static void RadioStandby( void )
{
    ral_t* ral_context = radio_board_get_ral_context_reference( );

    radio_board_start_radio_tcxo( );
    ral_set_standby( ral_context, RAL_STANDBY_CFG_RC );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    radio_board_set_ant_switch( false );
}

static void RadioRx( uint32_t timeout )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_board_start_radio_tcxo( );
    radio_board_set_ant_switch( false );
    // RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT, RAL_IRQ_RX_CRC_ERROR
    ral_set_dio_irq_params( ral_context, RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT | RAL_IRQ_RX_CRC_ERROR );
    ral_cfg_rx_boosted( ral_context, false );
    if( timeout != 0 )
    {
        TimerSetValue( &rx_timeout_timer, timeout );
        TimerStart( &rx_timeout_timer );
    }
    if( radio_context->radio_params.is_rx_continuous == true )
    {
        ral_set_rx( ral_context, RAL_RX_TIMEOUT_CONTINUOUS_MODE );
        radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX_C );
    }
    else
    {
        ral_set_rx( ral_context, radio_context->radio_params.rx_timeout_in_ms );
        radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX );
    }
}

#if !defined( SX127X )
static void RadioRxBoosted( uint32_t timeout )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_board_start_radio_tcxo( );
    radio_board_set_ant_switch( false );
    // RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT, RAL_IRQ_RX_CRC_ERROR
    ral_set_dio_irq_params( ral_context, RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT | RAL_IRQ_RX_CRC_ERROR );
    ral_cfg_rx_boosted( ral_context, true );
    if( timeout != 0 )
    {
        TimerSetValue( &rx_timeout_timer, timeout );
        TimerStart( &rx_timeout_timer );
    }
    if( radio_context->radio_params.is_rx_continuous == true )
    {
        ral_set_rx( ral_context, RAL_RX_TIMEOUT_CONTINUOUS_MODE );
        radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX_C );
    }
    else
    {
        ral_set_rx( ral_context, radio_context->radio_params.rx_timeout_in_ms );
        radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX );
    }
}

static void RadioSetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime )
{
    ral_t* ral_context = radio_board_get_ral_context_reference( );

    radio_board_start_radio_tcxo( );
    radio_board_set_ant_switch( false );
    ral_set_rx_duty_cycle( ral_context, rxTime, sleepTime );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_RX_DC );
}
#endif

static void RadioStartCad( void )
{
    ral_t* ral_context = radio_board_get_ral_context_reference( );

    radio_board_start_radio_tcxo( );
    radio_board_set_ant_switch( false );
    ral_set_lora_cad( ral_context );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_CAD );
}

static void RadioSetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time )
{
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );
    uint32_t         timeout       = ( uint32_t ) time * 1000;

    radio_board_start_radio_tcxo( );
    radio_board_set_ant_switch( true );
    if( radio_context->radio_params.is_image_calibrated == false )
    {
        uint16_t freq_in_mhz = freq / 1000000;

        ral_cal_img( ral_context, freq_in_mhz, freq_in_mhz );
        radio_context->radio_params.is_image_calibrated = true;
    }
    ral_set_rf_freq( ral_context, freq );
    ral_set_tx_cfg( ral_context, power, freq );
    ral_set_tx_cw( ral_context );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_TX );

    TimerSetValue( &tx_timeout_timer, timeout );
    TimerStart( &tx_timeout_timer );
}

static int16_t RadioRssi( RadioModems_t modem )
{
    ral_t*  ral_context = radio_board_get_ral_context_reference( );
    int16_t rssi        = 0;
    ral_get_rssi_inst( ral_context, &rssi );
    return rssi;
}

static void RadioWrite( uint32_t addr, uint8_t data )
{
}

static uint8_t RadioRead( uint32_t addr )
{
    uint8_t data = 0;
    return data;
}

static void RadioWriteBuffer( uint32_t addr, uint8_t* buffer, uint8_t size )
{
}

static void RadioReadBuffer( uint32_t addr, uint8_t* buffer, uint8_t size )
{
}

static void RadioSetMaxPayloadLength( RadioModems_t modem, uint8_t max )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.max_payload_length = max;
}

static void RadioSetPublicNetwork( bool enable )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.is_public_network = enable;
}

static uint32_t RadioGetWakeupTime( void )
{
    return radio_board_get_tcxo_wakeup_time_in_ms( ) + 3;
}

static ral_status_t radio_setup_gfsk( ral_t* context, radio_gfsk_params_t* params )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    status = ral_set_standby( context, RAL_STANDBY_CFG_RC );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    // Note: For SX127x radios this API returns unsupported feature.
    status = ral_stop_timer_on_preamble( context, false );
    if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
    {
        return status;
    }
    status = ral_set_pkt_type( context, RAL_PKT_TYPE_GFSK );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    if( radio_context->radio_params.is_image_calibrated == false )
    {
        uint16_t freq_in_mhz = params->rf_freq_in_hz / 1000000;

        status = ral_cal_img( ral_context, freq_in_mhz, freq_in_mhz );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
        radio_context->radio_params.is_image_calibrated = true;
    }
    status = ral_set_rf_freq( context, params->rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_tx_cfg( context, params->tx_rf_pwr_in_dbm, params->rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_gfsk_mod_params( context, &params->mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_gfsk_pkt_params( context, &params->pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    if( params->pkt_params.crc_type != RAL_GFSK_CRC_OFF )
    {
        // Note: For SX127x radios this API returns unsupported feature.
        status = ral_set_gfsk_crc_params( context, params->crc_seed, params->crc_polynomial );
        if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
        {
            return status;
        }
    }
    status = ral_set_gfsk_sync_word( context, params->sync_word, ( params->pkt_params.sync_word_len_in_bits + 7 ) / 8 );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    if( params->pkt_params.dc_free == RAL_GFSK_DC_FREE_WHITENING )
    {
        // Note: For SX127x radios this API returns unsupported feature.
        status = ral_set_gfsk_whitening_seed( context, params->whitening_seed );
        if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
        {
            return status;
        }
    }
    return status;
}

static ral_status_t radio_setup_lora( ral_t* context, radio_lora_params_t* params )
{
    ral_status_t     status        = RAL_STATUS_ERROR;
    ral_t*           ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    status = ral_set_standby( context, RAL_STANDBY_CFG_RC );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_STDBY );
    // Note: For SX127x radios this API returns unsupported feature.
    status = ral_stop_timer_on_preamble( context, false );
    if( ( status != RAL_STATUS_OK ) && ( status != RAL_STATUS_UNSUPPORTED_FEATURE ) )
    {
        return status;
    }
    status = ral_set_lora_symb_nb_timeout( context, params->symb_nb_timeout );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_pkt_type( context, RAL_PKT_TYPE_LORA );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    if( radio_context->radio_params.is_image_calibrated == false )
    {
        uint16_t freq_in_mhz = params->rf_freq_in_hz / 1000000;

        status = ral_cal_img( ral_context, freq_in_mhz, freq_in_mhz );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
        radio_context->radio_params.is_image_calibrated = true;
    }
    status = ral_set_rf_freq( context, params->rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_tx_cfg( context, params->tx_rf_pwr_in_dbm, params->rf_freq_in_hz );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_lora_mod_params( context, &params->mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_lora_pkt_params( context, &params->pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    status = ral_set_lora_sync_word( context, params->sync_word );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }
    return status;
}

static void RadioIrqProcess( void )
{
    ral_t*                       ral_context   = radio_board_get_ral_context_reference( );
    radio_context_t*             radio_context = radio_board_get_radio_context_reference( );
    ral_irq_t                    irq_flags     = RAL_IRQ_NONE;
    radio_board_operating_mode_t op_mode       = radio_board_get_operating_mode( );

    // Check if there is an interrupt pending
    if( radio_context->radio_params.is_irq_fired == false )
    {
        return;
    }

    ral_get_and_clear_irq_status( ral_context, &irq_flags );
    CRITICAL_SECTION_BEGIN( );
    // Clear IRQ flag
    radio_context->radio_params.is_irq_fired = false;
    // Check if DIO1 pin is High. If it is the case revert ral_context.radio_params.is_irq_fired to true
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
            sx126x_stop_rtc( ral_context->context );
            // WORKAROUND END
#endif
        }
    }
    // Process radio irq flags
    if( ( irq_flags & RAL_IRQ_TX_DONE ) == RAL_IRQ_TX_DONE )
    {
        TimerStop( &tx_timeout_timer );
        if( ( RadioEvents != NULL ) && ( RadioEvents->TxDone != NULL ) )
        {
            RadioEvents->TxDone( );
        }
    }
    if( ( ( irq_flags & RAL_IRQ_RX_DONE ) == RAL_IRQ_RX_DONE ) &&
        ( ( irq_flags & RAL_IRQ_RX_CRC_ERROR ) != RAL_IRQ_RX_CRC_ERROR ) )
    {
        TimerStop( &rx_timeout_timer );

        ral_pkt_type_t pkt_type;
        uint16_t       pld_len_in_bytes = 0;

        ral_get_pkt_payload( ral_context, radio_context->radio_params.max_payload_length,
                             radio_context->radio_params.buffer, &pld_len_in_bytes );

        ral_get_pkt_type( ral_context, &pkt_type );
        if( pkt_type == RAL_PKT_TYPE_LORA )
        {
            ral_lora_rx_pkt_status_t lora_rx_pkt_status;

            ral_get_lora_rx_pkt_status( ral_context, &lora_rx_pkt_status );
            if( ( RadioEvents != NULL ) && ( RadioEvents->RxDone != NULL ) )
            {
                RadioEvents->RxDone( radio_context->radio_params.buffer, pld_len_in_bytes,
                                     lora_rx_pkt_status.rssi_pkt_in_dbm, lora_rx_pkt_status.snr_pkt_in_db );
            }
        }
        else
        {
            ral_gfsk_rx_pkt_status_t gfsk_rx_pkt_status;

            ral_get_gfsk_rx_pkt_status( ral_context, &gfsk_rx_pkt_status );
            if( ( RadioEvents != NULL ) && ( RadioEvents->RxDone != NULL ) )
            {
                RadioEvents->RxDone( radio_context->radio_params.buffer, pld_len_in_bytes,
                                     gfsk_rx_pkt_status.rssi_avg_in_dbm, 0 );
            }
        }
    }
    if( ( irq_flags & RAL_IRQ_RX_CRC_ERROR ) == RAL_IRQ_RX_CRC_ERROR )
    {
        if( ( RadioEvents != NULL ) && ( RadioEvents->RxError != NULL ) )
        {
            RadioEvents->RxError( );
        }
    }
    if( ( ( irq_flags & RAL_IRQ_RX_TIMEOUT ) == RAL_IRQ_RX_TIMEOUT ) ||
        ( ( irq_flags & RAL_IRQ_RX_HDR_ERROR ) == RAL_IRQ_RX_HDR_ERROR ) )
    {
        if( op_mode == RADIO_BOARD_OP_MODE_TX )
        {
            TimerStop( &tx_timeout_timer );

            if( ( RadioEvents != NULL ) && ( RadioEvents->TxTimeout != NULL ) )
            {
                RadioEvents->TxTimeout( );
            }
        }
        else
        {
            TimerStop( &rx_timeout_timer );
            if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
            {
                RadioEvents->RxTimeout( );
            }
        }
    }
    if( ( irq_flags & RAL_IRQ_CAD_DONE ) == RAL_IRQ_CAD_DONE )
    {
        if( ( RadioEvents != NULL ) && ( RadioEvents->CadDone != NULL ) )
        {
            RadioEvents->CadDone( ( ( irq_flags & RAL_IRQ_CAD_OK ) == RAL_IRQ_CAD_OK ) );
        }
    }
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void RadioOnDioIrq( void* context )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->radio_params.is_irq_fired = true;
}

static void RadioOnTxTimeoutIrq( void* context )
{
#if( SX127X )
    sx127x_t* sx127x_context = radio_board_get_sx127x_context_reference( );

    sx127x_tx_timeout_irq_workaround( sx127x_context );
    radio_board_set_operating_mode( RADIO_BOARD_OP_MODE_SLEEP );
#endif
    if( ( RadioEvents != NULL ) && ( RadioEvents->TxTimeout != NULL ) )
    {
        RadioEvents->TxTimeout( );
    }
}

static void RadioOnRxTimeoutIrq( void* context )
{
    if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
    {
        RadioEvents->RxTimeout( );
    }
}

/* --- EOF ------------------------------------------------------------------ */
