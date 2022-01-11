/*!
 * \file  LmHandlerTypes.c
 *
 * \brief Defines the types used by LmHandler
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
#ifndef __LORAMAC_HANDLER_TYPES_H__
#define __LORAMAC_HANDLER_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "LoRaMac.h"

/*!
 * If set to 1 the new API defining \ref OnSysTimeUpdate callback is used.
 */
#define LMH_SYS_TIME_UPDATE_NEW_API                 1

/*!
 *
 */
typedef enum
{
    LORAMAC_HANDLER_ADR_OFF = 0,
    LORAMAC_HANDLER_ADR_ON = !LORAMAC_HANDLER_ADR_OFF
}LmHandlerAdrStates_t;

/*!
 *
 */
typedef enum
{
    LORAMAC_HANDLER_RESET = 0,
    LORAMAC_HANDLER_SET = !LORAMAC_HANDLER_RESET
}LmHandlerFlagStatus_t;

/*!
 *
 */
typedef enum
{
    LORAMAC_HANDLER_ERROR = -1,
    LORAMAC_HANDLER_SUCCESS = 0
}LmHandlerErrorStatus_t;

/*!
 *
 */
typedef enum
{
    LORAMAC_HANDLER_UNCONFIRMED_MSG = 0,
    LORAMAC_HANDLER_CONFIRMED_MSG = !LORAMAC_HANDLER_UNCONFIRMED_MSG
}LmHandlerMsgTypes_t;

/*!
 *
 */
typedef enum
{
    LORAMAC_HANDLER_FALSE = 0,
    LORAMAC_HANDLER_TRUE = !LORAMAC_HANDLER_FALSE
}LmHandlerBoolean_t;

typedef enum
{
    LORAMAC_HANDLER_BEACON_ACQUIRING,
    LORAMAC_HANDLER_BEACON_LOST,
    LORAMAC_HANDLER_BEACON_RX,
    LORAMAC_HANDLER_BEACON_NRX
}LmHandlerBeaconState_t;

typedef enum
{
    LORAMAC_HANDLER_NVM_RESTORE,
    LORAMAC_HANDLER_NVM_STORE,
}LmHandlerNvmContextStates_t;

/*!
 * Commissioning parameters
 */
typedef struct CommissioningParams_s
{
    bool IsOtaaActivation;
    uint8_t DevEui[8];
    uint8_t JoinEui[8];
    uint8_t SePin[4];
    uint32_t NetworkId;
    uint32_t DevAddr;
}CommissioningParams_t;

/*!
 * Application data structure
 */
typedef struct LmHandlerAppData_s
{
    uint8_t Port;
    uint8_t BufferSize;
    uint8_t *Buffer;
}LmHandlerAppData_t;

typedef struct LmHandlerRequestParams_s
{
    uint8_t IsMcpsRequest;
    LoRaMacStatus_t Status;
    union
    {
        Mcps_t Mcps;
        Mlme_t Mlme;
    }RequestType;
}LmHandlerRequestParams_t;

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_HANDLER_TYPES_H__
