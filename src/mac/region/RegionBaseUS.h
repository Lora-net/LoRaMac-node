/*!
 * \file      RegionBaseUS.h
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
 *
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
