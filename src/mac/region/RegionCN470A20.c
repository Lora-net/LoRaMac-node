/*!
 * \file  RegionCN470A20.c
 *
 * \brief Specific implementations of channel plan type A, 20MHz.
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
#include "RegionCN470A20.h"

uint32_t RegionCN470A20GetDownlinkFrequency( uint8_t channel, uint8_t joinChannelIndex, bool isPingSlot )
{
    return RegionCN470A20GetRx1Frequency( channel );
}

uint8_t RegionCN470A20GetBeaconChannelOffset( uint8_t joinChannelIndex )
{
    return ( joinChannelIndex * 8 );
}

uint8_t RegionCN470A20LinkAdrChMaskUpdate( uint16_t* channelsMask, uint8_t chMaskCntl,
                                           uint16_t chanMask, ChannelParams_t* channels )
{
    uint8_t status = 0x07;

    if( ( chMaskCntl == 4 ) || ( chMaskCntl == 5 ) )
    {
        // RFU
        status &= 0xFE; // Channel mask KO
    }
    else if( chMaskCntl == 6 )
    {
        // Enable all channels
        channelsMask[0] = 0xFFFF;
        channelsMask[1] = 0xFFFF;
        channelsMask[2] = 0xFFFF;
        channelsMask[3] = 0xFFFF;
        channelsMask[4] = 0x0000;
        channelsMask[5] = 0x0000;
    }
    else if( chMaskCntl == 7 )
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
        // chMaskCntl 0 to 3
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

bool RegionCN470A20VerifyRfFreq( uint32_t freq )
{
    // Downstream group 1 and 2
    if( RegionBaseUSVerifyFrequencyGroup( freq, CN470_A20_FIRST_RX_CHANNEL,
                                          CN470_A20_LAST_RX_CHANNEL,
                                          CN470_A20_STEPWIDTH_RX_CHANNEL ) == false )
    {
        return false;
    }
    return true;
}

void RegionCN470A20InitializeChannels( ChannelParams_t* channels )
{
    // Upstream group 1
    for( uint8_t i = 0; i < 32; i++ )
    {
        channels[i].Frequency = CN470_A20_FIRST_TX1_CHANNEL + i * CN470_A20_STEPWIDTH_RX_CHANNEL;
        channels[i].DrRange.Value = ( CN470_TX_MAX_DATARATE << 4 ) | CN470_TX_MIN_DATARATE;
        channels[i].Band = 0;
    }
    // Upstream group 2
    for( uint8_t i = 32; i < 64; i++ )
    {
        channels[i].Frequency = CN470_A20_FIRST_TX2_CHANNEL + ( i - 32 ) * CN470_A20_STEPWIDTH_RX_CHANNEL;
        channels[i].DrRange.Value = ( CN470_TX_MAX_DATARATE << 4 ) | CN470_TX_MIN_DATARATE;
        channels[i].Band = 0;
    }
}

void RegionCN470A20InitializeChannelsMask(  uint16_t* channelsDefaultMask )
{
    // Enable all possible channels
    channelsDefaultMask[0] = 0xFFFF;
    channelsDefaultMask[1] = 0xFFFF;
    channelsDefaultMask[2] = 0xFFFF;
    channelsDefaultMask[3] = 0xFFFF;
    channelsDefaultMask[4] = 0x0000;
    channelsDefaultMask[5] = 0x0000;
}

uint32_t RegionCN470A20GetRx1Frequency( uint8_t channel )
{
    // Base frequency for downstream group 1
    uint32_t baseFrequency = CN470_A20_FIRST_RX_CHANNEL;
    uint8_t offset = 0;

    if( channel >= 32 )
    {
        // Base frequency for downstream group 2
        baseFrequency = 490300000;
        offset = 32;
    }
    return ( baseFrequency + ( ( channel - offset ) * CN470_A20_STEPWIDTH_RX_CHANNEL ) );
}

uint32_t RegionCN470A20GetRx2Frequency( uint8_t joinChannelIndex, bool isOtaaDevice )
{
    uint32_t otaaFrequencies[] = CN470_A20_RX_WND_2_FREQ_OTAA;

    if( isOtaaDevice == true )
    {
        return otaaFrequencies[joinChannelIndex];
    }
    // ABP device
    return CN470_A20_RX_WND_2_FREQ_ABP;
}

