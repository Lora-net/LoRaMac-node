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
 * Indicates if the node is connected to a private or public network
 */
static bool PublicNetwork;

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

#if defined( USE_BAND_868 )
/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
    BAND1,
    BAND2,
    BAND3,
    BAND4,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
    LC4,
    LC5,
    LC6,
    LC7,
    LC8,
    LC9,
};
#else
/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#endif

/*!
 * LoRaMAC 2nd reception window settings
 */
static Rx2ChannelParams_t Rx2Channel = RX_WND_2_CHANNEL;

/*!
 * Datarate offset between uplink and downlink on first window
 */
static uint8_t Rx1DrOffset = 0;

/*!
 * Mask indicating which channels are enabled
 */
static uint16_t ChannelsMask;

/*!
 * Channels Tx output power
 */
static int8_t ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;

/*!
 * Channels datarate
 */
static int8_t ChannelsDatarate = LORAMAC_DEFAULT_DATARATE;

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
 * Agregated duty cycle management
 */
static uint16_t AggregatedDCycle;
static uint64_t AggregatedLastTxDoneTime;
static uint64_t AggregatedTimeOff;

/*!
 * Enables/Disables duty cycle management (Test only)
 */
static bool DutyCycleOn;

/*!
 * Current channel index
 */
static uint8_t Channel;

/*!
 * LoRaMac internal states
 */
enum LoRaMacState_e
{
    MAC_IDLE          = 0x00000000,
    MAC_TX_RUNNING    = 0x00000001,
    MAC_RX            = 0x00000002,
    MAC_ACK_REQ       = 0x00000004,
    MAC_ACK_RETRY     = 0x00000008,
    MAC_CHANNEL_CHECK = 0x00000010,
};

/*!
 * LoRaMac internal state
 */
uint32_t LoRaMacState = MAC_IDLE;

/*!
 * LoRaMac timer used to check the LoRaMacState (runs every second)
 */
static TimerEvent_t MacStateCheckTimer;

/*!
 * LoRaMac upper layer event functions
 */
static LoRaMacEvent_t *LoRaMacEvents;

/*!
 * LoRaMac notification event flags
 */
LoRaMacEventFlags_t LoRaMacEventFlags;

/*!
 * LoRaMac notification event info
 */
LoRaMacEventInfo_t LoRaMacEventInfo;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LoRaMac channel check timer
 */
static TimerEvent_t ChannelCheckTimer;

/*!
 * LoRaMac duty cycle delayed Tx timer
 */
static TimerEvent_t TxDelayedTimer;

/*!
 * LoRaMac reception windows timers
 */
static TimerEvent_t RxWindowTimer1;
static TimerEvent_t RxWindowTimer2;

/*!
 * LoRaMac reception windows delay from end of Tx
 */
static uint32_t ReceiveDelay1;
static uint32_t ReceiveDelay2;
static uint32_t JoinAcceptDelay1;
static uint32_t JoinAcceptDelay2;

/*!
 * LoRaMac reception windows delay
 * \remark normal frame: RxWindowXDelay = ReceiveDelayX - RADIO_WAKEUP_TIME
 *         join frame  : RxWindowXDelay = JoinAcceptDelayX - RADIO_WAKEUP_TIME
 */
static uint32_t RxWindow1Delay;
static uint32_t RxWindow2Delay;

/*!
 * LoRaMac maximum time a reception window stays open
 */
static uint32_t MaxRxWindow;

/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
static TimerEvent_t AckTimeoutTimer;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetries = 1;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetriesCounter = 1;

/*!
 * Indicates if the AckTimeout timer has expired or not
 */
static bool AckTimeoutRetry = false;

/*!
 * Last transmission time on air
 */
TimerTime_t TxTimeOnAir = 0;

/*!
 * Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void );

/*!
 * Function to be executed on Radio Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

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
 * Function executed on Resend Frame timer event.
 */
static void OnMacStateCheckTimerEvent( void );

/*!
 * Function executed on duty cycle delayed Tx  timer event
 */
static void OnTxDelayedTimerEvent( void );

/*!
 * Function executed on channel check timer event
 */
static void OnChannelCheckTimerEvent( void );

/*!
 * Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void );

/*!
 * Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void );

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
    uint64_t curTime = TimerGetCurrentTime( );

    memset( enabledChannels, 0, LORA_MAX_NB_CHANNELS );

    // Update Aggregated duty cycle
    if( AggregatedTimeOff < ( curTime - AggregatedLastTxDoneTime ) )
    {
        AggregatedTimeOff = 0;
    }

    // Update bands Time OFF
    uint64_t minTime = 0xFFFFFFFFFFFFFFFF;
    for( i = 0; i < LORA_MAX_NB_BANDS; i++ )
    {
        if( DutyCycleOn == true )
        {
            if( Bands[i].TimeOff < ( curTime - Bands[i].LastTxDoneTime ) )
            {
                Bands[i].TimeOff = 0;
            }
            if( Bands[i].TimeOff != 0 )
            {
                minTime = MIN( Bands[i].TimeOff, minTime );
            }
        }
        else
        {
            minTime = 0;
            Bands[i].TimeOff = 0;
        }
    }

    // Search how many channels are enabled
    for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
    {
        if( ( ( 1 << i ) & ChannelsMask ) != 0 )
        {
            if( Channels[i].Frequency == 0 )
            { // Check if the channel is enabled
                continue;
            }
            if( ( ( Channels[i].DrRange.Fields.Min <= ChannelsDatarate ) &&
                  ( ChannelsDatarate <= Channels[i].DrRange.Fields.Max ) ) == false )
            { // Check if the current channel selection supports the given datarate
                continue;
            }
            if( Bands[Channels[i].Band].TimeOff > 0 )
            { // Check if the band is available for transmission
                continue;
            }
            if( AggregatedTimeOff > 0 )
            { // Check if there is time available for transmission
                continue;
            }
            enabledChannels[nbEnabledChannels++] = i;
        }
    }
    if( nbEnabledChannels > 0 )
    {
        for( i = 0, j = randr( 0, nbEnabledChannels - 1 ); i < LORA_MAX_NB_CHANNELS; i++ )
        {
            channelNext = enabledChannels[j];
            j = ( j + 1 ) % nbEnabledChannels;

            if( Radio.IsChannelFree( MODEM_LORA, Channels[channelNext].Frequency, RSSI_FREE_TH ) == true )
            {
                // Free channel found
                Channel = channelNext;
                
                LoRaMacState &= ~MAC_CHANNEL_CHECK;
                TimerStop( &ChannelCheckTimer );
                return 0;
            }
        }
    }
    // No free channel found. 
    // Check again
    if( ( LoRaMacState & MAC_CHANNEL_CHECK ) == 0 )
    {
        TimerSetValue( &ChannelCheckTimer, minTime );
        TimerStart( &ChannelCheckTimer );
        LoRaMacState |= MAC_CHANNEL_CHECK;
    }
    return 1;
}

/*
 * TODO: Add documentation
 */
void OnChannelCheckTimerEvent( void )
{
    LoRaMacState &= ~MAC_CHANNEL_CHECK;
    if( LoRaMacSetNextChannel( ) == 0 )
    {
        if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
        {
           LoRaMacSendFrameOnChannel( Channels[Channel] );
        }
    }
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
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
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
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
            // No payload for this answer
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
    LoRaMacState = MAC_IDLE;

#if defined( USE_BAND_868 )
    ChannelsMask = LC( 1 ) + LC( 2 ) + LC( 3 ) + LC( 4 ) + LC( 5 ) + LC( 6 ) + LC( 7 ) + LC( 8 ) + LC( 9 );
#else
    ChannelsMask = LC( 1 ) + LC( 2 ) + LC( 3 );
#endif
    ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;
    ChannelsDatarate = LORAMAC_DEFAULT_DATARATE;
    ChannelsNbRep = 1;
    ChannelsNbRepCounter = 0;
    
    MaxDCycle = 0;
    AggregatedDCycle = 1;
    AggregatedLastTxDoneTime = 0;
    AggregatedTimeOff = 0;

    DutyCycleOn = true;

    MaxRxWindow = MAX_RX_WINDOW;
    ReceiveDelay1 = RECEIVE_DELAY1;
    ReceiveDelay2 = RECEIVE_DELAY2;
    JoinAcceptDelay1 = JOIN_ACCEPT_DELAY1;
    JoinAcceptDelay2 = JOIN_ACCEPT_DELAY2;

    TimerInit( &MacStateCheckTimer, OnMacStateCheckTimerEvent );
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );

    TimerInit( &ChannelCheckTimer, OnChannelCheckTimerEvent );
    TimerInit( &TxDelayedTimer, OnTxDelayedTimerEvent );
    TimerInit( &RxWindowTimer1, OnRxWindow1TimerEvent );
    TimerInit( &RxWindowTimer2, OnRxWindow2TimerEvent );
    TimerInit( &AckTimeoutTimer, OnAckTimeoutTimerEvent );
    
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

    PublicNetwork = true;
    LoRaMacSetPublicNetwork( PublicNetwork );
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

    macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    return LoRaMacSend( &macHdr, NULL, fPort, fBuffer, fBufferSize );
}

uint8_t LoRaMacSendConfirmedFrame( uint8_t fPort, void *fBuffer, uint16_t fBufferSize, uint8_t retries )
{
    LoRaMacHeader_t macHdr;

    AckTimeoutRetries = retries;
    AckTimeoutRetriesCounter = 1;
    
    macHdr.Value = 0;

    macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
    return LoRaMacSend( &macHdr, NULL, fPort, fBuffer, fBufferSize );
}

uint8_t LoRaMacSend( LoRaMacHeader_t *macHdr, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;

    fCtrl.Value = 0;

    fCtrl.Bits.FOptsLen      = 0;
    fCtrl.Bits.FPending      = 0;
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = AdrCtrlOn;

    if( LoRaMacSetNextChannel( ) == 0 )
    {
        return LoRaMacSendOnChannel( Channels[Channel], macHdr, &fCtrl, fOpts, fPort, fBuffer, fBufferSize );
    }
    return 5;
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
            RxWindow1Delay = JoinAcceptDelay1 - RADIO_WAKEUP_TIME;
            RxWindow2Delay = JoinAcceptDelay2 - RADIO_WAKEUP_TIME;

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
        case FRAME_TYPE_DATA_CONFIRMED_UP:
            NodeAckRequested = true;
            //Intentional falltrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if( IsLoRaMacNetworkJoined == false )
            {
                return 2; // No network has been joined yet
            }
            
            RxWindow1Delay = ReceiveDelay1 - RADIO_WAKEUP_TIME;
            RxWindow2Delay = ReceiveDelay2 - RADIO_WAKEUP_TIME;

            if( fOpts == NULL )
            {
                fCtrl->Bits.FOptsLen = 0;
            }

            if( SrvAckRequested == true )
            {
                SrvAckRequested = false;
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
                for( i = 0; i < fCtrl->Bits.FOptsLen; i++ )
                {
                    LoRaMacBuffer[pktHeaderLen++] = fOpts[i];
                }
            }
            if( ( MacCommandsBufferIndex + fCtrl->Bits.FOptsLen ) < 15 )
            {
                if( MacCommandsInNextTx == true )
                {
                    fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;
                    
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
    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    LoRaMacEventInfo.TxDatarate = Datarates[ChannelsDatarate];

    Radio.SetChannel( channel.Frequency );

    if( ChannelsDatarate == DR_FSK )
    { // High Speed FSK channel
        Radio.SetTxConfig( MODEM_FSK, TxPowers[ChannelsTxPower], 25e3, 0, Datarates[ChannelsDatarate] * 1e3, 0, 5, false, true, 0, 0, false, 3e6 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_FSK, LoRaMacBufferPktLen );
    }
    else if( ChannelsDatarate == DR_SF7H )
    { // High speed LoRa channel
        Radio.SetTxConfig( MODEM_LORA, TxPowers[ChannelsTxPower], 0, 1, Datarates[ChannelsDatarate], 1, 8, false, true, 0, 0, false, 3e6 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetTxConfig( MODEM_LORA, TxPowers[ChannelsTxPower], 0, 0, Datarates[ChannelsDatarate], 1, 8, false, true, 0, 0, false, 3e6 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
    
    if( MaxDCycle == 255 )
    {
        return 6;
    }
    if( MaxDCycle == 0 )
    {
        AggregatedTimeOff = 0;
    }

    LoRaMacState |= MAC_TX_RUNNING;
    // Starts the MAC layer status check timer
    TimerStart( &MacStateCheckTimer );
    
    if( MAX( Bands[channel.Band].TimeOff, AggregatedTimeOff ) > ( TimerGetCurrentTime( ) ) )
    {
        // Schedule transmission
        TimerSetValue( &TxDelayedTimer, MAX( Bands[channel.Band].TimeOff, AggregatedTimeOff ) );
        TimerStart( &TxDelayedTimer );
    }
    else
    {
        // Send now
        Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );
    }
    return 0;
}


void OnTxDelayedTimerEvent( void )
{
    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );
}

uint8_t LoRaMacSendOnChannel( ChannelParams_t channel, LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint8_t status = 0;
    
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
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
                if( AdrCtrlOn == false )
                { // ADR disabled don't handle ADR requests
                    // Answer the server with fail status
                    // Power ACK     = 0
                    // Data rate ACK = 0
                    // Channel mask  = 0
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, 0, 0 );
                    break;
                }
                {
                    uint8_t i;
                    uint8_t status = 0x07;
                    uint16_t chMask = 0;
                    int8_t txPower = 0;
                    int8_t datarate = 0;
                    uint8_t nbRep = 0;
                    uint8_t chMaskCntl = 0;
                    
                    datarate = payload[macIndex++];
                    txPower = datarate & 0x0F;
                    datarate = ( datarate >> 4 ) & 0x0F;

                    chMask = payload[macIndex++];
                    chMask |= payload[macIndex++] << 8;

                    nbRep = payload[macIndex++];
                    chMaskCntl = ( nbRep >> 4 ) & 0x07;
                    nbRep &= 0x0F;
                    if( nbRep == 0 )
                    {
                        nbRep = 1;
                    }
                    if( ( chMaskCntl == 0 ) && ( chMask == 0 ) )
                    {
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
                        {
                            if( chMaskCntl == 6 )
                            {
                                if( Channels[i].Frequency != 0 )
                                {
                                    chMask |= 1 << i;
                                }
                            }
                            else
                            {
                                if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                                    ( Channels[i].Frequency == 0 ) )
                                {// Trying to enable an undefined channel
                                    status &= 0xFE; // Channel mask KO
                                }
                            }
                        }
                    }
                    if( ( ( datarate < LORAMAC_MIN_DATARATE ) ||
                          ( datarate > LORAMAC_MAX_DATARATE ) ) == true )
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
                        ChannelsMask = chMask;
                        ChannelsNbRep = nbRep;
                    }
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                MaxDCycle = payload[macIndex++];
                AggregatedDCycle = 1 << MaxDCycle;
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX_PARAM_SETUP_REQ:
                {
                    uint8_t status = 0x07;
                    int8_t datarate = 0;
                    int8_t drOffset = 0;
                    uint32_t freq = 0;
                
                    drOffset = payload[macIndex++];
                    datarate = drOffset & 0x0F;
                    drOffset = ( drOffset >> 4 ) & 0x0F;
                    
                    freq = payload[macIndex++];
                    freq |= payload[macIndex++] << 8;
                    freq |= payload[macIndex++] << 16;
                    freq *= 100;
                    
                    if( Radio.CheckRfFrequency( freq ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }
                    
                    if( ( ( datarate < LORAMAC_MIN_DATARATE ) ||
                          ( datarate > LORAMAC_MAX_DATARATE ) ) == true )
                    {
                        status &= 0xFD; // Datarate KO
                    }

                    if( ( ( drOffset < 0 ) || ( drOffset > 5 ) ) == true )
                    {
                        status &= 0xFB; // Rx1DrOffset range KO
                    }
                    
                    if( ( status & 0x07 ) == 0x07 )
                    {
                        Rx2Channel.Datarate = datarate;
                        Rx2Channel.Frequency = freq;
                        Rx1DrOffset = drOffset;
                    }
                    AddMacCommand( MOTE_MAC_RX_PARAM_SETUP_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DEV_STATUS_REQ:
                AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, BoardMeasureBatterieLevel( ), LoRaMacEventInfo.RxSnr );
                break;
            case SRV_MAC_NEW_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;
                    int8_t channelIndex = 0;
                    ChannelParams_t chParam;
                    
                    channelIndex = payload[macIndex++];
                    chParam.Frequency = payload[macIndex++];
                    chParam.Frequency |= payload[macIndex++] << 8;
                    chParam.Frequency |= payload[macIndex++] << 16;
                    chParam.Frequency *= 100;
                    chParam.DrRange.Value = payload[macIndex++];
                    
                    if( ( channelIndex < 3 ) || ( channelIndex > LORA_MAX_NB_CHANNELS ) )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }
                
                    if( Radio.CheckRfFrequency( chParam.Frequency ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }

                    if( ( chParam.DrRange.Fields.Min > chParam.DrRange.Fields.Max ) ||
                        ( ( ( LORAMAC_MIN_DATARATE <= chParam.DrRange.Fields.Min ) &&
                            ( chParam.DrRange.Fields.Min <= LORAMAC_MAX_DATARATE ) ) == false ) ||
                        ( ( ( LORAMAC_MIN_DATARATE <= chParam.DrRange.Fields.Max ) &&
                            ( chParam.DrRange.Fields.Max <= LORAMAC_MAX_DATARATE ) ) == false ) )
                    {
                        status &= 0xFD; // Datarate range KO
                    }
                    if( ( status & 0x03 ) == 0x03 )
                    {
                        LoRaMacSetChannel( channelIndex, chParam );
                    }
                    AddMacCommand( MOTE_MAC_NEW_CHANNEL_ANS, status, 0 );
                }
                break;
            case SRV_MAC_RX_TIMING_SETUP_REQ:
                {
                    uint8_t delay = payload[macIndex++] & 0x0F;
                    
                    if( delay == 0 )
                    {
                        delay++;
                    }
                    ReceiveDelay1 = delay * 1e6;
                    ReceiveDelay2 = ReceiveDelay1 + 1e6;
                    AddMacCommand( MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
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
    uint64_t curTime = TimerGetCurrentTime( );
    Radio.Sleep( );

    // Update Band Time OFF
    Bands[Channels[Channel].Band].LastTxDoneTime = curTime;
    if( DutyCycleOn == true )
    {
        Bands[Channels[Channel].Band].TimeOff = TxTimeOnAir * Bands[Channels[Channel].Band].DCycle - TxTimeOnAir;
    }
    else
    {
        Bands[Channels[Channel].Band].TimeOff = 0;
    }
    // Update Agregated Time OFF
    AggregatedLastTxDoneTime = curTime;
    AggregatedTimeOff = AggregatedTimeOff + ( TxTimeOnAir * AggregatedDCycle - TxTimeOnAir );

    if( IsRxWindowsEnabled == true )
    {
        TimerSetValue( &RxWindowTimer1, RxWindow1Delay );
        TimerStart( &RxWindowTimer1 );
        TimerSetValue( &RxWindowTimer2, RxWindow2Delay );
        TimerStart( &RxWindowTimer2 );
    }
    else
    {
        LoRaMacEventFlags.Bits.Tx = 1;
        LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
    }
    
    if( NodeAckRequested == false )
    {
        ChannelsNbRepCounter++;
    }
}

/*!
 * Function to be executed on Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
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
            LoRaMacJoinDecrypt( payload + 1, size - 1, LoRaMacAppKey, LoRaMacPayload + 1 );

            LoRaMacPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacPayload, size - LORAMAC_MFR_LEN, LoRaMacAppKey, &mic );
            
            micRx |= LoRaMacPayload[size - LORAMAC_MFR_LEN];
            micRx |= ( LoRaMacPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( LoRaMacPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( LoRaMacPayload[size - LORAMAC_MFR_LEN + 3] << 24 );
            
            if( micRx == mic )
            {
                LoRaMacEventFlags.Bits.Rx = 1;
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
                
                // DLSettings
                Rx1DrOffset = ( LoRaMacPayload[11] >> 4 ) & 0x07;
                Rx2Channel.Datarate = LoRaMacPayload[11] & 0x0F;
                
                // RxDelay
                ReceiveDelay1 = ( LoRaMacPayload[12] & 0x0F );
                if( ReceiveDelay1 == 0 )
                {
                    ReceiveDelay1 = 1;
                }
                ReceiveDelay1 *= 1e6;
                ReceiveDelay2 = ReceiveDelay1 + 1e6;
                
                //CFList
                if( ( size - 1 ) > 16 )
                {
                    ChannelParams_t param;
                    param.DrRange.Value = ( DR_SF7 << 4 ) | DR_SF12;

                    for( uint8_t i = 3, j = 0; i < ( 5 + 3 ); i++, j += 3 )
                    {
                        param.Frequency = ( LoRaMacPayload[13 + j] | ( LoRaMacPayload[14 + j] << 8 ) | ( LoRaMacPayload[15 + j] << 16 ) ) * 100;
                        LoRaMacSetChannel( i, param );
                    }
                }
                
                LoRaMacEventFlags.Bits.JoinAccept = 1;
                IsLoRaMacNetworkJoined = true;
                
                LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
            }
            else
            {
                LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL;
            }

            LoRaMacEventFlags.Bits.Tx = 1;
            break;
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            {
                address = payload[pktHeaderLen++];
                address |= ( payload[pktHeaderLen++] << 8 );
                address |= ( payload[pktHeaderLen++] << 16 );
                address |= ( payload[pktHeaderLen++] << 24 );

                if( address != LoRaMacDevAddr )
                {
                    // We are not the destination of this frame.
                    LoRaMacEventFlags.Bits.Tx = 1;
                    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL;
                    return;
                }
                
                fCtrl.Value = payload[pktHeaderLen++];
                
                sequenceCounter |= payload[pktHeaderLen++];
                sequenceCounter |= payload[pktHeaderLen++] << 8;

                appPayloadStartIndex = 8 + fCtrl.Bits.FOptsLen;

                micRx |= payload[size - LORAMAC_MFR_LEN];
                micRx |= ( payload[size - LORAMAC_MFR_LEN + 1] << 8 );
                micRx |= ( payload[size - LORAMAC_MFR_LEN + 2] << 16 );
                micRx |= ( payload[size - LORAMAC_MFR_LEN + 3] << 24 );

                sequence = ( int32_t )sequenceCounter - ( int32_t )( DownLinkCounter & 0xFFFF );
                if( sequence < 0 )
                {
                    // sequence reset or roll over happened
                    DownLinkCounter = ( DownLinkCounter & 0xFFFF0000 ) | ( sequenceCounter + ( uint32_t )0x10000 );
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                    }
                    else
                    {
                        isMicOk = false;
                        // sequence reset
                        DownLinkCounter = sequenceCounter;
                        LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                    }
                }
                else
                {
                    DownLinkCounter = ( DownLinkCounter & 0xFFFF0000 ) | sequenceCounter;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                }

                if( ( isMicOk == true ) ||
                    ( micRx == mic ) )
                {
                    LoRaMacEventFlags.Bits.Rx = 1;
                    LoRaMacEventInfo.RxSnr = snr;
                    LoRaMacEventInfo.RxRssi = rssi;
                    LoRaMacEventInfo.RxBufferSize = 0;
                    AdrAckCounter = 0;
                
                    if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN )
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
                        LoRaMacEventInfo.TxAckReceived = true;

                        // Stop the AckTimeout timer as no more retransmissions 
                        // are needed.
                        TimerStop( &AckTimeoutTimer );
                    }
                    else
                    {
                        LoRaMacEventInfo.TxAckReceived = false;
                        if( AckTimeoutRetriesCounter > AckTimeoutRetries )
                        {
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

                            LoRaMacEventFlags.Bits.RxData = 1;
                            LoRaMacEventInfo.RxPort = port;
                            LoRaMacEventInfo.RxBuffer = LoRaMacPayload;
                            LoRaMacEventInfo.RxBufferSize = frameLen;
                        }
                    }
                    
                    if( fCtrl.Bits.FOptsLen > 0 )
                    {
                        // Decode Options field MAC commands
                        LoRaMacProcessMacCommands( payload, 8, 8 + fCtrl.Bits.FOptsLen );
                    }
                    if( port == 0 )
                    {
                        // Decode frame payload MAC commands
                        LoRaMacProcessMacCommands( LoRaMacPayload, 0, frameLen );
                    }

                    LoRaMacEventFlags.Bits.Tx = 1;
                    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                }
                else
                {
                    LoRaMacEventInfo.TxAckReceived = false;
                    
                    LoRaMacEventFlags.Bits.Tx = 1;
                    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_MIC_FAIL;
                }
            }
            break;
        case FRAME_TYPE_PROPRIETARY:
            //Intentional falltrough
        default:
            LoRaMacEventFlags.Bits.Tx = 1;
            LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            break;
    }
}

/*!
 * Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void )
{
    Radio.Sleep( );
    
    LoRaMacEventFlags.Bits.Tx = 1;
    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
}

/*!
 * Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void )
{
    Radio.Sleep( );
    if( LoRaMacEventFlags.Bits.RxSlot == 1 )
    {
        LoRaMacEventFlags.Bits.Tx = 1;
        LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
    }
}

/*!
 * Function executed on Radio Rx Error event
 */
static void OnRadioRxError( void )
{
    Radio.Sleep( );
    if( LoRaMacEventFlags.Bits.RxSlot == 1 )
    {
        LoRaMacEventFlags.Bits.Tx = 1;
        LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
    }
}

/*!
 * Initializes and opens the reception window
 *
 * \param [IN] freq window channel frequency
 * \param [IN] datarate window channel datarate
 * \param [IN] bandwidth window channel bandwidth
 * \param [IN] timeout window channel timeout
 */
void LoRaMacRxWindowSetup( uint32_t freq, int8_t datarate, uint32_t bandwidth, uint16_t timeout )
{
    if( Radio.Status( ) == RF_IDLE )
    {
        Radio.SetChannel( freq );
        if( datarate == DR_FSK )
        {
            Radio.SetRxConfig( MODEM_FSK, 50e3, Datarates[datarate] * 1e3, 0, 83.333e3, 5, 0, false, 0, true, 0, 0, false, false );
        }
        else
        {
            Radio.SetRxConfig( MODEM_LORA, bandwidth, Datarates[datarate], 1, 0, 8, timeout, false, 0, false, 0, 0, true, false );
        }
        Radio.Rx( MaxRxWindow );
    }
}

/*!
 * Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void )
{
    uint16_t symbTimeout = 5; // DR_SF10, DR_SF11, DR_SF12
    int8_t datarate = 0;
    uint32_t bandwidth = 0; // LoRa 125 kHz

    datarate = ChannelsDatarate - Rx1DrOffset;
    if( datarate < 0 )
    {
        datarate = DR_SF12;
    }

    // For higher datarates, we increase the number of symbols generating a Rx Timeout
    if( datarate >= DR_SF9 )
    { // DR_SF7H, DR_SF7, DR_SF8, DR_SF9
        symbTimeout = 8;
    }
    if( datarate == DR_SF7H )
    {// LoRa 250 kHz
        bandwidth  = 1;
    }
    LoRaMacEventFlags.Bits.RxSlot = 0;
    LoRaMacRxWindowSetup( Channels[Channel].Frequency, datarate, bandwidth, symbTimeout );
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
    
    uint16_t symbTimeout = 5; // DR_SF10, DR_SF11, DR_SF12
    uint32_t bandwidth = 0; // LoRa 125 kHz

    // For higher datarates, we increase the number of symbols generating a Rx Timeout
    if( Rx2Channel.Datarate >= DR_SF9 )
    { // DR_SF7H, DR_SF7, DR_SF8, DR_SF9
        symbTimeout = 8;
    }
    if( Rx2Channel.Datarate == DR_SF7H )
    {// LoRa 250 kHz
        bandwidth  = 1;
    }

    LoRaMacEventFlags.Bits.RxSlot = 1;
    LoRaMacRxWindowSetup( Rx2Channel.Frequency, Rx2Channel.Datarate, bandwidth, symbTimeout );
}

/*!
 * Function executed on MacStateCheck timer event
 */
static void OnMacStateCheckTimerEvent( void )
{
    if( LoRaMacEventFlags.Bits.Tx == 1 )
    {
        if( NodeAckRequested == false )
        {
            if( LoRaMacEventFlags.Bits.JoinAccept == true )
            {
                // Join messages aren't repeated automatically
                ChannelsNbRepCounter = ChannelsNbRep;
                UpLinkCounter = 0;
            }
            if( ChannelsNbRepCounter >= ChannelsNbRep )
            {
                ChannelsNbRepCounter = 0;

                LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;

                AdrAckCounter++;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }

                LoRaMacState &= ~MAC_TX_RUNNING;
            }
            else
            {
                LoRaMacEventFlags.Bits.Tx = 0;
                // Sends the same frame again
                if( LoRaMacSetNextChannel( ) == 0 )
                {
                    LoRaMacSendFrameOnChannel( Channels[Channel] );
                }
            }
        }

        if( LoRaMacEventFlags.Bits.Rx == 1 )
        {
            if( ( LoRaMacEventInfo.TxAckReceived == true ) || ( AckTimeoutRetriesCounter > AckTimeoutRetries ) )
            {
                AckTimeoutRetry = false;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }
                LoRaMacEventInfo.TxNbRetries = AckTimeoutRetriesCounter;
                
                LoRaMacState &= ~MAC_TX_RUNNING;
            }
        }
        
        if( ( AckTimeoutRetry == true ) && ( ( LoRaMacState & MAC_CHANNEL_CHECK ) == 0 ) )
        {
            AckTimeoutRetry = false;
            if( ( AckTimeoutRetriesCounter < AckTimeoutRetries ) && ( AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) )
            {
                AckTimeoutRetriesCounter++;
                
                if( ( AckTimeoutRetriesCounter % 2 ) == 1 )
                {
                    ChannelsDatarate = MAX( ChannelsDatarate - 1, LORAMAC_MIN_DATARATE );
                }
                LoRaMacEventFlags.Bits.Tx = 0;
                // Sends the same frame again
                if( LoRaMacSetNextChannel( ) == 0 )
                {
                    LoRaMacSendFrameOnChannel( Channels[Channel] );
                }
            }
            else
            {
                // Re-enable default channels LC1, LC2, LC3
                ChannelsMask = ChannelsMask | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );

                LoRaMacState &= ~MAC_TX_RUNNING;
                
                LoRaMacEventInfo.TxAckReceived = false;
                LoRaMacEventInfo.TxNbRetries = AckTimeoutRetriesCounter;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }
                LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
            }
        }
    }

    if( LoRaMacState == MAC_IDLE )
    {
        LoRaMacNotify( &LoRaMacEventFlags, &LoRaMacEventInfo );
    }
    else
    {
        // Operation not finished restart timer
        TimerStart( &MacStateCheckTimer );
    }
}

static void OnAckTimeoutTimerEvent( void )
{
    AckTimeoutRetry = true;
    LoRaMacState &= ~MAC_ACK_REQ;
}

/*!
 * ============================================================================
 * = LoRaMac test functions                                                   =
 * ============================================================================
 */

void LoRaMacSetPublicNetwork( bool enable )
{
    PublicNetwork = enable;
    Radio.SetModem( MODEM_LORA );
    if( PublicNetwork == true )
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }
}

void LoRaMacSetDutyCycleOn( bool enable )
{
    DutyCycleOn = enable;
}

void LoRaMacSetChannel( uint8_t id, ChannelParams_t params )
{
    params.Band = 0;
    Channels[id] = params;
    // Activate the newly created channel
    ChannelsMask |= 1 << id;

#if defined( USE_BAND_868 )
    if( ( Channels[id].Frequency >= 865000000 ) && ( Channels[id].Frequency <= 868000000 ) )
    {
        if( Channels[id].Band != BAND_G1_0 )
        {
            Channels[id].Band = BAND_G1_0;
        }
    }
    else if( ( Channels[id].Frequency > 868000000 ) && ( Channels[id].Frequency <= 868600000 ) )
    {
        if( Channels[id].Band != BAND_G1_1 )
        {
            Channels[id].Band = BAND_G1_1;
        }
    }
    else if( ( Channels[id].Frequency >= 868700000 ) && ( Channels[id].Frequency <= 869200000 ) )
    {
        if( Channels[id].Band != BAND_G1_2 )
        {
            Channels[id].Band = BAND_G1_2;
        }
    }
    else if( ( Channels[id].Frequency >= 869400000 ) && ( Channels[id].Frequency <= 869650000 ) )
    {
        if( Channels[id].Band != BAND_G1_3 )
        {
            Channels[id].Band = BAND_G1_3;
        }
    }
    else if( ( Channels[id].Frequency >= 869700000 ) && ( Channels[id].Frequency <= 870000000 ) )
    {
        if( Channels[id].Band != BAND_G1_4 )
        {
            Channels[id].Band = BAND_G1_4;
        }
    }
    else
    {
        Channels[id].Frequency = 0;
        Channels[id].DrRange.Value = 0;
    }
#endif
    // Check if it is a valid channel
    if( Channels[id].Frequency == 0 )
    {
        ChannelsMask &= ~( 1 << id );
    }
}

void LoRaMacSetRx2Channel( Rx2ChannelParams_t param )
{
    Rx2Channel = param;
}

void LoRaMacSetChannelsTxPower( int8_t txPower )
{
    ChannelsTxPower = txPower;
}

void LoRaMacSetChannelsDatarate( int8_t datrate )
{
    ChannelsDatarate = datrate;
}

void LoRaMacSetChannelsMask( uint16_t mask )
{
    ChannelsMask = mask;
}

void LoRaMacSetChannelsNbRep( uint8_t nbRep )
{
    if( nbRep < 1 )
    {
        nbRep = 1;
    }
    if( nbRep > 15 )
    {
        nbRep = 15;
    }
    ChannelsNbRep = nbRep;
}

void LoRaMacSetMaxRxWindow( uint32_t delay )
{
    MaxRxWindow = delay;
}

void LoRaMacSetReceiveDelay1( uint32_t delay )
{
    ReceiveDelay1 = delay;
}

void LoRaMacSetReceiveDelay2( uint32_t delay )
{
    ReceiveDelay2 = delay;
}

void LoRaMacSetJoinAcceptDelay1( uint32_t delay )
{
    JoinAcceptDelay1 = delay;
}

void LoRaMacSetJoinAcceptDelay2( uint32_t delay )
{
    JoinAcceptDelay2 = delay;
}

void LoRaMacTestRxWindowsOn( bool enable )
{
    IsRxWindowsEnabled = enable;
}

uint32_t LoRaMacGetUpLinkCounter( void )
{
    return UpLinkCounter;
}

uint32_t LoRaMacGetDownLinkCounter( void )
{
    return DownLinkCounter;
}

void LoRaMacSetMicTest( uint16_t upLinkCounter )
{
    UpLinkCounter = upLinkCounter;
    IsUpLinkCounterFixed = true;
}
