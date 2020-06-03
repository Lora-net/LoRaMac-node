/*!
 * \file      RegionCN470B26.c
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

