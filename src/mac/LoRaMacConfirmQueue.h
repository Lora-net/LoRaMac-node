/*!
 * \file  LoRaMacConfirmQueue.h
 *
 * \brief LoRa MAC confirm queue implementation
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
 * \defgroup  LORAMACCONFIRMQUEUE LoRa MAC confirm queue implementation
 *            This module specifies the API implementation of the LoRaMAC confirm queue.
 *            The confirm queue is implemented with as a ring buffer. The number of
 *            elements can be defined with \ref LORA_MAC_MLME_CONFIRM_QUEUE_LEN. The
 *            current implementation does not support multiple elements of the same
 *            Mlme_t type.
 * \{
 */
#ifndef __LORAMAC_CONFIRMQUEUE_H__
#define __LORAMAC_CONFIRMQUEUE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#include "LoRaMac.h"

/*!
 * LoRaMac MLME-Confirm queue length
 */
#define LORA_MAC_MLME_CONFIRM_QUEUE_LEN             5

/*!
 * Structure to hold multiple MLME request confirm data
 */
typedef struct sMlmeConfirmQueue
{
    /*!
     * Holds the previously performed MLME-Request
     */
    Mlme_t Request;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
    /*!
     * Set to true, if the request is ready to be handled
     */
    bool ReadyToHandle;
    /*!
     * Set to true, if it is not permitted to set the ReadyToHandle variable
     * with a function call to LoRaMacConfirmQueueSetStatusCmn.
     */
    bool RestrictCommonReadyToHandle;
}MlmeConfirmQueue_t;

/*!
 * \brief   Initializes the confirm queue
 *
 * \param   [IN] primitives - Pointer to the LoRaMac primitives.
 */
void LoRaMacConfirmQueueInit( LoRaMacPrimitives_t* primitive );

/*!
 * \brief   Adds an element to the confirm queue.
 *
 * \param   [IN] mlmeConfirm - Pointer to the element to add.
 *
 * \retval  [true - operation was successful, false - operation failed]
 */
bool LoRaMacConfirmQueueAdd( MlmeConfirmQueue_t* mlmeConfirm );

/*!
 * \brief   Removes the last element which was added into the queue.
 *
 * \retval  [true - operation was successful, false - operation failed]
 */
bool LoRaMacConfirmQueueRemoveLast( void );

/*!
 * \brief   Removes the first element which was added to the confirm queue.
 *
 * \retval  [true - operation was successful, false - operation failed]
 */
bool LoRaMacConfirmQueueRemoveFirst( void );

/*!
 * \brief   Sets the status of an element.
 *
 * \param   [IN] status - The status to set.
 *
 * \param   [IN] request - The related request to set the status.
 */
void LoRaMacConfirmQueueSetStatus( LoRaMacEventInfoStatus_t status, Mlme_t request );

/*!
 * \brief   Gets the status of an element.
 *
 * \param   [IN] request - The request to query the status.
 *
 * \retval  The status of the related MlmeRequest.
 */
LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatus( Mlme_t request );

/*!
 * \brief   Sets a common status for all elements in the queue.
 *
 * \param   [IN] status - The status to set.
 */
void LoRaMacConfirmQueueSetStatusCmn( LoRaMacEventInfoStatus_t status );

/*!
 * \brief   Gets the common status of all elements.
 *
 * \retval  The common status of all elements.
 */
LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatusCmn( void );

/*!
 * \brief   Verifies if a request is in the queue and active.
 *
 * \param   [IN] request - The request to verify.
 *
 * \retval  [true - element is in the queue, false - element is not in the queue].
 */
bool LoRaMacConfirmQueueIsCmdActive( Mlme_t request );

/*!
 * \brief   Handles all callbacks of active requests
 *
 * \param   [IN] mlmeConfirm - Pointer to the generic mlmeConfirm structure.
 */
void LoRaMacConfirmQueueHandleCb( MlmeConfirm_t* mlmeConfirm );

/*!
 * \brief   Query number of elements in the queue.
 *
 * \retval  Number of elements.
 */
uint8_t LoRaMacConfirmQueueGetCnt( void );

/*!
 * \brief   Verify if the confirm queue is full.
 *
 * \retval  [true - queue is full, false - queue is not full].
 */
bool LoRaMacConfirmQueueIsFull( void );

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_CONFIRMQUEUE_H__
