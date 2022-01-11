/*!
 * \file  LoRaMacClassBConfig.h
 *
 * \brief LoRa MAC Class B configuration
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
 * \defgroup  LORAMACCLASSB LoRa MAC Class B configuration
 *            This header file contains parameters to configure the class b operation.
 *            By default, all parameters are set according to the specification.
 * \{
 */
#ifndef __LORAMACCLASSBCONFIG_H__
#define __LORAMACCLASSBCONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * Defines the beacon interval in ms
 */
#define CLASSB_BEACON_INTERVAL                      128000

/*!
 * Beacon reserved time in ms
 */
#define CLASSB_BEACON_RESERVED                      2120

/*!
 * Beacon guard time in ms
 */
#define CLASSB_BEACON_GUARD                         3000

/*!
 * Beacon window time in ms
 */
#define CLASSB_BEACON_WINDOW                        122880

/*!
 * Beacon window time in numer of slots
 */
#define CLASSB_BEACON_WINDOW_SLOTS                  4096

/*!
 * Ping slot length time in ms
 */
#define CLASSB_PING_SLOT_WINDOW                     30

/*!
 * Maximum allowed beacon less time in ms
 */
#define CLASSB_MAX_BEACON_LESS_PERIOD               7200000

/*!
 * Delay time for the BeaconTimingAns in ms
 */
#define CLASSB_BEACON_DELAY_BEACON_TIMING_ANS       30

/*!
 * Default symbol timeout for beacons and ping slot windows
 */
#define CLASSB_BEACON_SYMBOL_TO_DEFAULT             8

/*!
 * Maximum symbol timeout for beacons
 */
#define CLASSB_BEACON_SYMBOL_TO_EXPANSION_MAX       255

/*!
 * Maximum symbol timeout for ping slots
 */
#define CLASSB_PING_SLOT_SYMBOL_TO_EXPANSION_MAX    30

/*!
 * Symbol expansion value for beacon windows in case of beacon
 * loss in symbols
 */
#define CLASSB_BEACON_SYMBOL_TO_EXPANSION_FACTOR    2

/*!
 * Defines the default window movement time
 */
#define CLASSB_WINDOW_MOVE_DEFAULT                  2

/*!
 * Defines the maximum time for the beacon movement
 */
#define CLASSB_WINDOW_MOVE_EXPANSION_MAX            256

/*!
 * Defines the expansion factor for the beacon movement
 */
#define CLASSB_WINDOW_MOVE_EXPANSION_FACTOR         2

#ifdef __cplusplus
}
#endif

#endif // __LORAMACCLASSBCONFIG_H__
