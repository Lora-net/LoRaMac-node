/*!
 * \file  RegionUS915.c
 *
 * \brief Region implementation for US915
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright Stackforce 2021. All rights reserved.
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
#include "loramac_radio.h"
#include "RegionCommon.h"
#include "RegionUS915.h"
#include "RegionBaseUS.h"

// Definitions
#define CHANNELS_MASK_SIZE              6

// A mask to select only valid 500KHz channels
#define CHANNELS_MASK_500KHZ_MASK       0x00FF

#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
/*!
 * LoRaWAN LR-FHSS sync word definition
 */
static const uint8_t* lr_fhss_sync_word = ( uint8_t[] ){ 0x2C, 0x0F, 0x79, 0x95 };
#endif

/*
 * Non-volatile module context.
 */
static RegionNvmDataGroup1_t* RegionNvmGroup1;
static RegionNvmDataGroup2_t* RegionNvmGroup2;
static Band_t* RegionBands;

#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
static void lr_fhss_dr_to_cr_bw( uint8_t dr, lr_fhss_v1_cr_t* cr, lr_fhss_v1_bw_t* bw )
{
    switch( dr )
    {
    case DR_5:
        *cr = LR_FHSS_V1_CR_1_3;
        *bw = LR_FHSS_V1_BW_1523438_HZ;
        break;
    case DR_6:
        *cr = LR_FHSS_V1_CR_2_3;
        *bw = LR_FHSS_V1_BW_1523438_HZ;
        break;
    default:
        // Panic
        while( 1 )
            ;
    }
}

static uint8_t lr_fhss_get_header_count( lr_fhss_v1_cr_t cr )
{
    if( cr == LR_FHSS_V1_CR_1_3 )
    {
        return 3;
    }
    else if( cr == LR_FHSS_V1_CR_2_3 )
    {
        return 2;
    }
    else
    {
        // Panic
        while( 1 )
            ;
        return 0;
    }
}

static int8_t GetNextLowerTxDr( RegionCommonGetNextLowerTxDrParams_t *params )
{
    int8_t drLocal = params->CurrentDr;

    if( params->CurrentDr == params->MinDr )
    {
        return params->MinDr;
    }
    else
    {
        do
        {
            if( drLocal == DR_5 )
            { // LR-FHSS min datarate go to minimum LoRa data rate for the region
                drLocal = params->MinDr;
            }
            else
            {
                drLocal = ( drLocal - 1 );
            }
        } while( ( drLocal != params->MinDr ) &&
                 ( RegionCommonChanVerifyDr( params->NbChannels, params->ChannelsMask, drLocal, params->MinDr, params->MaxDr, params->Channels  ) == false ) );

        return drLocal;
    }
}
#endif

static int8_t LimitTxPower( int8_t txPower, int8_t maxBandTxPower, int8_t datarate, uint16_t* channelsMask )
{
    int8_t txPowerResult = txPower;

    // Limit tx power to the band max
    txPowerResult =  RegionCommonLimitTxPower( txPower, maxBandTxPower );

    if( datarate == DR_4 )
    {// Limit tx power to max 26dBm
        txPowerResult = MAX( txPower, TX_POWER_2 );
    }
    else
    {
        if( RegionCommonCountChannels( channelsMask, 0, 4 ) < 50 )
        {// Limit tx power to max 21dBm
            txPowerResult = MAX( txPower, TX_POWER_5 );
        }
    }
    return txPowerResult;
}

static bool VerifyRfFreq( uint32_t freq )
{
    // Rx frequencies
    if( ( freq < US915_FIRST_RX1_CHANNEL ) ||
        ( freq > US915_LAST_RX1_CHANNEL ) ||
        ( ( ( freq - ( uint32_t ) US915_FIRST_RX1_CHANNEL ) % ( uint32_t ) US915_STEPWIDTH_RX1_CHANNEL ) != 0 ) )
    {
        return false;
    }

    // Test for frequency range - take RX and TX freqencies into account
    if( ( freq < 902300000 ) ||  ( freq > 927500000 ) )
    {
        return false;
    }
    return true;
}

static TimerTime_t GetTimeOnAir( int8_t datarate, uint16_t pktLen )
{
    int8_t phyDr = DataratesUS915[datarate];
    uint32_t bandwidth = RegionCommonGetBandwidth( datarate, BandwidthsUS915 );

#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
    if( ( datarate > DR_4 ) && ( datarate < DR_7 ) )
    {  // LR-FHSS
        lr_fhss_v1_cr_t                    lr_fhss_cr;
        lr_fhss_v1_bw_t                    lr_fhss_bw;
        loramac_radio_lr_fhss_time_on_air_params_t params;

        lr_fhss_dr_to_cr_bw( datarate, &lr_fhss_cr, &lr_fhss_bw );

        params.lr_fhss_params.device_offset                  = 0;
        params.lr_fhss_params.lr_fhss_params.sync_word       = lr_fhss_sync_word;
        params.lr_fhss_params.lr_fhss_params.modulation_type = LR_FHSS_V1_MODULATION_TYPE_GMSK_488;
        params.lr_fhss_params.lr_fhss_params.cr              = lr_fhss_cr;
        params.lr_fhss_params.lr_fhss_params.grid            = LR_FHSS_V1_GRID_25391_HZ;
        params.lr_fhss_params.lr_fhss_params.bw              = lr_fhss_bw;
        params.lr_fhss_params.lr_fhss_params.enable_hopping  = true;
        params.lr_fhss_params.lr_fhss_params.header_count    = lr_fhss_get_header_count( lr_fhss_cr );

        return loramac_radio_lr_fhss_get_time_on_air_in_ms( &params );
    }
    else
#endif
    {
        loramac_radio_lora_time_on_air_params_t lora_params = {
            .sf = ( ral_lora_sf_t ) phyDr,
            .bw = ( ral_lora_bw_t ) bandwidth,
            .cr = RAL_LORA_CR_4_5,
            .preamble_len_in_symb = 8,
            .is_pkt_len_fixed = false,
            .pld_len_in_bytes = ( uint8_t ) pktLen,
            .is_crc_on = true,
        };
        return loramac_radio_lora_get_time_on_air_in_ms( &lora_params );
    }
}

PhyParam_t RegionUS915GetPhyParam( GetPhyParams_t* getPhy )
{
    PhyParam_t phyParam = { 0 };

    switch( getPhy->Attribute )
    {
        case PHY_MIN_RX_DR:
        {
            phyParam.Value = US915_RX_MIN_DATARATE;
            break;
        }
        case PHY_MIN_TX_DR:
        {
            phyParam.Value = US915_TX_MIN_DATARATE;
            break;
        }
        case PHY_DEF_TX_DR:
        {
            phyParam.Value = US915_DEFAULT_DATARATE;
            break;
        }
        case PHY_NEXT_LOWER_TX_DR:
        {
            RegionCommonGetNextLowerTxDrParams_t nextLowerTxDrParams =
            {
                .CurrentDr = getPhy->Datarate,
                .MaxDr = ( int8_t )US915_TX_MAX_DATARATE,
                .MinDr = ( int8_t )US915_TX_MIN_DATARATE,
                .NbChannels = US915_MAX_NB_CHANNELS,
                .ChannelsMask = RegionNvmGroup2->ChannelsMask,
                .Channels = RegionNvmGroup2->Channels,
            };
#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
            phyParam.Value = GetNextLowerTxDr( &nextLowerTxDrParams );
#else
            phyParam.Value = RegionCommonGetNextLowerTxDr( &nextLowerTxDrParams );
#endif
            break;
        }
        case PHY_MAX_TX_POWER:
        {
            phyParam.Value = US915_MAX_TX_POWER;
            break;
        }
        case PHY_DEF_TX_POWER:
        {
            phyParam.Value = US915_DEFAULT_TX_POWER;
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
            phyParam.Value = MaxPayloadOfDatarateUS915[getPhy->Datarate];
            break;
        }
        case PHY_DUTY_CYCLE:
        {
            phyParam.Value = US915_DUTY_CYCLE_ENABLED;
            break;
        }
        case PHY_MAX_RX_WINDOW:
        {
            phyParam.Value = US915_MAX_RX_WINDOW;
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
            phyParam.Value = US915_RX_WND_2_FREQ;
            break;
        }
        case PHY_DEF_RX2_DR:
        {
            phyParam.Value = US915_RX_WND_2_DR;
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
            phyParam.Value = US915_MAX_NB_CHANNELS;
            break;
        }
        case PHY_CHANNELS:
        {
            phyParam.Channels = RegionNvmGroup2->Channels;
            break;
        }
        case PHY_DEF_UPLINK_DWELL_TIME:
        {
            phyParam.Value = US915_DEFAULT_UPLINK_DWELL_TIME;
            break;
        }
        case PHY_DEF_DOWNLINK_DWELL_TIME:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_DOWNLINK_DWELL_TIME;
            break;
        }
        case PHY_DEF_MAX_EIRP:
        {
            phyParam.fValue = US915_DEFAULT_MAX_ERP + 2.15f;
            break;
        }
        case PHY_DEF_ANTENNA_GAIN:
        {
            phyParam.fValue = 0;
            break;
        }
        case PHY_BEACON_CHANNEL_FREQ:
        {
            phyParam.Value = RegionBaseUSCalcDownlinkFrequency( getPhy->Channel,
                                                                US915_BEACON_CHANNEL_FREQ,
                                                                US915_BEACON_CHANNEL_STEPWIDTH );
            break;
        }
        case PHY_BEACON_FORMAT:
        {
            phyParam.BeaconFormat.BeaconSize = US915_BEACON_SIZE;
            phyParam.BeaconFormat.Rfu1Size = US915_RFU1_SIZE;
            phyParam.BeaconFormat.Rfu2Size = US915_RFU2_SIZE;
            break;
        }
        case PHY_BEACON_CHANNEL_DR:
        {
            phyParam.Value = US915_BEACON_CHANNEL_DR;
            break;
        }
        case PHY_BEACON_NB_CHANNELS:
        {
            phyParam.Value = US915_BEACON_NB_CHANNELS;
            break;
        }
        case PHY_PING_SLOT_CHANNEL_FREQ:
        {
            phyParam.Value = RegionBaseUSCalcDownlinkFrequency( getPhy->Channel,
                                                                US915_PING_SLOT_CHANNEL_FREQ,
                                                                US915_BEACON_CHANNEL_STEPWIDTH );
            break;
        }
        case PHY_PING_SLOT_CHANNEL_DR:
        {
            phyParam.Value = US915_PING_SLOT_CHANNEL_DR;
            break;
        }
        case PHY_PING_SLOT_NB_CHANNELS:
        {
            phyParam.Value = US915_BEACON_NB_CHANNELS;
            break;
        }
        case PHY_SF_FROM_DR:
        {
            phyParam.Value = DataratesUS915[getPhy->Datarate];
            break;
        }
        case PHY_BW_FROM_DR:
        {
            phyParam.Value = RegionCommonGetBandwidth( getPhy->Datarate, BandwidthsUS915 );
            break;
        }
        default:
        {
            break;
        }
    }

    return phyParam;
}

void RegionUS915SetBandTxDone( SetBandTxDoneParams_t* txDone )
{
    RegionCommonSetBandTxDone( &RegionBands[RegionNvmGroup2->Channels[txDone->Channel].Band],
                               txDone->LastTxAirTime, txDone->Joined, txDone->ElapsedTimeSinceStartUp );
}

void RegionUS915InitDefaults( InitDefaultsParams_t* params )
{
    Band_t bands[US915_MAX_NB_BANDS] =
    {
       US915_BAND0
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

            // Initialize 8 bit channel groups index
            RegionNvmGroup1->JoinChannelGroupsCurrentIndex = 0;

            // Initialize the join trials counter
            RegionNvmGroup1->JoinTrialsCounter = 0;

            // Default bands
            memcpy1( ( uint8_t* )RegionBands, ( uint8_t* )bands, sizeof( Band_t ) * US915_MAX_NB_BANDS );

            // Default channels
            for( uint8_t i = 0; i < US915_MAX_NB_CHANNELS - 8; i++ )
            {
                // 125 kHz channels
                RegionNvmGroup2->Channels[i].Frequency = 902300000 + i * 200000;
                RegionNvmGroup2->Channels[i].DrRange.Value = ( DR_3 << 4 ) | DR_0;
                RegionNvmGroup2->Channels[i].Band = 0;
            }
#if ( LORAMAC_LR_FHSS_IS_ON == 0 )
            for( uint8_t i = US915_MAX_NB_CHANNELS - 8; i < US915_MAX_NB_CHANNELS; i++ )
            {
                // 500 kHz channels
                RegionNvmGroup2->Channels[i].Frequency = 903000000 + ( i - ( US915_MAX_NB_CHANNELS - 8 ) ) * 1600000;
                RegionNvmGroup2->Channels[i].DrRange.Value = ( DR_4 << 4 ) | DR_4;
                RegionNvmGroup2->Channels[i].Band = 0;
            }
#else
            for( uint8_t i = US915_MAX_NB_CHANNELS - 8; i < US915_MAX_NB_CHANNELS; i++ )
            {
                // 500 kHz channels
                RegionNvmGroup2->Channels[i].Frequency = 903000000 + ( i - ( US915_MAX_NB_CHANNELS - 8 ) ) * 1600000;
                RegionNvmGroup2->Channels[i].DrRange.Value = ( DR_6 << 4 ) | DR_4;
                RegionNvmGroup2->Channels[i].Band = 0;
            }
#endif
            // Initialize channels default mask
            RegionNvmGroup2->ChannelsDefaultMask[0] = 0xFFFF;
            RegionNvmGroup2->ChannelsDefaultMask[1] = 0xFFFF;
            RegionNvmGroup2->ChannelsDefaultMask[2] = 0xFFFF;
            RegionNvmGroup2->ChannelsDefaultMask[3] = 0xFFFF;
            RegionNvmGroup2->ChannelsDefaultMask[4] = 0x00FF;
            RegionNvmGroup2->ChannelsDefaultMask[5] = 0x0000;

            // Copy channels default mask
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask, CHANNELS_MASK_SIZE );

            // Copy into channels mask remaining
            RegionCommonChanMaskCopy( RegionNvmGroup1->ChannelsMaskRemaining, RegionNvmGroup2->ChannelsMask, CHANNELS_MASK_SIZE );
            break;
        }
        case INIT_TYPE_RESET_TO_DEFAULT_CHANNELS:
        {
            // Intentional fallthrough
        }
        case INIT_TYPE_ACTIVATE_DEFAULT_CHANNELS:
        {
            // Copy channels default mask
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask, CHANNELS_MASK_SIZE );

            for( uint8_t i = 0; i < CHANNELS_MASK_SIZE; i++ )
            { // Copy-And the channels mask
                RegionNvmGroup1->ChannelsMaskRemaining[i] &= RegionNvmGroup2->ChannelsMask[i];
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

bool RegionUS915Verify( VerifyParams_t* verify, PhyAttribute_t phyAttribute )
{
    switch( phyAttribute )
    {
        case PHY_FREQUENCY:
        {
            return VerifyRfFreq( verify->Frequency );
        }
        case PHY_TX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, US915_TX_MIN_DATARATE, US915_TX_MAX_DATARATE );
        }
        case PHY_DEF_TX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, DR_0, DR_5 );
        }
        case PHY_RX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, US915_RX_MIN_DATARATE, US915_RX_MAX_DATARATE );
        }
        case PHY_DEF_TX_POWER:
        case PHY_TX_POWER:
        {
            // Remark: switched min and max!
            return RegionCommonValueInRange( verify->TxPower, US915_MAX_TX_POWER, US915_MIN_TX_POWER );
        }
        case PHY_DUTY_CYCLE:
        {
            return US915_DUTY_CYCLE_ENABLED;
        }
        default:
            return false;
    }
}

void RegionUS915ApplyCFList( ApplyCFListParams_t* applyCFList )
{
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

    // ChMask0 - ChMask4 must be set (every ChMask has 16 bit)
    for( uint8_t chMaskItr = 0, cntPayload = 0; chMaskItr <= 4; chMaskItr++, cntPayload+=2 )
    {
        RegionNvmGroup2->ChannelsMask[chMaskItr] = (uint16_t) (0x00FF & applyCFList->Payload[cntPayload]);
        RegionNvmGroup2->ChannelsMask[chMaskItr] |= (uint16_t) (applyCFList->Payload[cntPayload+1] << 8);
        if( chMaskItr == 4 )
        {
            RegionNvmGroup2->ChannelsMask[chMaskItr] = RegionNvmGroup2->ChannelsMask[chMaskItr] & CHANNELS_MASK_500KHZ_MASK;
        }
        // Set the channel mask to the remaining
        RegionNvmGroup1->ChannelsMaskRemaining[chMaskItr] &= RegionNvmGroup2->ChannelsMask[chMaskItr];
    }
}

bool RegionUS915ChanMaskSet( ChanMaskSetParams_t* chanMaskSet )
{
    uint8_t nbChannels = RegionCommonCountChannels( chanMaskSet->ChannelsMaskIn, 0, 4 );

    // Check the number of active channels
    if( ( nbChannels < 2 ) &&
        ( nbChannels > 0 ) )
    {
        return false;
    }

    switch( chanMaskSet->ChannelsMaskType )
    {
        case CHANNELS_MASK:
        {
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, chanMaskSet->ChannelsMaskIn, CHANNELS_MASK_SIZE );

            RegionNvmGroup2->ChannelsDefaultMask[4] = RegionNvmGroup2->ChannelsDefaultMask[4] & CHANNELS_MASK_500KHZ_MASK;
            RegionNvmGroup2->ChannelsDefaultMask[5] = 0x0000;

            for( uint8_t i = 0; i < CHANNELS_MASK_SIZE; i++ )
            { // Copy-And the channels mask
                RegionNvmGroup1->ChannelsMaskRemaining[i] &= RegionNvmGroup2->ChannelsMask[i];
            }
            break;
        }
        case CHANNELS_DEFAULT_MASK:
        {
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsDefaultMask, chanMaskSet->ChannelsMaskIn, CHANNELS_MASK_SIZE );
            break;
        }
        default:
            return false;
    }
    return true;
}

void RegionUS915ComputeRxWindowParameters( int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RxConfigParams_t *rxConfigParams )
{
    uint32_t tSymbolInUs = 0;

    // Get the datarate, perform a boundary check
    rxConfigParams->Datarate = MIN( datarate, US915_RX_MAX_DATARATE );
    rxConfigParams->Bandwidth = RegionCommonGetBandwidth( rxConfigParams->Datarate, BandwidthsUS915 );

#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
    if( ( rxConfigParams->Datarate > DR_4 ) && ( rxConfigParams->Datarate < DR_7 ) )
    {  // LR-FHSS  is not supported for downlinks
        // Panic
        while( 1 )
            ;
    }
    else
#endif
    {
        tSymbolInUs = RegionCommonComputeSymbolTimeLoRa( DataratesUS915[rxConfigParams->Datarate], BandwidthsUS915[rxConfigParams->Datarate] );
    }

    RegionCommonComputeRxWindowParameters( tSymbolInUs, minRxSymbols, rxError, loramac_radio_get_wakeup_time_in_ms( ), &rxConfigParams->WindowTimeout, &rxConfigParams->WindowOffset );
}

bool RegionUS915RxConfig( RxConfigParams_t* rxConfig, int8_t* datarate )
{
    int8_t dr = rxConfig->Datarate;
    int8_t phyDr = 0;
    uint32_t frequency = rxConfig->Frequency;

    if( loramac_radio_is_radio_idle( ) != true )
    {
        return false;
    }

#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
    // LR-FHSS is not supported for downlinks
    if( ( dr > DR_4 ) && ( dr < DR_7 ) )
    {
        return false;
    }
#endif

    if( rxConfig->RxSlot == RX_SLOT_WIN_1 )
    {
        // Apply window 1 frequency
        frequency = US915_FIRST_RX1_CHANNEL + ( rxConfig->Channel % 8 ) * US915_STEPWIDTH_RX1_CHANNEL;
    }

    // Read the physical datarate from the datarates table
    phyDr = DataratesUS915[dr];

    // Radio configuration
    loramac_radio_lora_cfg_params_t lora_params = {
        .rf_freq_in_hz = frequency,
        .sf = ( ral_lora_sf_t ) phyDr,
        .bw = ( ral_lora_bw_t ) rxConfig->Bandwidth,
        .cr = RAL_LORA_CR_4_5,
        .preamble_len_in_symb = 8,
        .is_pkt_len_fixed = false,
        .pld_len_in_bytes = ( uint8_t )( MaxPayloadOfDatarateUS915[dr] + LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE ),
        .is_crc_on = false,
        .invert_iq_is_on = true,
        .rx_sync_timeout_in_symb = rxConfig->WindowTimeout,
        .is_rx_continuous = rxConfig->RxContinuous,
    };
    loramac_radio_lora_set_cfg( &lora_params );

    *datarate = (uint8_t) dr;
    return true;
}

bool RegionUS915TxConfig( TxConfigParams_t* txConfig, int8_t* txPower, TimerTime_t* txTimeOnAir )
{
    int8_t phyDr = DataratesUS915[txConfig->Datarate];
    int8_t txPowerLimited = LimitTxPower( txConfig->TxPower, RegionBands[RegionNvmGroup2->Channels[txConfig->Channel].Band].TxMaxPower, txConfig->Datarate, RegionNvmGroup2->ChannelsMask );
    uint32_t bandwidth = RegionCommonGetBandwidth( txConfig->Datarate, BandwidthsUS915 );
    int8_t phyTxPower = 0;

    // Calculate physical TX power
    phyTxPower = RegionCommonComputeTxPower( txPowerLimited, US915_DEFAULT_MAX_ERP, 0 );

    // Radio configuration
#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
    if( ( txConfig->Datarate > DR_4 ) && ( txConfig->Datarate < DR_7 ) )
    {  // LR-FHSS
        uint32_t                           rf_freq_in_hz = RegionNvmGroup2->Channels[txConfig->Channel].Frequency;
        lr_fhss_v1_cr_t                    lr_fhss_cr;
        lr_fhss_v1_bw_t                    lr_fhss_bw;
        loramac_radio_lr_fhss_cfg_params_t cfg_params;

        lr_fhss_dr_to_cr_bw( txConfig->Datarate, &lr_fhss_cr, &lr_fhss_bw );

        cfg_params.tx_rf_pwr_in_dbm                              = phyTxPower;
        cfg_params.lr_fhss_params.center_frequency_in_hz         = rf_freq_in_hz;
        cfg_params.lr_fhss_params.device_offset                  = 0;
        cfg_params.lr_fhss_params.lr_fhss_params.sync_word       = lr_fhss_sync_word;
        cfg_params.lr_fhss_params.lr_fhss_params.modulation_type = LR_FHSS_V1_MODULATION_TYPE_GMSK_488;
        cfg_params.lr_fhss_params.lr_fhss_params.cr              = lr_fhss_cr;
        cfg_params.lr_fhss_params.lr_fhss_params.grid            = LR_FHSS_V1_GRID_25391_HZ;
        cfg_params.lr_fhss_params.lr_fhss_params.bw              = lr_fhss_bw;
        cfg_params.lr_fhss_params.lr_fhss_params.enable_hopping  = true;
        cfg_params.lr_fhss_params.lr_fhss_params.header_count    = lr_fhss_get_header_count( lr_fhss_cr );
        cfg_params.tx_timeout_in_ms                              = 4000;

        loramac_radio_lr_fhss_set_cfg( &cfg_params );
    }
    else
#endif
    {
        loramac_radio_lora_cfg_params_t lora_params = {
            .rf_freq_in_hz = RegionNvmGroup2->Channels[txConfig->Channel].Frequency,
            .tx_rf_pwr_in_dbm = phyTxPower,
            .sf = ( ral_lora_sf_t ) phyDr,
            .bw = ( ral_lora_bw_t ) bandwidth,
            .cr = RAL_LORA_CR_4_5,
            .preamble_len_in_symb = 8,
            .is_pkt_len_fixed = false,
            .pld_len_in_bytes = ( uint8_t ) txConfig->PktLen,
            .is_crc_on = true,
            .invert_iq_is_on = false,
            .tx_timeout_in_ms= 4000,
        };
        loramac_radio_lora_set_cfg( &lora_params );
    }
    // Update time-on-air
    *txTimeOnAir = GetTimeOnAir( txConfig->Datarate, txConfig->PktLen );

    *txPower = txPowerLimited;
    return true;
}

uint8_t RegionUS915LinkAdrReq( LinkAdrReqParams_t* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed )
{
    uint8_t status = 0x07;
    RegionCommonLinkAdrParams_t linkAdrParams = { 0 };
    uint8_t nextIndex = 0;
    uint8_t bytesProcessed = 0;
    uint16_t channelsMask[6] = { 0, 0, 0, 0, 0, 0 };
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    RegionCommonLinkAdrReqVerifyParams_t linkAdrVerifyParams;

    // Initialize local copy of channels mask
    RegionCommonChanMaskCopy( channelsMask, RegionNvmGroup2->ChannelsMask, CHANNELS_MASK_SIZE );

    while( bytesProcessed < linkAdrReq->PayloadSize )
    {
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
            // Apply chMask to channels 64 to 71
            channelsMask[4] = linkAdrParams.ChMask & CHANNELS_MASK_500KHZ_MASK;
        }
        else if( linkAdrParams.ChMaskCtrl == 7 )
        {
            // Disable all 125 kHz channels
            channelsMask[0] = 0x0000;
            channelsMask[1] = 0x0000;
            channelsMask[2] = 0x0000;
            channelsMask[3] = 0x0000;
            // Apply chMask to channels 64 to 71
            channelsMask[4] = linkAdrParams.ChMask & CHANNELS_MASK_500KHZ_MASK;
        }
        else if( linkAdrParams.ChMaskCtrl == 5 )
        {
            // Start value for comparision
            uint8_t bitMask = 1;

            // cntChannelMask for channelsMask[0] until channelsMask[3]
            uint8_t cntChannelMask = 0;

            // i will be 1, 2, 3, ..., 7
            for( uint8_t i = 0; i <= 7; i++ )
            {
                // 8 MSBs of ChMask are RFU
                // Checking if the ChMask is set, then true
                if( ( ( linkAdrParams.ChMask & 0x00FF ) & ( bitMask << i ) ) != 0 )
                {
                    if( ( i % 2 ) == 0 )
                    {
                        // Enable a bank of 8 125kHz channels, 8 LSBs
                        channelsMask[cntChannelMask] |= 0x00FF;
                        // Enable the corresponding 500kHz channel
                        channelsMask[4] |= ( bitMask << i );
                    }
                    else
                    {
                        // Enable a bank of 8 125kHz channels, 8 MSBs
                        channelsMask[cntChannelMask] |= 0xFF00;
                        // Enable the corresponding 500kHz channel
                        channelsMask[4] |= ( bitMask << i );
                        // cntChannelMask increment for uneven i
                        cntChannelMask++;
                    }
                }
                // ChMask is not set
                else
                {
                    if( ( i % 2 ) == 0 )
                    {
                        // Disable a bank of 8 125kHz channels, 8 LSBs
                        channelsMask[cntChannelMask] &= 0xFF00;
                        // Disable the corresponding 500kHz channel
                        channelsMask[4] &= ~( bitMask << i );
                    }
                    else
                    {
                        // Enable a bank of 8 125kHz channels, 8 MSBs
                        channelsMask[cntChannelMask] &= 0x00FF;
                        // Disable the corresponding 500kHz channel
                        channelsMask[4] &= ~( bitMask << i );
                        // cntChannelMask increment for uneven i
                        cntChannelMask++;
                    }
                }
            }
        }
        else
        {
            channelsMask[linkAdrParams.ChMaskCtrl] = linkAdrParams.ChMask;
        }
    }

    // FCC 15.247 paragraph F mandates to hop on at least 2 125 kHz channels
    if( ( linkAdrParams.Datarate < DR_4 ) && ( RegionCommonCountChannels( channelsMask, 0, 4 ) < 2 ) )
    {
        status &= 0xFE; // Channel mask KO
    }

    // Get the minimum possible datarate
    getPhy.Attribute = PHY_MIN_TX_DR;
    getPhy.UplinkDwellTime = linkAdrReq->UplinkDwellTime;
    phyParam = RegionUS915GetPhyParam( &getPhy );

    linkAdrVerifyParams.Status = status;
    linkAdrVerifyParams.AdrEnabled = linkAdrReq->AdrEnabled;
    linkAdrVerifyParams.Datarate = linkAdrParams.Datarate;
    linkAdrVerifyParams.TxPower = linkAdrParams.TxPower;
    linkAdrVerifyParams.NbRep = linkAdrParams.NbRep;
    linkAdrVerifyParams.CurrentDatarate = linkAdrReq->CurrentDatarate;
    linkAdrVerifyParams.CurrentTxPower = linkAdrReq->CurrentTxPower;
    linkAdrVerifyParams.CurrentNbRep = linkAdrReq->CurrentNbRep;
    linkAdrVerifyParams.NbChannels = US915_MAX_NB_CHANNELS;
    linkAdrVerifyParams.ChannelsMask = channelsMask;
    linkAdrVerifyParams.MinDatarate = ( int8_t )phyParam.Value;
    linkAdrVerifyParams.MaxDatarate = US915_TX_MAX_DATARATE;
    linkAdrVerifyParams.Channels = RegionNvmGroup2->Channels;
    linkAdrVerifyParams.MinTxPower = US915_MIN_TX_POWER;
    linkAdrVerifyParams.MaxTxPower = US915_MAX_TX_POWER;
    linkAdrVerifyParams.Version = linkAdrReq->Version;

    // Verify the parameters and update, if necessary
    status = RegionCommonLinkAdrReqVerifyParams( &linkAdrVerifyParams, &linkAdrParams.Datarate, &linkAdrParams.TxPower, &linkAdrParams.NbRep );

    // Update channelsMask if everything is correct
    if( status == 0x07 )
    {
        // Copy Mask
        RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, channelsMask, 6 );

        RegionNvmGroup1->ChannelsMaskRemaining[0] &= RegionNvmGroup2->ChannelsMask[0];
        RegionNvmGroup1->ChannelsMaskRemaining[1] &= RegionNvmGroup2->ChannelsMask[1];
        RegionNvmGroup1->ChannelsMaskRemaining[2] &= RegionNvmGroup2->ChannelsMask[2];
        RegionNvmGroup1->ChannelsMaskRemaining[3] &= RegionNvmGroup2->ChannelsMask[3];
        RegionNvmGroup1->ChannelsMaskRemaining[4] = RegionNvmGroup2->ChannelsMask[4];
        RegionNvmGroup1->ChannelsMaskRemaining[5] = RegionNvmGroup2->ChannelsMask[5];
    }

    // Update status variables
    *drOut = linkAdrParams.Datarate;
    *txPowOut = linkAdrParams.TxPower;
    *nbRepOut = linkAdrParams.NbRep;
    *nbBytesParsed = bytesProcessed;

    return status;
}

uint8_t RegionUS915RxParamSetupReq( RxParamSetupReqParams_t* rxParamSetupReq )
{
    uint8_t status = 0x07;

    // Verify radio frequency
    if( VerifyRfFreq( rxParamSetupReq->Frequency ) == false )
    {
        status &= 0xFE; // Channel frequency KO
    }

    // Verify datarate
    if( RegionCommonValueInRange( rxParamSetupReq->Datarate, US915_RX_MIN_DATARATE, US915_RX_MAX_DATARATE ) == false )
    {
        status &= 0xFD; // Datarate KO
    }
#if ( LORAMAC_LR_FHSS_IS_ON == 0 )
    if( ( RegionCommonValueInRange( rxParamSetupReq->Datarate, DR_5, DR_7 ) == true ) ||
        ( rxParamSetupReq->Datarate > DR_13 ) )
#else
    if( ( rxParamSetupReq->Datarate == DR_7 ) ||
        ( rxParamSetupReq->Datarate > DR_13 ) )
#endif
    {
        status &= 0xFD; // Datarate KO
    }

    // Verify datarate offset
    if( RegionCommonValueInRange( rxParamSetupReq->DrOffset, US915_MIN_RX1_DR_OFFSET, US915_MAX_RX1_DR_OFFSET ) == false )
    {
        status &= 0xFB; // Rx1DrOffset range KO
    }

    return status;
}

int8_t RegionUS915NewChannelReq( NewChannelReqParams_t* newChannelReq )
{
    // Do not accept the request
    return -1;
}

int8_t RegionUS915TxParamSetupReq( TxParamSetupReqParams_t* txParamSetupReq )
{
    // Do not accept the request
    return -1;
}

int8_t RegionUS915DlChannelReq( DlChannelReqParams_t* dlChannelReq )
{
    // Do not accept the request
    return -1;
}

int8_t RegionUS915AlternateDr( int8_t currentDr, AlternateDrType_t type )
{
    // Alternates the data rate according to the channel sequence:
    // Eight times a 125kHz DR_0 and then one 500kHz DR_4 channel
    if( type == ALTERNATE_DR )
    {
        RegionNvmGroup1->JoinTrialsCounter++;
    }
    else
    {
        RegionNvmGroup1->JoinTrialsCounter--;
    }

    if( RegionNvmGroup1->JoinTrialsCounter % 9 == 0 )
    {
        // Use DR_4 every 9th times.
        currentDr = DR_4;
    }
    else
    {
        currentDr = DR_0;
    }
    return currentDr;
}

LoRaMacStatus_t RegionUS915NextChannel( NextChanParams_t* nextChanParams, uint8_t* channel, TimerTime_t* time, TimerTime_t* aggregatedTimeOff )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t nbRestrictedChannels = 0;
    uint8_t enabledChannels[US915_MAX_NB_CHANNELS] = { 0 };
    RegionCommonIdentifyChannelsParam_t identifyChannelsParam;
    RegionCommonCountNbOfEnabledChannelsParams_t countChannelsParams;
    LoRaMacStatus_t status = LORAMAC_STATUS_NO_CHANNEL_FOUND;

    // Count 125kHz channels
    if( RegionCommonCountChannels( RegionNvmGroup1->ChannelsMaskRemaining, 0, 4 ) == 0 )
    { // Reactivate default channels
        RegionCommonChanMaskCopy( RegionNvmGroup1->ChannelsMaskRemaining, RegionNvmGroup2->ChannelsMask, 4  );

        RegionNvmGroup1->JoinChannelGroupsCurrentIndex = 0;
    }
    // Check other channels
    if( nextChanParams->Datarate >= DR_4 )
    {
        if( ( RegionNvmGroup1->ChannelsMaskRemaining[4] & CHANNELS_MASK_500KHZ_MASK ) == 0 )
        {
            RegionNvmGroup1->ChannelsMaskRemaining[4] = RegionNvmGroup2->ChannelsMask[4];
        }
    }

    // Search how many channels are enabled
    countChannelsParams.Joined = nextChanParams->Joined;
    countChannelsParams.Datarate = nextChanParams->Datarate;
    countChannelsParams.ChannelsMask = RegionNvmGroup1->ChannelsMaskRemaining;
    countChannelsParams.Channels = RegionNvmGroup2->Channels;
    countChannelsParams.Bands = RegionBands;
    countChannelsParams.MaxNbChannels = US915_MAX_NB_CHANNELS;
    countChannelsParams.JoinChannels = NULL;

    identifyChannelsParam.AggrTimeOff = nextChanParams->AggrTimeOff;
    identifyChannelsParam.LastAggrTx = nextChanParams->LastAggrTx;
    identifyChannelsParam.DutyCycleEnabled = nextChanParams->DutyCycleEnabled;
    identifyChannelsParam.MaxBands = US915_MAX_NB_BANDS;

    identifyChannelsParam.ElapsedTimeSinceStartUp = nextChanParams->ElapsedTimeSinceStartUp;
    identifyChannelsParam.LastTxIsJoinRequest = nextChanParams->LastTxIsJoinRequest;
    identifyChannelsParam.ExpectedTimeOnAir = GetTimeOnAir( nextChanParams->Datarate, nextChanParams->PktLen );

    identifyChannelsParam.CountNbOfEnabledChannelsParam = &countChannelsParams;

    status = RegionCommonIdentifyChannels( &identifyChannelsParam, aggregatedTimeOff, enabledChannels,
                                           &nbEnabledChannels, &nbRestrictedChannels, time );

    if( status == LORAMAC_STATUS_OK )
    {
        if( nextChanParams->Joined == true )
        {
            // Choose randomly on of the remaining channels
            *channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];
        }
        else
        {
            // For rapid network acquisition in mixed gateway channel plan environments, the device
            // follow a random channel selection sequence. It probes alternating one out of a
            // group of eight 125 kHz channels followed by probing one 500 kHz channel each pass.
            // Each time a 125 kHz channel will be selected from another group.

            // 125kHz Channels (0 - 63) DR0
            if( nextChanParams->Datarate == DR_0 )
            {
                if( RegionBaseUSComputeNext125kHzJoinChannel( ( uint16_t* ) RegionNvmGroup1->ChannelsMaskRemaining,
                    &RegionNvmGroup1->JoinChannelGroupsCurrentIndex, channel ) == LORAMAC_STATUS_PARAMETER_INVALID )
                {
                    return LORAMAC_STATUS_PARAMETER_INVALID;
                }
            }
            // 500kHz Channels (64 - 71) DR4 or LR-FHSS
            else
            {
                // Choose the next available channel
                uint8_t i = 0;
                while( ( ( RegionNvmGroup1->ChannelsMaskRemaining[4] & CHANNELS_MASK_500KHZ_MASK ) & ( 1 << i ) ) == 0 )
                {
                    i++;
                }
                *channel = 64 + i;
            }
        }

        // Disable the channel in the mask
        RegionCommonChanDisable( RegionNvmGroup1->ChannelsMaskRemaining, *channel, US915_MAX_NB_CHANNELS );
    }
    return status;
}

LoRaMacStatus_t RegionUS915ChannelAdd( ChannelAddParams_t* channelAdd )
{
    return LORAMAC_STATUS_PARAMETER_INVALID;
}

bool RegionUS915ChannelsRemove( ChannelRemoveParams_t* channelRemove  )
{
    return LORAMAC_STATUS_PARAMETER_INVALID;
}

uint8_t RegionUS915ApplyDrOffset( uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset )
{
    int8_t datarate = DatarateOffsetsUS915[dr][drOffset];

    if( datarate < 0 )
    {
        datarate = DR_0;
    }
    return datarate;
}

void RegionUS915RxBeaconSetup( RxBeaconSetup_t* rxBeaconSetup, uint8_t* outDr )
{
    RegionCommonRxBeaconSetupParams_t regionCommonRxBeaconSetup;

    regionCommonRxBeaconSetup.Datarates = DataratesUS915;
    regionCommonRxBeaconSetup.Frequency = rxBeaconSetup->Frequency;
    regionCommonRxBeaconSetup.BeaconSize = US915_BEACON_SIZE;
    regionCommonRxBeaconSetup.BeaconDatarate = US915_BEACON_CHANNEL_DR;
    regionCommonRxBeaconSetup.BeaconChannelBW = US915_BEACON_CHANNEL_BW;
    regionCommonRxBeaconSetup.RxTime = rxBeaconSetup->RxTime;
    regionCommonRxBeaconSetup.SymbolTimeout = rxBeaconSetup->SymbolTimeout;

    RegionCommonRxBeaconSetup( &regionCommonRxBeaconSetup );

    // Store downlink datarate
    *outDr = US915_BEACON_CHANNEL_DR;
}
