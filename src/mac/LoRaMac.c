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
#include "board.h"
#include "utilities.h"
#include "radio.h"

#include "LoRaMacCrypto.h"
#include "LoRaMac.h"

/*!
 * Enables/Disables acknowledgements retries specific code.
 * \remark At the current release time the http://iot.semtech.com server doesn't
 *         support this feature.
 */
#define ANCKNOWLEDGE_RETRIES_ENABLED                0

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

#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
/*!
 * If the node has sent a FRAME_TYPE_DATA_CONFIRMED this variable indicates
 * if the nodes needs to manage the server acknowledgement.
 */
static bool NodeAckRequested = false;
#endif

/*!
 * If the server has sent a FRAME_TYPE_DATA_CONFIRMED this variable indicates
 * if the ACK bit must be set for the next transmission
 */
static bool SrvAckRequested = false;

/*!
 * Last received packet raw SNR value
 */
static uint8_t RxRawSnr = 0;

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
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_NB_CHANNELS] =
{
    { true,  LC1,   7, 0, 14 },            // G1 LoRa SF=7-12     BW=125 kHz
    { true,  LC2,   7, 0, 14 },            // G1 LoRa SF=7-12     BW=125 kHz
    { true,  LC3,   7, 0, 14 },            // G1 LoRa SF=7-12     BW=125 kHz
    { false, LC4,   7, 0, 14 },            // G2 LoRa SF=7-12     BW=125 kHz
    { false, LC5,   7, 0, 14 },            // G2 LoRa SF=7-12     BW=125 kHz
    { false, LC6,   7, 0, 14 },            // G3 LoRa SF=7-12     BW=125 kHz
    { false, LC7,   7, 1, 14 },            // G1 LoRa SF=7        BW=250 kHz
    { false, FC1, 100, 0, 14 },            // G1 FSK  BR=100 kbps BW=250 kHz
};

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

#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
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
#endif

/*!
 * Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void );

/*!
 * Function to be executed on Radio Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t *size, double rssi, double snr, uint8_t rawSnr );

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

#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
/*!
 * Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void );
#endif

/*!
 * Searches and set the next random available channel
 *
 * \retval status  Function status [0: OK, 1: Unable to find a free channel]
 */
static uint8_t LoRaMacSetNextChannel( void )
{
    uint8_t i = 0;
    uint8_t channelPrev = Channel;
    uint8_t nbEnabledChannels = 0;
    uint8_t enabledChannels[LORA_NB_CHANNELS];

    // Search how many channels are enabled
    for( i = 0; i < LORA_NB_125_CHANNELS; i++ )
    {
        if( Channels[i].Enabled == true )
        {
            enabledChannels[nbEnabledChannels++] = i;
        }
    }
    if( nbEnabledChannels > 1 )
    {
        for( i = enabledChannels[randr( 0, nbEnabledChannels - 1 )]; i < LORA_NB_125_CHANNELS; i++, Channel++ )
        {
            i = i % ( enabledChannels[nbEnabledChannels - 1] + 1 );
            if( channelPrev == i )
            {
                continue;
            }
            if( Radio.IsChannelFree( MODEM_LORA, Channels[i].Frequency, RSSI_FREE_TH ) == true )
            {
                // Free channel found
                Channel = i;
                return 0;
            }
        }
        // No free channel found. Keep previous channel
        return 1;
    }
    else
    {
        Channel = enabledChannels[0];
        if( Radio.IsChannelFree( MODEM_LORA, Channels[Channel].Frequency, RSSI_FREE_TH ) == true )
        {
            // Free channel found
            return 0;
        }
        // No free channel found. Keep previous channel
        return 1;
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
 *                  MOTE_MAC_DEV_STATUS_ANS]
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
            MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
            MacCommandsBuffer[MacCommandsBufferIndex++] = p2;
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
    
    TimerInit( &RxWindowTimer1, OnRxWindow1TimerEvent ); 
    TimerSetValue( &RxWindowTimer1, CLS2_RECEIVE_DELAY1 );
    TimerInit( &RxWindowTimer2, OnRxWindow2TimerEvent ); 
    TimerSetValue( &RxWindowTimer2, CLS2_RECEIVE_DELAY2 );
#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
    TimerInit( &AckTimeoutTimer, OnAckTimeoutTimerEvent ); 
#endif
    
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
    Channel = LORA_NB_CHANNELS;

    Radio.Sleep( );
}

void LoRaMacSetAdrOn( bool enable )
{
    AdrCtrlOn = enable;
}

void LoRaMacInitNwkIds( uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey )
{
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

    macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED;

    return LoRaMacSend( &macHdr, NULL, fPort, fBuffer, fBufferSize );
}

uint8_t LoRaMacSendConfirmedFrame( uint8_t fPort, void *fBuffer, uint16_t fBufferSize, uint8_t retries )
{
    LoRaMacHeader_t macHdr;

#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
    AckTimeoutRetries = retries;
    AckTimeoutRetriesCounter = 0;
#endif
    
    macHdr.Value = 0;

    macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED;

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
    
#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
    NodeAckRequested = false;
#endif
    
    if( fBuffer == NULL )
    {
        fBufferSize = 0;
    }

    LoRaMacBuffer[pktHeaderLen++] = macHdr->Value;

    switch( macHdr->Bits.MType )
    {
        case FRAME_TYPE_JOIN_REQ:            
            LoRaMacBufferPktLen = pktHeaderLen;
        
            LoRaMacMemCpy( LoRaMacAppEui, LoRaMacBuffer + LoRaMacBufferPktLen, 8 );
            LoRaMacBufferPktLen += 8;
            LoRaMacMemCpy( LoRaMacDevEui, LoRaMacBuffer + LoRaMacBufferPktLen, 8 );
            LoRaMacBufferPktLen += 8;

            // TODO: Add DevNonce generation, currently LoRaMacDevNonce = 0
            
            LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevNonce & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen & 0xFF, LoRaMacAppKey, &mic );
            
            LoRaMacBuffer[LoRaMacBufferPktLen++] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 24 ) & 0xFF;

            break;
        case FRAME_TYPE_DATA_CONFIRMED:
#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
            NodeAckRequested = true;
#endif
         case FRAME_TYPE_DATA_UNCONFIRMED:
            if( IsLoRaMacNetworkJoined == false )
            {
                return 2; // No network has been joined yet
            }
            
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
                if( channel.Datarate == 12 )
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
                        if( channel.Datarate < 12 )
                        {
                            channel.Datarate++;
                        }
                        Channels[0].Datarate = channel.Datarate;
                        Channels[1].Datarate = channel.Datarate;
                        Channels[2].Datarate = channel.Datarate;
                        Channels[3].Datarate = channel.Datarate;
                        Channels[4].Datarate = channel.Datarate;
                        Channels[5].Datarate = channel.Datarate;
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
    LoRaMacEventInfo.TxDatarate = channel.Datarate;

    Radio.SetChannel( channel.Frequency );
    Radio.SetTxConfig( MODEM_LORA, channel.Power, 0, channel.Bw, channel.Datarate, 1, 8, false, true, false, 3000000 );

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
    const uint8_t DataRates[]  = { 12, 11, 10,  9,  8,  7,  7, 100 };
    const uint8_t Powers[]     = { 20, 14, 11,  8,  5,  2 };

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
                    uint8_t channel = 0;
                    uint8_t dataRate = 0;
                    uint8_t txPower = 0;
                    uint16_t chMask = 0;
                    
                    dataRate = payload[macIndex++];
                    txPower = dataRate & 0x0F;
                    dataRate = ( dataRate >> 4 ) & 0x0F;
                    chMask = payload[macIndex++];
                    chMask |= payload[macIndex++] << 8;
                    if( dataRate < 6 )
                    {
                        // Handle 125 kHz channels
                        while( channel < LORA_NB_125_CHANNELS )
                        {
                            Channels[channel].Datarate = DataRates[dataRate];
                            Channels[channel].Power = Powers[txPower];
                            channel++;
                        }
                    }
                    else
                    {
                        // TODO: Set the LoRaMac to work on a single channel,
                        //       either LC7 ( SF7, BW = 250 KHz ) or 
                        //              FC1 ( 100 kbps, BW = 250 KHz )
                    }
                    // TODO: Optimize by using a loop.
                    if( chMask == 0 )
                    {
                        // In case no channel is allowed default to LC1,2,3
                        chMask = 0x07;
                    }
                    if( ( chMask & 0x01 ) == 0x01 )
                    {
                        Channels[0].Enabled = true; // LC1
                    }
                    else
                    {
                        Channels[0].Enabled = false; // LC1
                    }
                    if( ( chMask & 0x02 ) == 0x02 )
                    {
                        Channels[1].Enabled = true; // LC2
                        //Channels[6].Enabled = true; // LC7
                        //Channels[7].Enabled = true; // FC1
                    }
                    else
                    {
                        Channels[1].Enabled = false; // LC2
                        Channels[6].Enabled = false; // LC7
                        Channels[7].Enabled = false; // FC1
                    }
                    if( ( chMask & 0x04 ) == 0x04 )
                    {
                        Channels[2].Enabled = true; // LC3
                    }
                    else
                    {
                        Channels[2].Enabled = false; // LC3
                    }
                    if( ( chMask & 0x08 ) == 0x08 )
                    {
                        Channels[3].Enabled = true; // LC4
                    }
                    else
                    {
                        Channels[3].Enabled = false; // LC4
                    }
                    if( ( chMask & 0x10 ) == 0x10 )
                    {
                        Channels[4].Enabled = true; // LC5
                    }
                    else
                    {
                        Channels[4].Enabled = false; // LC5
                    }
                    if( ( chMask & 0x20 ) == 0x20 )
                    {
                        Channels[5].Enabled = true; // LC6
                    }
                    else
                    {
                        Channels[5].Enabled = false; // LC6
                    }
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, RxRawSnr, 0 );
                }
                break;
            case SRV_MAC_DEV_STATUS_REQ:
                AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, BoardMeasureBatterieLevel( ), RxRawSnr );
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
#if defined( USE_DEBUG_PINS )
    GpioWrite( &DbgPin1, 1 );
#endif
    Radio.Sleep( );

    if( IsRxWindowsEnabled == true )
    {
        TimerStart( &RxWindowTimer1 );
        TimerStart( &RxWindowTimer2 );
    }
    
#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
    if( NodeAckRequested == false )
#endif
    {
        IsLoRaMacTransmitting = false;
        LoRaMacEventFlags.Bits.Tx = 1;
        LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;

        if( ( LoRaMacEvents != NULL ) && ( LoRaMacEvents->MacEvent != NULL ) )
        {
            LoRaMacEvents->MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
        }
        LoRaMacEventFlags.Value = 0;

        if( IsUpLinkCounterFixed == false )
        {
            UpLinkCounter++;
        }
    }

    AdrAckCounter++;
#if defined( USE_DEBUG_PINS )
    GpioWrite( &DbgPin1, 0 );
#endif
}

/*!
 * Function to be executed on Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t *size, double rssi, double snr, uint8_t rawSnr )
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

#if defined( USE_DEBUG_PINS )
    GpioWrite( &DbgPin3, 1 );
#endif
    macHdr.Value = payload[pktHeaderLen++];
    
    switch( macHdr.Bits.MType )
    {
        case FRAME_TYPE_JOIN_ACCEPT:
            LoRaMacJoinDecrypt( payload + 1, 16, LoRaMacAppKey, LoRaMacPayload + 1 );

            LoRaMacPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacPayload, 13, LoRaMacAppKey, &mic );
            
            micRx |= LoRaMacPayload[*size - 4];
            micRx |= ( LoRaMacPayload[*size - 3] << 8 );
            micRx |= ( LoRaMacPayload[*size - 2] << 16 );
            micRx |= ( LoRaMacPayload[*size - 1] << 24 );
            
            if( micRx == mic )
            {
                LoRaMacJoinComputeSKeys( LoRaMacAppKey, LoRaMacPayload + 1, LoRaMacDevNonce, LoRaMacNwkSKey, LoRaMacAppSKey );

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
                LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_MAC_ERROR;
            }
            
            if( ( LoRaMacEvents != NULL ) && ( LoRaMacEvents->MacEvent != NULL ) )
            {
                LoRaMacEvents->MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
            }
            LoRaMacEventFlags.Value = 0;
            break;
        case FRAME_TYPE_DATA_CONFIRMED:
        case FRAME_TYPE_DATA_UNCONFIRMED:
            {
                address = payload[pktHeaderLen++]; 
                address |= ( payload[pktHeaderLen++] << 8 );
                address |= ( payload[pktHeaderLen++] << 16 );
                address |= ( payload[pktHeaderLen++] << 24 );

                if( address != LoRaMacDevAddr )
                {
                    // We are not the destination of this frame. Just discard!
#if defined( USE_DEBUG_PINS )
                    GpioWrite( &DbgPin3, 0 );
#endif
                    return;
                }
                
                fCtrl.Value = payload[pktHeaderLen++];
                
                sequenceCounter |= payload[pktHeaderLen++];
                sequenceCounter |= payload[pktHeaderLen++] << 8;

                appPayloadStartIndex = 8 + fCtrl.Bits.OptionsLength;

                micRx |= payload[*size - 4];
                micRx |= ( payload[*size - 3] << 8 );
                micRx |= ( payload[*size - 2] << 16 );
                micRx |= ( payload[*size - 1] << 24 );
                
                sequence = ( int32_t )sequenceCounter - ( int32_t )( DownLinkCounter & 0xFFFF );
                if( sequence < 0 )
                {
                    // sequence reset or roll over happened
                    DownLinkCounter = ( DownLinkCounter & 0xFFFF0000 ) | ( sequenceCounter + ( uint32_t )0x10000 );
                    LoRaMacComputeMic( payload, *size - 4, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                    }
                    else
                    {
                        isMicOk = false;
                        // sequence reset
                        DownLinkCounter = sequenceCounter;
                        LoRaMacComputeMic( payload, *size - 4, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                    }
                }
                else
                {
                    DownLinkCounter = ( DownLinkCounter & 0xFFFF0000 ) | sequenceCounter;
                    LoRaMacComputeMic( payload, *size - 4, LoRaMacNwkSKey, address, DOWN_LINK, DownLinkCounter, &mic );
                }

                if( ( isMicOk == true ) ||
                    ( micRx == mic ) )
                {
                    RxRawSnr = rawSnr;
                    AdrAckCounter = 0;
                    
                    if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED )
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
#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
                        // An acknowledge has been received thus the UpLinkCounter
                        // is incremented.
                        if( IsUpLinkCounterFixed == false )
                        {
                            UpLinkCounter++;
                        }
                        // Stop the AckTimeout timer as no more retransmissions 
                        // are needed.
                        TimerStop( &AckTimeoutTimer );
#endif
                        
                        IsLoRaMacTransmitting = false;
                        LoRaMacEventFlags.Bits.Tx = 1;
                        LoRaMacEventInfo.TxAckReceived = true;
#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
                        LoRaMacEventInfo.TxNbRetries = AckTimeoutRetriesCounter;
#else
                        LoRaMacEventInfo.TxNbRetries = 0;
#endif
                    }
                    if( ( ( *size - 4 ) - appPayloadStartIndex ) > 0 )
                    {
                        port = payload[appPayloadStartIndex++];
                        frameLen = ( *size - 4 ) - appPayloadStartIndex;
                        
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
                }

                LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                if( ( LoRaMacEvents != NULL ) && ( LoRaMacEvents->MacEvent != NULL ) )
                {
                    LoRaMacEvents->MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
                }
                LoRaMacEventFlags.Value = 0;
            }
            break;
        case FRAME_TYPE_PROPRIETARY:
            //Intentional falltrough
        default:
            break;
    }
#if defined( USE_DEBUG_PINS )
    GpioWrite( &DbgPin3, 0 );
#endif
}


/*!
 * Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void )
{
    Radio.Sleep( );
    
    IsLoRaMacTransmitting = false;

    LoRaMacEventFlags.Value = 0;
    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    if( ( LoRaMacEvents != NULL ) && ( LoRaMacEvents->MacEvent != NULL ) )
    {
        LoRaMacEvents->MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
    }
}

/*!
 * Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void )
{
    Radio.Sleep( );
#if defined( USE_DEBUG_PINS )
    GpioWrite( &DbgPin2, 0 );
    GpioWrite( &DbgPin2, 1 );
    GpioWrite( &DbgPin2, 0 );
#endif

    LoRaMacEventFlags.Value = 0;
    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_RX_TIMEOUT;
    if( ( LoRaMacEvents != NULL ) && ( LoRaMacEvents->MacEvent != NULL ) )
    {
        LoRaMacEvents->MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
    }
}

/*!
 * Function executed on Radio Rx Error event
 */
static void OnRadioRxError( void )
{
    Radio.Sleep( );
#if defined( USE_DEBUG_PINS )
    GpioWrite( &DbgPin2, 0 );
    GpioWrite( &DbgPin2, 1 );
    GpioWrite( &DbgPin2, 0 );
#endif

    LoRaMacEventFlags.Value = 0;
    LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_RX_ERROR;
    if( ( LoRaMacEvents != NULL ) && ( LoRaMacEvents->MacEvent != NULL ) )
    {
        LoRaMacEvents->MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
    }
}

/*!
 * Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void )
{
    if( Radio.Status( ) == RF_IDLE )
    {
        /* For low SF, we increase the number of symbol generating a Rx Timeout */
        if( Channels[Channel].Datarate < 9 )
        {
            Radio.SetRxConfig( MODEM_LORA, Channels[Channel].Bw, Channels[Channel].Datarate, 1, 0, 8, 8, false, false, true, false );
        }
        else
        {
            Radio.SetRxConfig( MODEM_LORA, Channels[Channel].Bw, Channels[Channel].Datarate, 1, 0, 8, 5, false, false, true, false );
        }
        Radio.Rx( CLS2_MAX_RX_WINDOW );
    }
}

/*!
 * Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void )
{
#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
    if( NodeAckRequested == true )
    {
        TimerSetValue( &AckTimeoutTimer, ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
        TimerStart( &AckTimeoutTimer );
    }
#endif
    
    if( Radio.Status( ) == RF_IDLE )
    {
        /* For low SF, we increase the number of symbol generating a Rx Timeout */
        if( Channels[Channel].Datarate < 9 )
        {
            Radio.SetRxConfig( MODEM_LORA, Channels[Channel].Bw, Channels[Channel].Datarate, 1, 0, 8, 8, false, false, true, false );
        }
        else
        {
            Radio.SetRxConfig( MODEM_LORA, Channels[Channel].Bw, Channels[Channel].Datarate, 1, 0, 8, 5, false, false, true, false );
        }
        Radio.Rx( CLS2_MAX_RX_WINDOW );
    }
}

#if( ANCKNOWLEDGE_RETRIES_ENABLED == 1 )
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
            uint8_t datarate = Channels[0].Datarate;
            uint8_t channel = 0;
            
            // Handle 125 kHz channels
            while( channel < LORA_NB_125_CHANNELS )
            {
                Channels[channel].Datarate = MIN( datarate + 1, 12 );
                channel++;
            }
        }
        if( LoRaMacSetNextChannel( ) != 0 )
        {
            return;
        }

        // Sends the same frame again
        LoRaMacSendFrameOnChannel( Channels[Channel] );
    }
    else
    {
        IsLoRaMacTransmitting = false;
        LoRaMacEventFlags.Bits.Tx = 1;
        LoRaMacEventInfo.TxAckReceived = false;
        LoRaMacEventInfo.TxNbRetries = AckTimeoutRetriesCounter;
      
        if( IsUpLinkCounterFixed == false )
        {
            UpLinkCounter++;
        }
        
        LoRaMacEventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        if( ( LoRaMacEvents != NULL ) && ( LoRaMacEvents->MacEvent != NULL ) )
        {
            LoRaMacEvents->MacEvent( &LoRaMacEventFlags, &LoRaMacEventInfo );
        }
        LoRaMacEventFlags.Value = 0;
    }
}
#endif

/*!
 * ============================================================================
 * = LoRaMac test functions                                                   =
 * ============================================================================
 */

void LoRaMacTestRxWindowsOn( bool enable )
{
    IsRxWindowsEnabled = enable;
}

void LoRaMacSetMicTest( uint16_t upLinkCounter )
{
    UpLinkCounter = upLinkCounter;
    IsUpLinkCounterFixed = true;
}
