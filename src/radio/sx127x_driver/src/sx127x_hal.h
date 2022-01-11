/**
 * @file      sx127x_hal.h
 *
 * @brief     SX1272/3 and SX1276/7/8/9  HAL functions to be implemented
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
#ifndef SX127X_HAL_H
#define SX127X_HAL_H

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
#include "sx127x.h"

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

/**
 * @brief HAL API functions return status
 */
typedef enum sx127x_hal_status_e
{
    SX127X_HAL_STATUS_OK    = 0,
    SX127X_HAL_STATUS_ERROR = 3,
} sx127x_hal_status_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- API definitions to be implemented by the user ---------------------------
 */

/**
 * @brief Return the radio identifier
 *
 * @param [in] context Radio implementation parameters
 *
 * @returns radio_id The radio identifier
 */
sx127x_radio_id_t sx127x_hal_get_radio_id( const sx127x_t* radio );

/**
 * @brief Initialize and assign DIO interrupt handlers to internal radio driver
 *
 * @param [in] context Radio implementation parameters
 *
 * @returns status Operation status
 */
void sx127x_hal_dio_irq_attach( const sx127x_t* radio );

/**
 * @brief Radio data transfer - write
 *
 * @remark Must be implemented by the upper layer
 *
 * @param [in] context Radio implementation parameters
 * @param [in] address Radio register address
 * @param [in] data Pointer to the buffer to be transmitted
 * @param [in] data_len Buffer size to be transmitted
 *
 * @returns status Operation status
 */
sx127x_hal_status_t sx127x_hal_write( const sx127x_t* radio, const uint16_t address, const uint8_t* data,
                                      const uint16_t data_len );

/**
 * @brief Radio data transfer - read
 *
 * @remark Must be implemented by the upper layer
 *
 * @param [in] context Radio implementation parameters
 * @param [in] address Radio register address
 * @param [in] data Pointer to the buffer to be received
 * @param [in] data_len Buffer size to be received
 *
 * @returns status Operation status
 */
sx127x_hal_status_t sx127x_hal_read( const sx127x_t* radio, const uint16_t address, uint8_t* data,
                                     const uint16_t data_len );

/**
 * @brief Reset the radio
 *
 * @remark Must be implemented by the upper layer
 *
 * @param [in] context Radio implementation parameters
 */
void sx127x_hal_reset( const sx127x_t* radio );

/**
 * @brief Get current state of DIO1 pin state
 *
 * @remark DIO1 pin must be setup to trigger the IRQ on rising and falling edges
 *
 * @returns DIO1 pin current state
 */
uint32_t sx127x_hal_get_dio_1_pin_state( const sx127x_t* radio );

/**
 * @brief Start timer and execute provided callback when it expires
 *
 * @remark Radio context must be provided to the timer callback
 *
 * @param [in] context Radio implementation parameters
 * @param [in] time_in_ms Time required before calling the callback function
 * @param [in] callback Callback function to be called
 *
 * @returns status Operation status
 */
sx127x_hal_status_t sx127x_hal_timer_start( const sx127x_t* radio, const uint32_t time_in_ms,
                                            void ( *callback )( void* context ) );

/**
 * @brief Stop timer
 *
 * @param [in] context Radio implementation parameters
 *
 * @returns status Operation status
 */
sx127x_hal_status_t sx127x_hal_timer_stop( const sx127x_t* radio );

/**
 * @brief Return timer start/stop status
 *
 * @param [in] context Radio implementation parameters
 *
 * @returns is_started Is timer started
 */
bool sx127x_hal_timer_is_started( const sx127x_t* radio );

#ifdef __cplusplus
}
#endif

#endif  // SX127X_HAL_H
