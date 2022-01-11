/**
 * @file      sx127x.h
 *
 * @brief     SX1272/3 and SX1276/7/8/9 radio driver definition
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
#ifndef SX127X_H
#define SX127X_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stddef.h>   // NULL
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type
#include "sx127x_regs.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/**
 * @brief Maximum size of the internal Rx/Tx buffer
 */
#define SX127X_RX_TX_BUFFER_SIZE_MAX 256

/**
 * @brief Defines threshold between low and high RF bands (i.e.: 433 MHz and 868/915 MHz)
 */
#define RF_FREQUENCY_MID_BAND_THRESHOLD 525000000

/**
 * @brief Constant values required to compute the LoRa modem RSSI values
 */
#define RSSI_OFFSET_LF -164
#define RSSI_OFFSET_HF -157

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/**
 * @brief SX127X APIs return status enumeration definition
 */
typedef enum sx127x_status_e
{
    SX127X_STATUS_OK = 0,
    SX127X_STATUS_UNSUPPORTED_FEATURE,
    SX127X_STATUS_UNKNOWN_VALUE,
    SX127X_STATUS_ERROR,
    SX127X_STATUS_HAL_CONTEXT_UNINITIALIZED,
} sx127x_status_t;

/**
 * @brief Radio ID
 */
typedef enum sx127x_radio_id_e
{
    SX127X_RADIO_ID_SX1272,
    SX127X_RADIO_ID_SX1273,
    SX127X_RADIO_ID_SX1276,
    SX127X_RADIO_ID_SX1277,
    SX127X_RADIO_ID_SX1278,
    SX127X_RADIO_ID_SX1279,
} sx127x_radio_id_t;

/**
 * @brief SX127X interrupt masks enumeration definition
 */
enum sx127x_irq_masks_e
{
    SX127X_IRQ_NONE                = ( 0 << 0 ),
    SX127X_IRQ_TX_DONE             = ( 1 << 0 ),
    SX127X_IRQ_RX_DONE             = ( 1 << 1 ),
    SX127X_IRQ_PREAMBLE_DETECTED   = ( 1 << 2 ),
    SX127X_IRQ_SYNC_WORD_VALID     = ( 1 << 3 ),
    SX127X_IRQ_HEADER_VALID        = ( 1 << 4 ),
    SX127X_IRQ_HEADER_ERROR        = ( 1 << 5 ),
    SX127X_IRQ_CRC_ERROR           = ( 1 << 6 ),
    SX127X_IRQ_CAD_DONE            = ( 1 << 7 ),
    SX127X_IRQ_CAD_DETECTED        = ( 1 << 8 ),
    SX127X_IRQ_TIMEOUT             = ( 1 << 9 ),
    SX127X_IRQ_FHSS_CHANNEL_CHANGE = ( 1 << 10 ),
    SX127X_IRQ_ALL                 = SX127X_IRQ_TX_DONE | SX127X_IRQ_RX_DONE | SX127X_IRQ_PREAMBLE_DETECTED |
                     SX127X_IRQ_SYNC_WORD_VALID | SX127X_IRQ_HEADER_VALID | SX127X_IRQ_HEADER_ERROR |
                     SX127X_IRQ_CRC_ERROR | SX127X_IRQ_CAD_DONE | SX127X_IRQ_CAD_DETECTED | SX127X_IRQ_TIMEOUT |
                     SX127X_IRQ_FHSS_CHANNEL_CHANGE,
};

typedef uint16_t sx127x_irq_mask_t;

/**
 * @brief SX127X packet types enumeration definition
 */
typedef enum sx127x_pkt_types_e
{
    SX127X_PKT_TYPE_GFSK = 0,
    SX127X_PKT_TYPE_LORA = 2,
    SX127X_PKT_TYPE_OOK  = 3,
} sx127x_pkt_type_t;

/**
 * @brief SX127X power amplifier selection enumeration definition
 */
typedef enum sx127x_pa_select_e
{
    SX127X_PA_SELECT_RFO   = 0,
    SX127X_PA_SELECT_BOOST = 1,
} sx127x_pa_select_t;

/**
 * @brief SX127X power amplifier configuration parameters structure definition
 */
typedef struct sx127x_pa_cfg_params_s
{
    sx127x_pa_select_t pa_select;
    bool               is_20_dbm_output_on;
} sx127x_pa_cfg_params_t;

/**
 * @brief SX127X power amplifier ramp-up timings enumeration definition
 */
typedef enum sx127x_ramp_time_e
{
    SX127X_RAMP_3400_US = 0,
    SX127X_RAMP_2000_US = 1,
    SX127X_RAMP_1000_US = 2,
    SX127X_RAMP_500_US  = 3,
    SX127X_RAMP_250_US  = 4,
    SX127X_RAMP_125_US  = 5,
    SX127X_RAMP_100_US  = 6,
    SX127X_RAMP_62_US   = 7,
    SX127X_RAMP_50_US   = 8,
    SX127X_RAMP_40_US   = 9,
    SX127X_RAMP_31_US   = 10,
    SX127X_RAMP_25_US   = 11,
    SX127X_RAMP_20_US   = 12,
    SX127X_RAMP_15_US   = 13,
    SX127X_RAMP_12_US   = 14,
    SX127X_RAMP_10_US   = 15,
} sx127x_ramp_time_t;

/**
 * @brief SX127X GFSK modulation shaping enumeration definition
 */
typedef enum sx127x_gfsk_pulse_shape_e
{
    SX127X_GFSK_PULSE_SHAPE_OFF   = 0x00,
    SX127X_GFSK_PULSE_SHAPE_BT_03 = 0x01,
    SX127X_GFSK_PULSE_SHAPE_BT_05 = 0x02,
    SX127X_GFSK_PULSE_SHAPE_BT_1  = 0x03,
} sx127x_gfsk_pulse_shape_t;

/**
 * @brief SX127X OOK modulation shaping enumeration definition
 */
typedef enum sx127x_ook_pulse_shape_e
{
    SX127X_OOK_PULSE_SHAPE_OFF           = 0x00,
    SX127X_OOK_PULSE_SHAPE_F_CUT_OFF_BR  = 0x01,
    SX127X_OOK_PULSE_SHAPE_F_CUT_OFF_2BR = 0x02,
} sx127x_ook_pulse_shape_t;

/**
 * @brief SX127X GFSK Rx bandwidth enumeration definition
 */
typedef enum sx127x_gfsk_bw_e
{
    SX127X_GFSK_BW_2600   = 0x17,
    SX127X_GFSK_BW_3100   = 0x0F,
    SX127X_GFSK_BW_3900   = 0x07,
    SX127X_GFSK_BW_5200   = 0x16,
    SX127X_GFSK_BW_6300   = 0x0E,
    SX127X_GFSK_BW_7800   = 0x06,
    SX127X_GFSK_BW_10400  = 0x15,
    SX127X_GFSK_BW_12500  = 0x0D,
    SX127X_GFSK_BW_15600  = 0x05,
    SX127X_GFSK_BW_20800  = 0x14,
    SX127X_GFSK_BW_25000  = 0x0C,
    SX127X_GFSK_BW_31300  = 0x04,
    SX127X_GFSK_BW_41700  = 0x13,
    SX127X_GFSK_BW_50000  = 0x0B,
    SX127X_GFSK_BW_62500  = 0x03,
    SX127X_GFSK_BW_83333  = 0x12,
    SX127X_GFSK_BW_100000 = 0x0A,
    SX127X_GFSK_BW_125000 = 0x02,
    SX127X_GFSK_BW_166700 = 0x11,
    SX127X_GFSK_BW_200000 = 0x09,
    SX127X_GFSK_BW_250000 = 0x01,
} sx127x_gfsk_bw_t;

/**
 * @brief SX127X GFSK modulation parameters structure definition
 */
typedef struct sx127x_gfsk_mod_params_s
{
    uint32_t br_in_bps;
    uint32_t fdev_in_hz;
    union pulse_shape_u
    {
        sx127x_gfsk_pulse_shape_t gfsk;
        sx127x_ook_pulse_shape_t  ook;
    } pulse_shape;
    uint32_t bw_ssb_in_hz;
} sx127x_gfsk_mod_params_t;

/**
 * @brief SX127X LoRa spreading factor enumeration definition
 */
typedef enum sx127x_lora_sf_e
{
    SX127X_LORA_SF6  = 0x06,
    SX127X_LORA_SF7  = 0x07,
    SX127X_LORA_SF8  = 0x08,
    SX127X_LORA_SF9  = 0x09,
    SX127X_LORA_SF10 = 0x0A,
    SX127X_LORA_SF11 = 0x0B,
    SX127X_LORA_SF12 = 0x0C,
} sx127x_lora_sf_t;

/**
 * @brief SX127X LoRa bandwidth enumeration definition
 */
typedef enum sx127x_lora_bw_e
{
    SX127X_LORA_BW_007 = 0,
    SX127X_LORA_BW_010 = 1,
    SX127X_LORA_BW_015 = 2,
    SX127X_LORA_BW_020 = 3,
    SX127X_LORA_BW_031 = 4,
    SX127X_LORA_BW_041 = 5,
    SX127X_LORA_BW_062 = 6,
    SX127X_LORA_BW_125 = 7,
    SX127X_LORA_BW_250 = 8,
    SX127X_LORA_BW_500 = 9,
} sx127x_lora_bw_t;

/**
 * @brief SX127X LoRa coding rate enumeration definition
 */
typedef enum sx127x_lora_cr_e
{
    SX127X_LORA_CR_4_5 = 0x01,
    SX127X_LORA_CR_4_6 = 0x02,
    SX127X_LORA_CR_4_7 = 0x03,
    SX127X_LORA_CR_4_8 = 0x04,
} sx127x_lora_cr_t;

/**
 * @brief SX127X LoRa modulation parameters structure definition
 */
typedef struct sx127x_lora_mod_params_s
{
    sx127x_lora_sf_t sf;    //!< LoRa Spreading Factor
    sx127x_lora_bw_t bw;    //!< LoRa Bandwidth
    sx127x_lora_cr_t cr;    //!< LoRa Coding Rate
    uint8_t          ldro;  //!< Low DataRate Optimization configuration
} sx127x_lora_mod_params_t;

/**
 * @brief SX127X GFSK preamble length Rx detection size enumeration definition
 */
typedef enum sx127x_gfsk_preamble_detector_e
{
    SX127X_GFSK_PREAMBLE_DETECTOR_OFF     = 0x00,
    SX127X_GFSK_PREAMBLE_DETECTOR_1_BYTE  = 0x04,
    SX127X_GFSK_PREAMBLE_DETECTOR_2_BYTES = 0x05,
    SX127X_GFSK_PREAMBLE_DETECTOR_3_BYTES = 0x06
} sx127x_gfsk_preamble_detector_t;

/**
 * @brief SX127X GFSK address filtering configuration enumeration definition
 */
typedef enum sx127x_gfsk_address_filtering_e
{
    SX127X_GFSK_ADDRESS_FILTERING_DISABLE                      = 0x00,
    SX127X_GFSK_ADDRESS_FILTERING_NODE_ADDRESS                 = 0x01,
    SX127X_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES = 0x02,
} sx127x_gfsk_address_filtering_t;

/**
 * @brief SX127X GFSK packet length enumeration definition
 */
typedef enum sx127x_gfsk_pkt_len_modes_e
{
    SX127X_GFSK_PKT_FIX_LEN = 0x00,  //!< The packet length is known on both sides, no header included
    SX127X_GFSK_PKT_VAR_LEN = 0x01,  //!< The packet length is variable, header included
} sx127x_gfsk_pkt_len_modes_t;

/**
 * @brief SX127X GFSK CRC type enumeration definition
 */
typedef enum sx127x_gfsk_crc_types_e
{
    SX127X_GFSK_CRC_OFF         = 0x00,
    SX127X_GFSK_CRC_2_BYTES_INV = 0x01,
} sx127x_gfsk_crc_type_t;

/**
 * @brief SX127X GFSK whitening control enumeration definition
 */
typedef enum sx127x_gfsk_dc_free_e
{
    SX127X_GFSK_DC_FREE_OFF        = 0x00,
    SX127X_GFSK_DC_FREE_MANCHESTER = 0x01,
    SX127X_GFSK_DC_FREE_WHITENING  = 0x02,
} sx127x_gfsk_dc_free_t;

/**
 * @brief SX127X LoRa packet length enumeration definition
 */
typedef enum sx127x_lora_pkt_len_modes_e
{
    SX127X_LORA_PKT_EXPLICIT = 0x00,  //!< Header included in the packet
    SX127X_LORA_PKT_IMPLICIT = 0x01,  //!< Header not included in the packet
} sx127x_lora_pkt_len_modes_t;

/**
 * @brief SX127X LoRa packet parameters structure definition
 */
typedef struct sx127x_lora_pkt_params_s
{
    uint16_t                    preamble_len_in_symb;  //!< Preamble length in symbols
    sx127x_lora_pkt_len_modes_t header_type;           //!< Header type
    uint8_t                     pld_len_in_bytes;      //!< Payload length in bytes
    bool                        crc_is_on;             //!< CRC activation
    bool                        invert_iq_is_on;       //!< IQ polarity setup
} sx127x_lora_pkt_params_t;

/**
 * @brief SX127X GFSK packet parameters structure definition
 */
typedef struct sx127x_gfsk_pkt_params_s
{
    uint16_t                        preamble_len_in_bytes;   //!< Preamble length in bytes
    sx127x_gfsk_preamble_detector_t preamble_detector;       //!< Preamble detection length
    uint8_t                         sync_word_len_in_bytes;  //!< Sync word length in bytes
    sx127x_gfsk_address_filtering_t address_filtering;       //!< Address filtering configuration
    sx127x_gfsk_pkt_len_modes_t     header_type;             //!< Header type
    uint8_t                         pld_len_in_bytes;        //!< Payload length in bytes
    sx127x_gfsk_crc_type_t          crc_type;                //!< CRC type configuration
    sx127x_gfsk_dc_free_t           dc_free;                 //!< Whitening configuration
} sx127x_gfsk_pkt_params_t;

/**
 * @brief SX127X RX buffer status structure definition
 */
typedef struct sx127x_rx_buffer_status_s
{
    uint8_t pld_len_in_bytes;      //!< Number of bytes available in the buffer
    uint8_t buffer_start_pointer;  //!< Position of the first byte in the buffer
} sx127x_rx_buffer_status_t;

/**
 * @brief SX127X GFSK packet status structure definition
 */
typedef struct sx127x_gfsk_pkt_status_s
{
    uint8_t rx_status;
    int8_t  rssi_sync;  //!< The RSSI measured on last packet
    int8_t  rssi_avg;   //!< The averaged RSSI
} sx127x_gfsk_pkt_status_t;

/**
 * @brief SX127X LoRa packet status structure definition
 */
typedef struct sx127x_lora_pkt_status_s
{
    int8_t rssi_pkt_in_dbm;        //!< RSSI of the last packet
    int8_t snr_pkt_in_db;          //!< SNR of the last packet
    int8_t signal_rssi_pkt_in_db;  //!< Estimation of RSSI (after despreading)
} sx127x_lora_pkt_status_t;

/**
 * @brief SX127X context
 */
typedef struct sx127x_s
{
    /*
     * -------------------------------------------------------------------------
     * --- PUBLIC --------------------------------------------------------------
     */
    const void* hal_context;  //!< Provides a pointer to the HAL context.
    /*
     * -------------------------------------------------------------------------
     * --- PRIVATE - INTERNAL DRIVER USE ONLY-----------------------------------
     */
    // Public API handling state
    sx127x_radio_id_t        radio_id;
    sx127x_pkt_type_t        pkt_type;
    uint16_t                 irq_flags;
    uint16_t                 irq_flags_mask;
    sx127x_pa_cfg_params_t   pa_cfg_params;
    uint32_t                 rf_freq_in_hz;
    sx127x_gfsk_mod_params_t gfsk_mod_params;
    sx127x_lora_mod_params_t lora_mod_params;
    sx127x_gfsk_pkt_params_t gfsk_pkt_params;
    sx127x_lora_pkt_params_t lora_pkt_params;
    sx127x_gfsk_pkt_status_t gfsk_rx_status;
    sx127x_lora_pkt_status_t lora_rx_status;
    // Private API handling state
    uint8_t op_mode;
    uint8_t op_mode_irq;
    bool    is_ook_on;
    bool    is_rx_continuous;
    bool    invert_iq_is_on;
    // Data buffer
    uint8_t buffer[SX127X_RX_TX_BUFFER_SIZE_MAX];
    uint8_t buffer_index;
    // GFSK/OOK
    bool    is_gfsk_sync_word_detected;
    int16_t gfsk_rssi_in_dbm;
    uint8_t gfsk_fifo_threshold_in_bytes;
    uint8_t gfsk_pkt_chunk_len_in_bytes;
#if defined( IS_LORA_FREQUENCY_HOPPING_SUPPORTED )
    bool is_lora_freq_hopping_on;
#endif
    // Registers
    uint8_t reg_dio_mapping[2];
    uint8_t reg_pa_config;
    uint8_t reg_pa_ramp;
    uint8_t reg_ocp;
    uint8_t reg_pa_dac;
    uint8_t reg_lora_sync_word;
    /*
     * -------------------------------------------------------------------------
     * --- USER IRQ HANDLING - DRIVER PUBLIC USE--------------------------------
     */
    void* irq_handler_context;  //!< Holds the context to be passed when calling irq_handler callaback
    /**
     * @brief User interrupt notification
     *
     * @remark Initialized by calling \ref sx127x_irq_attach API
     *
     * @param [in] irq_handler_context User provided context \ref irq_handler_context
     */
    void ( *irq_handler )( void* irq_handler_context );
    /*
     * -------------------------------------------------------------------------
     * --- INTERNAL HW DIO IRQ HANDLING - DRIVER PRIVATE USE -------------------
     */
    /**
     * @brief Rx timer timeout interrupt handler
     *
     * @remark Initialized and assigned by calling \ref sx127x_init API
     *
     * @param [in] context Chip implementation context.
     */
    void ( *rx_timer_irq_handler )( void* context );
    /**
     * @brief DIO_0 interrupt handler
     *
     * @remark Initialized and assigned by calling \ref sx127x_hal_dio_irq_attach API
     *
     * @param [in] context Chip implementation context.
     */
    void ( *dio_0_irq_handler )( void* context );
    /**
     * @brief DIO_1 interrupt handler
     *
     * @remark Initialized and assigned by calling \ref sx127x_hal_dio_irq_attach API
     *
     * @param [in] context Chip implementation context.
     */
    void ( *dio_1_irq_handler )( void* context );
    /**
     * @brief DIO_2 interrupt handler
     *
     * @remark Initialized and assigned by calling \ref sx127x_hal_dio_irq_attach API
     *
     * @param [in] context Chip implementation context.
     */
    void ( *dio_2_irq_handler )( void* context );
} sx127x_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/**
 * @brief Initializes radio driver context variable
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_init( sx127x_t* radio );

/**
 * @brief Initialize the driver user interrupt handler
 *
 * @param [in] irq_handler_context User provided context
 *
 * @returns status Operation status
 */
void sx127x_irq_attach( sx127x_t* radio, void ( *irq_handler )( void* irq_handler_context ),
                        void*     irq_handler_context );

//
// Operational Modes Functions
//

/**
 * @brief Set the chip in sleep mode
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_sleep( sx127x_t* radio );

/**
 * @brief Set the chip in stand-by mode
 *
 * @param [in] radio Chip implementation context.
 * @param [in] cfg Stand-by mode configuration
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_standby( sx127x_t* radio );

/**
 * @brief Set the chip in Tx frequency synthesis mode
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_fs_tx( sx127x_t* radio );

/**
 * @brief Set the chip in Rx frequency synthesis mode
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_fs_rx( sx127x_t* radio );

/**
 * @brief Set the chip in transmission mode
 *
 * @remark The packet type shall be configured with @ref sx127x_set_pkt_type before using this command.
 *
 * @remark By default, the chip returns automatically to standby mode as soon as the packet is sent.
 *
 * @param [in] radio    Chip implementation context.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_tx( sx127x_t* radio );

/**
 * @brief Set the chip in reception mode
 *
 * @remark The packet type shall be configured with @ref sx127x_set_pkt_type before using this command.
 *
 * @remark By default, the chip returns automatically to standby mode as soon as a packet is received
 * or if no packet has been received before the timeout.
 *
 * @remark The timeout argument can have the following special values:
 *
 * | Special values | Meaning                                                                                         |
 * | -----------    | ------------------------------------------------------------------------------------------------|
 * | 0x000000       | RX single: the chip stays in RX mode until a packet is received, then switch to standby RC mode |
 * | 0xFFFFFF       | RX continuous: the chip stays in RX mode even after reception of a packet                       |
 *
 * @param [in] radio    Chip implementation context.
 * @param [in] timeout_ms The timeout configuration for Rx operation
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_rx( sx127x_t* radio, const uint32_t timeout_ms );

/**
 * @brief Set the chip in CAD (Channel Activity Detection) mode
 *
 * @remark The packet type shall be configured to SX127X_PKT_TYPE_GFSK using @ref sx127x_set_pkt_type before using
 * this command.
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_cad( sx127x_t* radio );

/**
 * @brief Set the chip in Tx continuous wave (RF tone).
 *
 * @remark The packet type shall be configured to SX127X_PKT_TYPE_GFSK using @ref sx127x_set_pkt_type before using
 * this command.
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_tx_cw( sx127x_t* radio );

/**
 * @brief Perform device operating frequency band image rejection calibration
 *
 * @remark The packet type shall be configured to SX127X_PKT_TYPE_GFSK using @ref sx127x_set_pkt_type before using
 * this command.
 *
 * @param [in] radio Chip implementation context.
 * @param [in] freq_in_hz Frequency in Hz used for the image calibration
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_cal_img( sx127x_t* radio, const uint32_t freq_in_hz );

/**
 * @brief Configure the PA (Power Amplifier)
 *
 * @details When using this command, the user selects the PA to be used by the device as well as its configuration.
 *
 * @param [in] radio Chip implementation context.
 * @param [in] params Power amplifier configuration parameters
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_pa_cfg( sx127x_t* radio, const sx127x_pa_cfg_params_t* params );

//
// Registers and Buffer Access
//

/**
 * @brief Write data into register memory space.
 *
 * @param [in] radio Chip implementation context.
 * @param [in] address The register memory address to start writing operation
 * @param [in] buffer The buffer of bytes to write into memory.
 * @param [in] size Number of bytes to write into memory, starting from address
 *
 * @see sx127x_read_register
 */
sx127x_status_t sx127x_write_register( sx127x_t* radio, const uint16_t address, const uint8_t* buffer,
                                       const uint8_t size );

/**
 * @brief Read data from register memory space.
 *
 * @param [in] radio Chip implementation context.
 * @param [in] address The register memory address to start reading operation
 * @param [in] buffer The buffer of bytes to be filled with data from registers
 * @param [in] size Number of bytes to read from memory, starting from address
 *
 * @see sx127x_write_register
 */
sx127x_status_t sx127x_read_register( sx127x_t* radio, const uint16_t address, uint8_t* buffer, const uint8_t size );

/**
 * @brief Write data into radio Tx buffer memory space.
 *
 * @param [in] radio Chip implementation context.
 * @param [in] offset Start address in the Tx buffer of the chip
 * @param [in] buffer The buffer of bytes to write into radio buffer
 * @param [in] size The number of bytes to write into Tx radio buffer
 *
 * @returns Operation status
 *
 * @see sx127x_read_buffer
 */
sx127x_status_t sx127x_write_buffer( sx127x_t* radio, const uint8_t offset, const uint8_t* buffer, const uint8_t size );

/**
 * @brief Read data from radio Rx buffer memory space.
 *
 * @param [in] radio Chip implementation context.
 * @param [in] offset Start address in the Rx buffer of the chip
 * @param [in] buffer The buffer of bytes to be filled with content from Rx radio buffer
 * @param [in] size The number of bytes to read from the Rx radio buffer
 *
 * @returns Operation status
 *
 * @see sx127x_write_buffer
 */
sx127x_status_t sx127x_read_buffer( sx127x_t* radio, const uint8_t offset, uint8_t* buffer, const uint8_t size );

//
// DIO and IRQ Control Functions
//

/**
 * @brief Set which interrupt are enabled
 *
 * @remark DIO pins configuration are automatically handled
 *
 * @param [in] radio Chip implementation context.
 * @param [in] irq_mask Variable that holds the system interrupt mask
 *
 * @returns Operation status
 *
 * @see sx127x_clear_irq_status, sx127x_get_irq_status
 */
sx127x_status_t sx127x_set_irq_mask( sx127x_t* radio, const uint16_t irq_mask );

/**
 * @brief Get system interrupt status
 *
 * @param [in] radio Chip implementation context.
 * @param [out] irq_flags Pointer to a variable that holds the system interrupt status
 *
 * @returns Operation status
 *
 * @see sx127x_clear_irq_status
 */
sx127x_status_t sx127x_get_irq_status( sx127x_t* radio, uint16_t* irq_flags );

/**
 * @brief Clear selected system interrupts
 *
 * @param [in] radio Chip implementation context.
 * @param [in] irq_mask Variable that holds the system interrupt to be cleared
 *
 * @returns Operation status
 *
 * @see sx127x_get_irq_status
 */
sx127x_status_t sx127x_clear_irq_status( sx127x_t* radio, const uint16_t irq_mask );

/**
 * @brief Clears any radio irq status flags that are set and returns the flags that
 * were cleared.
 *
 * @param [in] radio Chip implementation context.
 * @param [out] irq_flags Pointer to a variable for holding the system interrupt status. Can be NULL.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_get_and_clear_irq_status( sx127x_t* radio, uint16_t* irq_flags );

//
// RF Modulation and Packet-Related Functions
//

/**
 * @brief Set the RF frequency for future radio operations.
 *
 * @remark This commands shall be called only after a packet type is selected.
 *
 * @param [in] radio Chip implementation context.
 * @param [in] freq_in_hz The frequency in Hz to set for radio operations
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_rf_freq( sx127x_t* radio, const uint32_t freq_in_hz );

/**
 * @brief Set the packet type
 *
 * @param [in] radio Chip implementation context.
 *
 * @param [in] pkt_type Packet type to set
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_pkt_type( sx127x_t* radio, const sx127x_pkt_type_t pkt_type );

/**
 * @brief Get the current packet type
 *
 * @param [in] radio Chip implementation context.
 * @param [out] pkt_type Pointer to a variable holding the packet type
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_get_pkt_type( sx127x_t* radio, sx127x_pkt_type_t* pkt_type );

/**
 * @brief Set the parameters for TX power and power amplifier ramp time
 *
 * @param [in] radio Chip implementation context.
 * @param [in] pwr_in_dbm The Tx output power in dBm
 * @param [in] ramp_time The ramping time configuration for the PA
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_tx_params( sx127x_t* radio, const int8_t pwr_in_dbm, const sx127x_ramp_time_t ramp_time );

/**
 * @brief Set the modulation parameters for GFSK packets
 *
 * @param [in] radio Chip implementation context.
 * @param [in] params The structure of GFSK modulation configuration
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_gfsk_mod_params( sx127x_t* radio, const sx127x_gfsk_mod_params_t* params );

/**
 * @brief Set the modulation parameters for LoRa packets
 *
 * @param [in] radio Chip implementation context.
 * @param [in] params The structure of LoRa modulation configuration
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_lora_mod_params( sx127x_t* radio, const sx127x_lora_mod_params_t* params );

/**
 * @brief Set the packet parameters for GFSK packets
 *
 * @param [in] radio Chip implementation context.
 * @param [in] params The structure of GFSK packet configuration
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_gfsk_pkt_params( sx127x_t* radio, const sx127x_gfsk_pkt_params_t* params );

/**
 * @brief Set the packet parameters for LoRa packets
 *
 * @param [in] radio Chip implementation context.
 * @param [in] params The structure of LoRa packet configuration
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_lora_pkt_params( sx127x_t* radio, const sx127x_lora_pkt_params_t* params );

/**
 * @brief Set LoRa number of symbols for Rx single timeout
 *
 * @param [in] radio Chip implementation context.
 * @param [in] nb_of_symbs Number of symbols
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_lora_sync_timeout( sx127x_t* radio, const uint16_t nb_of_symbs );

//
// Communication Status Information
//

/**
 * @brief Get the current Rx buffer status for both LoRa and GFSK Rx operations
 *
 * @details This function is used to get the length of the received payload and the start address to be used when
 * reading data from the Rx buffer.
 *
 * @param [in] radio Chip implementation context.
 * @param [out] rx_buffer_status Pointer to a structure to store the current status
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_get_rx_buffer_status( sx127x_t* radio, sx127x_rx_buffer_status_t* rx_buffer_status );

/**
 * @brief Get the status of the last GFSK packet received
 *
 * @param [in] radio Chip implementation context.
 * @param [out] pkt_status Pointer to a structure to store the packet status
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_get_gfsk_pkt_status( sx127x_t* radio, sx127x_gfsk_pkt_status_t* pkt_status );

/**
 * @brief Get the status of the last LoRa packet received
 *
 * @param [in] radio Chip implementation context.
 * @param [out] pkt_status Pointer to a structure to store the packet status
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_get_lora_pkt_status( sx127x_t* radio, sx127x_lora_pkt_status_t* pkt_status );

/**
 * @brief Get the instantaneous RSSI value.
 *
 * @remark This function shall be called when in Rx mode.
 *
 * @param [in] radio Chip implementation context.
 * @param [out] rssi_in_dbm Pointer to a variable to store the RSSI value in dBm
 *
 * @returns Operation status
 *
 * @see sx127x_set_rx
 */
sx127x_status_t sx127x_get_rssi_inst( sx127x_t* radio, int16_t* rssi_in_dbm );

//
// Miscellaneous
//

/**
 * @brief Perform a hard reset of the chip
 *
 * @param [in] radio Chip implementation context.
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_reset( sx127x_t* radio );

/**
 * @brief Get the parameter corresponding to a GFSK Rx bandwith immediately above the minimum requested one.
 *
 * @param [in] bw Minimum required bandwith in Hz
 * @param [out] param Pointer to a value to store the parameter
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_get_gfsk_bw_param( const uint32_t bw, uint8_t* param );

/**
 * @brief Get the actual value in Hertz of a given LoRa bandwidth
 *
 * @param [in] bw LoRa bandwidth parameter
 *
 * @returns Actual LoRa bandwidth in Hertz
 */
uint32_t sx127x_get_lora_bw_in_hz( sx127x_lora_bw_t bw );

/**
 * @brief Compute the numerator for LoRa time-on-air computation.
 *
 * @remark To get the actual time-on-air in second, this value has to be divided by the LoRa bandwidth in Hertz.
 *
 * @param [in] pkt_params Pointer to the structure holding the LoRa packet parameters
 * @param [in] mod_params Pointer to the structure holding the LoRa modulation parameters
 *
 * @returns LoRa time-on-air numerator
 */
uint32_t sx127x_get_lora_time_on_air_numerator( const sx127x_lora_pkt_params_t* pkt_params,
                                                const sx127x_lora_mod_params_t* mod_params );

/**
 * @brief Get the time on air in ms for LoRa transmission
 *
 * @param [in] pkt_params Pointer to a structure holding the LoRa packet parameters
 * @param [in] mod_params Pointer to a structure holding the LoRa modulation parameters
 *
 * @returns Time-on-air value in ms for LoRa transmission
 */
uint32_t sx127x_get_lora_time_on_air_in_ms( const sx127x_lora_pkt_params_t* pkt_params,
                                            const sx127x_lora_mod_params_t* mod_params );

/**
 * @brief Compute the numerator for GFSK time-on-air computation.
 *
 * @remark To get the actual time-on-air in second, this value has to be divided by the GFSK bitrate in bits per
 * second.
 *
 * @param [in] pkt_params Pointer to the structure holding the GFSK packet parameters
 *
 * @returns GFSK time-on-air numerator
 */
uint32_t sx127x_get_gfsk_time_on_air_numerator( const sx127x_gfsk_pkt_params_t* pkt_params );

/**
 * @brief Get the time on air in ms for GFSK transmission
 *
 * @param [in] pkt_params Pointer to a structure holding the GFSK packet parameters
 * @param [in] mod_params Pointer to a structure holding the GFSK modulation parameters
 *
 * @returns Time-on-air value in ms for GFSK transmission
 */
uint32_t sx127x_get_gfsk_time_on_air_in_ms( const sx127x_gfsk_pkt_params_t* pkt_params,
                                            const sx127x_gfsk_mod_params_t* mod_params );

/**
 * @brief Generate one or more 32-bit random numbers.
 *
 * @remark SX127X_PKT_TYPE_LORA must have been configured with @ref sx127x_set_pkt_type
 *         before using this command.
 *
 * @param [in] radio Chip implementation context.
 * @param [out] numbers Array where numbers will be stored.
 * @param [in] n Number of desired random numbers.
 *
 * \returns Operation status
 *
 * This code can potentially result in interrupt generation. It is the responsibility of
 * the calling code to disable radio interrupts before calling this function,
 * and re-enable them afterwards if necessary, or be certain that any interrupts
 * generated during this process will not cause undesired side-effects in the software.
 *
 * Please note that the random numbers produced by the generator do not have a uniform or Gaussian distribution. If
 * uniformity is needed, perform appropriate software post-processing.
 */
sx127x_status_t sx127x_get_random_numbers( sx127x_t* radio, uint32_t* numbers, unsigned int n );

/**
 * @brief Get the number of PLL steps for a given frequency in Hertz
 *
 * @param [in] freq_in_hz Frequency in Hertz
 *
 * @returns Number of PLL steps
 */
uint32_t sx127x_convert_freq_in_hz_to_pll_step( uint32_t freq_in_hz );

/**
 * @brief Implement workaround to be called when a transmission timeout is detected.
 *
 * @remark A transmission timeout shouldn't happen.
 * Reported issue of SPI data corruption resulting in TX TIMEOUT is NOT related to a bug in radio transceiver.
 * It is mainly caused by improper PCB routing of SPI lines and/or violation of SPI specifications.
 * To mitigate redesign, Semtech offers a workaround which resets the radio transceiver and putting it into a known
 * state.
 *
 * @param [in] radio Chip implementation context.
 *
 * \returns Operation status
 */
sx127x_status_t sx127x_tx_timeout_irq_workaround( sx127x_t* radio );

//
// Registers access
//

/**
 * @brief Configure the sync word used in GFSK packet
 *
 * @param [in] radio Chip implementation context.
 * @param [in] sync_word Buffer holding the sync word to be configured
 * @param [in] sync_word_len Sync word length in byte
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_gfsk_sync_word( sx127x_t* radio, const uint8_t* sync_word, const uint8_t sync_word_len );

/**
 * @brief Configure the sync word used in LoRa packet
 *
 * @remark In the case of a LoRaWAN use case, the two following values are specified:
 *   - 0x12 for a private LoRaWAN network (default)
 *   - 0x34 for a public LoRaWAN network
 *
 * @param [in] radio Chip implementation context.
 * @param [in] sync_word Sync word to be configured
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_lora_sync_word( sx127x_t* radio, const uint8_t sync_word );

/**
 * @brief Configure the Over Current Protection (OCP) value
 *
 * @remark Imax = 45 + 5 * ocp_trim_value   [mA] if ocp_trim_value <= 15 (120 [mA])
           Imax = -30 + 10 * ocp_trim_value [mA] if 15 < ocp_trim_value <= 27 (130 to 240 [mA])
           Imax = 240 [mA] for higher settings
           Default Imax = 100mA (ocp_trim_value = 0x0B)
 *
 * @param [in] context Chip implementation context.
 * @param [in] ocp_trim_value OCP trim value
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_set_ocp_value( sx127x_t* radio, const uint8_t ocp_trim_value );

/**
 * @brief Get LoRa coding rate and CRC configurations from received header
 *
 * @remark The output of this function is only valid if the field header_type of pkt_params is equal to @ref
 * SX126X_LORA_PKT_EXPLICIT when calling @ref sx127x_set_lora_pkt_params()
 * @remark The values for rx_cr and is_crc_on are extracted from the header of the received LoRa packet
 *
 * @param [in]  context    Chip implementation context.
 * @param [out] rx_cr      LoRa coding rate
 * @param [out] is_crc_on  LoRa CRC configuration
 *
 * @returns Operation status
 */
sx127x_status_t sx127x_get_lora_params_from_header( sx127x_t* radio, sx127x_lora_cr_t* rx_cr, bool* is_crc_on );

#ifdef __cplusplus
}
#endif

#endif  // SX127X_H

/* --- EOF ------------------------------------------------------------------ */
