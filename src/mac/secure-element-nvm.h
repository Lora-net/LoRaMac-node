/*!
 * \file  secure-element-nvm.h
 *
 * \brief Secure Element non-volatile data.
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
 * \addtogroup  SECUREELEMENT
 *
 * \{
 *
 */
#ifndef __SECURE_ELEMENT_NVM_H__
#define __SECURE_ELEMENT_NVM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "LoRaMacTypes.h"

/*!
 * Secure-element keys size in bytes
 */
#define SE_KEY_SIZE             16

/*!
 * Secure-element EUI size in bytes
 */
#define SE_EUI_SIZE             8

/*!
 * Secure-element pin size in bytes
 */
#define SE_PIN_SIZE             4

#ifdef SOFT_SE
/*!
 * Number of supported crypto keys for the soft-se
 */
#define NUM_OF_KEYS             23

/*!
 * Key structure definition for the soft-se
 */
typedef struct sKey
{
    /*!
     * Key identifier
     */
    KeyIdentifier_t KeyID;
    /*!
     * Key value
     */
    uint8_t KeyValue[SE_KEY_SIZE];
} Key_t;
#endif

typedef struct sSecureElementNvCtx
{
    /*!
     * DevEUI storage
     */
    uint8_t DevEui[SE_EUI_SIZE];
    /*!
     * Join EUI storage
     */
    uint8_t JoinEui[SE_EUI_SIZE];
    /*!
     * Pin storage
     */
    uint8_t Pin[SE_PIN_SIZE];
#ifdef SOFT_SE
    /*!
     * The key list is required for the soft-se only. All other secure-elements
     * handle the storage on their own.
     */
    Key_t KeyList[NUM_OF_KEYS];
#endif
    /*!
     * CRC32 value of the SecureElement data structure.
     */
    uint32_t Crc32;
} SecureElementNvmData_t;


/*! \} addtogroup SECUREELEMENT */

#ifdef __cplusplus
}
#endif

#endif //  __SECURE_ELEMENT_NVM_H__
