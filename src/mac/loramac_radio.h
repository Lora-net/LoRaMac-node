/*!
 * \file  loramac_radio.h
 *
 * \brief LoRaMac stack radio interface definition
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright Stackforce 2021. All rights reserved.
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
#ifndef LORAMAC_RADIO_H
#define LORAMAC_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>
#include "ral.h"

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

/*!
 * API return status
 */
typedef enum loramac_radio_status_e
{
    LORAMAC_RADIO_STATUS_OK    = 0,
    LORAMAC_RADIO_STATUS_ERROR = 3,
} loramac_radio_status_t;

/*!
 * Radio GFSK configuration parameters
 */
typedef struct loramac_radio_gfsk_cfg_params_s
{
    uint32_t rf_freq_in_hz;            //!< Radio Channel RF frequency
    int8_t   tx_rf_pwr_in_dbm;         //!< Radio RF output power
    uint32_t br_in_bps;                //!< GFSK bitrate
    uint32_t fdev_in_hz;               //!< GFSK frequency deviation
    uint32_t bw_dsb_in_hz;             //!< GFSK bandwidth
    uint16_t preamble_len_in_bits;     //!< GFSK preamble length in bits
    uint8_t  sync_word_len_in_bits;    //!< GFSK sync word length in bits
    bool     is_pkt_len_fixed;         //!< GFSK packet length type
    uint8_t  pld_len_in_bytes;         //!< GFSK payload length in bytes
    bool     is_crc_on;                //!< GFSK CRC activation
    uint32_t rx_sync_timeout_in_symb;  //!< GFSK sync word detection timeout
    bool     is_rx_continuous;         //!< Radio continuous reception mode
    uint32_t tx_timeout_in_ms;         //!< Radio tx timeout
} loramac_radio_gfsk_cfg_params_t;

/*!
 * Radio LoRa configuration parameters
 */
typedef struct loramac_radio_lora_cfg_params_s
{
    uint32_t      rf_freq_in_hz;            //!< Radio Channel RF frequency
    int8_t        tx_rf_pwr_in_dbm;         //!< Radio RF output power
    ral_lora_sf_t sf;                       //!< LoRa Spreading Factor
    ral_lora_bw_t bw;                       //!< LoRa Bandwidth
    ral_lora_cr_t cr;                       //!< LoRa Coding Rate
    uint16_t      preamble_len_in_symb;     //!< LoRa preamble length in symbols
    bool          is_pkt_len_fixed;         //!< LoRa packet length type
    uint8_t       pld_len_in_bytes;         //!< LoRa payload length in bytes
    bool          is_crc_on;                //!< LoRa CRC activation
    bool          invert_iq_is_on;          //!< LoRa IQ polarity setup
    uint32_t      rx_sync_timeout_in_symb;  //!< LoRa sync word detection timeout
    bool          is_rx_continuous;         //!< Radio continuous reception mode
    uint32_t      tx_timeout_in_ms;         //!< Radio tx timeout
} loramac_radio_lora_cfg_params_t;

/*!
 * Radio LR-FHSS configuration parameters
 */
typedef struct loramac_radio_lr_fhss_cfg_params_s
{
    int8_t               tx_rf_pwr_in_dbm;  //!< Radio RF output power
    ral_lr_fhss_params_t lr_fhss_params;    //!< LR-FHSS parameters
    uint32_t             tx_timeout_in_ms;  //!< Radio tx timeout
} loramac_radio_lr_fhss_cfg_params_t;

/*!
 * Radio is channel free configuration parameters
 */
typedef struct loramac_radio_channel_free_cfg_params_s
{
    uint32_t rf_freq_in_hz;              //! Channel RF frequency
    uint32_t rx_bw_in_hz;                //! Rx bandwidth
    int16_t  rssi_threshold_in_dbm;      //! RSSI threshold in dBm
    uint32_t max_carrier_sense_time_ms;  //! Max time in milliseconds while the RSSI is measured
} loramac_radio_channel_free_cfg_params_t;

/*!
 * Radio GFSK time on air configuration parameters
 */
typedef struct loramac_radio_gfsk_time_on_air_params_s
{
    uint32_t br_in_bps;                //!< GFSK bitrate
    uint16_t preamble_len_in_bits;     //!< GFSK preamble length in bits
    uint8_t  sync_word_len_in_bits;    //!< GFSK sync word length in bits
    bool     is_address_filtering_on;  //!< GFSK address filtering activation
    bool     is_pkt_len_fixed;         //!< GFSK packet length type
    uint8_t  pld_len_in_bytes;         //!< GFSK payload length in bytes
    bool     is_crc_on;                //!< GFSK CRC activation
} loramac_radio_gfsk_time_on_air_params_t;

/*!
 * Radio LoRa time on air configuration parameters
 */
typedef struct loramac_radio_lora_time_on_air_params_s
{
    ral_lora_sf_t sf;                    //!< LoRa Spreading Factor
    ral_lora_bw_t bw;                    //!< LoRa Bandwidth
    ral_lora_cr_t cr;                    //!< LoRa Coding Rate
    uint16_t      preamble_len_in_symb;  //!< LoRa preamble length in symbols
    bool          is_pkt_len_fixed;      //!< LoRa packet length type
    uint8_t       pld_len_in_bytes;      //!< LoRa payload length in bytes
    bool          is_crc_on;             //!< LoRa CRC activation
} loramac_radio_lora_time_on_air_params_t;

/*!
 * Radio LoRa time on air configuration parameters
 */
typedef struct loramac_radio_lr_fhss_time_on_air_params_s
{
    ral_lr_fhss_params_t lr_fhss_params;    //!< LR-FHSS parameters
    uint8_t              pld_len_in_bytes;  //!< LoRa payload length in bytes
} loramac_radio_lr_fhss_time_on_air_params_t;

/*!
 * Radio Tx continuous wave configuration parameters
 */
typedef struct loramac_radio_tx_cw_cfg_params_s
{
    uint32_t rf_freq_in_hz;     //! Channel RF frequency
    int8_t   tx_rf_pwr_in_dbm;  //! RF output power
    uint16_t timeout_in_s;      //! Timeout for transmission operation
} loramac_radio_tx_cw_cfg_params_t;

/*!
 * Radio Rx done interrupt received parameters
 */
typedef struct loramac_radio_irq_rx_done_params_s
{
    uint8_t* buffer;         //! Pointer to the buffer to be filled with received data
    uint16_t size_in_bytes;  //! Size of the received buffer
    int16_t  rssi_in_dbm;    //! RSSI value of the received packet
    int8_t   snr_in_db;      //! SNR value of the received packet
} loramac_radio_irq_rx_done_params_t;

/*!
 * \brief Interrupt callback functions
 */
typedef struct loramac_radio_irq_s
{
    /*!
     * \brief  Tx done interrupt callback prototype.
     */
    void ( *loramac_radio_irq_tx_done )( void );
    /*!
     * \brief Rx done interrupt callback prototype.
     *
     * \param [out] params  Pointer to the received parameters
     */
    void ( *loramac_radio_irq_rx_done )( loramac_radio_irq_rx_done_params_t* params );
    /*!
     * \brief Rx error interrupt callback prototype.
     */
    void ( *loramac_radio_irq_rx_error )( void );
    /*!
     * \brief  Tx timeout interrupt callback prototype.
     */
    void ( *loramac_radio_irq_tx_timeout )( void );
    /*!
     * \brief  Rx timeout interrupt callback prototype.
     */
    void ( *loramac_radio_irq_rx_timeout )( void );
} loramac_radio_irq_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/*!
 * \brief Initialize the radio
 *
 * \param [in] irq_callbacks Pointer to irq callbacks data structure
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_init( const loramac_radio_irq_t* irq_callbacks );

/*!
 * \brief Indicate if the radio is idle or not
 *
 * \returns Radio state
 */
bool loramac_radio_is_radio_idle( void );

/*!
 * \brief Configure the radio GFSK modem parameters
 *
 * \param [in] cfg_params GFSK modem configuration parameters
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_gfsk_set_cfg( const loramac_radio_gfsk_cfg_params_t* cfg_params );

/*!
 * \brief Configure the radio LoRa modem parameters
 *
 * \param [in] cfg_params LoRa modem configuration parameters
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_lora_set_cfg( const loramac_radio_lora_cfg_params_t* cfg_params );

/*!
 * \brief Configure the radio LR-FHSS modem parameters
 *
 * \param [in] cfg_params LR-FHSS modem configuration parameters
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_lr_fhss_set_cfg( const loramac_radio_lr_fhss_cfg_params_t* cfg_params );

/*!
 * \brief Transmit the buffer of size in bytes.
 *
 * \remark Prepares the packet to be sent and sets the radio in transmission
 *
 * \param [in] buffer Pointer to the buffer to be transmitted
 * \param [in] size_in_bytes Size of the buffer to be transmitted
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_transmit( const uint8_t* buffer, const uint16_t size_in_bytes );

/*!
 * \brief Set the radio in sleep mode
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_set_sleep( void );

/*!
 * \brief Set the radio in stand-by mode
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_set_standby( void );

/*!
 * \brief Set the radio in reception mode
 *
 * \param [in] timeout_in_ms  Timeout for reception operation in milliseconds
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_set_rx( const uint32_t timeout_in_ms );

/*!
 * \brief Set the radio in Tx continuous wave (RF tone).
 *
 * \param [in] cfg_params Radio Tx continuos wave configuration parameters
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_set_tx_cw( const loramac_radio_tx_cw_cfg_params_t* cfg_params );

/*!
 * \brief Indicate if the channel is free for the given time
 *
 * \param [in] cfg_params           Pointer to channel free configuration parameters
 * \param [out] is_channel_free     Indication of channel status
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_is_channel_free( const loramac_radio_channel_free_cfg_params_t* cfg_params,
                                                      bool*                                          is_channel_free );

/*!
 * \brief Set LoRaWAN network type to private or public
 *
 * \param [in] is_public_network Public network is set to true and private if false
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_set_network_type( const bool is_public_network );

/*!
 * \brief Get the time on air in millisecond for GFSK packet
 *
 * \param [in] params Pointer to GFSK time on air parameters
 *
 * \returns Time-on-air value in ms for GFSK packet
 */
uint32_t loramac_radio_gfsk_get_time_on_air_in_ms( const loramac_radio_gfsk_time_on_air_params_t* params );

/*!
 * \brief Get the time on air in millisecond for LoRa packet
 *
 * \param [in] params Pointer to LoRa time on air parameters
 *
 * \returns Time-on-air value in ms for LoRa packet
 */
uint32_t loramac_radio_lora_get_time_on_air_in_ms( const loramac_radio_lora_time_on_air_params_t* params );

/*!
 * \brief Get the time on air in millisecond for LR-FHSS packet
 *
 * \param [in] params Pointer to LR-FHSS time on air parameters
 *
 * \returns Time-on-air value in ms for LR-FHSS packet
 */
uint32_t loramac_radio_lr_fhss_get_time_on_air_in_ms( const loramac_radio_lr_fhss_time_on_air_params_t* params );

/*!
 * \brief Get the time required for the board plus radio to get out of sleep.
 *
 * \returns Radio plus board wakeup time in ms.
 */
uint32_t loramac_radio_get_wakeup_time_in_ms( void );

/*!
 * \brief Get a 32-bit random number from the radio
 *
 * \remark Radio operating mode must be set into standby.
 *
 * \param [out] random_number 32-bit random number
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_get_random_number( uint32_t* random_number );

/*!
 * \brief Process radio interrupts
 *
 * \returns Operation status
 */
loramac_radio_status_t loramac_radio_irq_process( void );

#ifdef __cplusplus
}
#endif

#endif  // LORAMAC_RADIO_H

/* --- EOF ------------------------------------------------------------------ */
