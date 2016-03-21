/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jäckle ( STACKFORCE )
*/
#include "board.h"

#include "LoRaMac-api-v3.h"
#include "LoRaMacTest.h"

/*!
 *  Extern function declarations.
 */
extern LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort,
                             void *fBuffer, uint16_t fBufferSize );
extern LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl,
                                     uint8_t fPort, void *fBuffer, uint16_t fBufferSize );
extern LoRaMacStatus_t SendFrameOnChannel( ChannelParams_t channel );
extern uint32_t LoRaMacState;
extern LoRaMacFlags_t LoRaMacFlags;

/*!
 * Static variables
 */
static LoRaMacEventFlags_t LoRaMacEventFlags;
static LoRaMacEventInfo_t LoRaMacEventInfo;
static LoRaMacPrimitives_t LoRaMacPrimitives;
static LoRaMacCallback_t LoRaMacCallback;
static LoRaMacCallbacks_t LoRaMacCallbacks;

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    LoRaMacEventInfo.Status = mcpsConfirm->Status;
    LoRaMacEventFlags.Bits.Tx = 1;

    LoRaMacEventInfo.TxDatarate = mcpsConfirm->Datarate;
    LoRaMacEventInfo.TxNbRetries = mcpsConfirm->NbRetries;
    LoRaMacEventInfo.TxAckReceived = mcpsConfirm->AckReceived;

    if( ( LoRaMacFlags.Bits.McpsInd != 1 ) && ( LoRaMacFlags.Bits.MlmeReq != 1 ) )
    {
        LoRaMacCallbacks.MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
        LoRaMacEventFlags.Value = 0;
    }
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    LoRaMacEventInfo.Status = mcpsIndication->Status;
    LoRaMacEventFlags.Bits.Rx = 1;
    LoRaMacEventFlags.Bits.RxSlot = mcpsIndication->RxSlot;
    LoRaMacEventFlags.Bits.Multicast = mcpsIndication->Multicast;
    if( mcpsIndication->RxData == true )
    {
        LoRaMacEventFlags.Bits.RxData = 1;
    }

    LoRaMacEventInfo.RxPort = mcpsIndication->Port;
    LoRaMacEventInfo.RxBuffer = mcpsIndication->Buffer;
    LoRaMacEventInfo.RxBufferSize = mcpsIndication->BufferSize;
    LoRaMacEventInfo.RxRssi = mcpsIndication->Rssi;
    LoRaMacEventInfo.RxSnr = mcpsIndication->Snr;

    LoRaMacCallbacks.MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
    LoRaMacEventFlags.Value = 0;
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
    {
        switch( mlmeConfirm->MlmeRequest )
        {
            case MLME_JOIN:
            {
                // Status is OK, node has joined the network
                LoRaMacEventFlags.Bits.Tx = 1;
                LoRaMacEventFlags.Bits.Rx = 1;
                LoRaMacEventFlags.Bits.JoinAccept = 1;
                break;
            }
            case MLME_LINK_CHECK:
            {
                LoRaMacEventFlags.Bits.Tx = 1;
                LoRaMacEventFlags.Bits.Rx = 1;
                LoRaMacEventFlags.Bits.LinkCheck = 1;

                LoRaMacEventInfo.DemodMargin = mlmeConfirm->DemodMargin;
                LoRaMacEventInfo.NbGateways = mlmeConfirm->NbGateways;
                break;
            }
            default:
                break;
        }
    }
    LoRaMacEventInfo.Status = mlmeConfirm->Status;

    if( LoRaMacFlags.Bits.McpsInd != 1 )
    {
        LoRaMacCallbacks.MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
        LoRaMacEventFlags.Value = 0;
    }
}

void LoRaMacInit( LoRaMacCallbacks_t *callbacks )
{
    LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
    LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
    LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;

    LoRaMacCallbacks.MacEvent = callbacks->MacEvent;
    LoRaMacCallbacks.GetBatteryLevel = callbacks->GetBatteryLevel;
    LoRaMacCallback.GetBatteryLevel = callbacks->GetBatteryLevel;

    LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallback );
}

void LoRaMacSetAdrOn( bool enable )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_ADR;
    mibSet.Param.AdrEnable = enable;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacInitNwkIds( uint32_t netID, uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_NET_ID;
    mibSet.Param.NetID = netID;

    LoRaMacMibSetRequestConfirm( &mibSet );

    mibSet.Type = MIB_DEV_ADDR;
    mibSet.Param.DevAddr = devAddr;

    LoRaMacMibSetRequestConfirm( &mibSet );

    mibSet.Type = MIB_NWK_SKEY;
    mibSet.Param.NwkSKey = nwkSKey;

    LoRaMacMibSetRequestConfirm( &mibSet );

    mibSet.Type = MIB_APP_SKEY;
    mibSet.Param.AppSKey = appSKey;

    LoRaMacMibSetRequestConfirm( &mibSet );

    mibSet.Type = MIB_NETWORK_JOINED;
    mibSet.Param.IsNetworkJoined = true;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacMulticastChannelAdd( MulticastParams_t *channelParam )
{
    LoRaMacMulticastChannelLink( channelParam );
}

void LoRaMacMulticastChannelRemove( MulticastParams_t *channelParam )
{
    LoRaMacMulticastChannelUnlink( channelParam );
}

uint8_t LoRaMacJoinReq( uint8_t *devEui, uint8_t *appEui, uint8_t *appKey )
{
    MlmeReq_t mlmeRequest;
    uint8_t status;

    mlmeRequest.Type = MLME_JOIN;
    mlmeRequest.Req.Join.AppEui = appEui;
    mlmeRequest.Req.Join.AppKey = appKey;
    mlmeRequest.Req.Join.DevEui = devEui;

    switch( LoRaMacMlmeRequest( &mlmeRequest ) )
    {
        case LORAMAC_STATUS_OK:
        {
            status = 0;
            break;
        }
        case LORAMAC_STATUS_BUSY:
        {
            status = 1;
            break;
        }
        case LORAMAC_STATUS_NO_NETWORK_JOINED:
        {
            status = 2;
            break;
        }
        case LORAMAC_STATUS_LENGTH_ERROR:
        case LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR:
        {
            status = 3;
            break;
        }
        case LORAMAC_STATUS_SERVICE_UNKNOWN:
        {
            status = 4;
            break;
        }
        case LORAMAC_STATUS_DEVICE_OFF:
        {
            status = 6;
            break;
        }
        default:
        {
            status = 1;
            break;
        }
    }

    return status;
}

uint8_t LoRaMacLinkCheckReq( void )
{
    MlmeReq_t mlmeRequest;
    uint8_t status;

    mlmeRequest.Type = MLME_LINK_CHECK;

    switch( LoRaMacMlmeRequest( &mlmeRequest ) )
    {
        case LORAMAC_STATUS_OK:
        {
            status = 0;
            break;
        }
        case LORAMAC_STATUS_SERVICE_UNKNOWN:
        {
            status = 1;
            break;
        }
        default:
        {
            status = 1;
            break;
        }
    }

    return status;
}

uint8_t LoRaMacSendFrame( uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    MibRequestConfirm_t mibGet;
    McpsReq_t mcpsRequest;
    uint8_t retStatus;

    memset1( ( uint8_t* )&LoRaMacEventInfo, 0, sizeof( LoRaMacEventInfo ) );

    mibGet.Type = MIB_CHANNELS_DATARATE;
    LoRaMacMibGetRequestConfirm( &mibGet );

    mcpsRequest.Type = MCPS_UNCONFIRMED;
    mcpsRequest.Req.Unconfirmed.fBuffer = fBuffer;
    mcpsRequest.Req.Unconfirmed.fBufferSize = fBufferSize;
    mcpsRequest.Req.Unconfirmed.fPort = fPort;
    mcpsRequest.Req.Unconfirmed.Datarate = mibGet.Param.ChannelsDatarate;

    switch( LoRaMacMcpsRequest( &mcpsRequest ) )
    {
        case LORAMAC_STATUS_OK:
            retStatus = 0U;
            break;
        case LORAMAC_STATUS_BUSY:
            retStatus = 1U;
            break;
        case LORAMAC_STATUS_NO_NETWORK_JOINED:
            retStatus = 2U;
            break;
        case LORAMAC_STATUS_LENGTH_ERROR:
        case LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR:
            retStatus = 3U;
            break;
        case LORAMAC_STATUS_SERVICE_UNKNOWN:
            retStatus = 4U;
            break;
        case LORAMAC_STATUS_DEVICE_OFF:
            retStatus = 6U;
            break;
        default:
            retStatus = 1U;
            break;
    }

    return retStatus;
}

uint8_t LoRaMacSendConfirmedFrame( uint8_t fPort, void *fBuffer, uint16_t fBufferSize, uint8_t nbRetries )
{
    MibRequestConfirm_t mibGet;
    McpsReq_t mcpsRequest;
    uint8_t retStatus;

    memset1( ( uint8_t* )&LoRaMacEventInfo, 0, sizeof( LoRaMacEventInfo ) );

    mibGet.Type = MIB_CHANNELS_DATARATE;
    LoRaMacMibGetRequestConfirm( &mibGet );

    mcpsRequest.Type = MCPS_CONFIRMED;
    mcpsRequest.Req.Confirmed.fBuffer = fBuffer;
    mcpsRequest.Req.Confirmed.fBufferSize = fBufferSize;
    mcpsRequest.Req.Confirmed.fPort = fPort;
    mcpsRequest.Req.Confirmed.NbTrials = nbRetries;
    mcpsRequest.Req.Confirmed.Datarate = mibGet.Param.ChannelsDatarate;

    switch( LoRaMacMcpsRequest( &mcpsRequest ) )
    {
        case LORAMAC_STATUS_OK:
            retStatus = 0U;
            break;
        case LORAMAC_STATUS_BUSY:
            retStatus = 1U;
            break;
        case LORAMAC_STATUS_NO_NETWORK_JOINED:
            retStatus = 2U;
            break;
        case LORAMAC_STATUS_LENGTH_ERROR:
        case LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR:
            retStatus = 3U;
            break;
        case LORAMAC_STATUS_SERVICE_UNKNOWN:
            retStatus = 4U;
            break;
        case LORAMAC_STATUS_DEVICE_OFF:
            retStatus = 6U;
            break;
        default:
            retStatus = 1U;
            break;
    }

    return retStatus;
}

uint8_t LoRaMacSend( LoRaMacHeader_t *macHdr, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint8_t retStatus;

    memset1( ( uint8_t* ) &LoRaMacEventInfo, 0, sizeof( LoRaMacEventInfo ) );

    switch( Send( macHdr, fPort, fBuffer, fBufferSize ) )
    {
        case LORAMAC_STATUS_OK:
            retStatus = 0U;
            break;
        case LORAMAC_STATUS_BUSY:
            retStatus = 1U;
            break;
        case LORAMAC_STATUS_NO_NETWORK_JOINED:
            retStatus = 2U;
            break;
        case LORAMAC_STATUS_LENGTH_ERROR:
        case LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR:
            retStatus = 3U;
            break;
        case LORAMAC_STATUS_SERVICE_UNKNOWN:
            retStatus = 4U;
            break;
        case LORAMAC_STATUS_DEVICE_OFF:
            retStatus = 6U;
            break;
        default:
            retStatus = 1U;
            break;
    }

    return retStatus;
}

uint8_t LoRaMacPrepareFrame( ChannelParams_t channel,LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint8_t retStatus;

    switch( PrepareFrame( macHdr, fCtrl, fPort, fBuffer, fBufferSize ) )
    {
        case LORAMAC_STATUS_OK:
            retStatus = 0U;
            break;
        case LORAMAC_STATUS_BUSY:
            retStatus = 1U;
            break;
        case LORAMAC_STATUS_NO_NETWORK_JOINED:
            retStatus = 2U;
            break;
        case LORAMAC_STATUS_LENGTH_ERROR:
        case LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR:
            retStatus = 3U;
            break;
        case LORAMAC_STATUS_SERVICE_UNKNOWN:
            retStatus = 4U;
            break;
        default:
            retStatus = 1U;
            break;
    }

    return retStatus;
}

uint8_t LoRaMacSendFrameOnChannel( ChannelParams_t channel )
{
    memset1( ( uint8_t* ) &LoRaMacEventInfo, 0, sizeof( LoRaMacEventInfo ) );

    SendFrameOnChannel( channel );

    /* SendFrameOnChannel has always status "OK" */
    return 0;
}

uint8_t LoRaMacSendOnChannel( ChannelParams_t channel, LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint8_t status = 0;

    if( ( LoRaMacState & 0x00000001 ) == 0x00000001 )
    {
        return 1; // MAC is busy transmitting a previous frame
    }

    status = LoRaMacPrepareFrame( channel, macHdr, fCtrl, fOpts, fPort, fBuffer, fBufferSize );
    if( status != 0 )
    {
        return status;
    }

    LoRaMacEventInfo.TxNbRetries = 0;
    LoRaMacEventInfo.TxAckReceived = false;

    return LoRaMacSendFrameOnChannel( channel );
}

void LoRaMacSetDeviceClass( DeviceClass_t deviceClass )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_DEVICE_CLASS;
    mibSet.Param.Class = deviceClass;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetPublicNetwork( bool enable )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_PUBLIC_NETWORK;
    mibSet.Param.EnablePublicNetwork = enable;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetDutyCycleOn( bool enable )
{
    LoRaMacTestSetDutyCycleOn( enable );
}

void LoRaMacSetChannel( uint8_t id, ChannelParams_t params )
{
    LoRaMacChannelAdd( id, params );
}

void LoRaMacSetRx2Channel( Rx2ChannelParams_t param )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_RX2_CHANNEL;
    mibSet.Param.Rx2Channel = param;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetChannelsMask( uint16_t *mask )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_CHANNELS_MASK;
    mibSet.Param.ChannelsMask = mask;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetChannelsNbRep( uint8_t nbRep )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_CHANNELS_NB_REP;
    mibSet.Param.ChannelNbRep = nbRep;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetMaxRxWindow( uint32_t delay )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_MAX_RX_WINDOW_DURATION;
    mibSet.Param.MaxRxWindow = delay;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetReceiveDelay1( uint32_t delay )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_RECEIVE_DELAY_1;
    mibSet.Param.ReceiveDelay1 = delay;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetReceiveDelay2( uint32_t delay )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_RECEIVE_DELAY_2;
    mibSet.Param.ReceiveDelay2 = delay;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetJoinAcceptDelay1( uint32_t delay )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_JOIN_ACCEPT_DELAY_1;
    mibSet.Param.JoinAcceptDelay1 = delay;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetJoinAcceptDelay2( uint32_t delay )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_JOIN_ACCEPT_DELAY_2;
    mibSet.Param.JoinAcceptDelay2 = delay;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetChannelsDatarate( int8_t datarate )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_CHANNELS_DATARATE;
    mibSet.Param.ChannelsDatarate = datarate;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

void LoRaMacSetChannelsTxPower( int8_t txPower )
{
    MibRequestConfirm_t mibSet;

    mibSet.Type = MIB_CHANNELS_TX_POWER;
    mibSet.Param.ChannelsTxPower = txPower;

    LoRaMacMibSetRequestConfirm( &mibSet );
}

uint32_t LoRaMacGetUpLinkCounter( void )
{
    MibRequestConfirm_t mibGet;

    mibGet.Type = MIB_UPLINK_COUNTER;

    LoRaMacMibGetRequestConfirm( &mibGet );

    return mibGet.Param.UpLinkCounter;
}

uint32_t LoRaMacGetDownLinkCounter( void )
{
    MibRequestConfirm_t mibGet;

    mibGet.Type = MIB_DOWNLINK_COUNTER;

    LoRaMacMibGetRequestConfirm( &mibGet );

    return mibGet.Param.DownLinkCounter;
}

void LoRaMacSetMicTest( uint16_t txPacketCounter )
{
    LoRaMacTestSetMic( txPacketCounter );
}
