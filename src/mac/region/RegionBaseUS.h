/*!
 * \file  RegionBaseUS.h
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
/*!
 * \defgroup  REGIONBASEUS US region common implementations.
 * \{
 */
#ifndef __REGIONBASEUS_H__
#define __REGIONBASEUS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "LoRaMac.h"

/*!
 * \brief Computes the next 125kHz channel used for join requests.
 *        And it returns all the parameters updated.
 *
 * \param [IN]  channelsMaskRemaining pointer to remaining channels.
 *
 * \param [IN]  groupsCurrentIndex Index of current channel.
 *
 * \param [OUT] newChannelIndex Index of next available channel.
 *
 * \retval Status
 */
LoRaMacStatus_t RegionBaseUSComputeNext125kHzJoinChannel( uint16_t* channelsMaskRemaining,
                                                          uint8_t* groupsCurrentIndex, uint8_t* newChannelIndex );

/*!
 * \brief Verifies if the frequency is in the correct range with a
 *        specific stepwidth.
 *
 * \param [IN]  freq Frequency to verify.
 *
 * \param [IN]  minFreq Minimum frequency.
 *
 * \param [IN]  maxFreq Maximum frequency.
 *
 * \param [IN]  stepwidth Frequency stepwidth.
 *
 * \retval True, if the frequency is valid, false if not.
 */
bool RegionBaseUSVerifyFrequencyGroup( uint32_t freq, uint32_t minFreq, uint32_t maxFreq, uint32_t stepwidth );

/*!
 * \brief Calculates the downlink frequency for a given channel. This
 *        function is used in class B only.
 *
 * \param [IN] channel The channel according to the channel plan.
 *
 * \param [IN] frequency The base frequency.
 *
 * \param [IN] stepwidth The frequency stepwidth.
 *
 * \retval The downlink frequency.
 */
uint32_t RegionBaseUSCalcDownlinkFrequency( uint8_t channel, uint32_t frequency,
                                            uint32_t stepwidth );

/*! \} defgroup REGIONBASEUS */

#ifdef __cplusplus
}
#endif

#endif // __REGIONBASEUS_H__
