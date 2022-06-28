/*!
 * \file  radio_board.h
 *
 * \brief Radio board Hardware Abstraction Layer
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
#ifndef RADIO_BOARD_H
#define RADIO_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"
#include "spi.h"
#include "ral.h"
#if defined( SX126X )
#include "ral_sx126x_bsp.h"
#elif defined( LR11XX )
#include "ral_lr11xx_bsp.h"
#elif defined( SX127X )
#include "ral_sx127x_bsp.h"
#else
#error "Please define a radio to be used"
#endif

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
typedef enum radio_board_status_t
{
    RADIO_BOARD_STATUS_OK,
    RADIO_BOARD_STATUS_UNSUPPORTED_FEATURE,
    RADIO_BOARD_STATUS_UNKNOWN_VALUE,
    RADIO_BOARD_STATUS_ERROR,
} radio_board_status_t;

/*!
 * \brief Type describing the radio parameters
 */
typedef struct radio_params_s
{
    uint32_t rf_freq_in_hz;
    int8_t   tx_rf_pwr_in_dbm;
    bool     is_public_network;
    bool     is_image_calibrated;
    bool     is_irq_fired;
    bool     is_rx_continuous;
    uint8_t  max_payload_length;
    uint32_t tx_timeout_in_ms;
    uint32_t rx_timeout_in_ms;
#if( LORAMAC_LR_FHSS_IS_ON == 1 )
    struct
    {
        bool                 is_lr_fhss_on;
        uint16_t             hop_sequence_id;
        ral_lr_fhss_params_t lr_fhss_params;
    } lr_fhss;
#endif
    uint8_t buffer[255];
} radio_params_t;

/*!
 * \brief Radio operating modes
 */
typedef enum radio_board_operating_mode_e
{
    RADIO_BOARD_OP_MODE_SLEEP = 0x00,  //! The radio is in sleep mode
    RADIO_BOARD_OP_MODE_STDBY,         //! The radio is in standby mode
    RADIO_BOARD_OP_MODE_TX,            //! The radio is in transmit mode
    RADIO_BOARD_OP_MODE_RX,            //! The radio is in receive single mode
    RADIO_BOARD_OP_MODE_RX_C,          //! The radio is in receive continuous mode
    RADIO_BOARD_OP_MODE_RX_DC,         //! The radio is in receive duty cycle mode (not available for SX127x)
    RADIO_BOARD_OP_MODE_CAD            //! The radio is in channel activity detection mode
} radio_board_operating_mode_t;

/*!
 * Radio hardware and global parameters
 */
typedef struct radio_context_s
{
    Spi_t  spi;
    Gpio_t reset;
#if defined( SX126X ) || defined( LR11XX )
    Gpio_t busy;
    Gpio_t dio_1;
#elif defined( SX127X )
    Gpio_t dio_0;
    Gpio_t dio_1;
    Gpio_t dio_2;
#endif
#if defined( USE_RADIO_DEBUG )
    Gpio_t dbg_tx;
    Gpio_t dbg_rx;
#endif
    radio_board_operating_mode_t op_mode;
    radio_params_t               radio_params;
} radio_context_t;

/*!
 * Hardware IO IRQ callback function definition
 */
typedef void ( *radio_board_dio_irq_handler )( void* context );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/*!
 * \brief Get pointer to the radio board context variable
 *
 * \returns Pointer to the radio board context variable
 */
radio_context_t* radio_board_get_radio_context_reference( void );

#if( LORAMAC_LR_FHSS_IS_ON == 1 )
/*!
 * \brief Get pointer to the LR-FHSS state structure
 *
 * \returns Pointer to the LR-FHSS state structure
 */
void* radio_board_get_lr_fhss_state_reference( void );
#endif

#if defined( SX127X )
/*!
 * \brief Get pointer to the sx127x context variable
 *
 * \returns Pointer to the sx127x context variable
 */
sx127x_t* radio_board_get_sx127x_context_reference( void );
#endif

/*!
 * \brief Get pointer to the RAL context variable
 *
 * \returns Pointer to the RAL context variable
 */
ral_t* radio_board_get_ral_context_reference( void );

/*!
 * \brief Initializes the radio driver
 *
 * \param [in] ral_context RAL implementation context
 * \param [in] dio_irq Callback function to be attached to a GPIO
 *
 * \returns Operation status
 */
ral_status_t radio_board_init( const ral_t* ral_context, radio_board_dio_irq_handler dio_irq );

/*!
 * \brief Initializes the radio I/Os pins interface
 *
 * \param [in] ral_context RAL implementation context
 */
void radio_board_init_io( void );

/*!
 * \brief De-initializes the radio I/Os pins interface.
 *
 * \remark Useful when going in MCU low power modes
 *
 * \param [in] ral_context RAL implementation context
 */
void radio_board_deinit_io( void );

/*!
 * \brief Initializes the radio debug pins.
 *
 * \param [in] ral_context RAL implementation context
 */
void radio_board_init_dbg_io( void );

/*!
 * Set antenna switch for Tx operation or not.
 *
 * \param [in] ral_context RAL implementation context
 * \param [in] is_tx_on Indicates if the antenna switch must be set for Tx operation or not
 */
void radio_board_set_ant_switch( bool is_tx_on );

/*!
 * Start radio TCXO
 *
 * \remark In case used radio has no TCXO please implement an empty function
 */
void radio_board_start_radio_tcxo( void );

/*!
 * Stop radio TCXO
 *
 * \remark In case used radio has no TCXO please implement an empty function
 */
void radio_board_stop_radio_tcxo( void );

/*!
 * \brief Gets the time in milliseconds required for the TCXO to wakeup
 *
 * \returns Radio board TCXO wakeup time in ms.
 */
uint32_t radio_board_get_tcxo_wakeup_time_in_ms( void );

/*!
 * Get radio operating mode
 *
 * \returns Radio current operating mode
 */
radio_board_operating_mode_t radio_board_get_operating_mode( void );

/*!
 * Set radio operating mode
 *
 * \param [in] op_mode Radio operating mode
 */
void radio_board_set_operating_mode( const radio_board_operating_mode_t op_mode );

/*!
 * \brief Gets current state of DIO1 pin state.
 *
 * \returns DIO1 pin current state.
 */
uint32_t radio_board_get_dio_1_pin_state( void );

/*!
 * \brief Initializes the radio bsp
 *
 * \param [in] ral_context RAL implementation context
 * \param [in] dio_irq Callback function to be attached to a GPIO
 *
 * \returns Operation status
 */
#if defined( SX126X )
ral_status_t ral_sx126x_bsp_init( const ral_t* ral_context, radio_board_dio_irq_handler dio_irq );
#elif defined( LR11XX )
ral_status_t ral_lr11xx_bsp_init( const ral_t* ral_context, radio_board_dio_irq_handler dio_irq );
#elif defined( SX127X )
ral_status_t ral_sx127x_bsp_init( const ral_t* ral_context, radio_board_dio_irq_handler dio_irq );
#endif

#ifdef __cplusplus
}
#endif

#endif  // RADIO_BOARD_H

/* --- EOF ------------------------------------------------------------------ */
