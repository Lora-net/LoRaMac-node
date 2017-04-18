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

Description: LoRa MAC region CN470 implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jaeckle ( STACKFORCE )
*/
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "board.h"
#include "LoRaMac.h"

#include "utilities.h"

#include "Region.h"
#include "RegionCommon.h"
#include "RegionCN470.h"

// Definitions
#define CHANNELS_MASK_SIZE              6

// Global attributes
/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[CN470_MAX_NB_CHANNELS];

/*!
 * LoRaMac bands
 */
static Band_t Bands[CN470_MAX_NB_BANDS] =
{
    CN470_BAND0
};

/*!
 * LoRaMac channels mask
 */
static uint16_t ChannelsMask[CHANNELS_MASK_SIZE];

/*!
 * LoRaMac channels default mask
 */
static uint16_t ChannelsDefaultMask[CHANNELS_MASK_SIZE];

// Static functions
static uint32_t GetBandwidth( uint32_t drIndex )
{
    switch( BandwidthsCN470[drIndex] )
    {
        default:
        case 125000:
            return 0;
        case 250000:
            return 1;
        case 500000:
            return 2;
    }
}

static int8_t LimitTxPower( int8_t txPower, int8_t maxBandTxPower, int8_t datarate, uint16_t* channelsMask )
{
    int8_t txPowerResult = txPower;

    // Limit tx power to the band max
    txPowerResult =  MAX( txPower, maxBandTxPower );

    return txPowerResult;
}

static uint8_t CountNbOfEnabledChannels( uint8_t datarate, uint16_t* channelsMask, ChannelParams_t* channels, Band_t* bands, uint8_t* enabledChannels, uint8_t* delayTx )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t delayTransmission = 0;

    for( uint8_t i = 0, k = 0; i < CN470_MAX_NB_CHANNELS; i += 16, k++ )
    {
        for( uint8_t j = 0; j < 16; j++ )
        {
            if( ( channelsMask[k] & ( 1 << j ) ) != 0 )
            {
                if( channels[i + j].Frequency == 0 )
                { // Check if the channel is enabled
                    continue;
                }
                if( RegionCommonValueInRange( datarate, channels[i + j].DrRange.Fields.Min,
                                              channels[i + j].DrRange.Fields.Max ) == false )
                { // Check if the current channel selection supports the given datarate
                    continue;
                }
                if( bands[channels[i + j].Band].TimeOff > 0 )
                { // Check if the band is available for transmission
                    delayTransmission++;
                    continue;
                }
                enabledChannels[nbEnabledChannels++] = i + j;
            }
        }
    }

    *delayTx = delayTransmission;
    return nbEnabledChannels;
}

void RegionCN470GetPhyParam( GetPhyParams_t* getPhy )
{
    switch( getPhy->Attribute )
    {
        case PHY_MIN_DR:
        {
            getPhy->Param.Value = CN470_TX_MIN_DATARATE;
            break;
        }
        case PHY_DEF_TX_DR:
        {
            getPhy->Param.Value = CN470_DEFAULT_DATARATE;
            break;
        }
        case PHY_DEF_TX_POWER:
        {
            getPhy->Param.Value = CN470_DEFAULT_TX_POWER;
            break;
        }
        case PHY_MAX_PAYLOAD:
        {
            getPhy->Param.Value = MaxPayloadOfDatarateCN470[getPhy->Datarate];
            break;
        }
        case PHY_MAX_PAYLOAD_REPEATER:
        {
            getPhy->Param.Value = MaxPayloadOfDatarateRepeaterCN470[getPhy->Datarate];
            break;
        }
        case PHY_DUTY_CYCLE:
        {
            getPhy->Param.Value = CN470_DUTY_CYCLE_ENABLED;
            break;
        }
        case PHY_MAX_RX_WINDOW:
        {
            getPhy->Param.Value = CN470_MAX_RX_WINDOW;
            break;
        }
        case PHY_RECEIVE_DELAY1:
        {
            getPhy->Param.Value = CN470_RECEIVE_DELAY1;
            break;
        }
        case PHY_RECEIVE_DELAY2:
        {
            getPhy->Param.Value = CN470_RECEIVE_DELAY2;
            break;
        }
        case PHY_JOIN_ACCEPT_DELAY1:
        {
            getPhy->Param.Value = CN470_JOIN_ACCEPT_DELAY1;
            break;
        }
        case PHY_JOIN_ACCEPT_DELAY2:
        {
            getPhy->Param.Value = CN470_JOIN_ACCEPT_DELAY2;
            break;
        }
        case PHY_MAX_FCNT_GAP:
        {
            getPhy->Param.Value = CN470_MAX_FCNT_GAP;
            break;
        }
        case PHY_ACK_TIMEOUT:
        {
            getPhy->Param.Value = ( CN470_ACKTIMEOUT + randr( -CN470_ACK_TIMEOUT_RND, CN470_ACK_TIMEOUT_RND ) );
            break;
        }
        case PHY_DEF_DR1_OFFSET:
        {
            getPhy->Param.Value = CN470_DEFAULT_RX1_DR_OFFSET;
            break;
        }
        case PHY_DEF_RX2_FREQUENCY:
        {
            getPhy->Param.Value = CN470_RX_WND_2_FREQ;
            break;
        }
        case PHY_DEF_RX2_DR:
        {
            getPhy->Param.Value = CN470_RX_WND_2_DR;
            break;
        }
        case PHY_CHANNELS_MASK:
        {
            getPhy->Param.ChannelsMask = ChannelsMask;
            break;
        }
        case PHY_CHANNELS_DEFAULT_MASK:
        {
            getPhy->Param.ChannelsMask = ChannelsDefaultMask;
            break;
        }
        case PHY_MAX_NB_CHANNELS:
        {
            getPhy->Param.Value = CN470_MAX_NB_CHANNELS;
            break;
        }
        case PHY_CHANNELS:
        {
            getPhy->Param.Channels = Channels;
            break;
        }
        case PHY_DEF_UPLINK_DWELL_TIME:
        case PHY_DEF_DOWNLINK_DWELL_TIME:
        case PHY_DEF_MAX_EIRP:
        {
            getPhy->Param.Value = 0;
            break;
        }
        case PHY_NB_JOIN_TRIALS:
        case PHY_DEF_NB_JOIN_TRIALS:
        {
            getPhy->Param.Value = 48;
            break;
        }
        default:
        {
            return;
        }
    }
}

void RegionCN470SetBandTxDone( SetBandTxDoneParams_t* txDone )
{
    RegionCommonSetBandTxDone( &Bands[Channels[txDone->Channel].Band], txDone->LastTxDoneTime );
}

void RegionCN470InitDefaults( InitType_t type )
{
    switch( type )
    {
        case INIT_TYPE_INIT:
        {
            // Channels
            // 125 kHz channels
            for( uint8_t i = 0; i < CN470_MAX_NB_CHANNELS; i++ )
            {
                Channels[i].Frequency = 470.3e6 + i * 200e3;
                Channels[i].DrRange.Value = ( DR_5 << 4 ) | DR_0;
                Channels[i].Band = 0;
            }

            // Initialize the channels default mask
            ChannelsDefaultMask[0] = 0xFFFF;
            ChannelsDefaultMask[1] = 0xFFFF;
            ChannelsDefaultMask[2] = 0xFFFF;
            ChannelsDefaultMask[3] = 0xFFFF;
            ChannelsDefaultMask[4] = 0xFFFF;
            ChannelsDefaultMask[5] = 0xFFFF;

            // Update the channels mask
            RegionCommonChanMaskCopy( ChannelsMask, ChannelsDefaultMask, 6 );
            break;
        }
        case INIT_TYPE_RESTORE:
        {
            // Restore channels default mask
            RegionCommonChanMaskCopy( ChannelsMask, ChannelsDefaultMask, 6 );
            break;
        }
        default:
        {
            break;
        }
    }
}

bool RegionCN470Verify( VerifyParams_t* verify, PhyAttribute_t phyAttribute )
{
    switch( phyAttribute )
    {
        case PHY_TX_DR:
        {
            return RegionCommonValueInRange( verify->Datarate, CN470_TX_MIN_DATARATE, CN470_TX_MAX_DATARATE );
        }
        case PHY_DEF_TX_DR:
        {
            return RegionCommonValueInRange( verify->Datarate, DR_0, DR_5 );
        }
        case PHY_DEF_TX_POWER:
        case PHY_TX_POWER:
        {
            // Remark: switched min and max!
            return RegionCommonValueInRange( verify->TxPower, CN470_MAX_TX_POWER, CN470_MIN_TX_POWER );
        }
        case PHY_DUTY_CYCLE:
        {
            return CN470_DUTY_CYCLE_ENABLED;
        }
        case PHY_NB_JOIN_TRIALS:
        {
            if( verify->NbJoinTrials < 48 )
            {
                return false;
            }
            break;
        }
        default:
            return false;
    }
    return true;
}

void RegionCN470ApplyCFList( ApplyCFListParams_t* applyCFList )
{
    return;
}

bool RegionCN470ChanMaskSet( ChanMaskSetParams_t* chanMaskSet )
{
    switch( chanMaskSet->ChannelsMaskType )
    {
        case CHANNELS_MASK:
        {
            RegionCommonChanMaskCopy( ChannelsMask, chanMaskSet->ChannelsMaskIn, 6 );
            break;
        }
        case CHANNELS_DEFAULT_MASK:
        {
            RegionCommonChanMaskCopy( ChannelsDefaultMask, chanMaskSet->ChannelsMaskIn, 6 );
            break;
        }
        default:
            return false;
    }
    return true;
}

bool RegionCN470AdrNext( AdrNextParams_t* adrNext, int8_t* drOut, int8_t* txPowOut, uint32_t* adrAckCounter )
{
    bool adrAckReq = false;
    int8_t datarate = adrNext->Datarate;
    int8_t txPower = adrNext->TxPower;

    // Report back the adr ack counter
    *adrAckCounter = adrNext->AdrAckCounter;

    if( adrNext->AdrEnabled == true )
    {
        if( datarate == CN470_TX_MIN_DATARATE )
        {
            *adrAckCounter = 0;
            adrAckReq = false;
        }
        else
        {
            if( adrNext->AdrAckCounter >= CN470_ADR_ACK_LIMIT )
            {
                adrAckReq = true;
                txPower = CN470_MAX_TX_POWER;
            }
            else
            {
                adrAckReq = false;
            }
            if( adrNext->AdrAckCounter >= ( CN470_ADR_ACK_LIMIT + CN470_ADR_ACK_DELAY ) )
            {
                if( ( adrNext->AdrAckCounter % CN470_ADR_ACK_DELAY ) == 1 )
                {
                    if( datarate > CN470_TX_MIN_DATARATE )
                    {
                        datarate--;
                    }

                    if( datarate == CN470_TX_MIN_DATARATE )
                    {
                        if( adrNext->UpdateChanMask == true )
                        {
                            // Re-enable default channels
                            ChannelsMask[0] = 0xFFFF;
                            ChannelsMask[1] = 0xFFFF;
                            ChannelsMask[2] = 0xFFFF;
                            ChannelsMask[3] = 0xFFFF;
                            ChannelsMask[4] = 0xFFFF;
                            ChannelsMask[5] = 0xFFFF;
                        }
                    }
                }
            }
        }
    }

    *drOut = datarate;
    *txPowOut = txPower;
    return adrAckReq;
}

void RegionCN470ComputeRxWindowParameters( int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RxConfigParams_t *rxConfigParams )
{
    double tSymbol = 0.0;

    rxConfigParams->Datarate = datarate;
    rxConfigParams->Bandwidth = GetBandwidth( datarate );

    if( datarate == DR_7 )
    { // FSK
        tSymbol = RegionCommonComputeSymbolTimeFsk( DataratesCN470[datarate] );
    }
    else
    { // LoRa
        tSymbol = RegionCommonComputeSymbolTimeLoRa( DataratesCN470[datarate], BandwidthsCN470[datarate] );
    }

    RegionCommonComputeRxWindowParameters( tSymbol, minRxSymbols, rxError, RADIO_WAKEUP_TIME, &rxConfigParams->WindowTimeout, &rxConfigParams->WindowOffset );
}

bool RegionCN470RxConfig( RxConfigParams_t* rxConfig, int8_t* datarate )
{
    int8_t dr = rxConfig->Datarate;
    uint8_t maxPayload = 0;
    int8_t phyDr = 0;
    uint32_t frequency = rxConfig->Frequency;

    if( Radio.GetStatus( ) != RF_IDLE )
    {
        return false;
    }

    if( rxConfig->Window == 0 )
    {
        // Apply the datarate offset for RX window 1
        dr = RegionCN470ApplyDrOffset( rxConfig->DownlinkDwellTime, dr, rxConfig->DrOffset );
        // Apply window 1 frequency
        frequency = CN470_FIRST_RX1_CHANNEL + ( rxConfig->Channel % 48 ) * CN470_STEPWIDTH_RX1_CHANNEL;
    }

    // Read the physical datarate from the datarates table
    phyDr = DataratesCN470[dr];

    Radio.SetChannel( frequency );

    // Radio configuration
    Radio.SetRxConfig( MODEM_LORA, rxConfig->Bandwidth, phyDr, 1, 0, 8, rxConfig->WindowTimeout, false, 0, false, 0, 0, true, rxConfig->RxContinuous );

    if( rxConfig->RepeaterSupport == true )
    {
        maxPayload = MaxPayloadOfDatarateRepeaterCN470[dr];
    }
    else
    {
        maxPayload = MaxPayloadOfDatarateCN470[dr];
    }
    Radio.SetMaxPayloadLength( MODEM_LORA, maxPayload + LORA_MAC_FRMPAYLOAD_OVERHEAD );

    *datarate = (uint8_t) dr;
    return true;
}

bool RegionCN470TxConfig( TxConfigParams_t* txConfig, int8_t* txPower, TimerTime_t* txTimeOnAir )
{
    int8_t phyDr = DataratesCN470[txConfig->Datarate];
    int8_t txPowerLimited = LimitTxPower( txConfig->TxPower, Bands[Channels[txConfig->Channel].Band].TxMaxPower, txConfig->Datarate, ChannelsMask );
    int8_t phyTxPower = 0;

    phyTxPower = TxPowersCN470[txPowerLimited];

    // Setup the radio frequency
    Radio.SetChannel( Channels[txConfig->Channel].Frequency );

    // Setup maximum payload lenght of the radio driver
    Radio.SetMaxPayloadLength( MODEM_LORA, txConfig->PktLen );
    Radio.SetTxConfig( MODEM_LORA, phyTxPower, 0, 0, phyDr, 1, 8, false, true, 0, 0, false, 3e3 );
    // Get the time-on-air of the next tx frame
    *txTimeOnAir = Radio.TimeOnAir( MODEM_LORA,  txConfig->PktLen );

    *txPower = txConfig->TxPower;
    return true;
}

uint8_t RegionCN470LinkAdrReq( LinkAdrReqParams_t* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed )
{
    uint8_t status = 0x07;
    LinkAdrParams_t linkAdrParams;
    uint8_t nextIndex = 0;
    uint8_t bytesProcessed = 0;
    uint16_t channelsMask[6] = { 0, 0, 0, 0, 0, 0 };

    // Initialize local copy of channels mask
    RegionCommonChanMaskCopy( channelsMask, ChannelsMask, 6 );

    while( bytesProcessed < linkAdrReq->PayloadSize )
    {
        // Get ADR request parameters
        nextIndex = RegionCommonParseLinkAdrReq( &( linkAdrReq->Payload[bytesProcessed] ), &linkAdrParams );

        if( nextIndex == 0 )
            break; // break loop, since no more request has been found

        // Update bytes processed
        bytesProcessed += nextIndex;

        // Revert status, as we only check the last ADR request for the channel mask KO
        status = 0x07;

        if( linkAdrParams.ChMaskCtrl == 6 )
        {
            // Enable all 125 kHz channels
            channelsMask[0] = 0xFFFF;
            channelsMask[1] = 0xFFFF;
            channelsMask[2] = 0xFFFF;
            channelsMask[3] = 0xFFFF;
            channelsMask[4] = 0xFFFF;
            channelsMask[5] = 0xFFFF;
        }
        else if( linkAdrParams.ChMaskCtrl == 7 )
        {
            status &= 0xFE; // Channel mask KO
        }
        else
        {
            for( uint8_t i = 0; i < 16; i++ )
            {
                if( ( ( linkAdrParams.ChMask & ( 1 << i ) ) != 0 ) &&
                    ( Channels[linkAdrParams.ChMaskCtrl * 16 + i].Frequency == 0 ) )
                {// Trying to enable an undefined channel
                    status &= 0xFE; // Channel mask KO
                }
            }
            channelsMask[linkAdrParams.ChMaskCtrl] = linkAdrParams.ChMask;
        }
    }

    // Verify datarate
    if( RegionCommonChanVerifyDr( CN470_MAX_NB_CHANNELS, channelsMask, linkAdrParams.Datarate, CN470_TX_MIN_DATARATE, CN470_TX_MAX_DATARATE, Channels  ) == false )
    {
        status &= 0xFD; // Datarate KO
    }

    // Verify tx power
    if( RegionCommonValueInRange( linkAdrParams.TxPower, CN470_MAX_TX_POWER, CN470_MIN_TX_POWER ) == 0 )
    {
        // Verify if the maximum TX power is exceeded
        if( CN470_MAX_TX_POWER > linkAdrParams.TxPower )
        { // Apply maximum TX power. Accept TX power.
            linkAdrParams.TxPower = CN470_MAX_TX_POWER;
        }
        else
        {
            status &= 0xFB; // TxPower KO
        }
    }

    // Update channelsMask if everything is correct
    if( status == 0x07 )
    {
        if( linkAdrParams.NbRep == 0 )
        { // Value of 0 is not allowed, revert to default.
            linkAdrParams.NbRep = 1;
        }

        // Copy Mask
        RegionCommonChanMaskCopy( ChannelsMask, channelsMask, 6 );
    }

    // Update status variables
    *drOut = linkAdrParams.Datarate;
    *txPowOut = linkAdrParams.TxPower;
    *nbRepOut = linkAdrParams.NbRep;
    *nbBytesParsed = bytesProcessed;

    return status;
}

uint8_t RegionCN470RxParamSetupReq( RxParamSetupReqParams_t* rxParamSetupReq )
{
    uint8_t status = 0x07;
    uint32_t freq = rxParamSetupReq->Frequency;

    // Verify radio frequency
    if( ( Radio.CheckRfFrequency( freq ) == false ) ||
        ( freq < CN470_FIRST_RX1_CHANNEL ) ||
        ( freq > CN470_LAST_RX1_CHANNEL ) ||
        ( ( ( freq - ( uint32_t ) CN470_FIRST_RX1_CHANNEL ) % ( uint32_t ) CN470_STEPWIDTH_RX1_CHANNEL ) != 0 ) )
    {
        status &= 0xFE; // Channel frequency KO
    }

    // Verify datarate
    if( RegionCommonValueInRange( rxParamSetupReq->Datarate, CN470_RX_MIN_DATARATE, CN470_RX_MAX_DATARATE ) == false )
    {
        status &= 0xFD; // Datarate KO
    }

    // Verify datarate offset
    if( RegionCommonValueInRange( rxParamSetupReq->DrOffset, CN470_MIN_RX1_DR_OFFSET, CN470_MAX_RX1_DR_OFFSET ) == false )
    {
        status &= 0xFB; // Rx1DrOffset range KO
    }

    return status;
}

uint8_t RegionCN470NewChannelReq( NewChannelReqParams_t* newChannelReq )
{
    // Datarate and frequency KO
    return 0;
}

int8_t RegionCN470TxParamSetupReq( TxParamSetupReqParams_t* txParamSetupReq )
{
    return -1;
}

uint8_t RegionCN470DlChannelReq( DlChannelReqParams_t* dlChannelReq )
{
    return 0;
}

int8_t RegionCN470AlternateDr( AlternateDrParams_t* alternateDr )
{
    int8_t datarate = 0;

    if( ( alternateDr->NbTrials % 48 ) == 0 )
    {
        datarate = DR_0;
    }
    else if( ( alternateDr->NbTrials % 32 ) == 0 )
    {
        datarate = DR_1;
    }
    else if( ( alternateDr->NbTrials % 24 ) == 0 )
    {
        datarate = DR_2;
    }
    else if( ( alternateDr->NbTrials % 16 ) == 0 )
    {
        datarate = DR_3;
    }
    else if( ( alternateDr->NbTrials % 8 ) == 0 )
    {
        datarate = DR_4;
    }
    else
    {
        datarate = DR_5;
    }
    return datarate;
}

void RegionCN470CalcBackOff( CalcBackOffParams_t* calcBackOff )
{
    uint8_t channel = calcBackOff->Channel;
    uint16_t joinDutyCycle = 0;

    if( calcBackOff->Joined == false )
    {
        // Get the join duty cycle
        joinDutyCycle = RegionCommonGetJoinDc( calcBackOff->ElapsedTime );
        // Apply band time-off.
        Bands[Channels[channel].Band].TimeOff = calcBackOff->TxTimeOnAir * joinDutyCycle - calcBackOff->TxTimeOnAir;
    }
    else
    {
        Bands[Channels[channel].Band].TimeOff = 0;
    }
}

bool RegionCN470NextChannel( NextChanParams_t* nextChanParams, uint8_t* channel, TimerTime_t* time )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t delayTx = 0;
    uint8_t enabledChannels[CN470_MAX_NB_CHANNELS] = { 0 };
    TimerTime_t nextTxDelay = ( TimerTime_t )( -1 );

    // Count 125kHz channels
    if( RegionCommonCountChannels( ChannelsMask, 0, 6 ) == 0 )
    { // Reactivate default channels
        ChannelsMask[0] = 0xFFFF;
        ChannelsMask[1] = 0xFFFF;
        ChannelsMask[2] = 0xFFFF;
        ChannelsMask[3] = 0xFFFF;
        ChannelsMask[4] = 0xFFFF;
        ChannelsMask[5] = 0xFFFF;
    }

    if( nextChanParams->AggrTimeOff <= TimerGetElapsedTime( nextChanParams->LastAggrTx ) )
    {
        // Search how many channels are enabled
        nbEnabledChannels = CountNbOfEnabledChannels( nextChanParams->Datarate,
                                                      ChannelsMask, Channels,
                                                      Bands, enabledChannels, &delayTx );
    }
    else
    {
        delayTx++;
        nextTxDelay = nextChanParams->AggrTimeOff - TimerGetElapsedTime( nextChanParams->LastAggrTx );
    }

    if( nbEnabledChannels > 0 )
    {
        // We found a valid channel
        *channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];

        *time = 0;
        return true;
    }
    else
    {
        if( delayTx > 0 )
        {
            // Delay transmission due to AggregatedTimeOff or to a band time off
            *time = nextTxDelay;
            return true;
        }
        // Datarate not supported by any channel
        *time = 0;
        return false;
    }
}

LoRaMacStatus_t RegionCN470ChannelAdd( ChannelAddParams_t* channelAdd )
{
    return LORAMAC_STATUS_PARAMETER_INVALID;
}

bool RegionCN470ChannelsRemove( ChannelRemoveParams_t* channelRemove  )
{
    return LORAMAC_STATUS_PARAMETER_INVALID;
}

void RegionCN470SetContinuousWave( ContinuousWaveParams_t* continuousWave )
{
    int8_t txPowerLimited = LimitTxPower( continuousWave->TxPower, Bands[Channels[continuousWave->Channel].Band].TxMaxPower, continuousWave->Datarate, ChannelsMask );
    int8_t phyTxPower = 0;
    uint32_t frequency = Channels[continuousWave->Channel].Frequency;

    phyTxPower = TxPowersCN470[txPowerLimited];

    Radio.SetTxContinuousWave( frequency, phyTxPower, continuousWave->Timeout );
}

uint8_t RegionCN470ApplyDrOffset( uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset )
{
    int8_t datarate = dr - drOffset;

    if( datarate < 0 )
    {
        datarate = DR_0;
    }
    return datarate;
}
