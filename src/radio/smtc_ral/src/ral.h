/**
 * @file      ral.h
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

#ifndef RAL_H__
#define RAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>
#include "ral_defs.h"
#include "ral_drv.h"
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

typedef struct ral_s
{
    const void* context;
    ral_drv_t   driver;
} ral_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/**
 * @brief Indicate whether the radio driver is capable of driving a specific part number
 *
 * @param [in] part_number Part number to be queried
 *
 * @returns true, if the driver is capable of driving part part_number
 */
static inline bool ral_handles_part( const ral_t* radio, const char* part_number )
{
    return radio->driver.handles_part( part_number );
}

/**
 * @brief Reset the radio
 *
 * @remark This function performs a hardware of the radio
 *
 * @param [in] radio  Pointer to radio data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_reset( const ral_t* radio )
{
    return radio->driver.reset( radio->context );
}

/**
 * @brief Initialize the radio
 *
 * @remark This function shall be called after a power-on or when the chip leaves a sleep mode without retention.
 *
 * @param [in] radio Pointer to radio data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_init( const ral_t* radio )
{
    return radio->driver.init( radio->context );
}

/**
 * @brief Wake up the radio from sleep
 *
 * @param [in] radio  Pointer to radio data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_wakeup( const ral_t* radio )
{
    return radio->driver.wakeup( radio->context );
}

/**
 * @brief Set the radio in sleep mode
 *
 * @param [in] radio          Pointer to radio data structure
 * @param [in] retain_config  If true, radio configuration will be restored upon wakup
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_sleep( const ral_t* radio, const bool retain_config )
{
    return radio->driver.set_sleep( radio->context, retain_config );
}

/**
 * @brief Set the chip in stand-by mode
 *
 * @param [in] radio        Pointer to radio data structure
 * @param [in] standby_cfg  Stand-by mode configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_standby( const ral_t* radio, ral_standby_cfg_t standby_cfg )
{
    return radio->driver.set_standby( radio->context, standby_cfg );
}

/**
 * @brief Set the chip in frequency synthesis mode
 *
 * @param [in] radio  Pointer to radio data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_fs( const ral_t* radio )
{
    return radio->driver.set_fs( radio->context );
}

/**
 * @brief Set the chip in transmission mode
 *
 * @param [in] radio  Pointer to radio data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_tx( const ral_t* radio )
{
    return radio->driver.set_tx( radio->context );
}

/**
 * @brief Set the chip in reception mode
 *
 * @param [in] radio          Pointer to radio data structure
 * @param [in] timeout_in_ms  Timeout for reception operation - in millisecond
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_rx( const ral_t* radio, const uint32_t timeout_in_ms )
{
    return radio->driver.set_rx( radio->context, timeout_in_ms );
}

/**
 * @brief Configure the boost mode in reception
 *
 * @param [in] radio              Pointer to radio data structure
 * @param [in] enable_boost_mode  Rx boost mode activation
 *
 * @returns Operation status
 */
static inline ral_status_t ral_cfg_rx_boosted( const ral_t* radio, const bool enable_boost_mode )
{
    return radio->driver.cfg_rx_boosted( radio->context, enable_boost_mode );
}

/**
 * @brief Set chip mode to be used after successful transmission or reception.
 *
 * @remark This setting is not taken into account during Rx Duty Cycle mode or Auto TxRx - if available.
 *
 * @param [in] radio          Pointer to radio data structure
 * @param [in] fallback_mode  Selected fallback mode
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_rx_tx_fallback_mode( const ral_t* radio, const ral_fallback_modes_t fallback_mode )
{
    return radio->driver.set_rx_tx_fallback_mode( radio->context, fallback_mode );
}

/**
 * @brief Configure the event on which the Rx timeout is stopped
 *
 * @remark The two options are:
 *   - Syncword / Header detection (default)
 *   - Preamble detection
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] enable  If true, the timer stops on Syncword / Header detection.
 *
 * @returns Operation status
 */
static inline ral_status_t ral_stop_timer_on_preamble( const ral_t* radio, const bool enable )
{
    return radio->driver.stop_timer_on_preamble( radio->context, enable );
}

/**
 * @brief Set the chip in reception mode with duty cycling
 *
 * @param [in] radio             Pointer to radio data structure
 * @param [in] rx_time_in_ms     The timeout of Rx period - in millisecond
 * @param [in] sleep_time_in_ms  The length of sleep period - in millisecond
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_rx_duty_cycle( const ral_t* radio, const uint32_t rx_time_in_ms,
                                                  const uint32_t sleep_time_in_ms )
{
    return radio->driver.set_rx_duty_cycle( radio->context, rx_time_in_ms, sleep_time_in_ms );
}

/**
 * @brief Set the chip in LoRa CAD (Channel Activity Detection) mode
 *
 * @remark The LoRa packet type shall be selected with @ref ral_set_pkt_type before this function is called.
 *
 * @param [in] radio  Pointer to radio data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_lora_cad( const ral_t* radio )
{
    return radio->driver.set_lora_cad( radio->context );
}

/**
 * @brief Set the chip in Tx continuous wave (RF tone).
 *
 * @remark A packet type shall be configured with @ref ral_set_pkt_type before using this command.
 *
 * @param [in] radio  Pointer to radio data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_tx_cw( const ral_t* radio )
{
    return radio->driver.set_tx_cw( radio->context );
}

/**
 * @brief Set the chip in Tx infinite preamble (modulated signal).
 *
 * @remark A packet type shall be configured with @ref ral_get_pkt_type before using this command.
 *
 * @param [in] radio  Pointer to radio data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_tx_infinite_preamble( const ral_t* radio )
{
    return radio->driver.set_tx_infinite_preamble( radio->context );
}

/**
 * @brief Launch a band image rejection calibration valid for all frequencies inside an interval, in MHz
 *
 * @param [in] radio         Pointer to radio data structure
 * @param [in] freq1_in_mhz  Image calibration interval lower bound, in MHz
 * @param [in] freq2_in_mhz  Image calibration interval upper bound, in MHz
 *
 * @returns Operation status
 */
static inline ral_status_t ral_cal_img( const ral_t* radio, const uint16_t freq1_in_mhz, const uint16_t freq2_in_mhz )
{
    return radio->driver.cal_img( radio->context, freq1_in_mhz, freq2_in_mhz );
}

/**
 * @brief Configure the transmission-related parameters
 *
 * @details
 *
 * @param [in] radio              Pointer to radio data structure
 * @param [in] output_pwr_in_dbm  Output power - in dBm
 * @param [in] rf_freq_in_hz      RF frequency - in Hertz
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_tx_cfg( const ral_t* radio, const int8_t output_pwr_in_dbm,
                                           const uint32_t rf_freq_in_hz )
{
    return radio->driver.set_tx_cfg( radio->context, output_pwr_in_dbm, rf_freq_in_hz );
}

/**
 * @brief Fill the radio transmission buffer with data
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] buffer  Pointer to the buffer to be transmitted
 * @param [in] size    Size of the buffer to be transmitted
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_pkt_payload( const ral_t* radio, const uint8_t* buffer, const uint16_t size )
{
    return radio->driver.set_pkt_payload( radio->context, buffer, size );
}

/**
 * @brief Fetch data from the radio reception buffer
 *
 * @param [in] radio             Pointer to radio data structure
 * @param [in] max_size_in_bytes  Size of the application buffer - in bytes
 * @param [out] buffer           Pointer to the buffer to be filled with received data
 * @param [out] size_in_bytes     Size of the received buffer - in bytes
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_pkt_payload( const ral_t* radio, uint16_t max_size_in_bytes, uint8_t* buffer,
                                                uint16_t* size_in_bytes )
{
    return radio->driver.get_pkt_payload( radio->context, max_size_in_bytes, buffer, size_in_bytes );
}

/**
 * @brief Get the current radio irq status
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [out] irq   Pointer to interrupt mask
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_irq_status( const ral_t* radio, ral_irq_t* irq )
{
    return radio->driver.get_irq_status( radio->context, irq );
}

/**
 * @brief Clear radio irq status
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] irq    Interrupts to be cleared
 *
 * @returns Operation status
 */
static inline ral_status_t ral_clear_irq_status( const ral_t* radio, const ral_irq_t irq )
{
    return radio->driver.clear_irq_status( radio->context, irq );
}

/**
 * @brief Clear any radio irq status flags that are set and returns the flags that were cleared
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [out] irq   Pointer to interrupt mask
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_and_clear_irq_status( const ral_t* radio, ral_irq_t* irq )
{
    return radio->driver.get_and_clear_irq_status( radio->context, irq );
}

/**
 * @brief Enable interrupts. If the chip has several IRQ lines, the first is used.
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] irq    Interrupt to be enabled
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_dio_irq_params( const ral_t* radio, const ral_irq_t irq )
{
    return radio->driver.set_dio_irq_params( radio->context, irq );
}

/**
 * @brief Set the RF frequency for future radio operations
 *
 * @param [in] radio       Pointer to radio data structure
 * @param [in] freq_in_hz  The frequency to set for radio operations - in Hertz
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_rf_freq( const ral_t* radio, const uint32_t freq_in_hz )
{
    return radio->driver.set_rf_freq( radio->context, freq_in_hz );
}

/**
 * @brief Set the packet type
 *
 * @param [in] radio     Pointer to radio data structure
 * @param [in] pkt_type  Packet type to set
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_pkt_type( const ral_t* radio, const ral_pkt_type_t pkt_type )
{
    return radio->driver.set_pkt_type( radio->context, pkt_type );
}

/**
 * @brief Get the current packet type
 *
 * @param [in] radio     Pointer to radio data structure
 * @param [out] pkt_type Pointer to a variable holding the packet type
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_pkt_type( const ral_t* radio, ral_pkt_type_t* pkt_type )
{
    return radio->driver.get_pkt_type( radio->context, pkt_type );
}

/**
 * @brief Set the modulation parameters for GFSK packets
 *
 * @remark The command @ref ral_set_pkt_type with @ref RAL_PKT_TYPE_GFSK parameter must be called prior to this one.
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] params  The structure of GFSK modulation configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_gfsk_mod_params( const ral_t* radio, const ral_gfsk_mod_params_t* params )
{
    return radio->driver.set_gfsk_mod_params( radio->context, params );
}

/**
 * @brief Set the packet parameters for GFSK packets
 *
 * @remark The command @ref ral_set_pkt_type with @ref RAL_PKT_TYPE_GFSK parameter must be called prior to this one.
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] params  The structure of GFSK packet configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_gfsk_pkt_params( const ral_t* radio, const ral_gfsk_pkt_params_t* params )
{
    return radio->driver.set_gfsk_pkt_params( radio->context, params );
}

/**
 * @brief Set the modulation parameters for LoRa packets
 *
 * @remark The command @ref ral_set_pkt_type with @ref RAL_PKT_TYPE_LORA parameter must be called prior to this one.
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] params  The structure of LoRa modulation configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_lora_mod_params( const ral_t* radio, const ral_lora_mod_params_t* params )
{
    return radio->driver.set_lora_mod_params( radio->context, params );
}

/**
 * @brief Set the packet parameters for LoRa packets
 *
 * @remark The command @ref ral_set_pkt_type with @ref RAL_PKT_TYPE_LORA parameter must be called prior to this one.
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] params  The structure of LoRa packet configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_lora_pkt_params( const ral_t* radio, const ral_lora_pkt_params_t* params )
{
    return radio->driver.set_lora_pkt_params( radio->context, params );
}

/**
 * @brief Set the parameters for LoRa CAD operation
 *
 * @remark The command @ref ral_set_pkt_type with @ref RAL_PKT_TYPE_LORA parameter must be called prior to this one.
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] params  The structure of LoRa CAD configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_lora_cad_params( const ral_t* radio, const ral_lora_cad_params_t* params )
{
    return radio->driver.set_lora_cad_params( radio->context, params );
}

/**
 * @brief Configure the LoRa modem to issue a RX timeout after an exact number
 * of symbols given in parameter if no LoRa modulation is detected
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] nb_of_symbs number of symbols to compute the timeout
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_lora_symb_nb_timeout( const ral_t* radio, const uint8_t nb_of_symbs )
{
    return radio->driver.set_lora_symb_nb_timeout( radio->context, nb_of_symbs );
}

/**
 * @brief Set the modulation parameters for FLRC packets
 *
 * @remark The command @ref ral_set_pkt_type with @ref RAL_PKT_TYPE_FLRC parameter must be called prior to this one.
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] params  The structure of FLRC modulation configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_flrc_mod_params( const ral_t* radio, const ral_flrc_mod_params_t* params )
{
    return radio->driver.set_flrc_mod_params( radio->context, params );
}

/**
 * @brief Set the packet parameters for FLRC packets
 *
 * @remark The command @ref ral_set_pkt_type with @ref RAL_PKT_TYPE_FLRC parameter must be called prior to this one.
 *
 * @param [in] radio   Pointer to radio data structure
 * @param [in] params  The structure of FLRC packet configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_flrc_pkt_params( const ral_t* radio, const ral_flrc_pkt_params_t* params )
{
    return radio->driver.set_flrc_pkt_params( radio->context, params );
}

/**
 * @brief Get the status of the last GFSK packet received
 *
 * @param [in] radio           Pointer to radio data structure
 * @param [out] rx_pkt_status  Pointer to a structure to store the packet status
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_gfsk_rx_pkt_status( const ral_t* radio, ral_gfsk_rx_pkt_status_t* rx_pkt_status )
{
    return radio->driver.get_gfsk_rx_pkt_status( radio->context, rx_pkt_status );
}

/**
 * @brief Get the status of the last LoRa packet received
 *
 * @param [in] radio           Pointer to radio data structure
 * @param [out] rx_pkt_status  Pointer to a structure to store the packet status
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_lora_rx_pkt_status( const ral_t* radio, ral_lora_rx_pkt_status_t* rx_pkt_status )
{
    return radio->driver.get_lora_rx_pkt_status( radio->context, rx_pkt_status );
}

/**
 * @brief Get the status of the last FLRC packet received
 *
 * @param [in] radio           Pointer to radio data structure
 * @param [out] rx_pkt_status  Pointer to a structure to store the packet status
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_flrc_rx_pkt_status( const ral_t* radio, ral_flrc_rx_pkt_status_t* rx_pkt_status )
{
    return radio->driver.get_flrc_rx_pkt_status( radio->context, rx_pkt_status );
}

/**
 * @brief Get the instantaneous RSSI value.
 *
 * @remark This function shall be called when in reception mode.
 *
 * @param [in] radio         Pointer to radio data structure
 * @param [out] rssi_in_dbm  Pointer to a variable to store the RSSI value in dBm
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_rssi_inst( const ral_t* radio, int16_t* rssi_in_dbm )
{
    return radio->driver.get_rssi_inst( radio->context, rssi_in_dbm );
}

/**
 * @brief Get the time on air in ms for LoRa transmission
 *
 * @param [in] pkt_p  Pointer to a structure holding the LoRa packet parameters
 * @param [in] mod_p  Pointer to a structure holding the LoRa modulation parameters
 *
 * @returns Time-on-air value in ms for LoRa transmission
 */
static inline uint32_t ral_get_lora_time_on_air_in_ms( const ral_t* radio, const ral_lora_pkt_params_t* pkt_p,
                                                       const ral_lora_mod_params_t* mod_p )
{
    return radio->driver.get_lora_time_on_air_in_ms( pkt_p, mod_p );
}

/**
 * @brief Get the time on air in millisecond for GFSK transmission
 *
 * @param [in] pkt_p  Pointer to a structure holding the GFSK packet parameters
 * @param [in] mod_p  Pointer to a structure holding the GFSK modulation parameters
 *
 * @returns Time-on-air value in ms for GFSK transmission
 */
static inline uint32_t ral_get_gfsk_time_on_air_in_ms( const ral_t* radio, const ral_gfsk_pkt_params_t* pkt_p,
                                                       const ral_gfsk_mod_params_t* mod_p )
{
    return radio->driver.get_gfsk_time_on_air_in_ms( pkt_p, mod_p );
}

/**
 * @brief Get the time on air in millisecond for FLRC transmission
 *
 * @param [in] pkt_p  Pointer to a structure holding the FLRC packet parameters
 * @param [in] mod_p  Pointer to a structure holding the FLRC modulation parameters
 *
 * @returns Time-on-air value in ms for FLRC transmission
 */
static inline uint32_t ral_get_flrc_time_on_air_in_ms( const ral_t* radio, const ral_flrc_pkt_params_t* pkt_p,
                                                       const ral_flrc_mod_params_t* mod_p )
{
    return radio->driver.get_flrc_time_on_air_in_ms( pkt_p, mod_p );
}

/**
 * @brief Configure the sync word used in GFSK packet
 *
 * @param [in] radio          Pointer to radio data structure
 * @param [in] sync_word      Buffer holding the sync word to be configured
 * @param [in] sync_word_len  Sync word length in bytes
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_gfsk_sync_word( const ral_t* radio, const uint8_t* sync_word,
                                                   const uint8_t sync_word_len )
{
    return radio->driver.set_gfsk_sync_word( radio->context, sync_word, sync_word_len );
}

/**
 * @brief Configure the sync word used in LoRa packet
 *
 * @remark In the case of a LoRaWAN use case, the two following values are specified:
 *   - 0x12 for a private LoRaWAN network (default)
 *   - 0x34 for a public LoRaWAN network
 *
 * @param [in] radio      Pointer to radio data structure
 * @param [in] sync_word  Sync word to be configured
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_lora_sync_word( const ral_t* radio, const uint8_t sync_word )
{
    return radio->driver.set_lora_sync_word( radio->context, sync_word );
}

/**
 * @brief Configure the sync word used in FLRC packet
 *
 * @param [in] radio          Pointer to radio data structure
 * @param [in] sync_word      Buffer holding the sync word to be configured
 * @param [in] sync_word_len  Sync word length in bytes
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_flrc_sync_word( const ral_t* radio, const uint8_t* sync_word,
                                                   const uint8_t sync_word_len )
{
    return radio->driver.set_flrc_sync_word( radio->context, sync_word, sync_word_len );
}

/**
 * @brief Configure the seed used to compute CRC in GFSK packet
 *
 * @param [in] radio       Pointer to radio data structure
 * @param [in] seed        Seed value used to compute the CRC value
 * @param [in] polynomial  Polynomial value used to compute the CRC value
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_gfsk_crc_params( const ral_t* radio, const uint16_t seed, const uint16_t polynomial )
{
    return radio->driver.set_gfsk_crc_params( radio->context, seed, polynomial );
}

/**
 * @brief Configure the seed used to compute the CRC for FLRC packets
 *
 * @param [in] radio       Pointer to radio data structure
 * @param [in] seed        Seed value used to compute the CRC value
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_flrc_crc_params( const ral_t* radio, const uint32_t seed )
{
    return radio->driver.set_flrc_crc_params( radio->context, seed );
}

/**
 * @brief Configure the whitening seed used in GFSK packet
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] seed   Seed value used in data whitening
 *
 * @returns Operation status
 */
static inline ral_status_t ral_set_gfsk_whitening_seed( const ral_t* radio, const uint16_t seed )
{
    return radio->driver.set_gfsk_whitening_seed( radio->context, seed );
}

/**
 * @brief Initialise LR FHSS
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] lr_fhss_params  Pointer to lr fhss parameters data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_lr_fhss_init( const ral_t* radio, const ral_lr_fhss_params_t* lr_fhss_params )
{
    return radio->driver.lr_fhss_init( radio->context, lr_fhss_params );
}

/**
 * @brief Build frame for LR FHSS operation
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] lr_fhss_params  Pointer to lr fhss parameters data structure
 * @param [in,out] memory_state_holder  Pointer to memory allocated to hold lr fhss state
 * @param [in] hop_sequence_id  The hop sequence id to use
 * @param [in] payload  Pointer to the payload to send
 * @param [in] payload_length  Length of payload in bytes
 *
 * @returns Operation status
 */
static inline ral_status_t ral_lr_fhss_build_frame( const ral_t* radio, const ral_lr_fhss_params_t* lr_fhss_params,
                                                    ral_lr_fhss_memory_state_t memory_state_holder,
                                                    uint16_t hop_sequence_id, const uint8_t* payload,
                                                    uint16_t payload_length )
{
    return radio->driver.lr_fhss_build_frame( radio->context, lr_fhss_params, memory_state_holder, hop_sequence_id,
                                              payload, payload_length );
}

/**
 * @brief Handle HOP for LR FHSS operation
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] memory_state_holder  Pointer to memory allocated to hold lr fhss state
 * @param [in] lr_fhss_params  Pointer to lr fhss parameters data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_lr_fhss_handle_hop( const ral_t* radio, const ral_lr_fhss_params_t* lr_fhss_params,
                                                   ral_lr_fhss_memory_state_t memory_state_holder )
{
    return radio->driver.lr_fhss_handle_hop( radio->context, lr_fhss_params, memory_state_holder );
}

/**
 * @brief Handle TX DONE for LR FHSS operation
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] memory_state_holder  Pointer to memory allocated to hold lr fhss state
 * @param [in] lr_fhss_params  Pointer to lr fhss parameters data structure
 *
 * @returns Operation status
 */
static inline ral_status_t ral_lr_fhss_handle_tx_done( const ral_t* radio, const ral_lr_fhss_params_t* lr_fhss_params,
                                                       ral_lr_fhss_memory_state_t memory_state_holder )
{
    return radio->driver.lr_fhss_handle_tx_done( radio->context, lr_fhss_params, memory_state_holder );
}

/**
 * @brief Get the time on air in ms for LR-FHSS transmission
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] lr_fhss_params LR-FHSS parameter structure
 * @param [in] payload_length Length of application-layer payload
 *
 * @returns Time-on-air value in ms for LR-FHSS transmission
 */
static inline ral_status_t ral_lr_fhss_get_time_on_air_in_ms( const ral_t*                radio,
                                                              const ral_lr_fhss_params_t* lr_fhss_params,
                                                              uint16_t payload_length, uint32_t* time_on_air )
{
    return radio->driver.lr_fhss_get_time_on_air_in_ms( radio->context, lr_fhss_params, payload_length, time_on_air );
}

/**
 * @brief Return the number of hop sequences available using the given parameters
 *
 * @param [in] radio  Pointer to radio data structure
 * @param [in] lr_fhss_params  Pointer to lr fhss parameters data structure
 *
 * @return Returns the number of valid hop sequences (512 or 384)
 */
static inline unsigned int ral_lr_fhss_get_hop_sequence_count( const ral_t*                radio,
                                                               const ral_lr_fhss_params_t* lr_fhss_params )
{
    return radio->driver.lr_fhss_get_hop_sequence_count( radio->context, lr_fhss_params );
}

/**
 * @brief Get the coding rate and CRC configuration of the last received LoRa packet
 *
 * @param [in] radio            Pointer to radio data structure
 * @param [out] cr              LoRa coding rate
 * @param [out] is_crc_present  LoRa CRC configuration
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_lora_rx_pkt_cr_crc( const ral_t* radio, ral_lora_cr_t* cr, bool* is_crc_present )
{
    return radio->driver.get_lora_rx_pkt_cr_crc( radio->context, cr, is_crc_present );
}

/**
 * @brief Get TX power consumption, in micro ampere
 *
 * @param [in] radio                   Pointer to radio data structure
 * @param [in] output_pwr_in_dbm       System output power in dBm
 * @param [in] rf_freq_in_hz           RF frequency in Hertz
 * @param [out] pwr_consumption_in_ua  The power consumption in micro ampere
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_tx_consumption_in_ua( const ral_t* radio, const int8_t output_pwr_in_dbm,
                                                         const uint32_t rf_freq_in_hz, uint32_t* pwr_consumption_in_ua )
{
    return radio->driver.get_tx_consumption_in_ua( radio->context, output_pwr_in_dbm, rf_freq_in_hz,
                                                   pwr_consumption_in_ua );
}

/**
 * @brief Get GFSK RX power consumption, in micro ampere
 *
 * @param [in] radio                   Pointer to radio data structure
 * @param [in] br_in_bps               Bitrate in bit-per-second
 * @param [in] bw_dsb_in_hz            Bandwidth in Hz (Dual Side Band)
 * @param [in] rx_boosted              Rx boosted option
 * @param [out] pwr_consumption_in_ua  The power consumption in micro ampere
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_gfsk_rx_consumption_in_ua( const ral_t* radio, const uint32_t br_in_bps,
                                                              const uint32_t bw_dsb_in_hz, const bool rx_boosted,
                                                              uint32_t* pwr_consumption_in_ua )
{
    return radio->driver.get_gfsk_rx_consumption_in_ua( radio->context, br_in_bps, bw_dsb_in_hz, rx_boosted,
                                                        pwr_consumption_in_ua );
}

/**
 * @brief Get LoRa RX power consumption, in micro ampere
 *
 * @param [in] radio                   Pointer to radio data structure
 * @param [in] bw                      LoRa bandwidth
 * @param [in] rx_boosted              Rx boosted option
 * @param [out] pwr_consumption_in_ua  The power consumption in micro ampere
 *
 * @returns Operation status
 */
static inline ral_status_t ral_get_lora_rx_consumption_in_ua( const ral_t* radio, const ral_lora_bw_t bw,
                                                              const bool rx_boosted, uint32_t* pwr_consumption_in_ua )
{
    return radio->driver.get_lora_rx_consumption_in_ua( radio->context, bw, rx_boosted, pwr_consumption_in_ua );
}

/**
 * @brief Generate one or more 32-bit random numbers.
 *
 * @remark A valid packet type must have been configured with @ref ral_set_pkt_type
 *         before using this command.
 *
 * @param [in] radio    Pointer to radio data structure
 * @param [out] numbers Array where numbers will be stored
 * @param [in]  n       Number of desired random numbers
 *
 * @returns Operation status
 *
 * It is assumed that the transceiver is in standby mode when this API function is called.
 * Note that for certain transceivers this code can result in interrupt generation. It is the responsibility of
 * the caller to disable radio interrupts before calling this function,
 * and to re-enable them afterwards if necessary, or be certain that any interrupts
 * generated during this process will not cause undesired side-effects in the calling application.
 *
 * Please note that the random numbers produced by the generator do not necessarily have a uniform or
 * Gaussian distribution. If uniformity is needed, perform appropriate software post-processing.
 */
static inline ral_status_t ral_get_random_numbers( const ral_t* radio, uint32_t* numbers, unsigned int n )
{
    return radio->driver.get_random_numbers( radio->context, numbers, n );
}

#ifdef __cplusplus
}
#endif

#endif  // RAL_H__

/* --- EOF ------------------------------------------------------------------ */
