/*!
 * \file  RegionCN470B26.c
 *
 * \brief Specific implementations of Channel plan type B, 26MHz.
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
#include "RegionCN470A26.h"
#include "RegionCN470B26.h"

uint32_t RegionCN470B26GetDownlinkFrequency( uint8_t channel, uint8_t joinChannelIndex, bool isPingSlot )
{
    return CN470_B26_BEACON_FREQ;
}

uint8_t RegionCN470B26GetBeaconChannelOffset( uint8_t joinChannelIndex )
{
    return 0;
}

uint8_t RegionCN470B26LinkAdrChMaskUpdate( uint16_t* channelsMask, uint8_t chMaskCntl,
                                           uint16_t chanMask, ChannelParams_t* channels )
{
    return RegionCN470A26LinkAdrChMaskUpdate( channelsMask, chMaskCntl,
                                                 chanMask, channels );
}

bool RegionCN470B26VerifyRfFreq( uint32_t freq )
{
    // Downstream group 1
    if( RegionBaseUSVerifyFrequencyGroup( freq, CN470_B26_FIRST_RX_CHANNEL,
                                          CN470_B26_LAST_RX_CHANNEL,
                                          CN470_B26_STEPWIDTH_RX_CHANNEL ) == false )
    {
        return false;
    }
    return true;
}

void RegionCN470B26InitializeChannels( ChannelParams_t* channels )
{
    // Upstream group 1
    for( uint8_t i = 0; i < 48; i++ )
    {
        channels[i].Frequency = CN470_B26_FIRST_TX_CHANNEL + i * CN470_B26_STEPWIDTH_RX_CHANNEL;
        channels[i].DrRange.Value = ( CN470_TX_MAX_DATARATE << 4 ) | CN470_TX_MIN_DATARATE;
        channels[i].Band = 0;
    }
}

void RegionCN470B26InitializeChannelsMask( uint16_t* channelsDefaultMask )
{
    RegionCN470A26InitializeChannelsMask( channelsDefaultMask );
}

uint32_t RegionCN470B26GetRx1Frequency( uint8_t channel )
{
    return ( CN470_B26_FIRST_RX_CHANNEL + ( ( channel % 24 ) * CN470_B26_STEPWIDTH_RX_CHANNEL ) );
}

uint32_t RegionCN470B26GetRx2Frequency( uint8_t joinChannelIndex, bool isOtaaDevice )
{
    return CN470_B26_RX_WND_2_FREQ;
}

