/*!
 * \file RegionBaseUS.c
 *
 * \brief Implementations common with US region.
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
#include "LoRaMacTypes.h"
#include "region/Region.h"
#include "RegionBaseUS.h"


/*!
 * \brief Searches for available 125 kHz channels in the given channel mask.
 *
 * \param [IN] currentChannelMaskLeft The remaining channel mask.
 *
 * \param [OUT] findAvailableChannelsIndex List containing the indexes of all available 125 kHz channels.
 *
 * \param [OUT] availableChannels Number of available 125 kHz channels.
 *
 * \retval Status
 */
static LoRaMacStatus_t FindAvailable125kHzChannels( uint16_t currentChannelMaskLeft,
                                                    uint8_t* findAvailableChannelsIndex, uint8_t* availableChannels )
{
    // Nullpointer check
    if( findAvailableChannelsIndex == NULL || availableChannels == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    // Initialize counter
    *availableChannels = 0;
    for( uint8_t i = 0; i < 8; i++ )
    {
        // Find available channels
        if( ( currentChannelMaskLeft & ( 1 << i ) ) != 0 )
        {
            // Save available channel index
            findAvailableChannelsIndex[*availableChannels] = i;
            // Increment counter of available channels if the current channel is available
            ( *availableChannels )++;
        }
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t RegionBaseUSComputeNext125kHzJoinChannel( uint16_t* channelsMaskRemaining,
                                                          uint8_t* groupsCurrentIndex, uint8_t* newChannelIndex )
{
    uint8_t currentChannelMaskLeftIndex;
    uint16_t currentChannelMaskLeft;
    uint8_t findAvailableChannelsIndex[8] = { 0 };
    uint8_t availableChannels = 0;
    uint8_t startIndex;

    // Null pointer check
    if( channelsMaskRemaining == NULL || groupsCurrentIndex == NULL || newChannelIndex == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    // copy the current index.
    startIndex = *groupsCurrentIndex;

    do
    {
        // Current ChannelMaskRemaining, two groups per channel mask. For example Group 0 and 1 (8 bit) are ChannelMaskRemaining 0 (16 bit), etc.
        currentChannelMaskLeftIndex = (uint8_t) startIndex / 2;

        // For even numbers we need the 8 LSBs and for uneven the 8 MSBs
        if( ( startIndex % 2 ) == 0 )
        {
            currentChannelMaskLeft = ( channelsMaskRemaining[currentChannelMaskLeftIndex] & 0x00FF );
        }
        else
        {
            currentChannelMaskLeft = ( ( channelsMaskRemaining[currentChannelMaskLeftIndex] >> 8 ) & 0x00FF );
        }


        if( FindAvailable125kHzChannels( currentChannelMaskLeft, findAvailableChannelsIndex, &availableChannels ) == LORAMAC_STATUS_PARAMETER_INVALID )
        {
            return LORAMAC_STATUS_PARAMETER_INVALID;
        }

        if ( availableChannels > 0 )
        {
            // Choose randomly a free channel 125kHz
            *newChannelIndex = ( startIndex * 8 ) + findAvailableChannelsIndex[randr( 0, ( availableChannels - 1 ) )];
        }

        // Increment start index
        startIndex++;
        if ( startIndex > 7 )
        {
            startIndex = 0;
        }
    } while( ( availableChannels == 0 ) && ( startIndex != *groupsCurrentIndex ) );

    if ( availableChannels > 0 )
    {
        *groupsCurrentIndex = startIndex;
        return LORAMAC_STATUS_OK;
    }

    return LORAMAC_STATUS_PARAMETER_INVALID;
}

bool RegionBaseUSVerifyFrequencyGroup( uint32_t freq, uint32_t minFreq, uint32_t maxFreq, uint32_t stepwidth )
{
    if( ( freq < minFreq ) ||
        ( freq > maxFreq ) ||
        ( ( ( freq - ( uint32_t ) minFreq ) % ( uint32_t ) stepwidth ) != 0 ) )
    {
        return false;
    }
    return true;
}

uint32_t RegionBaseUSCalcDownlinkFrequency( uint8_t channel, uint32_t frequency,
                                            uint32_t stepwidth )
{
    // Calculate the frequency
    return frequency + ( channel * stepwidth );
}
