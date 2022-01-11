/*!
 * \file  RegionCN470A26.c
 *
 * \brief Specific implementations of Channel plan type A, 26MHz.
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

uint32_t RegionCN470A26GetDownlinkFrequency( uint8_t channel, uint8_t joinChannelIndex, bool isPingSlot )
{
    return CN470_A26_BEACON_FREQ;
}

uint8_t RegionCN470A26GetBeaconChannelOffset( uint8_t joinChannelIndex )
{
    return 0;
}

uint8_t RegionCN470A26LinkAdrChMaskUpdate( uint16_t* channelsMask, uint8_t chMaskCntl,
                                              uint16_t chanMask, ChannelParams_t* channels )
{
    uint8_t status = 0x07;

    if( ( chMaskCntl == 5 ) || ( chMaskCntl == 6 ) || ( chMaskCntl == 7 ) )
    {
        // RFU
        status &= 0xFE; // Channel mask KO
    }
    else if( chMaskCntl == 3 )
    {
        // Enable all channels
        channelsMask[0] = 0xFFFF;
        channelsMask[1] = 0xFFFF;
        channelsMask[2] = 0xFFFF;
        channelsMask[3] = 0xFFFF;
        channelsMask[4] = 0x0000;
        channelsMask[5] = 0x0000;
    }
    else if( chMaskCntl == 4 )
    {
        // Disable all channels
        channelsMask[0] = 0x0000;
        channelsMask[1] = 0x0000;
        channelsMask[2] = 0x0000;
        channelsMask[3] = 0x0000;
        channelsMask[4] = 0x0000;
        channelsMask[5] = 0x0000;
    }
    else
    {
        // chMaskCntl 0 to 2
        for( uint8_t i = 0; i < 16; i++ )
        {
            if( ( ( chanMask & ( 1 << i ) ) != 0 ) &&
                ( channels[chMaskCntl * 16 + i].Frequency == 0 ) )
            {// Trying to enable an undefined channel
                status &= 0xFE; // Channel mask KO
            }
        }
            channelsMask[chMaskCntl] = chanMask;
    }
    return status;
}

bool RegionCN470A26VerifyRfFreq( uint32_t freq )
{
    // Downstream group 1
    if( RegionBaseUSVerifyFrequencyGroup( freq, CN470_A26_FIRST_RX_CHANNEL,
                                          CN470_A26_LAST_RX_CHANNEL,
                                          CN470_A26_STEPWIDTH_RX_CHANNEL ) == false )
    {
        return false;
    }
    return true;
}

void RegionCN470A26InitializeChannels( ChannelParams_t* channels )
{
    // Upstream group 1
    for( uint8_t i = 0; i < 48; i++ )
    {
        channels[i].Frequency = CN470_A26_FIRST_TX_CHANNEL + i * CN470_A26_STEPWIDTH_RX_CHANNEL;
        channels[i].DrRange.Value = ( CN470_TX_MAX_DATARATE << 4 ) | CN470_TX_MIN_DATARATE;
        channels[i].Band = 0;
    }
}

void RegionCN470A26InitializeChannelsMask( uint16_t* channelsDefaultMask )
{
    // Enable all possible channels
    channelsDefaultMask[0] = 0xFFFF;
    channelsDefaultMask[1] = 0xFFFF;
    channelsDefaultMask[2] = 0xFFFF;
    channelsDefaultMask[3] = 0x0000;
    channelsDefaultMask[4] = 0x0000;
    channelsDefaultMask[5] = 0x0000;
}

uint32_t RegionCN470A26GetRx1Frequency( uint8_t channel )
{
    return ( CN470_A26_FIRST_RX_CHANNEL + ( ( channel % 24 ) * CN470_A26_STEPWIDTH_RX_CHANNEL ) );
}

uint32_t RegionCN470A26GetRx2Frequency( uint8_t joinChannelIndex, bool isOtaaDevice )
{
    return CN470_A26_RX_WND_2_FREQ;
}
