/*!
 * \file      RegionCN470.c
 *
 * \brief     Region implementation for CN470
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
#include "utilities.h"

#include "RegionCommon.h"
#include "RegionCN470.h"
#include "RegionCN470A20.h"
#include "RegionCN470B20.h"
#include "RegionCN470A26.h"
#include "RegionCN470B26.h"

// Definitions
#define CHANNELS_MASK_SIZE              6


#ifndef REGION_CN470_DEFAULT_CHANNEL_PLAN
#define REGION_CN470_DEFAULT_CHANNEL_PLAN CHANNEL_PLAN_20MHZ_TYPE_A
#endif

#ifndef REGION_CN470_DEFAULT_RX_WND_2_FREQ
#define REGION_CN470_DEFAULT_RX_WND_2_FREQ CN470_A20_RX_WND_2_FREQ_ABP
#endif


ChannelParams_t CommonJoinChannels[] = CN470_COMMON_JOIN_CHANNELS;

typedef enum eRegionCN470ChannelPlan
{
    CHANNEL_PLAN_UNKNOWN,
    CHANNEL_PLAN_20MHZ_TYPE_A,
    CHANNEL_PLAN_20MHZ_TYPE_B,
    CHANNEL_PLAN_26MHZ_TYPE_A,
    CHANNEL_PLAN_26MHZ_TYPE_B
}RegionCN470ChannelPlan_t;

/*!
 * Definition of the regional channel plan.
 */
typedef struct sRegionCN470ChannelPlanCtx
{
    /*!
     * Size of the channels mask. Must be smaller
     * or equal than CHANNELS_MASK_SIZE.
     */
    uint8_t ChannelsMaskSize;
    /*!
     * Number of elements in the join accept list.
     */
    uint8_t JoinAcceptListSize;
    /*!
     * Number of available channels for beaconing.
     */
    uint8_t NbBeaconChannels;
    /*!
     * Number of available channels for ping slots.
     */
    uint8_t NbPingSlotChannels;
    /*!
     * \brief Calculation of the beacon frequency.
     *
     * \param [IN] channel The Beacon channel number.
     *
     * \param [IN] joinChannelIndex The join channel index.
     *
     * \param [IN] isPingSlot Set to true, if its a ping slot.
     *
     * \retval Returns the beacon frequency.
     */
    uint32_t ( *GetDownlinkFrequency )( uint8_t channel, uint8_t joinChannelIndex, bool isPingSlot );
    /*!
     * \brief Performs the update of the channelsMask based on the input parameters.
     *
     * \param [IN] joinChannelIndex The join channel index.
     *
     * \retval Returns the offset for the given join channel.
     */
    uint8_t ( *GetBeaconChannelOffset )( uint8_t joinChannelIndex );
    /*!
     * \brief Performs the update of the channelsMask based on the input parameters.
     *
     * \param [IN] channelsMask A pointer to the channels mask.
     *
     * \param [IN] chMaskCntl The value of the chMaskCntl field of the LinkAdrReq.
     *
     * \param [IN] chanMask The value of the chanMask field of the LinkAdrReq.
     *
     * \param [IN] channels A pointer to the available channels.
     *
     * \retval Status of the operation. Return 0x07 if the channels mask is valid.
     */
    uint8_t ( *LinkAdrChMaskUpdate )( uint16_t* channelsMask, uint8_t chMaskCntl,
                                      uint16_t chanMask, ChannelParams_t* channels );
    /*!
     * \brief Verifies if the frequency provided is valid.
     *
     * \param [IN] frequency The frequency to verify.
     *
     * \retval Returns true, if the frequency is valid.
     */
    bool ( *VerifyRfFreq )( uint32_t frequency );
    /*!
     * \brief Initializes all channels, datarates, frequencies and bands.
     *
     * \param [IN] channels A pointer to the available channels.
     */
    void ( *InitializeChannels )( ChannelParams_t* channels );
    /*!
     * \brief Initializes the channels mask and the channels default mask.
     *
     * \param [IN] channelsDefaultMask A pointer to the channels default mask.
     */
    void ( *InitializeChannelsMask )( uint16_t* channelsDefaultMask );
    /*!
     * \brief Computes the frequency for the RX1 window.
     *
     * \param [IN] channel The channel utilized currently.
     *
     * \retval Returns the frequency which shall be used.
     */
    uint32_t ( *GetRx1Frequency )( uint8_t channel );
    /*!
     * \brief Computes the frequency for the RX2 window.
     *
     * \param [IN] joinChannelIndex The join channel index.
     *
     * \param [IN] isOtaaDevice Set to true, if the device is an OTAA device.
     *
     * \retval Returns the frequency which shall be used.
     */
    uint32_t ( *GetRx2Frequency )( uint8_t joinChannelIndex, bool isOtaaDevice );
}RegionCN470ChannelPlanCtx_t;

/*!
 * Region specific context
 */
typedef struct sRegionCN470NvmCtx
{
    /*!
     * Holds the channel plan.
     */
    RegionCN470ChannelPlan_t ChannelPlan;
    /*!
     * Holds the common join channel, if its an OTAA device, otherwise
     * this value is 0.
     */
    uint8_t CommonJoinChannelIndex;
    /*!
     * Identifier which specifies if the device is an OTAA device. Set
     * to true, if its an OTAA device.
     */
    bool IsOtaaDevice;
    /*!
     * LoRaMAC channels
     */
    ChannelParams_t Channels[ CN470_MAX_NB_CHANNELS ];
    /*!
     * LoRaMac bands
     */
    Band_t Bands[ CN470_MAX_NB_BANDS ];
    /*!
     * LoRaMac channels mask
     */
    uint16_t ChannelsMask[ CHANNELS_MASK_SIZE ];
    /*!
     * LoRaMac channels remaining
     */
    uint16_t ChannelsMaskRemaining[CHANNELS_MASK_SIZE];
    /*!
     * LoRaMac channels default mask
     */
    uint16_t ChannelsDefaultMask[ CHANNELS_MASK_SIZE ];
}RegionCN470NvmCtx_t;

/*
 * Non-volatile module context.
 */
static RegionCN470NvmCtx_t NvmCtx;

/*
 * Context for the current channel plan.
 */
static RegionCN470ChannelPlanCtx_t ChannelPlanCtx;

// Static functions
static void ApplyChannelPlanConfig( RegionCN470ChannelPlan_t channelPlan, RegionCN470ChannelPlanCtx_t* ctx )
{
    switch( channelPlan )
    {
        case CHANNEL_PLAN_20MHZ_TYPE_A:
        {
            ctx->ChannelsMaskSize = CN470_A20_CHANNELS_MASK_SIZE;
            ctx->JoinAcceptListSize = CN470_A20_JOIN_ACCEPT_LIST_SIZE;
            ctx->NbBeaconChannels = CN470_A20_BEACON_NB_CHANNELS;
            ctx->NbPingSlotChannels = CN470_A20_PING_SLOT_NB_CHANNELS;
            ctx->GetDownlinkFrequency = RegionCN470A20GetDownlinkFrequency;
            ctx->GetBeaconChannelOffset = RegionCN470A20GetBeaconChannelOffset;
            ctx->LinkAdrChMaskUpdate = RegionCN470A20LinkAdrChMaskUpdate;
            ctx->VerifyRfFreq = RegionCN470A20VerifyRfFreq;
            ctx->InitializeChannels = RegionCN470A20InitializeChannels;
            ctx->InitializeChannelsMask = RegionCN470A20InitializeChannelsMask;
            ctx->GetRx1Frequency = RegionCN470A20GetRx1Frequency;
            ctx->GetRx2Frequency = RegionCN470A20GetRx2Frequency;
            break;
        }
        case CHANNEL_PLAN_20MHZ_TYPE_B:
        {
            ctx->ChannelsMaskSize = CN470_B20_CHANNELS_MASK_SIZE;
            ctx->JoinAcceptListSize = CN470_B20_JOIN_ACCEPT_LIST_SIZE;
            ctx->NbBeaconChannels = CN470_B20_BEACON_NB_CHANNELS;
            ctx->NbPingSlotChannels = CN470_B20_PING_SLOT_NB_CHANNELS;
            ctx->GetDownlinkFrequency = RegionCN470B20GetDownlinkFrequency;
            ctx->GetBeaconChannelOffset = RegionCN470B20GetBeaconChannelOffset;
            ctx->LinkAdrChMaskUpdate = RegionCN470B20LinkAdrChMaskUpdate;
            ctx->VerifyRfFreq = RegionCN470B20VerifyRfFreq;
            ctx->InitializeChannels = RegionCN470B20InitializeChannels;
            ctx->InitializeChannelsMask = RegionCN470B20InitializeChannelsMask;
            ctx->GetRx1Frequency = RegionCN470B20GetRx1Frequency;
            ctx->GetRx2Frequency = RegionCN470B20GetRx2Frequency;
            break;
        }
        case CHANNEL_PLAN_26MHZ_TYPE_A:
        {
            ctx->ChannelsMaskSize = CN470_A26_CHANNELS_MASK_SIZE;
            ctx->JoinAcceptListSize = CN470_A26_JOIN_ACCEPT_LIST_SIZE;
            ctx->NbBeaconChannels = CN470_A26_BEACON_NB_CHANNELS;
            ctx->NbPingSlotChannels = CN470_A26_PING_SLOT_NB_CHANNELS;
            ctx->GetDownlinkFrequency = RegionCN470A26GetDownlinkFrequency;
            ctx->GetBeaconChannelOffset = RegionCN470A26GetBeaconChannelOffset;
            ctx->LinkAdrChMaskUpdate = RegionCN470A26LinkAdrChMaskUpdate;
            ctx->VerifyRfFreq = RegionCN470A26VerifyRfFreq;
            ctx->InitializeChannels = RegionCN470A26InitializeChannels;
            ctx->InitializeChannelsMask = RegionCN470A26InitializeChannelsMask;
            ctx->GetRx1Frequency = RegionCN470A26GetRx1Frequency;
            ctx->GetRx2Frequency = RegionCN470A26GetRx2Frequency;
            break;
        }
        case CHANNEL_PLAN_26MHZ_TYPE_B:
        {
            ctx->ChannelsMaskSize = CN470_B26_CHANNELS_MASK_SIZE;
            ctx->JoinAcceptListSize = CN470_B26_JOIN_ACCEPT_LIST_SIZE;
            ctx->NbBeaconChannels = CN470_B26_BEACON_NB_CHANNELS;
            ctx->NbPingSlotChannels = CN470_B26_PING_SLOT_NB_CHANNELS;
            ctx->GetDownlinkFrequency = RegionCN470B26GetDownlinkFrequency;
            ctx->GetBeaconChannelOffset = RegionCN470B26GetBeaconChannelOffset;
            ctx->LinkAdrChMaskUpdate = RegionCN470B26LinkAdrChMaskUpdate;
            ctx->VerifyRfFreq = RegionCN470B26VerifyRfFreq;
            ctx->InitializeChannels = RegionCN470B26InitializeChannels;
            ctx->InitializeChannelsMask = RegionCN470B26InitializeChannelsMask;
            ctx->GetRx1Frequency = RegionCN470B26GetRx1Frequency;
            ctx->GetRx2Frequency = RegionCN470B26GetRx2Frequency;
            break;
        }
        default:
        {
            // Apply CHANNEL_PLAN_20MHZ_TYPE_A
            ctx->ChannelsMaskSize = CN470_A20_CHANNELS_MASK_SIZE;
            ctx->JoinAcceptListSize = CN470_A20_JOIN_ACCEPT_LIST_SIZE;
            ctx->NbBeaconChannels = CN470_A20_BEACON_NB_CHANNELS;
            ctx->NbPingSlotChannels = CN470_A20_PING_SLOT_NB_CHANNELS;
            ctx->GetDownlinkFrequency = RegionCN470A20GetDownlinkFrequency;
            ctx->GetBeaconChannelOffset = RegionCN470A20GetBeaconChannelOffset;
            ctx->LinkAdrChMaskUpdate = RegionCN470A20LinkAdrChMaskUpdate;
            ctx->VerifyRfFreq = RegionCN470A20VerifyRfFreq;
            ctx->InitializeChannels = RegionCN470A20InitializeChannels;
            ctx->InitializeChannelsMask = RegionCN470A20InitializeChannelsMask;
            ctx->GetRx1Frequency = RegionCN470A20GetRx1Frequency;
            ctx->GetRx2Frequency = RegionCN470A20GetRx2Frequency;
            break;
        }
    }
}

static RegionCN470ChannelPlan_t IdentifyChannelPlan( uint8_t joinChannel )
{
    RegionCN470ChannelPlan_t channelPlan = CHANNEL_PLAN_UNKNOWN;

    if( joinChannel <= 7 )
    {
        channelPlan = CHANNEL_PLAN_20MHZ_TYPE_A;
    }
    else if ( ( joinChannel <= 9 ) && ( joinChannel >= 8 ) )
    {
        channelPlan = CHANNEL_PLAN_20MHZ_TYPE_B;
    }
    else if ( ( joinChannel <= 14 ) && ( joinChannel >= 10 ) )
    {
        channelPlan = CHANNEL_PLAN_26MHZ_TYPE_A;
    }
    else if( ( joinChannel <= 19 ) && ( joinChannel >= 15 ) )
    {
        channelPlan = CHANNEL_PLAN_26MHZ_TYPE_B;
    }
    return channelPlan;
}

static bool VerifyRfFreq( uint32_t frequency )
{
    // Check radio driver support
    if( Radio.CheckRfFrequency( frequency ) == false )
    {
        return false;
    }

    return ChannelPlanCtx.VerifyRfFreq( frequency );
}

static TimerTime_t GetTimeOnAir( int8_t datarate, uint16_t pktLen )
{
    int8_t phyDr = DataratesCN470[datarate];
    uint32_t bandwidth = RegionCommonGetBandwidth( datarate, BandwidthsCN470 );

    return Radio.TimeOnAir( MODEM_LORA, bandwidth, phyDr, 1, 8, false, pktLen, true );
}

PhyParam_t RegionCN470GetPhyParam( GetPhyParams_t* getPhy )
{
    PhyParam_t phyParam = { 0 };

    switch( getPhy->Attribute )
    {
        case PHY_MIN_RX_DR:
        {
            phyParam.Value = CN470_RX_MIN_DATARATE;
            break;
        }
        case PHY_MIN_TX_DR:
        {
            phyParam.Value = CN470_TX_MIN_DATARATE;
            break;
        }
        case PHY_DEF_TX_DR:
        {
            phyParam.Value = CN470_DEFAULT_DATARATE;
            break;
        }
        case PHY_NEXT_LOWER_TX_DR:
        {
            phyParam.Value = RegionCommonGetNextLowerTxDr( getPhy->Datarate, CN470_TX_MIN_DATARATE );
            break;
        }
        case PHY_MAX_TX_POWER:
        {
            phyParam.Value = CN470_MAX_TX_POWER;
            break;
        }
        case PHY_DEF_TX_POWER:
        {
            phyParam.Value = CN470_DEFAULT_TX_POWER;
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
            phyParam.Value = MaxPayloadOfDatarateCN470[getPhy->Datarate];
            break;
        }
        case PHY_DUTY_CYCLE:
        {
            phyParam.Value = CN470_DUTY_CYCLE_ENABLED;
            break;
        }
        case PHY_MAX_RX_WINDOW:
        {
            phyParam.Value = CN470_MAX_RX_WINDOW;
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
            phyParam.Value = REGION_CN470_DEFAULT_RX_WND_2_FREQ;

            if( NvmCtx.ChannelPlan != CHANNEL_PLAN_UNKNOWN )
            {
                phyParam.Value = ChannelPlanCtx.GetRx2Frequency( NvmCtx.CommonJoinChannelIndex, NvmCtx.IsOtaaDevice );
            }
            break;
        }
        case PHY_DEF_RX2_DR:
        {
            phyParam.Value = CN470_RX_WND_2_DR;
            break;
        }
        case PHY_CHANNELS_MASK:
        {
            phyParam.ChannelsMask = NvmCtx.ChannelsMask;
            break;
        }
        case PHY_CHANNELS_DEFAULT_MASK:
        {
            phyParam.ChannelsMask = NvmCtx.ChannelsDefaultMask;
            break;
        }
        case PHY_MAX_NB_CHANNELS:
        {
            phyParam.Value = CN470_MAX_NB_CHANNELS;
            break;
        }
        case PHY_CHANNELS:
        {
            phyParam.Channels = NvmCtx.Channels;
            break;
        }
        case PHY_DEF_UPLINK_DWELL_TIME:
        {
            phyParam.Value = CN470_DEFAULT_UPLINK_DWELL_TIME;
            break;
        }
        case PHY_DEF_DOWNLINK_DWELL_TIME:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_DOWNLINK_DWELL_TIME;
            break;
        }
        case PHY_DEF_MAX_EIRP:
        {
            phyParam.fValue = CN470_DEFAULT_MAX_EIRP;
            break;
        }
        case PHY_DEF_ANTENNA_GAIN:
        {
            phyParam.fValue = CN470_DEFAULT_ANTENNA_GAIN;
            break;
        }
        case PHY_BEACON_CHANNEL_FREQ:
        {
            phyParam.Value = REGION_CN470_DEFAULT_RX_WND_2_FREQ;

            // Implementation depending on the join channel
            if( NvmCtx.ChannelPlan != CHANNEL_PLAN_UNKNOWN )
            {
                phyParam.Value = ChannelPlanCtx.GetDownlinkFrequency( getPhy->Channel,
                                                                      NvmCtx.CommonJoinChannelIndex,
                                                                      false );
            }
            break;
        }
        case PHY_BEACON_FORMAT:
        {
            phyParam.BeaconFormat.BeaconSize = CN470_BEACON_SIZE;
            phyParam.BeaconFormat.Rfu1Size = CN470_RFU1_SIZE;
            phyParam.BeaconFormat.Rfu2Size = CN470_RFU2_SIZE;
            break;
        }
        case PHY_BEACON_CHANNEL_DR:
        {
            phyParam.Value = CN470_BEACON_CHANNEL_DR;
            break;
        }
        case PHY_BEACON_NB_CHANNELS:
        {
            // Implementation depending on the join channel
            if( NvmCtx.ChannelPlan != CHANNEL_PLAN_UNKNOWN )
            {
                phyParam.Value = ChannelPlanCtx.NbBeaconChannels;
            }
            break;
        }
        case PHY_BEACON_CHANNEL_OFFSET:
        {
            // Implementation depending on the join channel
            if( NvmCtx.ChannelPlan != CHANNEL_PLAN_UNKNOWN )
            {
                phyParam.Value = ChannelPlanCtx.GetBeaconChannelOffset( NvmCtx.CommonJoinChannelIndex );
            }
            break;
        }
        case PHY_PING_SLOT_CHANNEL_FREQ:
        {
            phyParam.Value = REGION_CN470_DEFAULT_RX_WND_2_FREQ;

            // Implementation depending on the join channel
            if( NvmCtx.ChannelPlan != CHANNEL_PLAN_UNKNOWN )
            {
                phyParam.Value = ChannelPlanCtx.GetDownlinkFrequency( getPhy->Channel,
                                                                      NvmCtx.CommonJoinChannelIndex,
                                                                      true );
            }
            break;
        }
        case PHY_PING_SLOT_CHANNEL_DR:
        {
            phyParam.Value = CN470_PING_SLOT_CHANNEL_DR;
            break;
        }
        case PHY_PING_SLOT_NB_CHANNELS:
        {
            // Implementation depending on the join channel
            if( NvmCtx.ChannelPlan != CHANNEL_PLAN_UNKNOWN )
            {
                phyParam.Value = ChannelPlanCtx.NbPingSlotChannels;
            }
            break;
        }
        case PHY_SF_FROM_DR:
        {
            phyParam.Value = DataratesCN470[getPhy->Datarate];
            break;
        }
        case PHY_BW_FROM_DR:
        {
            phyParam.Value = RegionCommonGetBandwidth( getPhy->Datarate, BandwidthsCN470 );
            break;
        }
        default:
        {
            break;
        }
    }

    return phyParam;
}

void RegionCN470SetBandTxDone( SetBandTxDoneParams_t* txDone )
{
    RegionCommonSetBandTxDone( &NvmCtx.Bands[NvmCtx.Channels[txDone->Channel].Band],
                               txDone->LastTxAirTime, txDone->Joined, txDone->ElapsedTimeSinceStartUp );
}

void RegionCN470InitDefaults( InitDefaultsParams_t* params )
{
    Band_t bands[CN470_MAX_NB_BANDS] =
    {
        CN470_BAND0
    };

    switch( params->Type )
    {
        case INIT_TYPE_DEFAULTS:
        {
            // Default bands
            memcpy1( ( uint8_t* )NvmCtx.Bands, ( uint8_t* )bands, sizeof( Band_t ) * CN470_MAX_NB_BANDS );

            // Verify that a default channel plan is available
            if( NvmCtx.ChannelPlan == CHANNEL_PLAN_UNKNOWN )
            {
                NvmCtx.ChannelPlan = REGION_CN470_DEFAULT_CHANNEL_PLAN;
                NvmCtx.CommonJoinChannelIndex = 0;
                NvmCtx.IsOtaaDevice = false;
            }

            // Apply the channel plan configuration
            ApplyChannelPlanConfig( NvmCtx.ChannelPlan, &ChannelPlanCtx );

            // Default channels
            ChannelPlanCtx.InitializeChannels( NvmCtx.Channels );

            // Default ChannelsMask
            ChannelPlanCtx.InitializeChannelsMask( NvmCtx.ChannelsDefaultMask );

            // Copy channels default mask
            RegionCommonChanMaskCopy( NvmCtx.ChannelsMask, NvmCtx.ChannelsDefaultMask, CHANNELS_MASK_SIZE );

            // Copy into channels mask remaining
            RegionCommonChanMaskCopy( NvmCtx.ChannelsMaskRemaining, NvmCtx.ChannelsMask, CHANNELS_MASK_SIZE );
            break;
        }
        case INIT_TYPE_RESET_TO_DEFAULT_CHANNELS:
        {
            // Intentional fallthrough
        }
        case INIT_TYPE_ACTIVATE_DEFAULT_CHANNELS:
        {
            // Restore channels default mask
            RegionCommonChanMaskCopy( NvmCtx.ChannelsMask, NvmCtx.ChannelsDefaultMask, CHANNELS_MASK_SIZE );

            for( uint8_t i = 0; i < CHANNELS_MASK_SIZE; i++ )
            { // Copy-And the channels mask
                NvmCtx.ChannelsMaskRemaining[i] &= NvmCtx.ChannelsMask[i];
            }
            break;
        }
        case INIT_TYPE_RESTORE_CTX:
        {
            if( params->NvmCtx != 0 )
            {
                memcpy1( (uint8_t*) &NvmCtx, (uint8_t*) params->NvmCtx, sizeof( NvmCtx ) );
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void* RegionCN470GetNvmCtx( GetNvmCtxParams_t* params )
{
    params->nvmCtxSize = sizeof( RegionCN470NvmCtx_t );
    return &NvmCtx;
}

bool RegionCN470Verify( VerifyParams_t* verify, PhyAttribute_t phyAttribute )
{
    switch( phyAttribute )
    {
        case PHY_FREQUENCY:
        {
            return VerifyRfFreq( verify->Frequency );
        }
        case PHY_TX_DR:
        case PHY_DEF_TX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, CN470_TX_MIN_DATARATE, CN470_TX_MAX_DATARATE );
        }
        case PHY_RX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, CN470_RX_MIN_DATARATE, CN470_RX_MAX_DATARATE );
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
        default:
            return false;
    }
}

void RegionCN470ApplyCFList( ApplyCFListParams_t* applyCFList )
{
    // Setup the channel plan based on the join channel
    NvmCtx.CommonJoinChannelIndex = applyCFList->JoinChannel;
    NvmCtx.IsOtaaDevice = true;
    NvmCtx.ChannelPlan = IdentifyChannelPlan( NvmCtx.CommonJoinChannelIndex );

    if( NvmCtx.ChannelPlan == CHANNEL_PLAN_UNKNOWN )
    {
        // Invalid channel plan, fallback to default
        NvmCtx.ChannelPlan = REGION_CN470_DEFAULT_CHANNEL_PLAN;
    }
    // Apply the configuration for the channel plan
    ApplyChannelPlanConfig( NvmCtx.ChannelPlan, &ChannelPlanCtx );

    // Size of the optional CF list must be 16 byte
    if( applyCFList->Size != 16 )
    {
        return;
    }

    // Last byte CFListType must be 0x01 to indicate the CFList contains a series of ChMask fields
    if( applyCFList->Payload[15] != 0x01 )
    {
        return;
    }

    // ChMask0 - ChMask5 must be set (every ChMask has 16 bit)
    for( uint8_t chMaskItr = 0, cntPayload = 0; chMaskItr < ChannelPlanCtx.JoinAcceptListSize; chMaskItr++, cntPayload+=2 )
    {
        NvmCtx.ChannelsMask[chMaskItr] = (uint16_t) (0x00FF & applyCFList->Payload[cntPayload]);
        NvmCtx.ChannelsMask[chMaskItr] |= (uint16_t) (applyCFList->Payload[cntPayload+1] << 8);

        // Set the channel mask to the remaining
        NvmCtx.ChannelsMaskRemaining[chMaskItr] &= NvmCtx.ChannelsMask[chMaskItr];
    }
}

bool RegionCN470ChanMaskSet( ChanMaskSetParams_t* chanMaskSet )
{
    switch( chanMaskSet->ChannelsMaskType )
    {
        case CHANNELS_MASK:
        {
            RegionCommonChanMaskCopy( NvmCtx.ChannelsMask, chanMaskSet->ChannelsMaskIn, CHANNELS_MASK_SIZE );

            for( uint8_t i = 0; i < CHANNELS_MASK_SIZE; i++ )
            { // Copy-And the channels mask
                NvmCtx.ChannelsMaskRemaining[i] &= NvmCtx.ChannelsMask[i];
            }
            break;
        }
        case CHANNELS_DEFAULT_MASK:
        {
            RegionCommonChanMaskCopy( NvmCtx.ChannelsDefaultMask, chanMaskSet->ChannelsMaskIn, CHANNELS_MASK_SIZE );
            break;
        }
        default:
            return false;
    }
    return true;
}

void RegionCN470ComputeRxWindowParameters( int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RxConfigParams_t *rxConfigParams )
{
    uint32_t tSymbolInUs = 0;

    // Get the datarate, perform a boundary check
    rxConfigParams->Datarate = MIN( datarate, CN470_RX_MAX_DATARATE );
    rxConfigParams->Bandwidth = RegionCommonGetBandwidth( rxConfigParams->Datarate, BandwidthsCN470 );

    tSymbolInUs = RegionCommonComputeSymbolTimeLoRa( DataratesCN470[rxConfigParams->Datarate], BandwidthsCN470[rxConfigParams->Datarate] );

    RegionCommonComputeRxWindowParameters( tSymbolInUs, minRxSymbols, rxError, Radio.GetWakeupTime( ), &rxConfigParams->WindowTimeout, &rxConfigParams->WindowOffset );
}

bool RegionCN470RxConfig( RxConfigParams_t* rxConfig, int8_t* datarate )
{
    int8_t dr = rxConfig->Datarate;
    int8_t phyDr = 0;
    uint32_t frequency = rxConfig->Frequency;

    if( Radio.GetStatus( ) != RF_IDLE )
    {
        return false;
    }

    // The RX configuration depends on whether the device has joined or not.
    if( rxConfig->NetworkActivation != ACTIVATION_TYPE_NONE )
    {
        // Update the downlink frequency in case of RX_SLOT_WIN_1 or RX_SLOT_WIN_2.
        // Keep the frequency for all other cases.
        if( rxConfig->RxSlot == RX_SLOT_WIN_1 )
        {
            // Apply window 1 frequency
            frequency = ChannelPlanCtx.GetRx1Frequency( rxConfig->Channel );
        }
        else if( rxConfig->RxSlot == RX_SLOT_WIN_2 )
        {
            // Apply window 2 frequency
            frequency = ChannelPlanCtx.GetRx2Frequency( NvmCtx.CommonJoinChannelIndex, NvmCtx.IsOtaaDevice );
        }
    }
    else
    {
        // In this case, only RX_SLOT_WIN_1 and RX_SLOT_WIN_2 is possible. There is
        // no need to verify it. The end device is not joined and is an OTAA device.
        frequency = CommonJoinChannels[rxConfig->Channel].Rx1Frequency;
    }

    // Read the physical datarate from the datarates table
    phyDr = DataratesCN470[dr];

    Radio.SetChannel( frequency );

    // Radio configuration
    Radio.SetRxConfig( MODEM_LORA, rxConfig->Bandwidth, phyDr, 1, 0, 8, rxConfig->WindowTimeout, false, 0, false, 0, 0, true, rxConfig->RxContinuous );

    Radio.SetMaxPayloadLength( MODEM_LORA, MaxPayloadOfDatarateCN470[dr] + LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE );

    *datarate = (uint8_t) dr;
    return true;
}

bool RegionCN470TxConfig( TxConfigParams_t* txConfig, int8_t* txPower, TimerTime_t* txTimeOnAir )
{
    RadioModems_t modem;
    int8_t phyDr = DataratesCN470[txConfig->Datarate];
    int8_t txPowerLimited = RegionCommonLimitTxPower( txConfig->TxPower, NvmCtx.Bands[NvmCtx.Channels[txConfig->Channel].Band].TxMaxPower );
    uint32_t bandwidth = RegionCommonGetBandwidth( txConfig->Datarate, BandwidthsCN470 );
    int8_t phyTxPower = 0;

    // Calculate physical TX power
    phyTxPower = RegionCommonComputeTxPower( txPowerLimited, txConfig->MaxEirp, txConfig->AntennaGain );

    // Setup the radio frequency
    Radio.SetChannel( NvmCtx.Channels[txConfig->Channel].Frequency );

    if( txConfig->Datarate == DR_7 )
    { // High Speed FSK channel
        modem = MODEM_FSK;
        Radio.SetTxConfig( modem, phyTxPower, 25000, bandwidth, phyDr * 1000, 0, 5, false, true, 0, 0, false, 4000 );
    }
    else
    {
        modem = MODEM_LORA;
        Radio.SetTxConfig( modem, phyTxPower, 0, bandwidth, phyDr, 1, 8, false, true, 0, 0, false, 4000 );
    }

    // Setup maximum payload length of the radio driver
    Radio.SetMaxPayloadLength( modem, txConfig->PktLen );
    // Update time-on-air
    *txTimeOnAir = GetTimeOnAir( txConfig->Datarate, txConfig->PktLen );

    *txPower = txPowerLimited;

    return true;
}

uint8_t RegionCN470LinkAdrReq( LinkAdrReqParams_t* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed )
{
    uint8_t status = 0x07;
    RegionCommonLinkAdrParams_t linkAdrParams = { 0 };
    uint8_t nextIndex = 0;
    uint8_t bytesProcessed = 0;
    uint16_t channelsMask[CHANNELS_MASK_SIZE] = { 0, 0, 0, 0, 0, 0 };
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    RegionCommonLinkAdrReqVerifyParams_t linkAdrVerifyParams;

    // Initialize local copy of channels mask
    RegionCommonChanMaskCopy( channelsMask, NvmCtx.ChannelsMask, CHANNELS_MASK_SIZE );

    while( bytesProcessed < linkAdrReq->PayloadSize )
    {
        // Get ADR request parameters
        nextIndex = RegionCommonParseLinkAdrReq( &( linkAdrReq->Payload[bytesProcessed] ), &linkAdrParams );

        if( nextIndex == 0 )
            break; // break loop, since no more request has been found

        // Update bytes processed
        bytesProcessed += nextIndex;

        // Update the channel plan
        status = ChannelPlanCtx.LinkAdrChMaskUpdate( channelsMask, linkAdrParams.ChMaskCtrl,
                                                     linkAdrParams.ChMask, NvmCtx.Channels );
    }

    // Make sure at least one channel is active
    if( RegionCommonCountChannels( channelsMask, 0, ChannelPlanCtx.ChannelsMaskSize ) == 0 )
    {
        status &= 0xFE; // Channel mask KO
    }

    // Get the minimum possible datarate
    getPhy.Attribute = PHY_MIN_TX_DR;
    getPhy.UplinkDwellTime = linkAdrReq->UplinkDwellTime;
    phyParam = RegionCN470GetPhyParam( &getPhy );

    linkAdrVerifyParams.Status = status;
    linkAdrVerifyParams.AdrEnabled = linkAdrReq->AdrEnabled;
    linkAdrVerifyParams.Datarate = linkAdrParams.Datarate;
    linkAdrVerifyParams.TxPower = linkAdrParams.TxPower;
    linkAdrVerifyParams.NbRep = linkAdrParams.NbRep;
    linkAdrVerifyParams.CurrentDatarate = linkAdrReq->CurrentDatarate;
    linkAdrVerifyParams.CurrentTxPower = linkAdrReq->CurrentTxPower;
    linkAdrVerifyParams.CurrentNbRep = linkAdrReq->CurrentNbRep;
    linkAdrVerifyParams.NbChannels = CN470_MAX_NB_CHANNELS;
    linkAdrVerifyParams.ChannelsMask = channelsMask;
    linkAdrVerifyParams.MinDatarate = ( int8_t )phyParam.Value;
    linkAdrVerifyParams.MaxDatarate = CN470_TX_MAX_DATARATE;
    linkAdrVerifyParams.Channels = NvmCtx.Channels;
    linkAdrVerifyParams.MinTxPower = CN470_MIN_TX_POWER;
    linkAdrVerifyParams.MaxTxPower = CN470_MAX_TX_POWER;
    linkAdrVerifyParams.Version = linkAdrReq->Version;

    // Verify the parameters and update, if necessary
    status = RegionCommonLinkAdrReqVerifyParams( &linkAdrVerifyParams, &linkAdrParams.Datarate, &linkAdrParams.TxPower, &linkAdrParams.NbRep );

    // Update channelsMask if everything is correct
    if( status == 0x07 )
    {
        // Copy Mask
        RegionCommonChanMaskCopy( NvmCtx.ChannelsMask, channelsMask, CHANNELS_MASK_SIZE );

        NvmCtx.ChannelsMaskRemaining[0] &= NvmCtx.ChannelsMask[0];
        NvmCtx.ChannelsMaskRemaining[1] &= NvmCtx.ChannelsMask[1];
        NvmCtx.ChannelsMaskRemaining[2] &= NvmCtx.ChannelsMask[2];
        NvmCtx.ChannelsMaskRemaining[3] &= NvmCtx.ChannelsMask[3];
        NvmCtx.ChannelsMaskRemaining[4] = NvmCtx.ChannelsMask[4];
        NvmCtx.ChannelsMaskRemaining[5] = NvmCtx.ChannelsMask[5];
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

    // Verify radio frequency
    if( VerifyRfFreq( rxParamSetupReq->Frequency ) == false )
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

int8_t RegionCN470AlternateDr( int8_t currentDr, AlternateDrType_t type )
{
    return currentDr;
}

LoRaMacStatus_t RegionCN470NextChannel( NextChanParams_t* nextChanParams, uint8_t* channel, TimerTime_t* time, TimerTime_t* aggregatedTimeOff )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t nbRestrictedChannels = 0;
    uint8_t enabledChannels[CN470_MAX_NB_CHANNELS] = { 0 };
    uint16_t joinChannelsMask[2] = CN470_JOIN_CHANNELS;
    RegionCommonIdentifyChannelsParam_t identifyChannelsParam;
    RegionCommonCountNbOfEnabledChannelsParams_t countChannelsParams;
    LoRaMacStatus_t status = LORAMAC_STATUS_NO_CHANNEL_FOUND;

    // Count 125kHz channels
    if( RegionCommonCountChannels( NvmCtx.ChannelsMaskRemaining, 0, ChannelPlanCtx.ChannelsMaskSize ) == 0 )
    { // Reactivate default channels
        RegionCommonChanMaskCopy( NvmCtx.ChannelsMaskRemaining, NvmCtx.ChannelsMask, ChannelPlanCtx.ChannelsMaskSize  );
    }

    // Search how many channels are enabled
    countChannelsParams.Joined = nextChanParams->Joined;
    countChannelsParams.Datarate = nextChanParams->Datarate;
    countChannelsParams.ChannelsMask = NvmCtx.ChannelsMaskRemaining;
    countChannelsParams.Channels = NvmCtx.Channels;
    countChannelsParams.Bands = NvmCtx.Bands;
    countChannelsParams.MaxNbChannels = CN470_MAX_NB_CHANNELS;
    countChannelsParams.JoinChannels = NULL;

    // Apply a different channel selection if the device is not joined yet
    // In this case the device shall not follow the individual channel plans for the
    // different type, but instead shall follow the common join channel plan.
    if( countChannelsParams.Joined == false )
    {
        countChannelsParams.ChannelsMask = joinChannelsMask;
        countChannelsParams.Channels = CommonJoinChannels;
        countChannelsParams.MaxNbChannels = CN470_COMMON_JOIN_CHANNELS_SIZE;
        countChannelsParams.JoinChannels = joinChannelsMask;
    }

    identifyChannelsParam.AggrTimeOff = nextChanParams->AggrTimeOff;
    identifyChannelsParam.LastAggrTx = nextChanParams->LastAggrTx;
    identifyChannelsParam.DutyCycleEnabled = nextChanParams->DutyCycleEnabled;
    identifyChannelsParam.MaxBands = CN470_MAX_NB_BANDS;

    identifyChannelsParam.ElapsedTimeSinceStartUp = nextChanParams->ElapsedTimeSinceStartUp;
    identifyChannelsParam.LastTxIsJoinRequest = nextChanParams->LastTxIsJoinRequest;
    identifyChannelsParam.ExpectedTimeOnAir = GetTimeOnAir( nextChanParams->Datarate, nextChanParams->PktLen );

    identifyChannelsParam.CountNbOfEnabledChannelsParam = &countChannelsParams;

    status = RegionCommonIdentifyChannels( &identifyChannelsParam, aggregatedTimeOff, enabledChannels,
                                           &nbEnabledChannels, &nbRestrictedChannels, time );

    if( status == LORAMAC_STATUS_OK )
    {
        // We found a valid channel. Selection is random.
        *channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];

        // Disable the channel in the mask
        RegionCommonChanDisable( NvmCtx.ChannelsMaskRemaining, *channel, ChannelPlanCtx.ChannelsMaskSize );
    }
    return status;
}

LoRaMacStatus_t RegionCN470ChannelAdd( ChannelAddParams_t* channelAdd )
{
    return LORAMAC_STATUS_PARAMETER_INVALID;
}

bool RegionCN470ChannelsRemove( ChannelRemoveParams_t* channelRemove  )
{
    return LORAMAC_STATUS_PARAMETER_INVALID;
}

uint8_t RegionCN470ApplyDrOffset( uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset )
{
    int8_t datarate = DatarateOffsetsCN470[dr][drOffset];

    if( datarate < 0 )
    {
        datarate = DR_0;
    }
    return datarate;
}

void RegionCN470RxBeaconSetup( RxBeaconSetup_t* rxBeaconSetup, uint8_t* outDr )
{
    RegionCommonRxBeaconSetupParams_t regionCommonRxBeaconSetup;

    regionCommonRxBeaconSetup.Datarates = DataratesCN470;
    regionCommonRxBeaconSetup.Frequency = rxBeaconSetup->Frequency;
    regionCommonRxBeaconSetup.BeaconSize = CN470_BEACON_SIZE;
    regionCommonRxBeaconSetup.BeaconDatarate = CN470_BEACON_CHANNEL_DR;
    regionCommonRxBeaconSetup.BeaconChannelBW = CN470_BEACON_CHANNEL_BW;
    regionCommonRxBeaconSetup.RxTime = rxBeaconSetup->RxTime;
    regionCommonRxBeaconSetup.SymbolTimeout = rxBeaconSetup->SymbolTimeout;

    RegionCommonRxBeaconSetup( &regionCommonRxBeaconSetup );

    // Store downlink datarate
    *outDr = CN470_BEACON_CHANNEL_DR;
}
