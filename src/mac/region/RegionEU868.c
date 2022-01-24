/*!
 * \file  RegionEU868.c
 *
 * \brief Region implementation for EU868
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
#include "RegionEU868.h"

// Definitions
#define CHANNELS_MASK_SIZE              1

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

// Static functions
#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
static void lr_fhss_dr_to_cr_bw( uint8_t dr, lr_fhss_v1_cr_t* cr, lr_fhss_v1_bw_t* bw )
{
    switch( dr )
    {
    case DR_8:
        *cr = LR_FHSS_V1_CR_1_3;
        *bw = LR_FHSS_V1_BW_136719_HZ;
        break;
    case DR_9:
        *cr = LR_FHSS_V1_CR_2_3;
        *bw = LR_FHSS_V1_BW_136719_HZ;
        break;
    case DR_10:
        *cr = LR_FHSS_V1_CR_1_3;
        *bw = LR_FHSS_V1_BW_335938_HZ;
        break;
    case DR_11:
        *cr = LR_FHSS_V1_CR_2_3;
        *bw = LR_FHSS_V1_BW_335938_HZ;
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
            if( ( drLocal == DR_8 ) || ( drLocal == DR_10 ) )
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

static bool VerifyRfFreq( uint32_t freq, uint8_t* band )
{
    // Check frequency bands
    if( ( freq >= 863000000 ) && ( freq < 865000000 ) )
    {
        *band = 2;
    }
    else if( ( freq >= 865000000 ) && ( freq <= 868000000 ) )
    {
        *band = 0;
    }
    else if( ( freq > 868000000 ) && ( freq <= 868600000 ) )
    {
        *band = 1;
    }
    else if( ( freq >= 868700000 ) && ( freq <= 869200000 ) )
    {
        *band = 5;
    }
    else if( ( freq >= 869400000 ) && ( freq <= 869650000 ) )
    {
        *band = 3;
    }
    else if( ( freq >= 869700000 ) && ( freq <= 870000000 ) )
    {
        *band = 4;
    }
    else
    {
        return false;
    }
    return true;
}

static TimerTime_t GetTimeOnAir( int8_t datarate, uint16_t pktLen )
{
    int8_t phyDr = DataratesEU868[datarate];
    uint32_t bandwidth = RegionCommonGetBandwidth( datarate, BandwidthsEU868 );
    TimerTime_t timeOnAir = 0;

#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
    if( ( datarate > DR_7 ) && ( datarate < DR_12 ) )
    {  // LR-FHSS
        lr_fhss_v1_cr_t                    lr_fhss_cr;
        lr_fhss_v1_bw_t                    lr_fhss_bw;
        loramac_radio_lr_fhss_time_on_air_params_t params;

        lr_fhss_dr_to_cr_bw( datarate, &lr_fhss_cr, &lr_fhss_bw );

        params.lr_fhss_params.device_offset                  = 0;
        params.lr_fhss_params.lr_fhss_params.sync_word       = lr_fhss_sync_word;
        params.lr_fhss_params.lr_fhss_params.modulation_type = LR_FHSS_V1_MODULATION_TYPE_GMSK_488;
        params.lr_fhss_params.lr_fhss_params.cr              = lr_fhss_cr;
        params.lr_fhss_params.lr_fhss_params.grid            = LR_FHSS_V1_GRID_3906_HZ;
        params.lr_fhss_params.lr_fhss_params.bw              = lr_fhss_bw;
        params.lr_fhss_params.lr_fhss_params.enable_hopping  = true;
        params.lr_fhss_params.lr_fhss_params.header_count    = lr_fhss_get_header_count( lr_fhss_cr );

        timeOnAir = loramac_radio_lr_fhss_get_time_on_air_in_ms( &params );
    }
    else if( datarate == DR_7 )
#else
    if( datarate == DR_7 )
#endif
    {
        loramac_radio_gfsk_time_on_air_params_t gfsk_params = {
            .br_in_bps =  ( uint32_t )phyDr * 1000,
            .preamble_len_in_bits = 40,
            .sync_word_len_in_bits = 24,
            .is_address_filtering_on = false,
            .is_pkt_len_fixed = false,
            .pld_len_in_bytes = ( uint8_t ) pktLen,
            .is_crc_on = true,
        };
        timeOnAir = loramac_radio_gfsk_get_time_on_air_in_ms( &gfsk_params );
    }
    else
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
        timeOnAir = loramac_radio_lora_get_time_on_air_in_ms( &lora_params );
    }
    return timeOnAir;
}

PhyParam_t RegionEU868GetPhyParam( GetPhyParams_t* getPhy )
{
    PhyParam_t phyParam = { 0 };

    switch( getPhy->Attribute )
    {
        case PHY_MIN_RX_DR:
        {
            phyParam.Value = EU868_RX_MIN_DATARATE;
            break;
        }
        case PHY_MIN_TX_DR:
        {
            phyParam.Value = EU868_TX_MIN_DATARATE;
            break;
        }
        case PHY_DEF_TX_DR:
        {
            phyParam.Value = EU868_DEFAULT_DATARATE;
            break;
        }
        case PHY_NEXT_LOWER_TX_DR:
        {
            RegionCommonGetNextLowerTxDrParams_t nextLowerTxDrParams =
            {
                .CurrentDr = getPhy->Datarate,
                .MaxDr = ( int8_t )EU868_TX_MAX_DATARATE,
                .MinDr = ( int8_t )EU868_TX_MIN_DATARATE,
                .NbChannels = EU868_MAX_NB_CHANNELS,
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
            phyParam.Value = EU868_MAX_TX_POWER;
            break;
        }
        case PHY_DEF_TX_POWER:
        {
            phyParam.Value = EU868_DEFAULT_TX_POWER;
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
            phyParam.Value = MaxPayloadOfDatarateEU868[getPhy->Datarate];
            break;
        }
        case PHY_DUTY_CYCLE:
        {
            phyParam.Value = EU868_DUTY_CYCLE_ENABLED;
            break;
        }
        case PHY_MAX_RX_WINDOW:
        {
            phyParam.Value = EU868_MAX_RX_WINDOW;
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
            phyParam.Value = EU868_RX_WND_2_FREQ;
            break;
        }
        case PHY_DEF_RX2_DR:
        {
            phyParam.Value = EU868_RX_WND_2_DR;
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
            phyParam.Value = EU868_MAX_NB_CHANNELS;
            break;
        }
        case PHY_CHANNELS:
        {
            phyParam.Channels = RegionNvmGroup2->Channels;
            break;
        }
        case PHY_DEF_UPLINK_DWELL_TIME:
        {
            phyParam.Value = EU868_DEFAULT_UPLINK_DWELL_TIME;
            break;
        }
        case PHY_DEF_DOWNLINK_DWELL_TIME:
        {
            phyParam.Value = REGION_COMMON_DEFAULT_DOWNLINK_DWELL_TIME;
            break;
        }
        case PHY_DEF_MAX_EIRP:
        {
            phyParam.fValue = EU868_DEFAULT_MAX_EIRP;
            break;
        }
        case PHY_DEF_ANTENNA_GAIN:
        {
            phyParam.fValue = EU868_DEFAULT_ANTENNA_GAIN;
            break;
        }
        case PHY_BEACON_CHANNEL_FREQ:
        {
            phyParam.Value = EU868_BEACON_CHANNEL_FREQ;
            break;
        }
        case PHY_BEACON_FORMAT:
        {
            phyParam.BeaconFormat.BeaconSize = EU868_BEACON_SIZE;
            phyParam.BeaconFormat.Rfu1Size = EU868_RFU1_SIZE;
            phyParam.BeaconFormat.Rfu2Size = EU868_RFU2_SIZE;
            break;
        }
        case PHY_BEACON_CHANNEL_DR:
        {
            phyParam.Value = EU868_BEACON_CHANNEL_DR;
            break;
        }
        case PHY_PING_SLOT_CHANNEL_FREQ:
        {
            phyParam.Value = EU868_PING_SLOT_CHANNEL_FREQ;
            break;
        }
        case PHY_PING_SLOT_CHANNEL_DR:
        {
            phyParam.Value = EU868_PING_SLOT_CHANNEL_DR;
            break;
        }
        case PHY_SF_FROM_DR:
        {
            phyParam.Value = DataratesEU868[getPhy->Datarate];
            break;
        }
        case PHY_BW_FROM_DR:
        {
            phyParam.Value = RegionCommonGetBandwidth( getPhy->Datarate, BandwidthsEU868 );
            break;
        }
        default:
        {
            break;
        }
    }

    return phyParam;
}

void RegionEU868SetBandTxDone( SetBandTxDoneParams_t* txDone )
{
    RegionCommonSetBandTxDone( &RegionBands[RegionNvmGroup2->Channels[txDone->Channel].Band],
                               txDone->LastTxAirTime, txDone->Joined, txDone->ElapsedTimeSinceStartUp );
}

void RegionEU868InitDefaults( InitDefaultsParams_t* params )
{
    Band_t bands[EU868_MAX_NB_BANDS] =
    {
        EU868_BAND0,
        EU868_BAND1,
        EU868_BAND2,
        EU868_BAND3,
        EU868_BAND4,
        EU868_BAND5,
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

            // Default bands
            memcpy1( ( uint8_t* )RegionBands, ( uint8_t* )bands, sizeof( Band_t ) * EU868_MAX_NB_BANDS );

            // Default channels
            RegionNvmGroup2->Channels[0] = ( ChannelParams_t ) EU868_LC1;
            RegionNvmGroup2->Channels[1] = ( ChannelParams_t ) EU868_LC2;
            RegionNvmGroup2->Channels[2] = ( ChannelParams_t ) EU868_LC3;

            // Default ChannelsMask
            RegionNvmGroup2->ChannelsDefaultMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );

            // Update the channels mask
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask, CHANNELS_MASK_SIZE );
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

bool RegionEU868Verify( VerifyParams_t* verify, PhyAttribute_t phyAttribute )
{
    switch( phyAttribute )
    {
        case PHY_FREQUENCY:
        {
            uint8_t band = 0;
            return VerifyRfFreq( verify->Frequency, &band );
        }
        case PHY_TX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, EU868_TX_MIN_DATARATE, EU868_TX_MAX_DATARATE );
        }
        case PHY_DEF_TX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, DR_0, DR_5 );
        }
        case PHY_RX_DR:
        {
            return RegionCommonValueInRange( verify->DatarateParams.Datarate, EU868_RX_MIN_DATARATE, EU868_RX_MAX_DATARATE );
        }
        case PHY_DEF_TX_POWER:
        case PHY_TX_POWER:
        {
            // Remark: switched min and max!
            return RegionCommonValueInRange( verify->TxPower, EU868_MAX_TX_POWER, EU868_MIN_TX_POWER );
        }
        case PHY_DUTY_CYCLE:
        {
            return EU868_DUTY_CYCLE_ENABLED;
        }
        default:
            return false;
    }
}

void RegionEU868ApplyCFList( ApplyCFListParams_t* applyCFList )
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
    for( uint8_t i = 0, chanIdx = EU868_NUMB_DEFAULT_CHANNELS; chanIdx < EU868_MAX_NB_CHANNELS; i+=3, chanIdx++ )
    {
        if( chanIdx < ( EU868_NUMB_CHANNELS_CF_LIST + EU868_NUMB_DEFAULT_CHANNELS ) )
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
            RegionEU868ChannelAdd( &channelAdd );
        }
        else
        {
            channelRemove.ChannelId = chanIdx;

            RegionEU868ChannelsRemove( &channelRemove );
        }
    }
}

bool RegionEU868ChanMaskSet( ChanMaskSetParams_t* chanMaskSet )
{
    switch( chanMaskSet->ChannelsMaskType )
    {
        case CHANNELS_MASK:
        {
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, chanMaskSet->ChannelsMaskIn, CHANNELS_MASK_SIZE );
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

void RegionEU868ComputeRxWindowParameters( int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RxConfigParams_t *rxConfigParams )
{
    uint32_t tSymbolInUs = 0;

    // Get the datarate, perform a boundary check
    rxConfigParams->Datarate = MIN( datarate, EU868_RX_MAX_DATARATE );
    rxConfigParams->Bandwidth = RegionCommonGetBandwidth( rxConfigParams->Datarate, BandwidthsEU868 );

#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
    if( ( rxConfigParams->Datarate > DR_7 ) && ( rxConfigParams->Datarate < DR_12 ) )
    {  // LR-FHSS  is not supported for downlinks
        // Panic
        while( 1 )
            ;
    }
    else if( rxConfigParams->Datarate == DR_7 )
#else
    if( rxConfigParams->Datarate == DR_7 )
#endif
    { // FSK
        tSymbolInUs = RegionCommonComputeSymbolTimeFsk( DataratesEU868[rxConfigParams->Datarate] );
    }
    else
    { // LoRa
        tSymbolInUs = RegionCommonComputeSymbolTimeLoRa( DataratesEU868[rxConfigParams->Datarate], BandwidthsEU868[rxConfigParams->Datarate] );
    }

    RegionCommonComputeRxWindowParameters( tSymbolInUs, minRxSymbols, rxError, loramac_radio_get_wakeup_time_in_ms( ), &rxConfigParams->WindowTimeout, &rxConfigParams->WindowOffset );
}

bool RegionEU868RxConfig( RxConfigParams_t* rxConfig, int8_t* datarate )
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
    if( dr > DR_7 )
    {
        return false;
    }
#endif

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
    phyDr = DataratesEU868[dr];

    // Radio configuration
    if( dr == DR_7 )
    {
        loramac_radio_gfsk_cfg_params_t gfsk_params = {
            .rf_freq_in_hz = frequency,
            .br_in_bps =  ( uint32_t )phyDr * 1000,
            .bw_dsb_in_hz = 50000,
            .preamble_len_in_bits = 40,
            .sync_word_len_in_bits = 24,
            .is_pkt_len_fixed = false,
            .pld_len_in_bytes = ( uint8_t )( MaxPayloadOfDatarateEU868[dr] + LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE ),
            .is_crc_on = true,
            .rx_sync_timeout_in_symb = rxConfig->WindowTimeout,
            .is_rx_continuous = rxConfig->RxContinuous,
        };
        loramac_radio_gfsk_set_cfg( &gfsk_params );
    }
    else
    {
        loramac_radio_lora_cfg_params_t lora_params = {
            .rf_freq_in_hz = frequency,
            .sf = ( ral_lora_sf_t ) phyDr,
            .bw = ( ral_lora_bw_t ) rxConfig->Bandwidth,
            .cr = RAL_LORA_CR_4_5,
            .preamble_len_in_symb = 8,
            .is_pkt_len_fixed = false,
            .pld_len_in_bytes = ( uint8_t )( MaxPayloadOfDatarateEU868[dr] + LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE ),
            .is_crc_on = false,
            .invert_iq_is_on = true,
            .rx_sync_timeout_in_symb = rxConfig->WindowTimeout,
            .is_rx_continuous = rxConfig->RxContinuous,
        };
        loramac_radio_lora_set_cfg( &lora_params );
    }

    *datarate = (uint8_t) dr;
    return true;
}

bool RegionEU868TxConfig( TxConfigParams_t* txConfig, int8_t* txPower, TimerTime_t* txTimeOnAir )
{
    int8_t phyDr = DataratesEU868[txConfig->Datarate];
    int8_t txPowerLimited = RegionCommonLimitTxPower( txConfig->TxPower, RegionBands[RegionNvmGroup2->Channels[txConfig->Channel].Band].TxMaxPower );
    uint32_t bandwidth = RegionCommonGetBandwidth( txConfig->Datarate, BandwidthsEU868 );
    int8_t phyTxPower = 0;

    // Calculate physical TX power
    phyTxPower = RegionCommonComputeTxPower( txPowerLimited, txConfig->MaxEirp, txConfig->AntennaGain );

    // Radio configuration
#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
    if( ( txConfig->Datarate > DR_7 ) && ( txConfig->Datarate < DR_12 ) )
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
        cfg_params.lr_fhss_params.lr_fhss_params.grid            = LR_FHSS_V1_GRID_3906_HZ;
        cfg_params.lr_fhss_params.lr_fhss_params.bw              = lr_fhss_bw;
        cfg_params.lr_fhss_params.lr_fhss_params.enable_hopping  = true;
        cfg_params.lr_fhss_params.lr_fhss_params.header_count    = lr_fhss_get_header_count( lr_fhss_cr );
        cfg_params.tx_timeout_in_ms                              = 4000;

        loramac_radio_lr_fhss_set_cfg( &cfg_params );
    }
    else if( txConfig->Datarate == DR_7 )
#else
    if( txConfig->Datarate == DR_7 )
#endif
    {
        loramac_radio_gfsk_cfg_params_t gfsk_params = {
            .rf_freq_in_hz = RegionNvmGroup2->Channels[txConfig->Channel].Frequency,
            .tx_rf_pwr_in_dbm = phyTxPower,
            .br_in_bps =  ( uint32_t )phyDr * 1000,
            .fdev_in_hz = 25000,
            .bw_dsb_in_hz = 50000,
            .preamble_len_in_bits = 40,
            .sync_word_len_in_bits = 24,
            .is_pkt_len_fixed = false,
            .pld_len_in_bytes = ( uint8_t ) txConfig->PktLen,
            .is_crc_on = true,
            .tx_timeout_in_ms= 4000,
        };
        loramac_radio_gfsk_set_cfg( &gfsk_params );
    }
    else
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

uint8_t RegionEU868LinkAdrReq( LinkAdrReqParams_t* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed )
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
            for( uint8_t i = 0; i < EU868_MAX_NB_CHANNELS; i++ )
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
    phyParam = RegionEU868GetPhyParam( &getPhy );

    linkAdrVerifyParams.Status = status;
    linkAdrVerifyParams.AdrEnabled = linkAdrReq->AdrEnabled;
    linkAdrVerifyParams.Datarate = linkAdrParams.Datarate;
    linkAdrVerifyParams.TxPower = linkAdrParams.TxPower;
    linkAdrVerifyParams.NbRep = linkAdrParams.NbRep;
    linkAdrVerifyParams.CurrentDatarate = linkAdrReq->CurrentDatarate;
    linkAdrVerifyParams.CurrentTxPower = linkAdrReq->CurrentTxPower;
    linkAdrVerifyParams.CurrentNbRep = linkAdrReq->CurrentNbRep;
    linkAdrVerifyParams.NbChannels = EU868_MAX_NB_CHANNELS;
    linkAdrVerifyParams.ChannelsMask = &chMask;
    linkAdrVerifyParams.MinDatarate = ( int8_t )phyParam.Value;
    linkAdrVerifyParams.MaxDatarate = EU868_TX_MAX_DATARATE;
    linkAdrVerifyParams.Channels = RegionNvmGroup2->Channels;
    linkAdrVerifyParams.MinTxPower = EU868_MIN_TX_POWER;
    linkAdrVerifyParams.MaxTxPower = EU868_MAX_TX_POWER;
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

uint8_t RegionEU868RxParamSetupReq( RxParamSetupReqParams_t* rxParamSetupReq )
{
    uint8_t status = 0x07;
    uint8_t band = 0;

    // Verify radio frequency
    if( VerifyRfFreq( rxParamSetupReq->Frequency, &band ) == false )
    {
        status &= 0xFE; // Channel frequency KO
    }

    // Verify datarate
    if( RegionCommonValueInRange( rxParamSetupReq->Datarate, EU868_RX_MIN_DATARATE, EU868_RX_MAX_DATARATE ) == false )
    {
        status &= 0xFD; // Datarate KO
    }

    // Verify datarate offset
    if( RegionCommonValueInRange( rxParamSetupReq->DrOffset, EU868_MIN_RX1_DR_OFFSET, EU868_MAX_RX1_DR_OFFSET ) == false )
    {
        status &= 0xFB; // Rx1DrOffset range KO
    }

    return status;
}

int8_t RegionEU868NewChannelReq( NewChannelReqParams_t* newChannelReq )
{
    uint8_t status = 0x03;
    ChannelAddParams_t channelAdd;
    ChannelRemoveParams_t channelRemove;

    if( newChannelReq->NewChannel->Frequency == 0 )
    {
        channelRemove.ChannelId = newChannelReq->ChannelId;

        // Remove
        if( RegionEU868ChannelsRemove( &channelRemove ) == false )
        {
            status &= 0xFC;
        }
    }
    else
    {
        channelAdd.NewChannel = newChannelReq->NewChannel;
        channelAdd.ChannelId = newChannelReq->ChannelId;

        switch( RegionEU868ChannelAdd( &channelAdd ) )
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

int8_t RegionEU868TxParamSetupReq( TxParamSetupReqParams_t* txParamSetupReq )
{
    // Do not accept the request
    return -1;
}

int8_t RegionEU868DlChannelReq( DlChannelReqParams_t* dlChannelReq )
{
    uint8_t status = 0x03;
    uint8_t band = 0;

    if( dlChannelReq->ChannelId >= ( CHANNELS_MASK_SIZE * 16 ) )
    {
        return 0;
    }

    // Verify if the frequency is supported
    if( VerifyRfFreq( dlChannelReq->Rx1Frequency, &band ) == false )
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

int8_t RegionEU868AlternateDr( int8_t currentDr, AlternateDrType_t type )
{
    return currentDr;
}

LoRaMacStatus_t RegionEU868NextChannel( NextChanParams_t* nextChanParams, uint8_t* channel, TimerTime_t* time, TimerTime_t* aggregatedTimeOff )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t nbRestrictedChannels = 0;
    uint8_t enabledChannels[EU868_MAX_NB_CHANNELS] = { 0 };
    RegionCommonIdentifyChannelsParam_t identifyChannelsParam;
    RegionCommonCountNbOfEnabledChannelsParams_t countChannelsParams;
    LoRaMacStatus_t status = LORAMAC_STATUS_NO_CHANNEL_FOUND;
    uint16_t joinChannels = EU868_JOIN_CHANNELS;

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
    countChannelsParams.MaxNbChannels = EU868_MAX_NB_CHANNELS;
    countChannelsParams.JoinChannels = &joinChannels;

    identifyChannelsParam.AggrTimeOff = nextChanParams->AggrTimeOff;
    identifyChannelsParam.LastAggrTx = nextChanParams->LastAggrTx;
    identifyChannelsParam.DutyCycleEnabled = nextChanParams->DutyCycleEnabled;
    identifyChannelsParam.MaxBands = EU868_MAX_NB_BANDS;

    identifyChannelsParam.ElapsedTimeSinceStartUp = nextChanParams->ElapsedTimeSinceStartUp;
    identifyChannelsParam.LastTxIsJoinRequest = nextChanParams->LastTxIsJoinRequest;
    identifyChannelsParam.ExpectedTimeOnAir = GetTimeOnAir( nextChanParams->Datarate, nextChanParams->PktLen );

    identifyChannelsParam.CountNbOfEnabledChannelsParam = &countChannelsParams;

    status = RegionCommonIdentifyChannels( &identifyChannelsParam, aggregatedTimeOff, enabledChannels,
                                           &nbEnabledChannels, &nbRestrictedChannels, time );

    if( status == LORAMAC_STATUS_OK )
    {
        // We found a valid channel
        *channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];
    }
    else if( status == LORAMAC_STATUS_NO_CHANNEL_FOUND )
    {
        // Datarate not supported by any channel, restore defaults
        RegionNvmGroup2->ChannelsMask[0] |= LC( 1 ) + LC( 2 ) + LC( 3 );
    }
    return status;
}

LoRaMacStatus_t RegionEU868ChannelAdd( ChannelAddParams_t* channelAdd )
{
    uint8_t band = 0;
    bool drInvalid = false;
    bool freqInvalid = false;
    uint8_t id = channelAdd->ChannelId;

    if( id < EU868_NUMB_DEFAULT_CHANNELS )
    {
        return LORAMAC_STATUS_FREQ_AND_DR_INVALID;
    }

    if( id >= EU868_MAX_NB_CHANNELS )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    // Validate the datarate range
    if( RegionCommonValueInRange( channelAdd->NewChannel->DrRange.Fields.Min, EU868_TX_MIN_DATARATE, EU868_TX_MAX_DATARATE ) == false )
    {
        drInvalid = true;
    }
    if( RegionCommonValueInRange( channelAdd->NewChannel->DrRange.Fields.Max, EU868_TX_MIN_DATARATE, EU868_TX_MAX_DATARATE ) == false )
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
        if( VerifyRfFreq( channelAdd->NewChannel->Frequency, &band ) == false )
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
    RegionNvmGroup2->Channels[id].Band = band;
    RegionNvmGroup2->ChannelsMask[0] |= ( 1 << id );
    return LORAMAC_STATUS_OK;
}

bool RegionEU868ChannelsRemove( ChannelRemoveParams_t* channelRemove  )
{
    uint8_t id = channelRemove->ChannelId;

    if( id < EU868_NUMB_DEFAULT_CHANNELS )
    {
        return false;
    }

    // Remove the channel from the list of channels
    RegionNvmGroup2->Channels[id] = ( ChannelParams_t ){ 0, 0, { 0 }, 0 };

    return RegionCommonChanDisable( RegionNvmGroup2->ChannelsMask, id, EU868_MAX_NB_CHANNELS );
}

uint8_t RegionEU868ApplyDrOffset( uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset )
{
#if ( LORAMAC_LR_FHSS_IS_ON == 1 )
    int8_t datarate = DatarateOffsetsEU868[dr][drOffset];
#else
    int8_t datarate = dr - drOffset;
#endif

    if( datarate < 0 )
    {
        datarate = DR_0;
    }
    return datarate;
}

void RegionEU868RxBeaconSetup( RxBeaconSetup_t* rxBeaconSetup, uint8_t* outDr )
{
    RegionCommonRxBeaconSetupParams_t regionCommonRxBeaconSetup;

    regionCommonRxBeaconSetup.Datarates = DataratesEU868;
    regionCommonRxBeaconSetup.Frequency = rxBeaconSetup->Frequency;
    regionCommonRxBeaconSetup.BeaconSize = EU868_BEACON_SIZE;
    regionCommonRxBeaconSetup.BeaconDatarate = EU868_BEACON_CHANNEL_DR;
    regionCommonRxBeaconSetup.BeaconChannelBW = EU868_BEACON_CHANNEL_BW;
    regionCommonRxBeaconSetup.RxTime = rxBeaconSetup->RxTime;
    regionCommonRxBeaconSetup.SymbolTimeout = rxBeaconSetup->SymbolTimeout;

    RegionCommonRxBeaconSetup( &regionCommonRxBeaconSetup );

    // Store downlink datarate
    *outDr = EU868_BEACON_CHANNEL_DR;
}
