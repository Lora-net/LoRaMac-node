/*!
 * \file      RegionKR920.c
 *
 * \brief     Region implementation for KR920
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
 *              (C)2013-2017 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
*/
#include "radio.h"
#include "RegionCommon.h"
#include "RegionKR920.h"

// Definitions
#define CHANNELS_MASK_SIZE                1

/*!
 * Specifies the reception bandwidth to be used while executing the LBT
 * Max channel bandwidth is 200 kHz
 */
#define KR920_LBT_RX_BANDWIDTH            200000

/*!
 * RSSI threshold for a free channel [dBm]
 */
#define KR920_RSSI_FREE_TH                          -65

/*!
 * Specifies the time the node performs a carrier sense
 */
#define KR920_CARRIER_SENSE_TIME                    6

/*
 * Non-volatile module context.
 */
static RegionNvmDataGroup1_t* RegionNvmGroup1;
static RegionNvmDataGroup2_t* RegionNvmGroup2;
static Band_t* RegionBands;

// Static functions
static int8_t GetMaxEIRP( uint32_t freq )
{
    if( freq >= 922100000 )
    {// Limit to 14dBm
        return KR920_DEFAULT_MAX_EIRP_HIGH;
    }
    // Limit to 10dBm
    return KR920_DEFAULT_MAX_EIRP_LOW;
}

static bool VerifyRfFreq( uint32_t freq )
{
    uint32_t tmpFreq = freq;

    // Check radio driver support
    if( Radio.CheckRfFrequency( tmpFreq ) == false )
    {
        return false;
    }

    // Verify if the frequency is valid. The frequency must be in a specified
    // range and can be set to specific values.
    if( ( tmpFreq >= 920900000 ) && ( tmpFreq <=923300000 ) )
    {
        // Range ok, check for specific value
        tmpFreq -= 920900000;
        if( ( tmpFreq % 200000 ) == 0 )
        {
            return true;
        }
    }
    return false;
}

static TimerTime_t GetTimeOnAir( int8_t datarate, uint16_t pktLen )
{
    int8_t phyDr = DataratesKR920[datarate];
    uint32_t bandwidth = RegionCommonGetBandwidth( datarate, BandwidthsKR920 );

    return Radio.TimeOnAir( MODEM_LORA, bandwidth, phyDr, 1, 8, false, pktLen, true );
}

PhyParam_t RegionKR920GetPhyParam( GetPhyParams_t* getPhy )
{
    PhyParam_t phyParam = { 0 };

    switch( getPhy->Attribute )
    {
        case PHY_MIN_RX_DR:
        {
            phyParam.Value = KR920_RX_MIN_DATARATE;
            break;
        }
        case PHY_MIN_TX_DR:
        {
            phyParam.Value = KR920_TX_MIN_DATARATE;
            break;
        }
        case PHY_DEF_TX_DR:
        {
            phyParam.Value = KR920_DEFAULT_DATARATE;
            break;
        }
        case PHY_NEXT_LOWER_TX_DR:
        {
            RegionCommonGetNextLowerTxDrParams_t nextLowerTxDrParams =
            {
                .CurrentDr = getPhy->Datarate,
                .MaxDr = ( int8_t )KR920_TX_MAX_DATARATE,
                .MinDr = ( int8_t )KR920_TX_MIN_DATARATE,
                .NbChannels = KR920_MAX_NB_CHANNELS,
                .ChannelsMask = RegionNvmGroup2->ChannelsMask,
                .Channels = RegionNvmGroup2->Channels,
            };
            phyParam.Value = RegionCommonGetNextLowerTxDr( &nextLowerTxDrParams );
            break;
        }
        case PHY_MAX_TX_POWER:
        {
            phyParam.Value = KR920_MAX_TX_POWER;
            break;
        }
        case PHY_DEF_TX_POWER:
        {
            phyParam.Value = KR920_DEFAULT_TX_POWER;
            break;
        }
        case PHY_DEF_ADR_ACK_LIMIT:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_ADR_ACK_LIMIT;
            break;
        }
        case PHY_DEF_ADR_ACK_DELAY:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_ADR_ACK_DELAY;
            break;
        }
        case PHY_MAX_PAYLOAD:
        {
            phyParam.Value = MaxPayloadOfDatarateKR920[getPhy->Datarate];
            break;
        }
        case PHY_DUTY_CYCLE:
        {
            phyParam.Value = KR920_DUTY_CYCLE_ENABLED;
            break;
        }
        case PHY_MAX_RX_WINDOW:
        {
            phyParam.Value = KR920_MAX_RX_WINDOW;
            break;
        }
        case PHY_RECEIVE_DELAY1:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_RECEIVE_DELAY1;
            break;
        }
        case PHY_RECEIVE_DELAY2:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_RECEIVE_DELAY2;
            break;
        }
        case PHY_JOIN_ACCEPT_DELAY1:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_JOIN_ACCEPT_DELAY1;
            break;
        }
        case PHY_JOIN_ACCEPT_DELAY2:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_JOIN_ACCEPT_DELAY2;
            break;
        }
        case PHY_RETRANSMIT_TIMEOUT:
        {
            phyParam.Value = ( REGION_COMMON_DEFAULT_RETRANSMIT_TIMEOUT + randr( -REGION_COMMON_DEFAULT_RETRANSMIT_TIMEOUT_RND, REGION_COMMON_DEFAULT_RETRANSMIT_TIMEOUT_RND ) );
            break;
        }
        case PHY_DEF_DR1_OFFSET:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_RX1_DR_OFFSET;
            break;
        }
        case PHY_DEF_RX2_FREQUENCY:
        {
            phyParam.Value = KR920_RX_WND_2_FREQ;
            break;
        }
        case PHY_DEF_RX2_DR:
        {
            phyParam.Value = KR920_RX_WND_2_DR;
            break;
        }
        case PHY_CHANNELS_MASK:
        {
            phyParam.ChannelsMask = RegionNvmGroup2->ChannelsMask;
            break;
        }
        case PHY_CHANNELS_DEFAULT_MASK:
        {
            phyParam.ChannelsMask = RegionNvmGroup2->ChannelsDefaultMask;
            break;
        }
        case PHY_MAX_NB_CHANNELS:
        {
            phyParam.Value = KR920_MAX_NB_CHANNELS;
            break;
        }
        case PHY_CHANNELS:
        {
            phyParam.Channels = RegionNvmGroup2->Channels;
            break;
        }
        case PHY_DEF_UPLINK_DWELL_TIME:
        {
            phyParam.Value = KR920_DEFAULT_UPLINK_DWELL_TIME;
            break;
        }
        case PHY_DEF_DOWNLINK_DWELL_TIME:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_DOWNLINK_DWELL_TIME;
            break;
        }
        case PHY_DEF_MAX_EIRP:
        {
            // We set the higher maximum EIRP as default value.
            // The reason for this is, that the frequency may
            // change during a channel selection for the next uplink.
            // The value has to be recalculated in the TX configuration.
            phyParam.fValue = KR920_DEFAULT_MAX_EIRP_HIGH;
            break;
        }
        case PHY_DEF_ANTENNA_GAIN:
        {
            phyParam.fValue = KR920_DEFAULT_ANTENNA_GAIN;
            break;
        }
        case PHY_BEACON_CHANNEL_FREQ:
        {
            phyParam.Value = KR920_BEACON_CHANNEL_FREQ;
            break;
        }
        case PHY_BEACON_FORMAT:
        {
            phyParam.BeaconFormat.BeaconSize = KR920_BEACON_SIZE;
            phyParam.BeaconFormat.Rfu1Size = KR920_RFU1_SIZE;
            phyParam.BeaconFormat.Rfu2Size = KR920_RFU2_SIZE;
            break;
        }
        case PHY_BEACON_CHANNEL_DR:
        {
            phyParam.Value = KR920_BEACON_CHANNEL_DR;
            break;
        }
        case PHY_PING_SLOT_CHANNEL_FREQ:
        {
            phyParam.Value = KR920_PING_SLOT_CHANNEL_FREQ;
            break;
        }
        case PHY_PING_SLOT_CHANNEL_DR:
        {
            phyParam.Value = KR920_PING_SLOT_CHANNEL_DR;
            break;
        }
        case PHY_SF_FROM_DR:
        {
            phyParam.Value = DataratesKR920[getPhy->Datarate];
            break;
        }
        case PHY_BW_FROM_DR:
        {
            phyParam.Value = RegionCommonGetBandwidth( getPhy->Datarate, BandwidthsKR920 );
            break;
        }
        default:
        {
            break;
        }
    }

    return phyParam;
}

void RegionKR920SetBandTxDone( SetBandTxDoneParams_t* txDone )
{
    RegionCommonSetBandTxDone( &RegionBands[RegionNvmGroup2->Channels[txDone->Channel].Band],
                               txDone->LastTxAirTime, txDone->Joined, txDone->ElapsedTimeSinceStartUp );
}

void RegionKR920InitDefaults( InitDefaultsParams_t* params )
{
    Band_t bands[KR920_MAX_NB_BANDS] =
    {
        KR920_BAND0
    };

    switch( params->Type )
    {
        case INIT_TYPE_DEFAULTS:
        {
            if( ( params->NvmGroup1 == NULL ) || ( params->NvmGroup2 == NULL ) )
            {
                return;
            }

            RegionNvmGroup1 = (RegionNvmDataGroup1_t*) params->NvmGroup1;
            RegionNvmGroup2 = (RegionNvmDataGroup2_t*) params->NvmGroup2;
            RegionBands = (Band_t*) params->Bands;

            // Initialize bands
            memcpy1( ( uint8_t* )RegionBands, ( uint8_t* )bands, sizeof( Band_t ) * KR920_MAX_NB_BANDS );

            // Default channels
            RegionNvmGroup2->Channels[0] = ( ChannelParams_t ) KR920_LC1;
            RegionNvmGroup2->Channels[1] = ( ChannelParams_t ) KR920_LC2;
            RegionNvmGroup2->Channels[2] = ( ChannelParams_t ) KR920_LC3;

            // Default ChannelsMask
            RegionNvmGroup2->ChannelsDefaultMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );

            // Update the channels mask
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask, CHANNELS_MASK_SIZE );

            RegionNvmGroup2->RssiFreeThreshold = KR920_RSSI_FREE_TH;
            RegionNvmGroup2->CarrierSenseTime = KR920_CARRIER_SENSE_TIME;
            break;
        }
        case INIT_TYPE_RESET_TO_DEFAULT_CHANNELS:
        {
            // Reset Channels Rx1Frequency to default 0
            RegionNvmGroup2->Channels[0].Rx1Frequency = 0;
            RegionNvmGroup2->Channels[1].Rx1Frequency = 0;
            RegionNvmGroup2->Channels[2].Rx1Frequency = 0;
            // Update the channels mask
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask, CHANNELS_MASK_SIZE );
            break;
        }
        case INIT_TYPE_ACTIVATE_DEFAULT_CHANNELS:
        {
            // Restore channels default mask
            RegionNvmGroup2->ChannelsMask[0] |= RegionNvmGroup2->ChannelsDefaultMask[0];
            break;
        }
        default:
        {
            break;
        }
    }
}

bool RegionKR920Verify( VerifyParams_t* verify, PhyAttribute_t phyAttribute )
{
    switch( phyAttribute )
    {
        case PHY_FREQUENCY:
        {
            return VerifyRfFreq( verify->Frequency );
        }
        case PHY_TX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, KR920_TX_MIN_DATARATE, KR920_TX_MAX_DATARATE );
        }
        case PHY_DEF_TX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, DR_0, DR_5 );
        }
        case PHY_RX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, KR920_RX_MIN_DATARATE, KR920_RX_MAX_DATARATE );
        }
        case PHY_DEF_TX_POWER:
        case PHY_TX_POWER:
        {
            // Remark: switched min and max!
            return RegionCommonValueInRange( verify->TxPower, KR920_MAX_TX_POWER, KR920_MIN_TX_POWER );
        }
        case PHY_DUTY_CYCLE:
        {
            return KR920_DUTY_CYCLE_ENABLED;
        }
        default:
            return false;
    }
}

void RegionKR920ApplyCFList( ApplyCFListParams_t* applyCFList )
{
    ChannelParams_t newChannel;
    ChannelAddParams_t channelAdd;
    ChannelRemoveParams_t channelRemove;

    // Setup default datarate range
    newChannel.DrRange.Value = ( DR_5 << 4 ) | DR_0;

    // Size of the optional CF list
    if( applyCFList->Size != 16 )
    {
        return;
    }

    // Last byte CFListType must be 0 to indicate the CFList contains a list of frequencies
    if( applyCFList->Payload[15] != 0 )
    {
        return;
    }

    // Last byte is RFU, don't take it into account
    for( uint8_t i = 0, chanIdx = KR920_NUMB_DEFAULT_CHANNELS; chanIdx < KR920_MAX_NB_CHANNELS; i+=3, chanIdx++ )
    {
        if( chanIdx < ( KR920_NUMB_CHANNELS_CF_LIST + KR920_NUMB_DEFAULT_CHANNELS ) )
        {
            // Channel frequency
            newChannel.Frequency = (uint32_t) applyCFList->Payload[i];
            newChannel.Frequency |= ( (uint32_t) applyCFList->Payload[i + 1] << 8 );
            newChannel.Frequency |= ( (uint32_t) applyCFList->Payload[i + 2] << 16 );
            newChannel.Frequency *= 100;

            // Initialize alternative frequency to 0
            newChannel.Rx1Frequency = 0;
        }
        else
        {
            newChannel.Frequency = 0;
            newChannel.DrRange.Value = 0;
            newChannel.Rx1Frequency = 0;
        }

        if( newChannel.Frequency != 0 )
        {
            channelAdd.NewChannel = &newChannel;
            channelAdd.ChannelId = chanIdx;

            // Try to add all channels
            RegionKR920ChannelAdd( &channelAdd );
        }
        else
        {
            channelRemove.ChannelId = chanIdx;

            RegionKR920ChannelsRemove( &channelRemove );
        }
    }
}

bool RegionKR920ChanMaskSet( ChanMaskSetParams_t* chanMaskSet )
{
    switch( chanMaskSet->ChannelsMaskType )
    {
        case CHANNELS_MASK:
        {
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, chanMaskSet->ChannelsMaskIn, 1 );
            break;
        }
        case CHANNELS_DEFAULT_MASK:
        {
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsDefaultMask, chanMaskSet->ChannelsMaskIn, 1 );
            break;
        }
        default:
            return false;
    }
    return true;
}

void RegionKR920ComputeRxWindowParameters( int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RxConfigParams_t *rxConfigParams )
{
    uint32_t tSymbolInUs = 0;

    // Get the datarate, perform a boundary check
    rxConfigParams->Datarate = MIN( datarate, KR920_RX_MAX_DATARATE );
    rxConfigParams->Bandwidth = RegionCommonGetBandwidth( rxConfigParams->Datarate, BandwidthsKR920 );

    tSymbolInUs = RegionCommonComputeSymbolTimeLoRa( DataratesKR920[rxConfigParams->Datarate], BandwidthsKR920[rxConfigParams->Datarate] );

    RegionCommonComputeRxWindowParameters( tSymbolInUs, minRxSymbols, rxError, Radio.GetWakeupTime( ), &rxConfigParams->WindowTimeout, &rxConfigParams->WindowOffset );
}

bool RegionKR920RxConfig( RxConfigParams_t* rxConfig, int8_t* datarate )
{
    int8_t dr = rxConfig->Datarate;
    int8_t phyDr = 0;
    uint32_t frequency = rxConfig->Frequency;

    if( Radio.GetStatus( ) != RF_IDLE )
    {
        return false;
    }

    if( rxConfig->RxSlot == RX_SLOT_WIN_1 )
    {
        // Apply window 1 frequency
        frequency = RegionNvmGroup2->Channels[rxConfig->Channel].Frequency;
        // Apply the alternative RX 1 window frequency, if it is available
        if( RegionNvmGroup2->Channels[rxConfig->Channel].Rx1Frequency != 0 )
        {
            frequency = RegionNvmGroup2->Channels[rxConfig->Channel].Rx1Frequency;
        }
    }

    // Read the physical datarate from the datarates table
    phyDr = DataratesKR920[dr];

    Radio.SetChannel( frequency );

    // Radio configuration
    Radio.SetRxConfig( MODEM_LORA, rxConfig->Bandwidth, phyDr, 1, 0, 8, rxConfig->WindowTimeout, false, 0, false, 0, 0, true, rxConfig->RxContinuous );
    Radio.SetMaxPayloadLength( MODEM_LORA, MaxPayloadOfDatarateKR920[dr] + LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE );

    *datarate = (uint8_t) dr;
    return true;
}

bool RegionKR920TxConfig( TxConfigParams_t* txConfig, int8_t* txPower, TimerTime_t* txTimeOnAir )
{
    int8_t phyDr = DataratesKR920[txConfig->Datarate];
    int8_t txPowerLimited = RegionCommonLimitTxPower( txConfig->TxPower, RegionBands[RegionNvmGroup2->Channels[txConfig->Channel].Band].TxMaxPower );
    uint32_t bandwidth = RegionCommonGetBandwidth( txConfig->Datarate, BandwidthsKR920 );
    float maxEIRP = GetMaxEIRP( RegionNvmGroup2->Channels[txConfig->Channel].Frequency );
    int8_t phyTxPower = 0;

    // Take the minimum between the maxEIRP and txConfig->MaxEirp.
    // The value of txConfig->MaxEirp could have changed during runtime, e.g. due to a MAC command.
    maxEIRP = MIN( txConfig->MaxEirp, maxEIRP );

    // Calculate physical TX power
    phyTxPower = RegionCommonComputeTxPower( txPowerLimited, maxEIRP, txConfig->AntennaGain );

    // Setup the radio frequency
    Radio.SetChannel( RegionNvmGroup2->Channels[txConfig->Channel].Frequency );

    Radio.SetTxConfig( MODEM_LORA, phyTxPower, 0, bandwidth, phyDr, 1, 8, false, true, 0, 0, false, 4000 );

    // Setup maximum payload lenght of the radio driver
    Radio.SetMaxPayloadLength( MODEM_LORA, txConfig->PktLen );
    // Update time-on-air
    *txTimeOnAir = GetTimeOnAir( txConfig->Datarate, txConfig->PktLen );

    *txPower = txPowerLimited;
    return true;
}

uint8_t RegionKR920LinkAdrReq( LinkAdrReqParams_t* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed )
{
    uint8_t status = 0x07;
    RegionCommonLinkAdrParams_t linkAdrParams = { 0 };
    uint8_t nextIndex = 0;
    uint8_t bytesProcessed = 0;
    uint16_t chMask = 0;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    RegionCommonLinkAdrReqVerifyParams_t linkAdrVerifyParams;

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

        // Setup temporary channels mask
        chMask = linkAdrParams.ChMask;

        // Verify channels mask
        if( ( linkAdrParams.ChMaskCtrl == 0 ) && ( chMask == 0 ) )
        {
            status &= 0xFE; // Channel mask KO
        }
        else if( ( ( linkAdrParams.ChMaskCtrl >= 1 ) && ( linkAdrParams.ChMaskCtrl <= 5 )) ||
                ( linkAdrParams.ChMaskCtrl >= 7 ) )
        {
            // RFU
            status &= 0xFE; // Channel mask KO
        }
        else
        {
            for( uint8_t i = 0; i < KR920_MAX_NB_CHANNELS; i++ )
            {
                if( linkAdrParams.ChMaskCtrl == 6 )
                {
                    if( RegionNvmGroup2->Channels[i].Frequency != 0 )
                    {
                        chMask |= 1 << i;
                    }
                }
                else
                {
                    if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                        ( RegionNvmGroup2->Channels[i].Frequency == 0 ) )
                    {// Trying to enable an undefined channel
                        status &= 0xFE; // Channel mask KO
                    }
                }
            }
        }
    }

    // Get the minimum possible datarate
    getPhy.Attribute = PHY_MIN_TX_DR;
    getPhy.UplinkDwellTime = linkAdrReq->UplinkDwellTime;
    phyParam = RegionKR920GetPhyParam( &getPhy );

    linkAdrVerifyParams.Status = status;
    linkAdrVerifyParams.AdrEnabled = linkAdrReq->AdrEnabled;
    linkAdrVerifyParams.Datarate = linkAdrParams.Datarate;
    linkAdrVerifyParams.TxPower = linkAdrParams.TxPower;
    linkAdrVerifyParams.NbRep = linkAdrParams.NbRep;
    linkAdrVerifyParams.CurrentDatarate = linkAdrReq->CurrentDatarate;
    linkAdrVerifyParams.CurrentTxPower = linkAdrReq->CurrentTxPower;
    linkAdrVerifyParams.CurrentNbRep = linkAdrReq->CurrentNbRep;
    linkAdrVerifyParams.NbChannels = KR920_MAX_NB_CHANNELS;
    linkAdrVerifyParams.ChannelsMask = &chMask;
    linkAdrVerifyParams.MinDatarate = ( int8_t )phyParam.Value;
    linkAdrVerifyParams.MaxDatarate = KR920_TX_MAX_DATARATE;
    linkAdrVerifyParams.Channels = RegionNvmGroup2->Channels;
    linkAdrVerifyParams.MinTxPower = KR920_MIN_TX_POWER;
    linkAdrVerifyParams.MaxTxPower = KR920_MAX_TX_POWER;
    linkAdrVerifyParams.Version = linkAdrReq->Version;

    // Verify the parameters and update, if necessary
    status = RegionCommonLinkAdrReqVerifyParams( &linkAdrVerifyParams, &linkAdrParams.Datarate, &linkAdrParams.TxPower, &linkAdrParams.NbRep );

    // Update channelsMask if everything is correct
    if( status == 0x07 )
    {
        // Set the channels mask to a default value
        memset1( ( uint8_t* ) RegionNvmGroup2->ChannelsMask, 0, sizeof( RegionNvmGroup2->ChannelsMask ) );
        // Update the channels mask
        RegionNvmGroup2->ChannelsMask[0] = chMask;
    }

    // Update status variables
    *drOut = linkAdrParams.Datarate;
    *txPowOut = linkAdrParams.TxPower;
    *nbRepOut = linkAdrParams.NbRep;
    *nbBytesParsed = bytesProcessed;

    return status;
}

uint8_t RegionKR920RxParamSetupReq( RxParamSetupReqParams_t* rxParamSetupReq )
{
    uint8_t status = 0x07;

    // Verify radio frequency
    if( VerifyRfFreq( rxParamSetupReq->Frequency ) == false )
    {
        status &= 0xFE; // Channel frequency KO
    }

    // Verify datarate
    if( RegionCommonValueInRange( rxParamSetupReq->Datarate, KR920_RX_MIN_DATARATE, KR920_RX_MAX_DATARATE ) == false )
    {
        status &= 0xFD; // Datarate KO
    }

    // Verify datarate offset
    if( RegionCommonValueInRange( rxParamSetupReq->DrOffset, KR920_MIN_RX1_DR_OFFSET, KR920_MAX_RX1_DR_OFFSET ) == false )
    {
        status &= 0xFB; // Rx1DrOffset range KO
    }

    return status;
}

int8_t RegionKR920NewChannelReq( NewChannelReqParams_t* newChannelReq )
{
    uint8_t status = 0x03;
    ChannelAddParams_t channelAdd;
    ChannelRemoveParams_t channelRemove;

    if( newChannelReq->NewChannel->Frequency == 0 )
    {
        channelRemove.ChannelId = newChannelReq->ChannelId;

        // Remove
        if( RegionKR920ChannelsRemove( &channelRemove ) == false )
        {
            status &= 0xFC;
        }
    }
    else
    {
        channelAdd.NewChannel = newChannelReq->NewChannel;
        channelAdd.ChannelId = newChannelReq->ChannelId;

        switch( RegionKR920ChannelAdd( &channelAdd ) )
        {
            case LORAMAC_STATUS_OK:
            {
                break;
            }
            case LORAMAC_STATUS_FREQUENCY_INVALID:
            {
                status &= 0xFE;
                break;
            }
            case LORAMAC_STATUS_DATARATE_INVALID:
            {
                status &= 0xFD;
                break;
            }
            case LORAMAC_STATUS_FREQ_AND_DR_INVALID:
            {
                status &= 0xFC;
                break;
            }
            default:
            {
                status &= 0xFC;
                break;
            }
        }
    }

    return status;
}

int8_t RegionKR920TxParamSetupReq( TxParamSetupReqParams_t* txParamSetupReq )
{
    // Do not accept the request
    return -1;
}

int8_t RegionKR920DlChannelReq( DlChannelReqParams_t* dlChannelReq )
{
    uint8_t status = 0x03;

    if( dlChannelReq->ChannelId >= ( CHANNELS_MASK_SIZE * 16 ) )
    {
        return 0;
    }

    // Verify if the frequency is supported
    if( VerifyRfFreq( dlChannelReq->Rx1Frequency ) == false )
    {
        status &= 0xFE;
    }

    // Verify if an uplink frequency exists
    if( RegionNvmGroup2->Channels[dlChannelReq->ChannelId].Frequency == 0 )
    {
        status &= 0xFD;
    }

    // Apply Rx1 frequency, if the status is OK
    if( status == 0x03 )
    {
        RegionNvmGroup2->Channels[dlChannelReq->ChannelId].Rx1Frequency = dlChannelReq->Rx1Frequency;
    }

    return status;
}

int8_t RegionKR920AlternateDr( int8_t currentDr, AlternateDrType_t type )
{
    return currentDr;
}

LoRaMacStatus_t RegionKR920NextChannel( NextChanParams_t* nextChanParams, uint8_t* channel, TimerTime_t* time, TimerTime_t* aggregatedTimeOff )
{
    uint8_t channelNext = 0;
    uint8_t nbEnabledChannels = 0;
    uint8_t nbRestrictedChannels = 0;
    uint8_t enabledChannels[KR920_MAX_NB_CHANNELS] = { 0 };
    RegionCommonIdentifyChannelsParam_t identifyChannelsParam;
    RegionCommonCountNbOfEnabledChannelsParams_t countChannelsParams;
    LoRaMacStatus_t status = LORAMAC_STATUS_NO_CHANNEL_FOUND;
    uint16_t joinChannels = KR920_JOIN_CHANNELS;

    if( RegionCommonCountChannels( RegionNvmGroup2->ChannelsMask, 0, 1 ) == 0 )
    { // Reactivate default channels
        RegionNvmGroup2->ChannelsMask[0] |= LC( 1 ) + LC( 2 ) + LC( 3 );
    }

    // Search how many channels are enabled
    countChannelsParams.Joined = nextChanParams->Joined;
    countChannelsParams.Datarate = nextChanParams->Datarate;
    countChannelsParams.ChannelsMask = RegionNvmGroup2->ChannelsMask;
    countChannelsParams.Channels = RegionNvmGroup2->Channels;
    countChannelsParams.Bands = RegionBands;
    countChannelsParams.MaxNbChannels = KR920_MAX_NB_CHANNELS;
    countChannelsParams.JoinChannels = &joinChannels;

    identifyChannelsParam.AggrTimeOff = nextChanParams->AggrTimeOff;
    identifyChannelsParam.LastAggrTx = nextChanParams->LastAggrTx;
    identifyChannelsParam.DutyCycleEnabled = nextChanParams->DutyCycleEnabled;
    identifyChannelsParam.MaxBands = KR920_MAX_NB_BANDS;

    identifyChannelsParam.ElapsedTimeSinceStartUp = nextChanParams->ElapsedTimeSinceStartUp;
    identifyChannelsParam.LastTxIsJoinRequest = nextChanParams->LastTxIsJoinRequest;
    identifyChannelsParam.ExpectedTimeOnAir = GetTimeOnAir( nextChanParams->Datarate, nextChanParams->PktLen );

    identifyChannelsParam.CountNbOfEnabledChannelsParam = &countChannelsParams;

    status = RegionCommonIdentifyChannels( &identifyChannelsParam, aggregatedTimeOff, enabledChannels,
                                           &nbEnabledChannels, &nbRestrictedChannels, time );

    if( status == LORAMAC_STATUS_OK )
    {
        for( uint8_t  i = 0, j = randr( 0, nbEnabledChannels - 1 ); i < KR920_MAX_NB_CHANNELS; i++ )
        {
            channelNext = enabledChannels[j];
            j = ( j + 1 ) % nbEnabledChannels;

            // Perform carrier sense for KR920_CARRIER_SENSE_TIME
            // If the channel is free, we can stop the LBT mechanism
            if( Radio.IsChannelFree( RegionNvmGroup2->Channels[channelNext].Frequency, KR920_LBT_RX_BANDWIDTH, RegionNvmGroup2->RssiFreeThreshold, RegionNvmGroup2->CarrierSenseTime ) == true )
            {
                // Free channel found
                *channel = channelNext;
                return LORAMAC_STATUS_OK;
            }
        }
        // Even if one or more channels are available according to the channel plan, no free channel
        // was found during the LBT procedure.
        status = LORAMAC_STATUS_NO_FREE_CHANNEL_FOUND;
    }
    else if( status == LORAMAC_STATUS_NO_CHANNEL_FOUND )
    {
        // Datarate not supported by any channel, restore defaults
        RegionNvmGroup2->ChannelsMask[0] |= LC( 1 ) + LC( 2 ) + LC( 3 );
    }
    return status;
}

LoRaMacStatus_t RegionKR920ChannelAdd( ChannelAddParams_t* channelAdd )
{
    bool drInvalid = false;
    bool freqInvalid = false;
    uint8_t id = channelAdd->ChannelId;

    if( id < KR920_NUMB_DEFAULT_CHANNELS )
    {
        return LORAMAC_STATUS_FREQ_AND_DR_INVALID;
    }

    if( id >= KR920_MAX_NB_CHANNELS )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    // Validate the datarate range
    if( RegionCommonValueInRange( channelAdd->NewChannel->DrRange.Fields.Min, KR920_TX_MIN_DATARATE, KR920_TX_MAX_DATARATE ) == false )
    {
        drInvalid = true;
    }
    if( RegionCommonValueInRange( channelAdd->NewChannel->DrRange.Fields.Max, KR920_TX_MIN_DATARATE, KR920_TX_MAX_DATARATE ) == false )
    {
        drInvalid = true;
    }
    if( channelAdd->NewChannel->DrRange.Fields.Min > channelAdd->NewChannel->DrRange.Fields.Max )
    {
        drInvalid = true;
    }

    // Check frequency
    if( freqInvalid == false )
    {
        if( VerifyRfFreq( channelAdd->NewChannel->Frequency ) == false )
        {
            freqInvalid = true;
        }
    }

    // Check status
    if( ( drInvalid == true ) && ( freqInvalid == true ) )
    {
        return LORAMAC_STATUS_FREQ_AND_DR_INVALID;
    }
    if( drInvalid == true )
    {
        return LORAMAC_STATUS_DATARATE_INVALID;
    }
    if( freqInvalid == true )
    {
        return LORAMAC_STATUS_FREQUENCY_INVALID;
    }

    memcpy1( ( uint8_t* ) &(RegionNvmGroup2->Channels[id]), ( uint8_t* ) channelAdd->NewChannel, sizeof( RegionNvmGroup2->Channels[id] ) );
    RegionNvmGroup2->Channels[id].Band = 0;
    RegionNvmGroup2->ChannelsMask[0] |= ( 1 << id );
    return LORAMAC_STATUS_OK;
}

bool RegionKR920ChannelsRemove( ChannelRemoveParams_t* channelRemove  )
{
    uint8_t id = channelRemove->ChannelId;

    if( id < KR920_NUMB_DEFAULT_CHANNELS )
    {
        return false;
    }

    // Remove the channel from the list of channels
    RegionNvmGroup2->Channels[id] = ( ChannelParams_t ){ 0, 0, { 0 }, 0 };

    return RegionCommonChanDisable( RegionNvmGroup2->ChannelsMask, id, KR920_MAX_NB_CHANNELS );
}

uint8_t RegionKR920ApplyDrOffset( uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset )
{
    int8_t datarate = dr - drOffset;

    if( datarate < 0 )
    {
        datarate = DR_0;
    }
    return datarate;
}

void RegionKR920RxBeaconSetup( RxBeaconSetup_t* rxBeaconSetup, uint8_t* outDr )
{
    RegionCommonRxBeaconSetupParams_t regionCommonRxBeaconSetup;

    regionCommonRxBeaconSetup.Datarates = DataratesKR920;
    regionCommonRxBeaconSetup.Frequency = rxBeaconSetup->Frequency;
    regionCommonRxBeaconSetup.BeaconSize = KR920_BEACON_SIZE;
    regionCommonRxBeaconSetup.BeaconDatarate = KR920_BEACON_CHANNEL_DR;
    regionCommonRxBeaconSetup.BeaconChannelBW = KR920_BEACON_CHANNEL_BW;
    regionCommonRxBeaconSetup.RxTime = rxBeaconSetup->RxTime;
    regionCommonRxBeaconSetup.SymbolTimeout = rxBeaconSetup->SymbolTimeout;

    RegionCommonRxBeaconSetup( &regionCommonRxBeaconSetup );

    // Store downlink datarate
    *outDr = KR920_BEACON_CHANNEL_DR;
}
