/*!
 * \file  LoRaMacCryptoNvm.h
 *
 * \brief LoRa MAC layer cryptographic NVM data.
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
/*
 * \addtogroup LORAMAC
 * \{
 *
 */
#ifndef __LORAMAC_CRYPTO_NVM_H__
#define __LORAMAC_CRYPTO_NVM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "utilities.h"
#include "LoRaMacTypes.h"


/*!
 * LoRaWAN Frame counter list.
 */
typedef struct sFCntList
{
    /*!
     * Uplink frame counter which is incremented with each uplink.
     */
    uint32_t FCntUp;
    /*!
     * Network downlink frame counter which is incremented with each downlink on FPort 0
     * or when the FPort field is missing.
     */
    uint32_t NFCntDown;
    /*!
     * Application downlink frame counter which is incremented with each downlink
     * on a port different than 0.
     */
    uint32_t AFCntDown;
    /*!
     * In case if the device is connected to a LoRaWAN 1.0 Server,
     * this counter is used for every kind of downlink frame.
     */
    uint32_t FCntDown;
    /*!
     * Multicast downlink counters
     */
    uint32_t McFCntDown[LORAMAC_MAX_MC_CTX];
    /*!
     * RJcount1 is a counter incremented with every Rejoin request Type 1
     * frame transmitted.
     */
    uint16_t RJcount1;
}FCntList_t;

/*!
 * LoRaMac Crypto Non Volatile Context structure
 */
typedef struct sLoRaMacCryptoNvmData
{
    /*!
     * Stores the information if the device is connected to a LoRaWAN network
     * server with prior to 1.1.0 implementation.
     */
    Version_t LrWanVersion;
    /*!
     * Device nonce is a counter starting at 0 when the device is initially
     * powered up and incremented with every JoinRequest.
     */
    uint16_t DevNonce;
    /*!
     * JoinNonce is a device specific counter value (that never repeats itself)
     * provided by the join server and incremented with every JoinAccept message.
     */
    uint32_t JoinNonce;
    /*!
     * Frame counter list
     */
    FCntList_t FCntList;
    /*!
     * LastDownFCnt stores the information which frame counter was used to
     * decrypt the last frame. This information is needed to compute ConfFCnt in
     * B1 block for the MIC.
     */
    uint32_t LastDownFCnt;
    /*!
     * CRC32 value of the Crypto data structure.
     */
    uint32_t Crc32;
}LoRaMacCryptoNvmData_t;

/*! \} addtogroup LORAMAC */

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_CRYPTO_NVM_H__
