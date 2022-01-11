/**
 * @file      lr_fhss_v1_base_types.h
 *
 * @brief     Radio-independent LR-FHSS base type definitions, version 1
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

#ifndef LR_FHSS_V1_BASE_TYPES_H__
#define LR_FHSS_V1_BASE_TYPES_H__

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>

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
 * @brief LR-FHSS modulation type
 */
typedef enum lr_fhss_v1_modulation_type_e
{
    LR_FHSS_V1_MODULATION_TYPE_GMSK_488 = 0,
} lr_fhss_v1_modulation_type_t;

/**
 * @brief LR-FHSS coding rate
 */
typedef enum lr_fhss_v1_cr_e
{
    LR_FHSS_V1_CR_5_6 = 0x00,
    LR_FHSS_V1_CR_2_3 = 0x01,
    LR_FHSS_V1_CR_1_2 = 0x02,
    LR_FHSS_V1_CR_1_3 = 0x03,
} lr_fhss_v1_cr_t;

/**
 * @brief LR-FHSS grid
 */
typedef enum lr_fhss_v1_grid_e
{
    LR_FHSS_V1_GRID_25391_HZ = 0x00,
    LR_FHSS_V1_GRID_3906_HZ  = 0x01,
} lr_fhss_v1_grid_t;

/**
 * @brief LR-FHSS bandwidth
 */
typedef enum lr_fhss_v1_bw_e
{
    LR_FHSS_V1_BW_39063_HZ   = 0x00,
    LR_FHSS_V1_BW_85938_HZ   = 0x01,
    LR_FHSS_V1_BW_136719_HZ  = 0x02,
    LR_FHSS_V1_BW_183594_HZ  = 0x03,
    LR_FHSS_V1_BW_335938_HZ  = 0x04,
    LR_FHSS_V1_BW_386719_HZ  = 0x05,
    LR_FHSS_V1_BW_722656_HZ  = 0x06,
    LR_FHSS_V1_BW_773438_HZ  = 0x07,
    LR_FHSS_V1_BW_1523438_HZ = 0x08,
    LR_FHSS_V1_BW_1574219_HZ = 0x09,
} lr_fhss_v1_bw_t;

/**
 * @brief LR-FHSS parameter structure
 */
typedef struct lr_fhss_v1_params_s
{
    const uint8_t*               sync_word; /**< 4-byte sync word */
    lr_fhss_v1_modulation_type_t modulation_type;
    lr_fhss_v1_cr_t              cr;
    lr_fhss_v1_grid_t            grid;
    lr_fhss_v1_bw_t              bw;
    bool                         enable_hopping;
    uint8_t                      header_count; /**< Number of header blocks */
} lr_fhss_v1_params_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

#endif  // LR_FHSS_V1_BASE_TYPES_H__

/* --- EOF ------------------------------------------------------------------ */
