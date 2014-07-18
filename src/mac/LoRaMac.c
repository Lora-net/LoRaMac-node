/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdlib.h>
#include <math.h>
#include "board.h"
#include "utilities.h"
#include "radio.h"

#include "LoRaMacCrypto.h"
#include "LoRaMac.h"

/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      64

/*!
 * Device IEEE EUI
 */
static uint8_t *LoRaMacDevEui;

/*!
 * Application IEEE EUI
 */
static uint8_t *LoRaMacAppEui;

/*!
 * AES encryption/decryption cipher application key
 */
static uint8_t *LoRaMacAppKey;

/*!
 * AES encryption/decryption cipher network session key
 */
static uint8_t LoRaMacNwkSKey[] = 
{ 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * AES encryption/decryption cipher application session key
 */
static uint8_t LoRaMacAppSKey[] = 
{ 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * Device nonce is a random value extracted by issuing a sequence of RSSI
 * measurements
 */
static uint16_t LoRaMacDevNonce;

/*!
 * Network ID ( 3 bytes )
 */
static uint32_t LoRaMacNetID;

/*!
 * Mote Address
 */
static uint32_t LoRaMacDevAddr;

/*!
 * Buffer containing the data to be sent or received.
 */
static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaMacBuffer
 */
static uint16_t LoRaMacBufferPktLen = 0;

/*!
 * Buffer containing the upper layer data.
 */
static uint8_t LoRaMacPayload[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * LoRaMAC frame counter. Each time a packet is sent the counter is incremented.
 * Only the 16 LSB bits are sent
 */
static uint32_t UpLinkCounter = 1;

/*!
 * LoRaMAC frame counter. Each time a packet is received the counter is incremented.
 * Only the 16 LSB bits are received
 */
static uint32_t DownLinkCounter = 0;

/*!
 * IsPacketCounterFixed enables the MIC field tests by fixing the
 * UpLinkCounter value
 */
static bool IsUpLinkCounterFixed = false;

/*!
 * Used for test purposes. Disables the opening of the reception windows.
 */
static bool IsRxWindowsEnabled = true;

/*!
 * Indicates if the MAC layer is already transmitting a frame.
 */
static bool IsLoRaMacTransmitting = false;

/*!
 * Indicates if the MAC layer has already joined a network.
 */
static bool IsLoRaMacNetworkJoined = false;

/*!
 * LoRaMac ADR control status
 */
static bool AdrCtrlOn = false;

/*!
 * Counts the number of missed ADR acknowledgements
 */
static uint32_t AdrAckCounter = 0;

/*!
 * If the node has sent a FRAME_TYPE_DATA_CONFIRMED_UP this variable indicates
 * if the nodes needs to manage the server acknowledgement.
 */
static bool NodeAckRequested = false;

/*!
 * If the server has sent a FRAME_TYPE_DATA_CONFIRMED_DOWN this variable indicates
 * if the ACK bit must be set for the next transmission
 */
static bool SrvAckRequested = false;

/*!
 * Indicates if the MAC layer wants to send MAC commands 
 */
static bool MacCommandsInNextTx = false;

/*!
 * Contains the current MacCommandsBuffer index
 */
static uint8_t MacCommandsBufferIndex = 0;

/*!
 * Buffer containing the MAC layer commands
 */
static uint8_t MacCommandsBuffer[15];

/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};

/*!
 * LoRaMAC 2nd reception window settings
 */
static Rx2ChannelParams_t Rx2Channel = RX_WND_2_CHANNEL;

/*!
 * Mask indicating which channels are enabled
 * \remark By default LC1, LC2 and LC3 are enabled
 */
static uint16_t ChannelsMask = LC( 1 ) + LC( 2 ) + LC( 3 );

/*!
 * Channels Tx output power
 */
static int8_t ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;

/*!
 * Channels datarate
 */
static int8_t ChannelsDatarate = LORAMAC_DEFAULT_DATARATE;

/*!
 * Unconfirmed frames repetition timeout timer.
 */
static TimerEvent_t NbRepTimeoutTimer;

/*!
 * Number of uplink messages repetitions [1:15] (unconfirmed messages only)
 */
static uint8_t ChannelsNbRep = 1; // 

/*!
 * Uplink messages repetitions counter
 */
static uint8_t ChannelsNbRepCounter = 0;

/*!
 * Maximum duty cycle
 * \remark Possibility to shutdown the device. Everything else not implemented.
 */
static uint8_t MaxDCycle = 0;

/*!
 * Agregated duty cycle
 * \remark Not implemented
 */
static double AggregatedDCycle = 1.0; // No limitation

/*!
 * Current channel index
 */
static uint8_t Channel;

/*!
 * LoRaMac upper layer event functions
 */
static LoRaMacEvent_t *LoRaMacEvents;

/*!
 * LoRaMac 
 */
LoRaMacEventFlags_t LoRaMacEventFlags;

/*!
 * LoRaMac 
 */
LoRaMacEventInfo_t LoRaMacEventInfo;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LoRaMac reception windows timers
 */
static TimerEvent_t RxWindowTimer1;
static TimerEvent_t RxWindowTimer2;

static uint32_t RxWindow1Delay;
static uint32_t RxWindow2Delay;

/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
static TimerEvent_t AckTimeoutTimer;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetries = 4;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetriesCounter = 0;

/*!
 * Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void );

/*!
 * Function to be executed on Radio Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int8_t rssi, int8_t snr );

/*!
 * Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void );

/*!
 * Function executed on Radio Rx error event
 */
static void OnRadioRxError( void );

/*!
 * Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void );

/*!
 * Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void );

/*!
 * Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void );

/*!
 * Function executed on NbRepTimeout timer event
 */
static void OnNbRepTimeoutTimerEvent( void );

/*!
 * Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void );

/*!
 * Searches and set the next random available channel
 *
 * \retval status  Function status [0: OK, 1: Unable to find a free channel]
 */
static uint8_t LoRaMacSetNextChannel( void )
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t channelNext = Channel;
    uint8_t nbEnabledChannels = 0;
    uint8_t enabledChannels[LORA_MAX_NB_CHANNELS];

    // Search how many channels are enabled
    for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
    {
        if( ( ( 1 << i ) & ChannelsMask ) != 0 )
        {
            enabledChannels[nbEnabledChannels++] = i;
        }
    }

    for( i = 0, j = randr( 0, nbEnabledChannels - 1 ); i < LORA_MAX_NB_CHANNELS; i++ )
    {
        channelNext = enabledChannels[j];
        j = ( j + 1 ) % nbEnabledChannels;

        if( ( ( Channels[channelNext].DrRange.Fields.Min <= ChannelsDatarate ) &&
              ( ChannelsDatarate <= Channels[channelNext].DrRange.Fields.Max ) ) == false )
        { // Check if the current channel selection supports the given datarate
            continue;
        }
        if( Radio.IsChannelFree( MODEM_LORA, Channels[channelNext].Frequency, RSSI_FREE_TH ) == true )
        {
            // Free channel found
            Channel = channelNext;
            return 0;
        }
    }
    // No free channel found. Keep previous channel
    return 1;
}

/*!
 * Adds a new MAC command to be sent.
 *
 * \Remark MAC layer internal function
 *
 * \param [in] cmd MAC command to be added
 *                 [MOTE_MAC_LINK_CHECK_REQ,
 *                  MOTE_MAC_LINK_ADR_ANS,
 *                  MOTE_MAC_DUTY_CYCLE_ANS,
 *                  MOTE_MAC_RX2_PARAM_SET_ANS,
 *                  MOTE_MAC_DEV_STATUS_ANS
 *                  MOTE_MAC_NEW_CHANNEL_ANS]
 * \param [in] p1  1st parameter ( optional depends on the command )
 * \param [in] p2  2nd parameter ( optional depends on the command )
 *
 * \retval status  Function status [0: OK, 1: Unknown command, 2: Buffer full]
 */
static uint8_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 )
{
    MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
    switch( cmd )
    {
        case MOTE_MAC_LINK_CHECK_REQ:
            // No payload for this command
            break;
        case MOTE_MAC_LINK_ADR_ANS:
            // Margin
            MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
            break;
        case MOTE_MAC_DUTY_CYCLE_ANS:
            // No payload for this answer
            break;
        case MOTE_MAC_RX2_SETUP_ANS:
            // Status: Datarate ACK, Channel ACK
            MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            // 1st byte Battery
            // 2nd byte Margin
            MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
            MacCommandsBuffer[MacCommandsBufferIndex++] = p2;
            break;
        case MOTE_MAC_NEW_CHANNEL_ANS:
            // Status: Datarate range OK, Channel frequency OK
            MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
            break;
        default:
            return 1;
    }
    if( MacCommandsBufferIndex < 15 )
    {
        MacCommandsInNextTx = true;
        return 0;
    }
    else
    {
        return 2;
    }
}

// TODO: Add Documentation
static void LoRaMacNotify( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
    if( ( LoRaMacEvents != NULL ) && ( LoRaMacEvents->MacEvent != NULL ) )
    {
        LoRaMacEvents->MacEvent( flags, info );
    }
    flags->Value = 0;
}

void LoRaMacInit( LoRaMacEvent_t *events )
{
    LoRaMacEvents = events;

    LoRaMacEventFlags.Value = 0;
    
    LoRaMacEventInfo.TxAckReceived = false;
    LoRaMacEventInfo.TxNbRetries = 0;
    LoRaMacEventInfo.TxDatarate = 7;
    LoRaMacEventInfo.RxPort = 1;
    LoRaMacEventInfo.RxBuffer = NULL;
    LoRaMacEventInfo.RxBufferSize = 0;
    LoRaMacEventInfo.RxRssi = 0;
    LoRaMacEventInfo.RxSnr = 0;
    LoRaMacEventInfo.Energy = 0;
    LoRaMacEventInfo.DemodMargin = 0;
    LoRaMacEventInfo.NbGateways = 0;
    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
    
    UpLinkCounter = 1;
    DownLinkCounter = 0;
    
    IsLoRaMacNetworkJoined = false;
    IsLoRaMacTransmitting = false;

    ChannelsMask = LC( 1 ) + LC( 2 ) + LC( 3 );;
    ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;
    ChannelsDatarate = LORAMAC_DEFAULT_DATARATE;
    ChannelsNbRep = 1;
    ChannelsNbRepCounter = 0;
    AggregatedDCycle = 1.0;

    TimerInit( &RxWindowTimer1, OnRxWindow1TimerEvent ); 
    TimerInit( &RxWindowTimer2, OnRxWindow2TimerEvent ); 
    TimerInit( &AckTimeoutTimer, OnAckTimeoutTimerEvent ); 
    TimerInit( &NbRepTimeoutTimer, OnNbRepTimeoutTimerEvent );
    
    // Initialize Radio driver
    RadioEvents.TxDone = OnRadioTxDone;
    RadioEvents.RxDone = OnRadioRxDone;
    RadioEvents.RxError = OnRadioRxError;
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    RadioEvents.RxTimeout = OnRadioRxTimeout;
    Radio.Init( &RadioEvents );

    // Random seed initialization
    srand( RAND_SEED );

    // Initialize channel index.
    Channel = LORA_MAX_NB_CHANNELS;

#if defined( LORAMAC_R3 )
    // Change LoRa modem SyncWord
    Radio.SetModem( MODEM_LORA );
    Radio.Write( 0x39, 0x34 );
#endif

    Radio.Sleep( );
}

void LoRaMacSetAdrOn( bool enable )
{
    AdrCtrlOn = enable;
}

void LoRaMacInitNwkIds( uint32_t netID, uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey )
{
    LoRaMacNetID = netID;
    LoRaMacDevAddr = devAddr;
    LoRaMacMemCpy( nwkSKey, LoRaMacNwkSKey, 16 );
    LoRaMacMemCpy( appSKey, LoRaMacAppSKey, 16 );
    
    IsLoRaMacNetworkJoined = true;
}

uint8_t LoRaMacJoinReq( uint8_t *devEui, uint8_t *appEui, uint8_t *appKey )
{
    LoRaMacHeader_t macHdr;

    LoRaMacDevEui = devEui;
    LoRaMacAppEui = appEui;
    LoRaMacAppKey = appKey;
    
    macHdr.Value = 0;
    macHdr.Bits.MType        = FRAME_TYPE_JOIN_REQ;
    
    return LoRaMacSend( &macHdr, NULL, 0, NULL, 0 );
}

uint8_t LoRaMacLinkCheckReq( void )
{
    return AddMacCommand( MOTE_MAC_LINK_CHECK_REQ, 0, 0 );
}

uint8_t LoRaMacSendFrame( uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacHeader_t macHdr;

    macHdr.Value = 0;

#if defined( LORAMAC_R3 )
    macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
#else
    macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED;
#endif
    return LoRaMacSend( &macHdr, NULL, fPort, fBuffer, fBufferSize );
}

uint8_t LoRaMacSendConfirmedFrame( uint8_t fPort, void *fBuffer, uint16_t fBufferSize, uint8_t retries )
{
    LoRaMacHeader_t macHdr;

    AckTimeoutRetries = retries;
    AckTimeoutRetriesCounter = 0;
    
    macHdr.Value = 0;

#if defined( LORAMAC_R3 )
    macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
#else
    macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED;
#endif
    return LoRaMacSend( &macHdr, NULL, fPort, fBuffer, fBufferSize );
}

uint8_t LoRaMacSend( LoRaMacHeader_t *macHdr, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;

    fCtrl.Value = 0;

    fCtrl.Bits.OptionsLength = 0;
    fCtrl.Bits.FPending      = 0;
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = AdrCtrlOn;

    if( MaxDCycle == 255 )
    {
        return 6;
    }
    if( LoRaMacSetNextChannel( ) != 0 )
    {
        return 5;
    }
    
    return LoRaMacSendOnChannel( Channels[Channel], macHdr, &fCtrl, fOpts, fPort, fBuffer, fBufferSize );
}

uint8_t LoRaMacPrepareFrame( ChannelParams_t channel, LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint16_t i;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    
    LoRaMacBufferPktLen = 0;
    
    NodeAckRequested = false;
    
    if( fBuffer == NULL )
    {
        fBufferSize = 0;
    }

    LoRaMacBuffer[pktHeaderLen++] = macHdr->Value;

    switch( macHdr->Bits.MType )
    {
        case FRAME_TYPE_JOIN_REQ:            
            RxWindow1Delay = JOIN_ACEPT_DELAY1;
            RxWindow2Delay = JOIN_ACEPT_DELAY2;

            LoRaMacBufferPktLen = pktHeaderLen;
        
            LoRaMacMemCpy( LoRaMacAppEui, LoRaMacBuffer + LoRaMacBufferPktLen, 8 );
            LoRaMacBufferPktLen += 8;
            LoRaMacMemCpy( LoRaMacDevEui, LoRaMacBuffer + LoRaMacBufferPktLen, 8 );
            LoRaMacBufferPktLen += 8;

            LoRaMacDevNonce = Radio.Random( );
            
            LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevNonce & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen & 0xFF, LoRaMacAppKey, &mic );
            
            LoRaMacBuffer[LoRaMacBufferPktLen++] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 24 ) & 0xFF;
            
            break;
#if defined( LORAMAC_R3 )
        case FRAME_TYPE_DATA_CONFIRMED_UP:
#else
        case FRAME_TYPE_DATA_CONFIRMED:
#endif            
            NodeAckRequested = true;
            //Intentional falltrough
#if defined( LORAMAC_R3 )
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
#else
        case FRAME_TYPE_DATA_UNCONFIRMED:
#endif             
            if( IsLoRaMacNetworkJoined == false )
            {
                return 2; // No network has been joined yet
            }
            
            RxWindow1Delay = CLS2_RECEIVE_DELAY1;
            RxWindow2Delay = CLS2_RECEIVE_DELAY2;

            if( fOpts == NULL )
            {
                fCtrl->Bits.OptionsLength = 0;
            }

            if( SrvAckRequested == true )
            {
                fCtrl->Bits.Ack = 1;
            }
            
            if( fCtrl->Bits.Adr == true )
            {
                if( ChannelsDatarate == LORAMAC_MIN_DATARATE )
                {
                    AdrAckCounter = 0;
                    fCtrl->Bits.AdrAckReq = false;
                }
                else
                {
                    if( AdrAckCounter > ADR_ACK_LIMIT )
                    {
                        fCtrl->Bits.AdrAckReq = true;
                    }
                    else
                    {
                        fCtrl->Bits.AdrAckReq = false;
                    }
                    if( AdrAckCounter > ( ADR_ACK_LIMIT + ADR_ACK_DELAY ) )
                    {
                        AdrAckCounter = 0;
                        if( ChannelsDatarate > LORAMAC_MIN_DATARATE )
                        {
                            ChannelsDatarate--;
                        }
                        else
                        {
                            // Re-enable default channels LC1, LC2, LC3
                            ChannelsMask = ChannelsMask | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
                        }
                    }
                }
            }
            
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;

            LoRaMacBuffer[pktHeaderLen++] = fCtrl->Value;

            LoRaMacBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

            if( fOpts != NULL )
            {
                for( i = 0; i < fCtrl->Bits.OptionsLength; i++ )
                {
                    LoRaMacBuffer[pktHeaderLen++] = fOpts[i];
                }
            }
            if( ( MacCommandsBufferIndex + fCtrl->Bits.OptionsLength ) < 15 )
            {
                if( MacCommandsInNextTx == true )
                {
                    fCtrl->Bits.OptionsLength += MacCommandsBufferIndex;
                    
                    // Update FCtrl field with new value of OptionsLength
                    LoRaMacBuffer[0x05] = fCtrl->Value;
                    for( i = 0; i < MacCommandsBufferIndex; i++ )
                    {
                        LoRaMacBuffer[pktHeaderLen++] = MacCommandsBuffer[i];
                    }
                }
                MacCommandsInNextTx = false;
                MacCommandsBufferIndex = 0;
            }
            
            if( ( pktHeaderLen + fBufferSize ) > LORAMAC_PHY_MAXPAYLOAD )
            {
                return 3;
            }

            if( fBuffer != NULL )
            {
                LoRaMacBuffer[pktHeaderLen] = fPort;
                
                if( fPort == 0 )
                {
                    LoRaMacPayloadEncrypt( fBuffer, fBufferSize, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, LoRaMacPayload );
                }
                else
                {
                    LoRaMacPayloadEncrypt( fBuffer, fBufferSize, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, LoRaMacPayload );
                }
                LoRaMacMemCpy( LoRaMacPayload, LoRaMacBuffer + pktHeaderLen + 1, fBufferSize );
            }
            LoRaMacBufferPktLen = pktHeaderLen + 1 + fBufferSize;

            LoRaMacComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &mic );

            if( ( LoRaMacBufferPktLen + LORAMAC_MFR_LEN ) > LORAMAC_PHY_MAXPAYLOAD )
            {
                return 3;
            }
            LoRaMacBuffer[LoRaMacBufferPktLen + 0] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;
            
            LoRaMacBufferPktLen += LORAMAC_MFR_LEN;
            break;
        default:
            return 4;
    }

    return 0;
}

uint8_t LoRaMacSendFrameOnChannel( ChannelParams_t channel )
{
    LoRaMacEventInfo.TxDatarate = Datarates[ChannelsDatarate];

    Radio.SetChannel( channel.Frequency );

    if( ChannelsDatarate == DR_FSK )
    { // High Speed FSK channel
        Radio.SetTxConfig( MODEM_FSK, TxPowers[ChannelsTxPower], 25e3, 0, Datarates[ChannelsDatarate] * 1e3, 0, 5, false, true, false, 3e6 );
    }
    else if( ChannelsDatarate == DR_SF7H )
    { // High speed LoRa channel
        Radio.SetTxConfig( MODEM_LORA, TxPowers[ChannelsTxPower], 0, 1, Datarates[ChannelsDatarate], 1, 8, false, true, false, 3e6 );
    }
    else
    { // Normal LoRa channel
        Radio.SetTxConfig( MODEM_LORA, TxPowers[ChannelsTxPower], 0, 0, Datarates[ChannelsDatarate], 1, 8, false, true, false, 3e6 );
    }

    IsLoRaMacTransmitting = true;
    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );
    
    return 0;
}

uint8_t LoRaMacSendOnChannel( ChannelParams_t channel, LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint8_t status = 0;
    
    if( IsLoRaMacTransmitting == true )
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

static void LoRaMacProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize )
{
    while( macIndex < commandsSize )
    {
        // Decode Frame MAC commands
        switch( payload[macIndex++] )
        {
            case SRV_MAC_LINK_CHECK_ANS:
                LoRaMacEventFlags.Bits.LinkCheck = 1;
                LoRaMacEventInfo.DemodMargin = payload[macIndex++];
                LoRaMacEventInfo.NbGateways = payload[macIndex++];
                break;
            case SRV_MAC_LINK_ADR_REQ:
                {
                    uint8_t i;
                    uint8_t status = 0x07;
                    uint8_t drOkCounter = 0;
                    uint16_t mask = 0;
                    int8_t txPower = 0;
                    int8_t datarate = 0;
                    uint8_t nbRep = 0;
                    
                    datarate = payload[macIndex++];
                    txPower = datarate & 0x0F;
                    datarate = ( datarate >> 4 ) & 0x0F;

                    mask = payload[macIndex++];
                    mask |= payload[macIndex++] << 8;

                    nbRep = payload[macIndex++] & 0x0F;
                    if( nbRep == 0 )
                    {
                        nbRep = 1;
                    }

                    if( mask == 0 )
                    {
                        status &= 0xFE; // Channel mask KO
                    }
                    
                    for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
                    {
                        if( ( ( ( 1 << i ) & mask ) != 0 ) &&
                            ( Channels[i].Frequency == 0 ) )
                        {
                            status &= 0xFE; // Channel mask KO
                        }
                        if( ( Channels[i].Frequency != 0 ) &&
                            ( ( Channels[i].DrRange.Fields.Min <= datarate ) &&
                              ( datarate <= Channels[i].DrRange.Fields.Max ) ) )
                        {
                            drOkCounter++;
                        }
                    }
                    if( drOkCounter == 0 )
                    {
                        status &= 0xFD; // Datarate KO
                    }
                    //
                    // Remark MaxTxPower = 0 and MinTxPower = 5
                    //
                    if( ( ( LORAMAC_MAX_TX_POWER <= txPower ) &&
                          ( txPower <= LORAMAC_MIN_TX_POWER ) ) == false )
                    {
                        status &= 0xFB; // TxPower KO
                    }
                    if( ( status & 0x07 ) == 0x07 )
                    {
                        ChannelsDatarate = datarate;
                        ChannelsTxPower = txPower;
                        ChannelsMask = mask;
                        ChannelsNbRep = nbRep;
                    }
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                AggregatedDCycle = 1.0 / POW2( payload[macIndex++] );
                
                // TODO: remove next line. Currently used to suppress 
                //       Keil compiler warning
                AggregatedDCycle = AggregatedDCycle;
                
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX2_SETUP_REQ:
                {
                    uint8_t status = 0x03;
                    int8_t datarate = 0;
                    uint32_t freq = 0;
                
                    datarate = payload[macIndex++];
                    freq = payload[macIndex++];
                    freq |= payload[macIndex++] << 8;
                    freq |= payload[macIndex++] << 16;
                    
                    if( Radio.CheckRfFrequency( freq ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }

                    if( ( ( LORAMAC_MIN_DATARATE <= datarate ) &&
                          ( datarate <= LORAMAC_MAX_DATARATE ) ) == false )
                    {
                        status &= 0xFD; // Datarate range KO
                    }
                
                    if( ( status & 0x03 ) == 0x03 )
                    {
                        Rx2Channel.Datarate = datarate;
                        Rx2Channel.Frequency = freq;
                    }
                    AddMacCommand( MOTE_MAC_RX2_SETUP_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DEV_STATUS_REQ:
                AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, BoardMeasureBatterieLevel( ), LoRaMacEventInfo.RxSnr );
                break;
            case SRV_MAC_NEW_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;
                    int8_t channelIndex = 0;
                    uint32_t freq = 0;
                    DrRange_t drRange = { 0 };
                    uint8_t dutyCycle = 0;
                
                    channelIndex = payload[macIndex++];
                    freq = payload[macIndex++];
                    freq |= payload[macIndex++] << 8;
                    freq |= payload[macIndex++] << 16;
                    freq *= 100;
                    drRange.Value = payload[macIndex++];
                    dutyCycle = payload[macIndex++];
                
                    if( ( channelIndex < 3 ) && ( channelIndex > LORA_MAX_NB_CHANNELS ) )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }
                
                    if( Radio.CheckRfFrequency( freq ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }

                    if( ( drRange.Fields.Min > drRange.Fields.Max ) &&
                        ( ( LORAMAC_MIN_DATARATE <= drRange.Fields.Min ) &&
                          ( drRange.Fields.Min <= LORAMAC_MAX_DATARATE ) == false ) &&
                        ( ( LORAMAC_MIN_DATARATE <= drRange.Fields.Max ) &&
                          ( drRange.Fields.Max <= LORAMAC_MAX_DATARATE ) == false ) )
                    {
                        status &= 0xFD; // Datarate range KO
                    }
                    if( ( status & 0x03 ) == 0x03 )
                    {
                        Channels[channelIndex].Frequency = freq;
                        Channels[channelIndex].DrRange = drRange;
                        Channels[channelIndex].DutyCycle = dutyCycle;
                    }
                    AddMacCommand( MOTE_MAC_RX2_SETUP_ANS, status, 0 );
                }
                break;
            default:
                // Unknown command. ABORT MAC commands processing
                return;
        }
    }
}

/*!
 * Function to be executed on Tx Done event
 */
static void OnRadioTxDone( void )
{
    Radio.Sleep( );

    if( IsRxWindowsEnabled == true )
    {
        TimerSetValue( &RxWindowTimer1, RxWindow1Delay );
        TimerStart( &RxWindowTimer1 );
        TimerSetValue( &RxWindowTimer2, RxWindow2Delay );
        TimerStart( &RxWindowTimer2 );
    }
    
    if( NodeAckRequested == false )
    {
        ChannelsNbRepCounter++;
        if( ChannelsNbRepCounter >= ChannelsNbRep )
        {
            ChannelsNbRepCounter = 0;

            IsLoRaMacTransmitting = false;
            LoRaMacEventFlags.Bits.Tx = 1;
            LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;

            LoRaMacNotify( &LoRaMacEventFlags, &LoRaMacEventInfo );
        
            AdrAckCounter++;
            if( IsUpLinkCounterFixed == false )
            {
                UpLinkCounter++;
            }
        }
        else
        {
            TimerSetValue( &NbRepTimeoutTimer, RxWindow1Delay + 
                                               RxWindow2Delay +
                                               ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
            TimerStart( &NbRepTimeoutTimer );
        }
    }
}

/*!
 * Function to be executed on Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int8_t rssi, int8_t snr )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;

    uint8_t pktHeaderLen = 0;
    uint32_t address = 0;
    uint16_t sequenceCounter = 0;
    int32_t sequence = 0;
    uint8_t appPayloadStartIndex = 0;
    uint8_t port = 0xFF;
    uint8_t frameLen = 0;
    uint32_t mic = 0;
    uint32_t micRx = 0;
    
    bool isMicOk = false;

    Radio.Sleep( );

    TimerStop( &RxWindowTimer2 );

    macHdr.Value = payload[pktHeaderLen++];
    
    switch( macHdr.Bits.MType )
    {
        case FRAME_TYPE_JOIN_ACCEPT:
            LoRaMacJoinDecrypt( payload + 1, 16, LoRaMacAppKey, LoRaMacPayload + 1 );

            LoRaMacPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacPayload, 13, LoRaMacAppKey, &mic );
            
            micRx |= LoRaMacPayload[size - 4];
            micRx |= ( LoRaMacPayload[size - 3] << 8 );
            micRx |= ( LoRaMacPayload[size - 2] << 16 );
            micRx |= ( LoRaMacPayload[size - 1] << 24 );
            
            if( micRx == mic )
            {
                LoRaMacEventInfo.RxSnr = snr;
                LoRaMacEventInfo.RxRssi = rssi;

                LoRaMacJoinComputeSKeys( LoRaMacAppKey, LoRaMacPayload + 1, LoRaMacDevNonce, LoRaMacNwkSKey, LoRaMacAppSKey );

                LoRaMacNetID = LoRaMacPayload[4];
                LoRaMacNetID |= ( LoRaMacPayload[5] << 8 );
                LoRaMacNetID |= ( LoRaMacPayload[6] << 16 );
                
                LoRaMacDevAddr = LoRaMacPayload[7]; 
                LoRaMacDevAddr |= ( LoRaMacPayload[8] << 8 );
                LoRaMacDevAddr |= ( LoRaMacPayload[9] << 16 );
                LoRaMacDevAddr |= ( LoRaMacPayload[10] << 24 );
                
                LoRaMacEventFlags.Bits.JoinAccept = 1;
                IsLoRaMacNetworkJoined = true;
                
                LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
            }
            else
            {
                LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            }

            IsLoRaMacTransmitting = false;

            LoRaMacNotify( &LoRaMacEventFlags, &LoRaMacEventInfo );
            break;
#if defined( LORAMAC_R3 )
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
#else
        case FRAME_TYPE_DATA_CONFIRMED:
        case FRAME_TYPE_DATA_UNCONFIRMED:
#endif
            {
                address = payload[pktHeaderLen++]; 
                address |= ( payload[pktHeaderLen++] << 8 );
                address |= ( payload[pktHeaderLen++] << 16 );
                address |= ( payload[pktHeaderLen++] << 24 );

                if( address != LoRaMacDevAddr )
                {
                    if( NodeAckRequested == true )
                    {
                        TimerSetValue( &AckTimeoutTimer, ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
                        TimerStart( &AckTimeoutTimer );
                    }
                    else
                    {
                        IsLoRaMacTransmitting = false;
                    }
                    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                    LoRaMacNotify( &LoRaMacEventFlags, &LoRaMacEventInfo );

                    // We are not the destination of this frame. Just discard!
                    return;
                }
                
                fCtrl.Value = payload[pktHeaderLen++];
                
                sequenceCounter |= payload[pktHeaderLen++];
                sequenceCounter |= payload[pktHeaderLen++] << 8;

                appPayloadStartIndex = 8 + fCtrl.Bits.OptionsLength;

                micRx |= payload[size - 4];
                micRx |= ( payload[size - 3] << 8 );
                micRx |= ( payload[size - 2] << 16 );
                micRx |= ( payload[size - 1] << 24 );
                
                sequence = ( int32_t )sequenceCounter - ( int32_t )( DownLinkCounter & 0xFFFF );
                if( sequence < 0 )
                {
                    // sequence reset or roll over happened
                    DownLinkCounter = ( DownLinkCounter & 0xFFFF0000 ) | ( sequenceCounter + ( uint32_t )0x10000 );
                    LoRaMacComputeMic( payload, size - 4, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                    }
                    else
                    {
                        isMicOk = false;
                        // sequence reset
                        DownLinkCounter = sequenceCounter;
                        LoRaMacComputeMic( payload, size - 4, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                    }
                }
                else
                {
                    DownLinkCounter = ( DownLinkCounter & 0xFFFF0000 ) | sequenceCounter;
                    LoRaMacComputeMic( payload, size - 4, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                }

                if( ( isMicOk == true ) ||
                    ( micRx == mic ) )
                {
                    LoRaMacEventInfo.RxSnr = snr;
                    LoRaMacEventInfo.RxRssi = rssi;
                    AdrAckCounter = 0;
                
#if defined( LORAMAC_R3 )
                    if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN )
#else
                    if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED )
#endif
                    {
                        SrvAckRequested = true;
                    }
                    else
                    {
                        SrvAckRequested = false;
                    }
                    // Check if the frame is an acknowledgement
                    if( fCtrl.Bits.Ack == 1 )
                    {
                        // An acknowledge has been received thus the UpLinkCounter
                        // is incremented.
                        if( IsUpLinkCounterFixed == false )
                        {
                            UpLinkCounter++;
                        }
                        // Stop the AckTimeout timer as no more retransmissions 
                        // are needed.
                        TimerStop( &AckTimeoutTimer );
                        
                        IsLoRaMacTransmitting = false;
                        LoRaMacEventFlags.Bits.Tx = 1;
                        LoRaMacEventInfo.TxAckReceived = true;
                        LoRaMacEventInfo.TxNbRetries = AckTimeoutRetriesCounter;
                    }
                    else
                    {
                        LoRaMacEventInfo.TxAckReceived = false;
                        if( AckTimeoutRetriesCounter > AckTimeoutRetries )
                        {
                            IsLoRaMacTransmitting = false;
                            LoRaMacEventFlags.Bits.Tx = 1;
                            LoRaMacEventInfo.TxAckReceived = false;
                            LoRaMacEventInfo.TxNbRetries = AckTimeoutRetriesCounter;
                          
                            if( IsUpLinkCounterFixed == false )
                            {
                                UpLinkCounter++;
                            }
        
                            // Stop the AckTimeout timer as no more retransmissions 
                            // are needed.
                            TimerStop( &AckTimeoutTimer );
                        }
                    }
                    if( ( ( size - 4 ) - appPayloadStartIndex ) > 0 )
                    {
                        port = payload[appPayloadStartIndex++];
                        frameLen = ( size - 4 ) - appPayloadStartIndex;
                        
                        if( port == 0 )
                        {
                            LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                   frameLen,
                                                   LoRaMacNwkSKey,
                                                   address,
                                                   DOWN_LINK,
                                                   DownLinkCounter,
                                                   LoRaMacPayload );
                        }
                        else
                        {
                            LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                   frameLen,
                                                   LoRaMacAppSKey,
                                                   address,
                                                   DOWN_LINK,
                                                   DownLinkCounter,
                                                   LoRaMacPayload );

                            LoRaMacEventFlags.Bits.Rx = 1;
                            LoRaMacEventInfo.RxPort = port;
                            LoRaMacEventInfo.RxBuffer = LoRaMacPayload;
                            LoRaMacEventInfo.RxBufferSize = frameLen;
                        }
                    }
                    
                    if( ( fCtrl.Bits.OptionsLength > 0 ) || ( port == 0 ) )
                    {
                        // Decode Options field MAC commands
                        LoRaMacProcessMacCommands( payload, 8, appPayloadStartIndex );
                        if( port == 0 )
                        {
                            // Decode frame payload MAC commands
                            LoRaMacProcessMacCommands( payload, appPayloadStartIndex, frameLen );
                        }
                    }
                    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                }
                else
                {
                    if( NodeAckRequested == true )
                    {
                        TimerSetValue( &AckTimeoutTimer, ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
                        TimerStart( &AckTimeoutTimer );
                    }
                    else
                    {
                        IsLoRaMacTransmitting = false;
                    }
                    LoRaMacEventInfo.TxAckReceived = false;
                    LoRaMacEventInfo.TxNbRetries = AckTimeoutRetriesCounter;
                    
                    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                }

                LoRaMacNotify( &LoRaMacEventFlags, &LoRaMacEventInfo );
            }
            break;
        case FRAME_TYPE_PROPRIETARY:
            LoRaMacEventInfo.RxSnr = snr;
            LoRaMacEventInfo.RxRssi = rssi;
            //Intentional falltrough
        default:
            if( NodeAckRequested == true )
            {
                TimerSetValue( &AckTimeoutTimer, ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
                TimerStart( &AckTimeoutTimer );
            }
            else
            {
                IsLoRaMacTransmitting = false;
            }
            LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            
            LoRaMacNotify( &LoRaMacEventFlags, &LoRaMacEventInfo );
            break;
    }
}

/*!
 * Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void )
{
    Radio.Sleep( );
    
    IsLoRaMacTransmitting = false;

    LoRaMacEventFlags.Value = 0;
    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    LoRaMacNotify( &LoRaMacEventFlags, &LoRaMacEventInfo );
}

/*!
 * Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void )
{
    Radio.Sleep( );
}

/*!
 * Function executed on Radio Rx Error event
 */
static void OnRadioRxError( void )
{
    Radio.Sleep( );
}

/*!
 * Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void )
{
    if( Radio.Status( ) == RF_IDLE )
    {
        uint16_t symbTimeout = 5;
    
        if( ChannelsDatarate == DR_FSK )
        {
            Radio.SetRxConfig( MODEM_FSK, 50e3, Datarates[ChannelsDatarate] * 1e3, 0, 83.333e3, 5, 0, false, true, false, false );
        }
        else if( ChannelsDatarate == DR_SF7H )
        {
            symbTimeout = 8;
            Radio.SetRxConfig( MODEM_LORA, 1, Datarates[ChannelsDatarate], 1, 0, 8, symbTimeout, false, false, true, false );
        }
        else
        {
            // For low SF, we increase the number of symbols generating a Rx Timeout
            if( ChannelsDatarate < DR_SF9 )
            { // DR_SF10 = 2, DR_SF11 = 1, DR_SF12 = 0
                symbTimeout = 5;
            }
            else
            { // DR_SF7 = 5, DR_SF8 = 4, DR_SF9 = 3
                symbTimeout = 8;
            }
            Radio.SetRxConfig( MODEM_LORA, 0, Datarates[ChannelsDatarate], 1, 0, 8, symbTimeout, false, false, true, false );
        }
        Radio.Rx( CLS2_MAX_RX_WINDOW );
    }
}

/*!
 * Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void )
{
    if( NodeAckRequested == true )
    {
        TimerSetValue( &AckTimeoutTimer, ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
        TimerStart( &AckTimeoutTimer );
    }
    
    if( Radio.Status( ) == RF_IDLE )
    {
        uint16_t symbTimeout = 5;
    
        // RX2 Defaults to SF9/125kHz - 869.525MHz 
        Radio.SetChannel( Rx2Channel.Frequency );

        if( ChannelsDatarate == DR_FSK )
        {
            Radio.SetRxConfig( MODEM_FSK, 50e3, Datarates[Rx2Channel.Datarate] * 1e3, 0, 83.333e3, 5, 0, false, true, false, false );
        }
        else if( ChannelsDatarate == DR_SF7H )
        {
            symbTimeout = 8;
            Radio.SetRxConfig( MODEM_LORA, 1, Datarates[Rx2Channel.Datarate], 1, 0, 8, symbTimeout, false, false, true, false );
        }
        else
        {
            // For low SF, we increase the number of symbols generating a Rx Timeout
            if( Rx2Channel.Datarate < DR_SF9 )
            { // DR_SF10 = 2, DR_SF11 = 1, DR_SF12 = 0
                symbTimeout = 5;
            }
            else
            { // DR_SF7 = 5, DR_SF8 = 4, DR_SF9 = 3
                symbTimeout = 8;
            }
            Radio.SetRxConfig( MODEM_LORA, 0, Datarates[Rx2Channel.Datarate], 1, 0, 8, symbTimeout, false, false, true, false );
        }
        Radio.Rx( CLS2_MAX_RX_WINDOW );
    }
}

/*!
 * Function executed on AckTimeout timer event
 */
static void OnNbRepTimeoutTimerEvent( void )
{
    // Send same frame again
    if( LoRaMacSetNextChannel( ) != 0 )
    {
        TimerSetValue( &NbRepTimeoutTimer, RxWindow1Delay + 
                                           RxWindow2Delay +
                                           ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
        TimerStart( &NbRepTimeoutTimer );
        return;
    }

    // Sends the same frame again
    LoRaMacSendFrameOnChannel( Channels[Channel] );
}

/*!
 * Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void )
{
    if( ( AckTimeoutRetriesCounter < AckTimeoutRetries ) && ( AckTimeoutRetriesCounter < MAX_ACK_RETRIES ) )
    {
        AckTimeoutRetriesCounter++;
        
        if( ( AckTimeoutRetriesCounter % 2 ) == 0 )
        {
            ChannelsDatarate = MAX( ChannelsDatarate - 1, LORAMAC_MIN_DATARATE );
        }
        if( LoRaMacSetNextChannel( ) != 0 )
        {
            TimerSetValue( &AckTimeoutTimer, ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
            TimerStart( &AckTimeoutTimer );
            return;
        }

        // Sends the same frame again
        LoRaMacSendFrameOnChannel( Channels[Channel] );
    }
    else
    {
        // Re-enable default channels LC1, LC2, LC3
        ChannelsMask = ChannelsMask | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );

        IsLoRaMacTransmitting = false;
        LoRaMacEventFlags.Bits.Tx = 1;
        LoRaMacEventInfo.TxAckReceived = false;
        LoRaMacEventInfo.TxNbRetries = AckTimeoutRetriesCounter;
      
        if( IsUpLinkCounterFixed == false )
        {
            UpLinkCounter++;
        }
        
        LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        LoRaMacNotify( &LoRaMacEventFlags, &LoRaMacEventInfo );
    }
}

/*!
 * ============================================================================
 * = LoRaMac test functions                                                   =
 * ============================================================================
 */

void LoRaMacSetChannelsDatarate( int8_t datrate )
{
    ChannelsDatarate = datrate;
}

void LoRaMacSetChannelsTxPower( int8_t txPower )
{
    ChannelsTxPower = txPower;
}

void LoRaMacTestRxWindowsOn( bool enable )
{
    IsRxWindowsEnabled = enable;
}

void LoRaMacSetMicTest( uint16_t upLinkCounter )
{
    UpLinkCounter = upLinkCounter;
    IsUpLinkCounterFixed = true;
}
