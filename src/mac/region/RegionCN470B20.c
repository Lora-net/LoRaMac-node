/*!
 * \file      RegionCN470B20.c
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

