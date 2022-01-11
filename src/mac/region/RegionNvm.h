/*!
 * \file  RegionNvm.h
 *
 * \brief Region independent non-volatile data.
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
 * \addtogroup REGIONCOMMON
 *
 * \{
 */
#ifndef __REGIONNVM_H__
#define __REGIONNVM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "LoRaMacTypes.h"

/*!
 * Channel plan for region CN470
 */
typedef enum eRegionCN470ChannelPlan
{
    CHANNEL_PLAN_UNKNOWN,
    CHANNEL_PLAN_20MHZ_TYPE_A,
    CHANNEL_PLAN_20MHZ_TYPE_B,
    CHANNEL_PLAN_26MHZ_TYPE_A,
    CHANNEL_PLAN_26MHZ_TYPE_B
}RegionCN470ChannelPlan_t;

// Selection of REGION_NVM_MAX_NB_CHANNELS
#if defined( REGION_CN470 )
    #define REGION_NVM_MAX_NB_CHANNELS                 96
#elif defined( REGION_US915 ) || defined( REGION_AU915 )
    #define REGION_NVM_MAX_NB_CHANNELS                 72
#elif defined( REGION_AS923 ) || defined( REGION_CN779 ) || \
      defined( REGION_EU433 ) || defined( REGION_EU868 ) || \
      defined( REGION_IN865 ) || defined( REGION_KR920 )
    #define REGION_NVM_MAX_NB_CHANNELS                 16
#else
    // Region_RU864
    #define REGION_NVM_MAX_NB_CHANNELS                 8
#endif

// Selection of REGION_NVM_MAX_NB_BANDS
#if defined( REGION_EU868 )
    #define REGION_NVM_MAX_NB_BANDS                    6
#else
    // All others
    #define REGION_NVM_MAX_NB_BANDS                    1
#endif

// Selection of REGION_NVM_CHANNELS_MASK_SIZE
#if defined( REGION_CN470 ) || defined( REGION_US915 ) || \
    defined( REGION_AU915 )
    #define REGION_NVM_CHANNELS_MASK_SIZE              6
#else
    // All others
    #define REGION_NVM_CHANNELS_MASK_SIZE              1
#endif

/*!
 * Region specific data which must be stored in the NVM.
 */
typedef struct sRegionNvmDataGroup1
{
#if defined( REGION_US915 ) || defined( REGION_AU915 ) || defined( REGION_CN470 )
    /*!
     * LoRaMac channels remaining
     */
    uint16_t ChannelsMaskRemaining[ REGION_NVM_CHANNELS_MASK_SIZE ];
#endif
#if defined( REGION_US915 ) || defined( REGION_AU915 )
    /*!
     * Index of current in use 8 bit group (0: bit 0 - 7, 1: bit 8 - 15, ...,
     * 7: bit 56 - 63)
     */
    uint8_t JoinChannelGroupsCurrentIndex;
    /*!
     * Counter of join trials needed to alternate between datarates.
     */
    uint8_t JoinTrialsCounter;
#endif
    /*!
     * CRC32 value of the Region data structure.
     */
    uint32_t Crc32;
}RegionNvmDataGroup1_t;

/*!
 * Region specific data which must be stored in the NVM.
 * Parameters which do not change very frequently.
 */
typedef struct sRegionNvmDataGroup2
{
    /*!
     * LoRaMAC channels
     */
    ChannelParams_t Channels[ REGION_NVM_MAX_NB_CHANNELS ];
    /*!
     * LoRaMac channels mask
     */
    uint16_t ChannelsMask[ REGION_NVM_CHANNELS_MASK_SIZE ];
    /*!
     * LoRaMac channels default mask
     */
    uint16_t ChannelsDefaultMask[ REGION_NVM_CHANNELS_MASK_SIZE ];
#if defined( REGION_CN470 )
    /*!
     * Holds the channel plan.
     */
    RegionCN470ChannelPlan_t ChannelPlan;
    /*!
     * Holds the common join channel, if its an OTAA device, otherwise
     * this value is 0.
     */
    uint8_t CommonJoinChannelIndex;
    /*!
     * Identifier which specifies if the device is an OTAA device. Set
     * to true, if its an OTAA device.
     */
    bool IsOtaaDevice;
#endif
    /*!
     * CRC32 value of the Region data structure.
     */
    uint32_t Crc32;
}RegionNvmDataGroup2_t;

/*! \} addtogroup REGIONCOMMON */

#ifdef __cplusplus
}
#endif

#endif // __REGIONNVM_H__
