/*!
 * \file      RegionCN470A20.c
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

