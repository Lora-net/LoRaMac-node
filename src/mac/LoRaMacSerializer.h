/*!
 * \file  LoRaMacSerializer.h
 *
 * \brief LoRa MAC layer message serializer functionality implementation
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
 * \addtogroup LORAMAC
 * \{
 *
 */
#ifndef __LORAMAC_SERIALIZER_H__
#define __LORAMAC_SERIALIZER_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include "LoRaMacMessageTypes.h"


/*!
 * LoRaMac Serializer Status
 */
typedef enum eLoRaMacSerializerStatus
{
    /*!
     * No error occurred
     */
    LORAMAC_SERIALIZER_SUCCESS = 0,
    /*!
     * Null pointer exception
     */
    LORAMAC_SERIALIZER_ERROR_NPE,
    /*!
     * Incompatible buffer size
     */
    LORAMAC_SERIALIZER_ERROR_BUF_SIZE,
    /*!
     * Undefined Error occurred
     */
    LORAMAC_SERIALIZER_ERROR,
}LoRaMacSerializerStatus_t;

/*!
 * Creates serialized MAC message of structured object.
 *
 * \param[IN/OUT] macMsg        - Join-request message object
 * \retval                      - Status of the operation
 */
LoRaMacSerializerStatus_t LoRaMacSerializerJoinRequest( LoRaMacMessageJoinRequest_t* macMsg );

/*!
 * Creates serialized MAC message of structured object.
 *
 * \param[IN/OUT] macMsg        - Join-request message object
 * \retval                      - Status of the operation
 */
LoRaMacSerializerStatus_t LoRaMacSerializerReJoinType1( LoRaMacMessageReJoinType1_t* macMsg );

/*!
 * Creates serialized MAC message of structured object.
 *
 * \param[IN/OUT] macMsg        - Join-request message object
 * \retval                      - Status of the operation
 */
LoRaMacSerializerStatus_t LoRaMacSerializerReJoinType0or2( LoRaMacMessageReJoinType0or2_t* macMsg );

/*!
 * Creates serialized MAC message of structured object.
 *
 * \param[IN/OUT] macMsg        - Data message object
 * \retval                      - Status of the operation
 */
LoRaMacSerializerStatus_t LoRaMacSerializerData( LoRaMacMessageData_t* macMsg );

/*! \} addtogroup LORAMAC */

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_SERIALIZER_H__

