/*!
 * \file  RegionCN470B20.c
 *
 * \brief Specific implementations of Channel plan type B, 20MHz.
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
/*!
 * \defgroup  REGIONCN470
 */
#include "RegionCN470.h"
#include "RegionBaseUS.h"
#include "RegionCN470B20.h"
#include "RegionCN470A20.h"

uint32_t RegionCN470B20GetDownlinkFrequency( uint8_t channel, uint8_t joinChannelIndex, bool isPingSlot )
{
    if( isPingSlot == true)
    {
        return RegionCN470B20GetRx1Frequency( channel );
    }
    else
    {
        if( joinChannelIndex == 8 )
        {
            return RegionCN470B20GetRx1Frequency( 23 );
        }
        else
        {
            return RegionCN470B20GetRx1Frequency( 55 );
        }
    }
}

uint8_t RegionCN470B20GetBeaconChannelOffset( uint8_t joinChannelIndex )
{
    return ( joinChannelIndex - 8 ) * 32;
}

uint8_t RegionCN470B20LinkAdrChMaskUpdate( uint16_t* channelsMask, uint8_t chMaskCntl,
                                              uint16_t chanMask, ChannelParams_t* channels )
{
    // It follows the same implementation as type A
    return RegionCN470A20LinkAdrChMaskUpdate( channelsMask, chMaskCntl,
                                                 chanMask, channels );
}

bool RegionCN470B20VerifyRfFreq( uint32_t freq )
{
    bool group1Status = false;
    bool group2Status = false;

    // Downstream group 1
    group1Status = RegionBaseUSVerifyFrequencyGroup( freq, CN470_B20_FIRST_RX1_CHANNEL,
                                                     CN470_B20_LAST_RX1_CHANNEL,
                                                     CN470_B20_STEPWIDTH_RX1_CHANNEL );
    // Downstream group 2
    group2Status = RegionBaseUSVerifyFrequencyGroup( freq, CN470_B20_FIRST_RX2_CHANNEL,
                                                     CN470_B20_LAST_RX2_CHANNEL,
                                                     CN470_B20_STEPWIDTH_RX2_CHANNEL );

    // The frequency must be available in one of the groups
    if( ( group1Status == false ) && ( group2Status == false ) )
    {
        return false;
    }
    return true;
}

void RegionCN470B20InitializeChannels( ChannelParams_t* channels )
{
    // Upstream group 1
    for( uint8_t i = 0; i < 32; i++ )
    {
        channels[i].Frequency = CN470_B20_FIRST_TX1_CHANNEL + i * CN470_B20_STEPWIDTH_TX1_CHANNEL;
        channels[i].DrRange.Value = ( CN470_TX_MAX_DATARATE << 4 ) | CN470_TX_MIN_DATARATE;
        channels[i].Band = 0;
    }
    // Upstream group 2
    for( uint8_t i = 32; i < 64; i++ )
    {
        channels[i].Frequency = CN470_B20_FIRST_TX2_CHANNEL + ( i - 32 ) * CN470_B20_STEPWIDTH_TX2_CHANNEL;
        channels[i].DrRange.Value = ( CN470_TX_MAX_DATARATE << 4 ) | CN470_TX_MIN_DATARATE;
        channels[i].Band = 0;
    }
}

void RegionCN470B20InitializeChannelsMask( uint16_t* channelsDefaultMask )
{
    RegionCN470A20InitializeChannelsMask( channelsDefaultMask );
}

uint32_t RegionCN470B20GetRx1Frequency( uint8_t channel )
{
    // Base frequency for downstream group 1
    uint32_t baseFrequency = CN470_B20_FIRST_RX1_CHANNEL;
    uint8_t offset = 0;

    if( channel >= 32 )
    {
        // Base frequency for downstream group 2
        baseFrequency = CN470_B20_FIRST_RX2_CHANNEL;
        offset = 32;
    }
    return ( baseFrequency + ( ( channel - offset ) * CN470_B20_STEPWIDTH_RX1_CHANNEL ) );
}

uint32_t RegionCN470B20GetRx2Frequency( uint8_t joinChannelIndex, bool isOtaaDevice )
{
    uint32_t otaaFrequencies[] = CN470_B20_RX_WND_2_FREQ_OTAA;

    if( isOtaaDevice == true )
    {
        return otaaFrequencies[joinChannelIndex - 8];
    }
    // ABP device
    return CN470_B20_RX_WND_2_FREQ_ABP;
}

