/*!
 * \file      LmHandler.c
 *
 * \brief     Implements the LoRaMac layer handling. 
 *            Provides the possibility to register applicative packages.
 *
 * \remark    Inspired by the examples provided on the en.i-cube_lrwan fork.
 *            MCD Application Team ( STMicroelectronics International )
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "utilities.h"
#include "timer.h"
#include "Commissioning.h"
#include "NvmCtxMgmt.h"
#include "LmHandler.h"
#include "LmhPackage.h"
#include "LmhpCompliance.h"
#include "LmhpClockSync.h"
#include "LmhpRemoteMcastSetup.h"
#include "LmhpFragmentation.h"

#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif

#if defined( REGION_EU868 )

#include "LoRaMacTest.h"

#endif

static CommissioningParams_t CommissioningParams = 
{
    .IsOtaaActivation = OVER_THE_AIR_ACTIVATION,
    .DevEui = LORAWAN_DEVICE_EUI,
    .JoinEui = LORAWAN_JOIN_EUI,
#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
    .GenAppKey = LORAWAN_GEN_APP_KEY,
#else
    .AppKey = LORAWAN_APP_KEY,
#endif
    .NwkKey = LORAWAN_NWK_KEY,

#if( OVER_THE_AIR_ACTIVATION == 0 )

    .NetworkId = LORAWAN_NETWORK_ID,
    .DevAddr = LORAWAN_DEVICE_ADDRESS,
    .FNwkSIntKey = LORAWAN_F_NWK_S_INT_KEY,
    .SNwkSIntKey = LORAWAN_S_NWK_S_INT_KEY,
    .NwkSEncKey = LORAWAN_NWK_S_ENC_KEY,
    .AppSKey = LORAWAN_APP_S_KEY,

#endif
};

static LmhPackage_t *LmHandlerPackages[PKG_MAX_NUMBER];

/*!
 * Upper layer LoRaMac parameters
 */
static LmHandlerParams_t *LmHandlerParams;

/*!
 * Upper layer callbacks
 */
static LmHandlerCallbacks_t *LmHandlerCallbacks;

/*!
 * Used to notify LmHandler of LoRaMac events
 */
static LoRaMacPrimitives_t LoRaMacPrimitives;

/*!
 * LoRaMac callbacks
 */
static LoRaMacCallback_t LoRaMacCallbacks;

static LmHandlerJoinParams_t JoinParams = 
{
    .CommissioningParams = &CommissioningParams,
    .Datarate = DR_0,
    .Status = LORAMAC_HANDLER_ERROR
};

static LmHandlerTxParams_t TxParams = 
{
    .CommissioningParams = &CommissioningParams,
    .MsgType = LORAMAC_HANDLER_UNCONFIRMED_MSG,
    .AckReceived = 0,
    .Datarate = DR_0,
    .UplinkCounter = 0,
    .AppData =
    {
        .Port = 0,
        .BufferSize = 0,
        .Buffer = NULL
    },
    .TxPower = TX_POWER_0,
    .Channel = 0
};

static LmHandlerRxParams_t RxParams = 
{
    .CommissioningParams = &CommissioningParams,
    .Rssi = 0,
    .Snr = 0,
    .DownlinkCounter = 0,
    .RxSlot = -1
};

static LoRaMAcHandlerBeaconParams_t BeaconParams = 
{
    .State = LORAMAC_HANDLER_BEACON_ACQUIRING,
    .Info = 
    { 
        .Time = { .Seconds = 0, .SubSeconds = 0 },
        .Frequency = 0,
        .Datarate = 0,
        .Rssi = 0,
        .Snr = 0,
        .GwSpecific =
        {
            .InfoDesc = 0,
            .Info = { 0 }
        }
    }
};

/*!
 * Indicates if a switch to Class B operation is pending or not.
 * 
 * TODO: Create a new structure to store the current handler states/status
 *       and add the below variable to it.
 */
static bool IsClassBSwitchPending = false;

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *                             containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm );

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication( McpsIndication_t *mcpsIndication );

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] MlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm );

/*!
 * \brief   MLME-Indication event function
 *
 * \param   [IN] mlmeIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void MlmeIndication( MlmeIndication_t *mlmeIndication );

/*!
 * Requests network server time update
 *
 * \retval status Returns \ref LORAMAC_HANDLER_SET if joined else \ref LORAMAC_HANDLER_RESET
 */
static LmHandlerErrorStatus_t LmHandlerDeviceTimeReq( void );

/*!
 * Starts the beacon search
 *
 * \retval status Returns \ref LORAMAC_HANDLER_SET if joined else \ref LORAMAC_HANDLER_RESET
 */
static LmHandlerErrorStatus_t LmHandlerBeaconReq( void );

/*
 *=============================================================================
 * PACKAGES HANDLING
 *=============================================================================
 */
typedef enum PackageNotifyTypes_e
{
    PACKAGE_MCPS_CONFIRM,
    PACKAGE_MCPS_INDICATION,
    PACKAGE_MLME_CONFIRM,
    PACKAGE_MLME_INDICATION,
}PackageNotifyTypes_t;

/*!
 * Notifies the package to process the LoRaMac callbacks.
 * 
 * \param [IN] notifyType MAC notification type [PACKAGE_MCPS_CONFIRM,
 *                                               PACKAGE_MCPS_INDICATION,
 *                                               PACKAGE_MLME_CONFIRM,
 *                                               PACKAGE_MLME_INDICATION]
 * \param[IN] params      Notification parameters. The params type can be
 *                        [McpsConfirm_t, McpsIndication_t, MlmeConfirm_t, MlmeIndication_t]
 */
static void LmHandlerPackagesNotify( PackageNotifyTypes_t notifyType, void *params );

static void LmHandlerPackagesProcess( void );

LmHandlerErrorStatus_t LmHandlerInit( LmHandlerCallbacks_t *handlerCallbacks,
                                      LmHandlerParams_t *handlerParams )
{
    //
    MibRequestConfirm_t mibReq;
    LmHandlerParams = handlerParams;
    LmHandlerCallbacks = handlerCallbacks;

    LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
    LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
    LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
    LoRaMacPrimitives.MacMlmeIndication = MlmeIndication;
    LoRaMacCallbacks.GetBatteryLevel = LmHandlerCallbacks->GetBatteryLevel;
    LoRaMacCallbacks.GetTemperatureLevel = LmHandlerCallbacks->GetTemperature;
    LoRaMacCallbacks.NvmContextChange = NvmCtxMgmtEvent;
    LoRaMacCallbacks.MacProcessNotify = LmHandlerCallbacks->OnMacProcess;

    IsClassBSwitchPending = false;

    if( LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LmHandlerParams->Region ) != LORAMAC_STATUS_OK )
    {
        return LORAMAC_HANDLER_ERROR;
    }

    // Try to restore from NVM and query the mac if possible.
    if( NvmCtxMgmtRestore( ) == NVMCTXMGMT_STATUS_SUCCESS )
    {
        LmHandlerCallbacks->OnNvmContextChange( LORAMAC_HANDLER_NVM_RESTORE );
    }
    else
    {
#if( OVER_THE_AIR_ACTIVATION == 0 )
        // Tell the MAC layer which network server version are we connecting too.
        mibReq.Type = MIB_ABP_LORAWAN_VERSION;
        mibReq.Param.AbpLrWanVersion.Value = ABP_ACTIVATION_LRWAN_VERSION;
        LoRaMacMibSetRequestConfirm( &mibReq );
#endif

#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
        mibReq.Type = MIB_GEN_APP_KEY;
        mibReq.Param.GenAppKey = CommissioningParams.GenAppKey;
        LoRaMacMibSetRequestConfirm( &mibReq );
#else
        mibReq.Type = MIB_APP_KEY;
        mibReq.Param.AppKey = CommissioningParams.AppKey;
        LoRaMacMibSetRequestConfirm( &mibReq );
#endif

        mibReq.Type = MIB_NWK_KEY;
        mibReq.Param.NwkKey = CommissioningParams.NwkKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

#if( STATIC_DEVICE_EUI != 1 )
        LmHandlerCallbacks->GetUniqueId( CommissioningParams.DevEui );
#endif

#if( OVER_THE_AIR_ACTIVATION == 0 )

#if( STATIC_DEVICE_ADDRESS != 1 )
        // Random seed initialization
        srand1( LmHandlerCallbacks->GetRandomSeed( ) );
        // Choose a random device address
        CommissioningParams.DevAddr = randr( 0, 0x01FFFFFF );
#endif

        mibReq.Type = MIB_NET_ID;
        mibReq.Param.NetID = LORAWAN_NETWORK_ID;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_DEV_ADDR;
        mibReq.Param.DevAddr = CommissioningParams.DevAddr;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_F_NWK_S_INT_KEY;
        mibReq.Param.FNwkSIntKey = CommissioningParams.FNwkSIntKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_S_NWK_S_INT_KEY;
        mibReq.Param.SNwkSIntKey = CommissioningParams.SNwkSIntKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_NWK_S_ENC_KEY;
        mibReq.Param.NwkSEncKey = CommissioningParams.NwkSEncKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_APP_S_KEY;
        mibReq.Param.AppSKey = CommissioningParams.AppSKey;
        LoRaMacMibSetRequestConfirm( &mibReq );
#endif
    }
    mibReq.Type = MIB_PUBLIC_NETWORK;
    mibReq.Param.EnablePublicNetwork = LmHandlerParams->PublicNetworkEnable;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_ADR;
    mibReq.Param.AdrEnable = LmHandlerParams->AdrEnable;
    LoRaMacMibSetRequestConfirm( &mibReq );

    LoRaMacTestSetDutyCycleOn( LmHandlerParams->DutyCycleEnabled );

    mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR;
    mibReq.Param.SystemMaxRxError = 20;
    LoRaMacMibSetRequestConfirm( &mibReq );

    LoRaMacStart( );

    mibReq.Type = MIB_NETWORK_ACTIVATION;
    if( LoRaMacMibGetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
    {
        if( mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE )
        {
            LmHandlerCallbacks->OnNetworkParametersChange( &CommissioningParams );
        }
    }
    return LORAMAC_HANDLER_SUCCESS;
}

bool LmHandlerIsBusy( void )
{
    if( LoRaMacIsBusy( ) == true )
    {
        return true;
    }
    if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
    {
        // The network isn't yet joined, try again later.
        LmHandlerJoin( );
        return true;
    }

    if( LmHandlerPackages[PACKAGE_ID_COMPLIANCE]->IsRunning( ) == true )
    {
        return true;
    }
    return false;
}

void LmHandlerProcess( void )
{
    // Process Radio IRQ
    if( Radio.IrqProcess != NULL )
    {
        Radio.IrqProcess( );
    }

    // Processes the LoRaMac events
    LoRaMacProcess( );

    // Call all packages process functions
    LmHandlerPackagesProcess( );

    if( NvmCtxMgmtStore( ) == NVMCTXMGMT_STATUS_SUCCESS )
    {
        LmHandlerCallbacks->OnNvmContextChange( LORAMAC_HANDLER_NVM_STORE );
    }
}

/*!
 * Join a LoRa Network in classA
 *
 * \Note if the device is ABP, this is a pass through function
 * 
 * \param [IN] isOtaa Indicates which activation mode must be used
 */
static void LmHandlerJoinRequest( bool isOtaa )
{
    if( isOtaa == true )
    {
        MlmeReq_t mlmeReq;

        mlmeReq.Type = MLME_JOIN;
        mlmeReq.Req.Join.DevEui = CommissioningParams.DevEui;
        mlmeReq.Req.Join.JoinEui = CommissioningParams.JoinEui;
        mlmeReq.Req.Join.Datarate = LmHandlerParams->TxDatarate;
        // Update commissioning parameters activation type variable.
        CommissioningParams.IsOtaaActivation = true;

        // Starts the OTAA join procedure
        LmHandlerCallbacks->OnMacMlmeRequest( LoRaMacMlmeRequest( &mlmeReq ), &mlmeReq );
    }
    else
    {
        MibRequestConfirm_t mibReq;
        LmHandlerJoinParams_t joinParams = 
        {
            .CommissioningParams = &CommissioningParams,
            .Datarate = LmHandlerParams->TxDatarate,
            .Status = LORAMAC_HANDLER_SUCCESS
        };

        mibReq.Type = MIB_NETWORK_ACTIVATION;
        mibReq.Param.NetworkActivation = ACTIVATION_TYPE_ABP;
        LoRaMacMibSetRequestConfirm( &mibReq );

        // Notify upper layer
        LmHandlerCallbacks->OnJoinRequest( &joinParams );
    }
}

void LmHandlerJoin( void )
{
    LmHandlerJoinRequest( CommissioningParams.IsOtaaActivation );
}

LmHandlerFlagStatus_t LmHandlerJoinStatus( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    mibReq.Type = MIB_NETWORK_ACTIVATION;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK )
    {
        if( mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE )
        {
            return LORAMAC_HANDLER_RESET;
        }
        else
        {
            return LORAMAC_HANDLER_SET;
        }
    }
    else
    {
        return LORAMAC_HANDLER_RESET;
    }
}

LmHandlerErrorStatus_t LmHandlerSend( LmHandlerAppData_t *appData, LmHandlerMsgTypes_t isTxConfirmed )
{
    LoRaMacStatus_t status;
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    if( LmHandlerJoinStatus( ) != LORAMAC_HANDLER_SET )
    {
        // The network isn't joined, try again.
        LmHandlerJoinRequest( CommissioningParams.IsOtaaActivation );
        return LORAMAC_HANDLER_ERROR;
    }

    if( ( LmHandlerPackages[PACKAGE_ID_COMPLIANCE]->IsRunning( ) == true ) && ( appData->Port != LmHandlerPackages[PACKAGE_ID_COMPLIANCE]->Port ) && ( appData->Port != 0 ) )
    {
        return LORAMAC_HANDLER_ERROR;
    }

    mcpsReq.Req.Unconfirmed.Datarate = LmHandlerParams->TxDatarate;
    if( LoRaMacQueryTxPossible( appData->BufferSize, &txInfo ) != LORAMAC_STATUS_OK )
    {
        // Send empty frame in order to flush MAC commands
        TxParams.MsgType = LORAMAC_HANDLER_UNCONFIRMED_MSG;
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
    }
    else
    {
        TxParams.MsgType = isTxConfirmed;
        mcpsReq.Req.Unconfirmed.fPort = appData->Port;
        mcpsReq.Req.Unconfirmed.fBufferSize = appData->BufferSize;
        mcpsReq.Req.Unconfirmed.fBuffer = appData->Buffer;
        if( isTxConfirmed == LORAMAC_HANDLER_UNCONFIRMED_MSG )
        {
            mcpsReq.Type = MCPS_UNCONFIRMED;
        }
        else
        {
            mcpsReq.Type = MCPS_CONFIRMED;
        }
    }

    TxParams.AppData = *appData;
    TxParams.Datarate = LmHandlerParams->TxDatarate;

    status = LoRaMacMcpsRequest( &mcpsReq );
    LmHandlerCallbacks->OnMacMcpsRequest( status, &mcpsReq );

    if( status == LORAMAC_STATUS_OK )
    {
        return LORAMAC_HANDLER_SUCCESS;
    }
    else
    {
        return LORAMAC_HANDLER_ERROR;
    }
}

static LmHandlerErrorStatus_t LmHandlerDeviceTimeReq( void )
{
    LoRaMacStatus_t status;
    MlmeReq_t mlmeReq;

    mlmeReq.Type = MLME_DEVICE_TIME;

    status = LoRaMacMlmeRequest( &mlmeReq );
    LmHandlerCallbacks->OnMacMlmeRequest( status, &mlmeReq );
    
    if( status == LORAMAC_STATUS_OK )
    {
        return LORAMAC_HANDLER_SUCCESS;
    }
    else
    {
        return LORAMAC_HANDLER_ERROR;
    }
}

static LmHandlerErrorStatus_t LmHandlerBeaconReq( void )
{
    LoRaMacStatus_t status;
    MlmeReq_t mlmeReq;

    mlmeReq.Type = MLME_BEACON_ACQUISITION;

    status = LoRaMacMlmeRequest( &mlmeReq );
    LmHandlerCallbacks->OnMacMlmeRequest( status, &mlmeReq );

    if( status == LORAMAC_STATUS_OK )
    {
        return LORAMAC_HANDLER_SUCCESS;
    }
    else
    {
        return LORAMAC_HANDLER_ERROR;
    }
}

LmHandlerErrorStatus_t LmHandlerPingSlotReq( uint8_t periodicity )
{
    LoRaMacStatus_t status;
    MlmeReq_t mlmeReq;

    mlmeReq.Type = MLME_PING_SLOT_INFO;
    mlmeReq.Req.PingSlotInfo.PingSlot.Fields.Periodicity = periodicity;
    mlmeReq.Req.PingSlotInfo.PingSlot.Fields.RFU = 0;

    status = LoRaMacMlmeRequest( &mlmeReq );
    LmHandlerCallbacks->OnMacMlmeRequest( status, &mlmeReq );

    if( status == LORAMAC_STATUS_OK )
    {
        // Send an empty message
        LmHandlerAppData_t appData =
        {
            .Buffer = NULL,
            .BufferSize = 0,
            .Port = 0
        };
        return LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
    }
    else
    {
        return LORAMAC_HANDLER_ERROR;
    }
}

LmHandlerErrorStatus_t LmHandlerRequestClass( DeviceClass_t newClass )
{
    MibRequestConfirm_t mibReq;
    DeviceClass_t currentClass;
    LmHandlerErrorStatus_t errorStatus = LORAMAC_HANDLER_SUCCESS;

    mibReq.Type = MIB_DEVICE_CLASS;
    LoRaMacMibGetRequestConfirm( &mibReq );
    currentClass = mibReq.Param.Class;

    // Attempt to switch only if class update
    if( currentClass != newClass )
    {
        switch( newClass )
        {
        case CLASS_A:
            {
                if( currentClass != CLASS_A )
                {
                    mibReq.Param.Class = CLASS_A;
                    if( LoRaMacMibSetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
                    {
                        // Switch is instantaneous
                        LmHandlerCallbacks->OnClassChange( CLASS_A );
                    }
                    else
                    {
                        errorStatus = LORAMAC_HANDLER_ERROR;
                    }
                }
            }
            break;
        case CLASS_B:
            {
                if( currentClass != CLASS_A )
                {
                    errorStatus = LORAMAC_HANDLER_ERROR;
                }
                // Beacon must first be acquired
                errorStatus = LmHandlerDeviceTimeReq( );
                IsClassBSwitchPending = true;
            }
            break;
        case CLASS_C:
            {
                if( currentClass != CLASS_A )
                {
                    errorStatus = LORAMAC_HANDLER_ERROR;
                }
                // Switch is instantaneous
                mibReq.Param.Class = CLASS_C;
                if( LoRaMacMibSetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
                {
                    LmHandlerCallbacks->OnClassChange( CLASS_C );
                }
                else
                {
                    errorStatus = LORAMAC_HANDLER_ERROR;
                }
            }
            break;
        default:
            break;
        }
    }
    return errorStatus;
}

DeviceClass_t LmHandlerGetCurrentClass( void )
{
    MibRequestConfirm_t mibReq;

    mibReq.Type = MIB_DEVICE_CLASS;
    LoRaMacMibGetRequestConfirm( &mibReq );

    return mibReq.Param.Class;
}

int8_t LmHandlerGetCurrentDatarate( void )
{
    MibRequestConfirm_t mibGet;

    mibGet.Type = MIB_CHANNELS_DATARATE;
    LoRaMacMibGetRequestConfirm( &mibGet );

    return mibGet.Param.ChannelsDatarate;
}

LoRaMacRegion_t LmHandlerGetActiveRegion( void )
{
    return LmHandlerParams->Region;
}

/*
 *=============================================================================
 * LORAMAC NOTIFICATIONS HANDLING
 *=============================================================================
 */

static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    TxParams.IsMcpsConfirm = 1;
    TxParams.Status = mcpsConfirm->Status;
    TxParams.Datarate = mcpsConfirm->Datarate;
    TxParams.UplinkCounter = mcpsConfirm->UpLinkCounter;
    TxParams.TxPower = mcpsConfirm->TxPower;
    TxParams.Channel = mcpsConfirm->Channel;
    TxParams.AckReceived = mcpsConfirm->AckReceived;

    LmHandlerCallbacks->OnTxData( &TxParams );
    
    LmHandlerPackagesNotify( PACKAGE_MCPS_CONFIRM, mcpsConfirm );
}

static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    LmHandlerAppData_t appData;

    RxParams.IsMcpsIndication = 1;
    RxParams.Status = mcpsIndication->Status;

    if( RxParams.Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        return;
    }

    RxParams.Datarate = mcpsIndication->RxDatarate;
    RxParams.Rssi = mcpsIndication->Rssi;
    RxParams.Snr = mcpsIndication->Snr;
    RxParams.DownlinkCounter = mcpsIndication->DownLinkCounter;
    RxParams.RxSlot = mcpsIndication->RxSlot;

    appData.Port = mcpsIndication->Port;
    appData.BufferSize = mcpsIndication->BufferSize;
    appData.Buffer = mcpsIndication->Buffer;

    LmHandlerCallbacks->OnRxData( &appData, &RxParams );

    if( mcpsIndication->DeviceTimeAnsReceived == true )
    {
        LmHandlerCallbacks->OnSysTimeUpdate( );
    }
    // Call packages RxProcess function
    LmHandlerPackagesNotify( PACKAGE_MCPS_INDICATION, mcpsIndication );

    if( ( mcpsIndication->FramePending == true ) && ( LmHandlerGetCurrentClass( ) == CLASS_A ) )
    {
        // The server signals that it has pending data to be sent.
        // We schedule an uplink as soon as possible to flush the server.

        // Send an empty message
        LmHandlerAppData_t appData =
        {
            .Buffer = NULL,
            .BufferSize = 0,
            .Port = 0
        };
        LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
    }
}

static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    TxParams.IsMcpsConfirm = 0;
    TxParams.Status = mlmeConfirm->Status;
    LmHandlerCallbacks->OnTxData( &TxParams );

    LmHandlerPackagesNotify( PACKAGE_MLME_CONFIRM, mlmeConfirm );

    switch( mlmeConfirm->MlmeRequest )
    {
    case MLME_JOIN:
        {
            MibRequestConfirm_t mibReq;
            mibReq.Type = MIB_DEV_ADDR;
            LoRaMacMibGetRequestConfirm( &mibReq );
            JoinParams.CommissioningParams->DevAddr = mibReq.Param.DevAddr;
            JoinParams.Datarate = LmHandlerGetCurrentDatarate( );

            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                // Status is OK, node has joined the network
                JoinParams.Status = LORAMAC_HANDLER_SUCCESS;
            }
            else
            {
                // Join was not successful. Try to join again
                JoinParams.Status = LORAMAC_HANDLER_ERROR;
            }
            // Notify upper layer
            LmHandlerCallbacks->OnJoinRequest( &JoinParams );
        }
        break;
    case MLME_LINK_CHECK:
        {
            // Check DemodMargin
            // Check NbGateways
        }
        break;
    case MLME_DEVICE_TIME:
        {
            if( IsClassBSwitchPending == true )
            {
                LmHandlerBeaconReq( );
            }
        }
        break;
    case MLME_BEACON_ACQUISITION:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                // Beacon has been acquired
                // Request server for ping slot
                LmHandlerPingSlotReq( 0 );
            }
            else
            {
                // Beacon not acquired
                // Request Device Time again.
                LmHandlerDeviceTimeReq( );
            }
        }
        break;
    case MLME_PING_SLOT_INFO:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                MibRequestConfirm_t mibReq;

                // Class B is now activated
                mibReq.Type = MIB_DEVICE_CLASS;
                mibReq.Param.Class = CLASS_B;
                LoRaMacMibSetRequestConfirm( &mibReq );
                // Notify upper layer
                LmHandlerCallbacks->OnClassChange( CLASS_B );
                IsClassBSwitchPending = false;
            }
            else
            {
                LmHandlerPingSlotReq( 0 );
            }
        }
        break;
    default:
        break;
    }
}

static void MlmeIndication( MlmeIndication_t *mlmeIndication )
{
    RxParams.IsMcpsIndication = 0;
    RxParams.Status = mlmeIndication->Status;
    if( RxParams.Status != LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED )
    {
        LmHandlerCallbacks->OnRxData( NULL, &RxParams );
    }

    // Call packages RxProcess function
    LmHandlerPackagesNotify( PACKAGE_MLME_INDICATION, mlmeIndication );

    switch( mlmeIndication->MlmeIndication )
    {
    case MLME_SCHEDULE_UPLINK:
        {// The MAC signals that we shall provide an uplink as soon as possible
            // Send an empty message
            LmHandlerAppData_t appData =
            {
                .Buffer = NULL,
                .BufferSize = 0,
                .Port = 0
            };
            LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
        }
        break;
    case MLME_BEACON_LOST:
        {
            MibRequestConfirm_t mibReq;
            // Switch to class A again
            mibReq.Type = MIB_DEVICE_CLASS;
            mibReq.Param.Class = CLASS_A;
            LoRaMacMibSetRequestConfirm( &mibReq );

            BeaconParams.State = LORAMAC_HANDLER_BEACON_LOST;
            BeaconParams.Info.Time.Seconds = 0;
            BeaconParams.Info.GwSpecific.InfoDesc = 0;
            memset1( BeaconParams.Info.GwSpecific.Info, 0, 6 );

            LmHandlerCallbacks->OnClassChange( CLASS_A );
            LmHandlerCallbacks->OnBeaconStatusChange( &BeaconParams );

            LmHandlerDeviceTimeReq( );
        }
        break;
    case MLME_BEACON:
    {
        if( mlmeIndication->Status == LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED )
        {
            BeaconParams.State = LORAMAC_HANDLER_BEACON_RX;
            BeaconParams.Info = mlmeIndication->BeaconInfo;

            LmHandlerCallbacks->OnBeaconStatusChange( &BeaconParams );
        }
        else
        {
            BeaconParams.State = LORAMAC_HANDLER_BEACON_NRX;
            BeaconParams.Info = mlmeIndication->BeaconInfo;

            LmHandlerCallbacks->OnBeaconStatusChange( &BeaconParams );
        }
        break;
    }
    default:
        break;
    }
}

/*
 *=============================================================================
 * PACKAGES HANDLING
 *=============================================================================
 */

LmHandlerErrorStatus_t LmHandlerPackageRegister( uint8_t id, void *params )
{
    LmhPackage_t *package = NULL;
    switch( id )
    {
        case PACKAGE_ID_COMPLIANCE:
        {
            package = LmphCompliancePackageFactory( );
            break;
        }
        case PACKAGE_ID_CLOCK_SYNC:
        {
            package = LmphClockSyncPackageFactory( );
            break;
        }
        case PACKAGE_ID_REMOTE_MCAST_SETUP:
        {
            package = LmhpRemoteMcastSetupPackageFactory( );
            break;
        }
        case PACKAGE_ID_FRAGMENTATION:
        {
            package = LmhpFragmentationPackageFactory( );
            break;
        }
    }
    if( package != NULL )
    {
        LmHandlerPackages[id] = package;
        LmHandlerPackages[id]->OnMacMcpsRequest = LmHandlerCallbacks->OnMacMcpsRequest;
        LmHandlerPackages[id]->OnMacMlmeRequest = LmHandlerCallbacks->OnMacMlmeRequest;
        LmHandlerPackages[id]->OnJoinRequest = LmHandlerJoinRequest;
        LmHandlerPackages[id]->OnSendRequest = LmHandlerSend;
        LmHandlerPackages[id]->OnDeviceTimeRequest = LmHandlerDeviceTimeReq;
        LmHandlerPackages[id]->OnSysTimeUpdate = LmHandlerCallbacks->OnSysTimeUpdate;
        LmHandlerPackages[id]->Init( params, LmHandlerParams->DataBuffer, LmHandlerParams->DataBufferMaxSize );

        return LORAMAC_HANDLER_SUCCESS;
    }
    else
    {
        return LORAMAC_HANDLER_ERROR;
    }
}

bool LmHandlerPackageIsInitialized( uint8_t id )
{
    if( LmHandlerPackages[id]->IsInitialized != NULL )
    {
        return LmHandlerPackages[id]->IsInitialized( );
    }
    else
    {
        return false;
    }
}

bool LmHandlerPackageIsRunning( uint8_t id )
{
    if( LmHandlerPackages[id]->IsRunning != NULL )
    {
        return LmHandlerPackages[id]->IsRunning( );
    }
    else
    {
        return false;
    }
}

static void LmHandlerPackagesNotify( PackageNotifyTypes_t notifyType, void *params )
{
    for( int8_t i = 0; i < PKG_MAX_NUMBER; i++ )
    {
        if( LmHandlerPackages[i] != NULL )
        {
            switch( notifyType )
            {
                case PACKAGE_MCPS_CONFIRM:
                {
                    if( LmHandlerPackages[i]->OnMcpsConfirmProcess != NULL )
                    {
                        LmHandlerPackages[i]->OnMcpsConfirmProcess( params );
                    }
                    break;
                }
                case PACKAGE_MCPS_INDICATION:
                {
                    if( ( LmHandlerPackages[i]->OnMcpsIndicationProcess != NULL ) &&
                        ( LmHandlerPackages[i]->Port == ( ( McpsIndication_t* )params )->Port ) )
                    {
                        LmHandlerPackages[i]->OnMcpsIndicationProcess( params );
                    }
                    break;
                }
                case PACKAGE_MLME_CONFIRM:
                {
                    if( LmHandlerPackages[i]->OnMlmeConfirmProcess != NULL )
                    {
                        LmHandlerPackages[i]->OnMlmeConfirmProcess( params );
                    }
                    break;
                }
                case PACKAGE_MLME_INDICATION:
                {
                    if( LmHandlerPackages[i]->OnMlmeIndicationProcess != NULL )
                    {
                        LmHandlerPackages[i]->OnMlmeIndicationProcess( params );
                    }
                    break;
                }
            }
        }
    }
}

static void LmHandlerPackagesProcess( void )
{
    for( int8_t i = 0; i < PKG_MAX_NUMBER; i++ )
    {
        if( ( LmHandlerPackages[i] != NULL ) &&
            ( LmHandlerPackages[i]->Process != NULL ) &&
            ( LmHandlerPackageIsInitialized( i ) != false ) )
        {
            LmHandlerPackages[i]->Process( );
        }
    }
}
