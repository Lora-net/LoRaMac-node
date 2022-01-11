/*!
 * \file  LoRaMacClassBNvm.h
 *
 * \brief LoRa MAC Class B non-volatile data.
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
 * \addtogroup LORAMACCLASSB
 *
 * \{
 */
#ifndef __LORAMACCLASSBNVM_H__
#define __LORAMACCLASSBNVM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/*!
 * LoRaMac Class B Context structure for NVM parameters
 * related to ping slots
 */
typedef struct sLoRaMacClassBPingSlotNvmData
{
    struct sPingSlotCtrlNvm
    {
        /*!
         * Set when the server assigned a ping slot to the node
         */
        uint8_t Assigned         : 1;
        /*!
         * Set when a custom frequency is used
         */
        uint8_t CustomFreq       : 1;
    }Ctrl;
    /*!
     * Number of ping slots
     */
    uint8_t PingNb;
    /*!
     * Period of the ping slots
     */
    uint16_t PingPeriod;
    /*!
     * Reception frequency of the ping slot windows
     */
    uint32_t Frequency;
    /*!
     * Datarate of the ping slot
     */
    int8_t Datarate;
    /*!
     * Set to 1, if the FPending bit is set
     */
    uint8_t FPendingSet;
} LoRaMacClassBPingSlotNvmData_t;

/*!
 * LoRaMac Class B Context structure for NVM parameters
 * related to beaconing
 */
typedef struct sLoRaMacClassBBeaconNvmData
{
    struct sBeaconCtrlNvm
    {
        /*!
         * Set if the node has a custom frequency for beaconing and ping slots
         */
        uint8_t CustomFreq          : 1;
    }Ctrl;
    /*!
     * Beacon reception frequency
     */
    uint32_t Frequency;
} LoRaMacClassBBeaconNvmData_t;

/*!
 * LoRaMac Class B Context structure
 */
typedef struct sLoRaMacClassBNvmData
{
    /*!
     * Class B ping slot context
     */
    LoRaMacClassBPingSlotNvmData_t PingSlotCtx;
    /*!
     * Class B beacon context
     */
    LoRaMacClassBBeaconNvmData_t BeaconCtx;
    /*!
     * CRC32 value of the ClassB data structure.
     */
    uint32_t Crc32;
} LoRaMacClassBNvmData_t;

#ifdef __cplusplus
}
#endif

#endif // __LORAMACCLASSBNVM_H__
