/*!
 * \file      RegionCN470B20.h
 *
 * \brief     Specific implementations of Channel plan type B, 20MHz.
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
 *
 * \{
 */
#ifndef __REGION_CN470_B20_H__
#define __REGION_CN470_B20_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "region/Region.h"

/*!
 * The maximum number of channels.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_CHANNELS_MASK_SIZE        4

/*!
 * The number of entries in the join accept list.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_JOIN_ACCEPT_LIST_SIZE     4

/*!
 * This is a number which is used to calculate the
 * beacon channel in case of frequency hopping.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_BEACON_NB_CHANNELS        1

/*!
 * This is a number which is used to calculate the
 * ping slot channel in case of frequency hopping.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_PING_SLOT_NB_CHANNELS     32

/*!
 * The first RX channel, downstream group 1.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_FIRST_RX1_CHANNEL         476900000

/*!
 * The last RX channel, downstream group 1.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_LAST_RX1_CHANNEL          483100000

/*!
 * The frequency stepwidth between RX channels,
 * downstream group 1.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_STEPWIDTH_RX1_CHANNEL     200000

/*!
 * The first RX channel, downstream group 2.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_FIRST_RX2_CHANNEL         496900000

/*!
 * The last RX channel, downstream group 2.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_LAST_RX2_CHANNEL          503100000

/*!
 * The frequency stepwidth between RX channels,
 * downstream group 2.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_STEPWIDTH_RX2_CHANNEL     200000

/*!
 * The first TX channel, upstream group 1.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_FIRST_TX1_CHANNEL         CN470_B20_FIRST_RX1_CHANNEL

/*!
 * The last TX channel, upstream group 1.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_LAST_TX1_CHANNEL          CN470_B20_LAST_RX1_CHANNEL

/*!
 * The frequency stepwidth between RX channels,
 * upstream group 1.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_STEPWIDTH_TX1_CHANNEL     CN470_B20_STEPWIDTH_RX1_CHANNEL

/*!
 * The first TX channel, upstream group 2.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_FIRST_TX2_CHANNEL         CN470_B20_FIRST_RX2_CHANNEL

/*!
 * The last TX channel, upstream group 2.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_LAST_TX2_CHANNEL          CN470_B20_LAST_RX2_CHANNEL

/*!
 * The frequency stepwidth between RX channels,
 * upstream group 2.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_STEPWIDTH_TX2_CHANNEL     CN470_B20_STEPWIDTH_RX2_CHANNEL

/*!
 * The default frequency for RX window 2, when its
 * an ABP device.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_RX_WND_2_FREQ_ABP         498300000

/*!
 * The channel plan frequencies for RX window 2,
 * when its an OTAA device.
 * Channel plan type B, 20MHz.
 */
#define CN470_B20_RX_WND_2_FREQ_OTAA        { 478300000, 498300000 }

/*!
 * \brief Calculation of the beacon frequency.
 *
 * \param [IN] channel The Beacon channel number.
 *
 * \param [IN] joinChannelIndex The join channel index.
 *
 * \param [IN] isPingSlot Set to true, if its a ping slot.
 *
 * \retval Returns the beacon frequency.
 */
uint32_t RegionCN470B20GetDownlinkFrequency( uint8_t channel, uint8_t joinChannelIndex, bool isPingSlot );

/*!
 * \brief Performs the update of the channelsMask based on the input parameters
 *        for the Channel plan type B, 20MHz.
 *
 * \param [IN] joinChannelIndex The join channel index.
 *
 * \retval Returns the offset for the given join channel.
 */
uint8_t RegionCN470B20GetBeaconChannelOffset( uint8_t joinChannelIndex );

/*!
 * \brief Performs the update of the channelsMask based on the input parameters
 *        for the Channel plan type B, 20MHz.
 *
 * \param [IN] channelsMask A pointer to the channels mask.
 *
 * \param [IN] chMaskCntl The value of the chMaskCntl field of the LinkAdrReq.
 *
 * \param [IN] chanMask The value of the chanMask field of the LinkAdrReq.
 *
 * \param [IN] channels A pointer to the available channels.
 *
 * \retval Status of the operation. Return 0x07 if the channels mask is valid.
 */
uint8_t RegionCN470B20LinkAdrChMaskUpdate( uint16_t* channelsMask, uint8_t chMaskCntl,
                                              uint16_t chanMask, ChannelParams_t* channels );

/*!
 * \brief Verifies if the frequency provided is valid
 *        for the Channel plan type B, 20MHz.
 *
 * \param [IN] frequency The frequency to verify.
 *
 * \retval Returns true, if the frequency is valid.
 */
bool RegionCN470B20VerifyRfFreq( uint32_t frequency );

/*!
 * \brief Initializes all channels, datarates, frequencies and bands
 *        for the Channel plan type B, 20MHz.
 *
 * \param [IN] channels A pointer to the available channels.
 */
void RegionCN470B20InitializeChannels( ChannelParams_t* channels );

/*!
 * \brief Initializes the channels default mask
 *        for the Channel plan type B, 20MHz.
 *
 * \param [IN] channelsDefaultMask A pointer to the channels default mask.
 */
void RegionCN470B20InitializeChannelsMask( uint16_t* channelsDefaultMask );

/*!
 * \brief Computes the frequency for the RX1 window
 *        for the Channel plan type B, 20MHz.
 *
 * \param [IN] channel The channel utilized currently.
 *
 * \retval Returns the frequency which shall be used.
 */
uint32_t RegionCN470B20GetRx1Frequency( uint8_t channel );

/*!
 * \brief Computes the frequency for the RX2 window
 *        for the Channel plan type B, 20MHz.
 *
 * \param [IN] joinChannelIndex The join channel index.
  *
 * \param [IN] isOtaaDevice Set to true, if the device is an OTAA device.
 *
 * \retval Returns the frequency which shall be used.
 */
uint32_t RegionCN470B20GetRx2Frequency( uint8_t joinChannelIndex, bool isOtaaDevice );

/*! \} defgroup REGIONCN470 */

#ifdef __cplusplus
}
#endif

#endif // __REGION_CN470_B20_H__
