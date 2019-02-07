/*!
 * \file      LmHandlerTypes.h
 *
 * \brief     Defines the types used by LmHandler
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */
#ifndef __LORAMAC_HANDLER_TYPES_H__
#define __LORAMAC_HANDLER_TYPES_H__

#include "LoRaMac.h"

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
#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
    uint8_t GenAppKey[16];
#else
    uint8_t AppKey[16];
#endif
    uint8_t NwkKey[16];
    uint32_t NetworkId;
    uint32_t DevAddr;
#if ( OVER_THE_AIR_ACTIVATION == 0 )
    uint8_t FNwkSIntKey[16];
    uint8_t SNwkSIntKey[16];
    uint8_t NwkSEncKey[16];
    uint8_t AppSKey[16];
#endif
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

#endif // __LORAMAC_HANDLER_TYPES_H__