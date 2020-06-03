/*!
 * \file      RegionCN470A26.c
 *
 * \brief
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
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
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
