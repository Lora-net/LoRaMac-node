/*!
 * \file      RegionBaseUS.c
 *
 * \brief     Implementations common with US region.
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
 *
 * \author    Phanindra Kumar Yellapu ( STACKFORCE )
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
