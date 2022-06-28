/**
 * @file      ral_defs.h
 *
 * @brief     Radio abstraction layer types definition
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

#ifndef RAL_DEFS_H__
#define RAL_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>
#include "lr_fhss_v1_base_types.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/**
 * @brief Number of state bytes necessary to guarantee functionality for all radios
 */
#define RAL_LR_FHSS_STATE_MAXSIZE ( 24 )

/**
 * @brief Length, in bytes, of a LR-FHSS sync word
 */
#define LR_FHSS_SYNC_WORD_BYTES ( 4 )

/**
 * @brief Reserved value used to configure a reception in continuous mode
 */
#define RAL_RX_TIMEOUT_CONTINUOUS_MODE 0xFFFFFFFF

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

typedef enum ral_standby_cfg_e
{
    RAL_STANDBY_CFG_RC,
    RAL_STANDBY_CFG_XOSC,
} ral_standby_cfg_t;

typedef enum ral_fallback_modes_e
{
    RAL_FALLBACK_STDBY_RC,
    RAL_FALLBACK_STDBY_XOSC,
    RAL_FALLBACK_FS,
} ral_fallback_modes_t;

/**
 * API return status
 */
typedef enum ral_status_e
{
    RAL_STATUS_OK,
    RAL_STATUS_UNSUPPORTED_FEATURE,
    RAL_STATUS_UNKNOWN_VALUE,
    RAL_STATUS_ERROR,
} ral_status_t;

typedef enum ral_gfsk_crc_type_e
{
    RAL_GFSK_CRC_OFF,
    RAL_GFSK_CRC_1_BYTE,
    RAL_GFSK_CRC_2_BYTES,
    RAL_GFSK_CRC_1_BYTE_INV,
    RAL_GFSK_CRC_2_BYTES_INV,
    RAL_GFSK_CRC_3_BYTES,
} ral_gfsk_crc_type_t;

typedef enum ral_gfsk_pulse_shape_e
{
    RAL_GFSK_PULSE_SHAPE_OFF,
    RAL_GFSK_PULSE_SHAPE_BT_03,
    RAL_GFSK_PULSE_SHAPE_BT_05,
    RAL_GFSK_PULSE_SHAPE_BT_07,
    RAL_GFSK_PULSE_SHAPE_BT_1,
} ral_gfsk_pulse_shape_t;

typedef enum ral_lora_sf_e
{
    RAL_LORA_SF5 = 5,
    RAL_LORA_SF6,
    RAL_LORA_SF7,
    RAL_LORA_SF8,
    RAL_LORA_SF9,
    RAL_LORA_SF10,
    RAL_LORA_SF11,
    RAL_LORA_SF12,
} ral_lora_sf_t;

typedef enum ral_lora_bw_e
{
    RAL_LORA_BW_007_KHZ,   // All except SX128X and SX1272
    RAL_LORA_BW_010_KHZ,   // All except SX128X and SX1272
    RAL_LORA_BW_015_KHZ,   // All except SX128X and SX1272
    RAL_LORA_BW_020_KHZ,   // All except SX128X and SX1272
    RAL_LORA_BW_031_KHZ,   // All except SX128X and SX1272
    RAL_LORA_BW_041_KHZ,   // All except SX128X and SX1272
    RAL_LORA_BW_062_KHZ,   // All except SX128X and SX1272
    RAL_LORA_BW_125_KHZ,   // All except SX128X
    RAL_LORA_BW_200_KHZ,   // LR112X and SX128X only
    RAL_LORA_BW_250_KHZ,   // All except SX128X
    RAL_LORA_BW_400_KHZ,   // LR112X and SX128X only
    RAL_LORA_BW_500_KHZ,   // All except SX128X
    RAL_LORA_BW_800_KHZ,   // LR112X and SX128X only
    RAL_LORA_BW_1600_KHZ,  // SX128X only
} ral_lora_bw_t;

typedef enum ral_lora_cr_e
{
    RAL_LORA_CR_4_5 = 1,
    RAL_LORA_CR_4_6,
    RAL_LORA_CR_4_7,
    RAL_LORA_CR_4_8,
    RAL_LORA_CR_LI_4_5,
    RAL_LORA_CR_LI_4_6,
    RAL_LORA_CR_LI_4_8,
} ral_lora_cr_t;

typedef enum ral_flrc_cr_e
{
    RAL_FLRC_CR_1_2 = 0,
    RAL_FLRC_CR_3_4,
    RAL_FLRC_CR_1_1,
} ral_flrc_cr_t;

typedef enum ral_flrc_crc_type_e
{
    RAL_FLRC_CRC_OFF = 0,
    RAL_FLRC_CRC_2_BYTES,
    RAL_FLRC_CRC_3_BYTES,
    RAL_FLRC_CRC_4_BYTES
} ral_flrc_crc_type_t;

typedef enum ral_flrc_pulse_shape_e
{
    RAL_FLRC_PULSE_SHAPE_OFF,
    RAL_FLRC_PULSE_SHAPE_BT_05,
    RAL_FLRC_PULSE_SHAPE_BT_1,
} ral_flrc_pulse_shape_t;

/**
 * @brief GFSK preamble length Rx detection size enumeration definition
 */
typedef enum ral_gfsk_preamble_detector_e
{
    RAL_GFSK_PREAMBLE_DETECTOR_OFF,
    RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS,
    RAL_GFSK_PREAMBLE_DETECTOR_MIN_16BITS,
    RAL_GFSK_PREAMBLE_DETECTOR_MIN_24BITS,
    RAL_GFSK_PREAMBLE_DETECTOR_MIN_32BITS,
} ral_gfsk_preamble_detector_t;

/**
 * @brief LoRa packet length enumeration definition
 */
typedef enum ral_lora_pkt_len_modes_e
{
    RAL_LORA_PKT_EXPLICIT,  //!< LoRa header included in the packet
    RAL_LORA_PKT_IMPLICIT,  //!< LoRa header not included in the packet
} ral_lora_pkt_len_modes_t;

typedef enum ral_pkt_types_e
{
    RAL_PKT_TYPE_GFSK,
    RAL_PKT_TYPE_LORA,
    RAL_PKT_TYPE_FLRC,
} ral_pkt_type_t;

/**
 * @brief GFSK address filtering configuration enumeration definition
 */
typedef enum ral_gfsk_address_filtering_e
{
    RAL_GFSK_ADDRESS_FILTERING_DISABLE,
    RAL_GFSK_ADDRESS_FILTERING_NODE_ADDRESS,
    RAL_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES,
} ral_gfsk_address_filtering_t;

/*!
 * \brief GFSK data whitening configurations
 */
typedef enum ral_gfsk_dc_free_e
{
    RAL_GFSK_DC_FREE_OFF,        //!< GFSK data whitening deactivated
    RAL_GFSK_DC_FREE_WHITENING,  //!< GFSK data whitening enabled
} ral_gfsk_dc_free_t;

enum ral_rx_status_gfsk_e
{
    RAL_RX_STATUS_PKT_SENT     = ( 1 << 0 ),
    RAL_RX_STATUS_PKT_RECEIVED = ( 1 << 1 ),
    RAL_RX_STATUS_ABORT_ERROR  = ( 1 << 2 ),
    RAL_RX_STATUS_LENGTH_ERROR = ( 1 << 3 ),
    RAL_RX_STATUS_CRC_ERROR    = ( 1 << 4 ),
    RAL_RX_STATUS_ADDR_ERROR   = ( 1 << 5 ),
};

/*!
 * @brief GFSK Header Type configurations
 *
 * This parameter indicates whether or not the payload length is sent and read over the air.
 *
 * If the payload length is known beforehand by both transmitter and receiver, therefore there is no need to send it
 * over the air and the parameter can be set to RAL_GFSK_PKT_FIX_LEN. Otherwise, setting this parameter to
 * RAL_GFSK_PKT_VAR_LEN will make the modem to automatically prepend a byte containing the payload length to the payload
 * on transmitter side. On receiver side, this first byte is read to set the payload length to read.
 *
 * This configuration is only available for GFSK packet types.
 */
typedef enum ral_gfsk_pkt_len_modes_s
{
    RAL_GFSK_PKT_FIX_LEN,  //!< Payload length is not sent/read over the air
    RAL_GFSK_PKT_VAR_LEN,  //!< Payload length is sent/read over the air
} ral_gfsk_pkt_len_modes_t;

typedef uint16_t ral_rx_status_gfsk_t;

typedef struct ral_gfsk_rx_pkt_status_s
{
    ral_rx_status_gfsk_t rx_status;
    int16_t              rssi_sync_in_dbm;
    int16_t              rssi_avg_in_dbm;
} ral_gfsk_rx_pkt_status_t;

typedef struct ral_lora_rx_pkt_status_s
{
    int16_t rssi_pkt_in_dbm;
    int16_t snr_pkt_in_db;
    int16_t signal_rssi_pkt_in_dbm;  // Last packet received LoRa signal RSSI power in dB (after despreading)
} ral_lora_rx_pkt_status_t;

typedef struct ral_flrc_rx_pkt_status_s
{
    int16_t rssi_sync_in_dbm;
} ral_flrc_rx_pkt_status_t;

/*!
 *  Represents the number of symbs to be used for channel activity detection operation
 */
typedef enum ral_lora_cad_symbs_e
{
    RAL_LORA_CAD_01_SYMB,
    RAL_LORA_CAD_02_SYMB,
    RAL_LORA_CAD_04_SYMB,
    RAL_LORA_CAD_08_SYMB,
    RAL_LORA_CAD_16_SYMB,
} ral_lora_cad_symbs_t;

/*!
 *  Represents the Channel Activity Detection actions after the CAD operation is finished
 */
typedef enum ral_lora_cad_exit_modes_e
{
    RAL_LORA_CAD_ONLY,
    RAL_LORA_CAD_RX,
    RAL_LORA_CAD_LBT,
} ral_lora_cad_exit_modes_t;

/**
 * @brief GFSK modulation parameters structure definition
 */
typedef struct ral_gfsk_mod_params_s
{
    uint32_t               br_in_bps;
    uint32_t               fdev_in_hz;
    uint32_t               bw_dsb_in_hz;
    ral_gfsk_pulse_shape_t pulse_shape;
} ral_gfsk_mod_params_t;

/**
 * @brief GFSK packet parameters structure definition
 */
typedef struct ral_gfsk_pkt_params_s
{
    uint16_t                     preamble_len_in_bits;   //!< GFSK Preamble length in bits
    ral_gfsk_preamble_detector_t preamble_detector;      //!< GFSK Preamble detection length
    uint8_t                      sync_word_len_in_bits;  //!< GFSK Sync word length in bits
    ral_gfsk_address_filtering_t address_filtering;      //!< GFSK Address filtering configuration
    ral_gfsk_pkt_len_modes_t     header_type;            //!< GFSK Header type
    uint8_t                      pld_len_in_bytes;       //!< GFSK Payload length in bytes
    ral_gfsk_crc_type_t          crc_type;               //!< GFSK CRC type configuration
    ral_gfsk_dc_free_t           dc_free;                //!< GFSK Whitening configuration
} ral_gfsk_pkt_params_t;

/**
 * @brief LoRa modulation parameters structure definition
 */
typedef struct ral_lora_mod_params_s
{
    ral_lora_sf_t sf;    //!< LoRa Spreading Factor
    ral_lora_bw_t bw;    //!< LoRa Bandwidth
    ral_lora_cr_t cr;    //!< LoRa Coding Rate
    uint8_t       ldro;  //!< LoRa Low DataRate Optimization configuration
} ral_lora_mod_params_t;

/**
 * @brief LoRa packet parameters structure definition
 */
typedef struct ral_lora_pkt_params_s
{
    uint16_t                 preamble_len_in_symb;  //!< LoRa preamble length in symbols
    ral_lora_pkt_len_modes_t header_type;           //!< LoRa header type
    uint8_t                  pld_len_in_bytes;      //!< LoRa payload length in bytes
    bool                     crc_is_on;             //!< LoRa CRC activation
    bool                     invert_iq_is_on;       //!< LoRa IQ polarity setup
} ral_lora_pkt_params_t;

/*!
 * Defines \ref ral_set_cad_params function parameters.
 */
typedef struct ral_lora_cad_param_s
{
    ral_lora_cad_symbs_t      cad_symb_nb;
    uint8_t                   cad_det_peak_in_symb;
    uint8_t                   cad_det_min_in_symb;
    ral_lora_cad_exit_modes_t cad_exit_mode;
    uint32_t                  cad_timeout_in_ms;
} ral_lora_cad_params_t;

/**
 * @brief FLRC modulation parameters structure definition
 */
typedef struct ral_flrc_mod_params_s
{
    uint32_t               br_in_bps;
    uint32_t               bw_dsb_in_hz;
    ral_flrc_cr_t          cr;
    ral_flrc_pulse_shape_t pulse_shape;
} ral_flrc_mod_params_t;

/**
 * @brief FLRC packet parameters structure definition
 */
typedef struct ral_flrc_pkt_params_s
{
    uint16_t            preamble_len_in_bits;
    bool                sync_word_is_on;
    bool                pld_is_fix;
    uint8_t             pld_len_in_bytes;
    ral_flrc_crc_type_t crc_type;
} ral_flrc_pkt_params_t;

/*!
 * @brief LR FHSS parameters
 */
typedef struct ral_lr_fhss_params_s
{
    lr_fhss_v1_params_t lr_fhss_params;
    uint32_t            center_frequency_in_hz;
    int8_t              device_offset;
} ral_lr_fhss_params_t;

/*!
 * @brief Memory allocated to hold LR-FHSS state
 *
 * This memory is to be allocated by the caller. It is used by the radio driver to store internal
 * LR-FHSS state. This is not needed by all radio drivers, but in order to support all transceivers,
 * this should point to a block of at least RAL_LR_FHSS_STATE_MAXSIZE bytes of RAM. If you are only
 * targeting LR11XX, for instance, you may use NULL.
 */
typedef void* ral_lr_fhss_memory_state_t;

/**
 * @brief IRQ definitions
 *
 * @remark This enumeration cannot accept more than 16 entries in addition to @ref RAL_IRQ_NONE and @ref RAL_IRQ_ALL -
 * this is related to the fact that @ref ral_irq_t is 16-bit long
 */
enum ral_irq_e
{
    RAL_IRQ_NONE                 = ( 0 << 0 ),
    RAL_IRQ_TX_DONE              = ( 1 << 1 ),
    RAL_IRQ_RX_DONE              = ( 1 << 2 ),
    RAL_IRQ_RX_TIMEOUT           = ( 1 << 3 ),
    RAL_IRQ_RX_PREAMBLE_DETECTED = ( 1 << 4 ),
    RAL_IRQ_RX_HDR_OK            = ( 1 << 5 ),
    RAL_IRQ_RX_HDR_ERROR         = ( 1 << 6 ),
    RAL_IRQ_RX_CRC_ERROR         = ( 1 << 7 ),
    RAL_IRQ_CAD_DONE             = ( 1 << 8 ),
    RAL_IRQ_CAD_OK               = ( 1 << 9 ),
    RAL_IRQ_LR_FHSS_HOP          = ( 1 << 10 ),
    RAL_IRQ_WIFI_SCAN_DONE       = ( 1 << 11 ),
    RAL_IRQ_GNSS_SCAN_DONE       = ( 1 << 12 ),
    RAL_IRQ_ALL = RAL_IRQ_TX_DONE | RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT | RAL_IRQ_RX_PREAMBLE_DETECTED |
                  RAL_IRQ_RX_HDR_OK | RAL_IRQ_RX_HDR_ERROR | RAL_IRQ_RX_CRC_ERROR | RAL_IRQ_CAD_DONE | RAL_IRQ_CAD_OK |
                  RAL_IRQ_LR_FHSS_HOP | RAL_IRQ_WIFI_SCAN_DONE | RAL_IRQ_GNSS_SCAN_DONE,
};

typedef uint16_t ral_irq_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTION PROTOTYPES ----------------------------------------------
 */

static inline uint8_t ral_compute_lora_ldro( const ral_lora_sf_t sf, const ral_lora_bw_t bw )
{
    switch( bw )
    {
    case RAL_LORA_BW_500_KHZ:
        return 0;
    case RAL_LORA_BW_250_KHZ:
        if( sf == RAL_LORA_SF12 )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    case RAL_LORA_BW_800_KHZ:
    case RAL_LORA_BW_400_KHZ:
    case RAL_LORA_BW_200_KHZ:
    case RAL_LORA_BW_125_KHZ:
        if( ( sf == RAL_LORA_SF12 ) || ( sf == RAL_LORA_SF11 ) )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    case RAL_LORA_BW_062_KHZ:
        if( ( sf == RAL_LORA_SF12 ) || ( sf == RAL_LORA_SF11 ) || ( sf == RAL_LORA_SF10 ) )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    case RAL_LORA_BW_041_KHZ:
        if( ( sf == RAL_LORA_SF12 ) || ( sf == RAL_LORA_SF11 ) || ( sf == RAL_LORA_SF10 ) || ( sf == RAL_LORA_SF9 ) )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    case RAL_LORA_BW_031_KHZ:
    case RAL_LORA_BW_020_KHZ:
    case RAL_LORA_BW_015_KHZ:
    case RAL_LORA_BW_010_KHZ:
    case RAL_LORA_BW_007_KHZ:
        return 1;
    default:
        return 0;
    }
}

#ifdef __cplusplus
}
#endif

#endif  // RAL_DEFS_H__

/* --- EOF ------------------------------------------------------------------ */
