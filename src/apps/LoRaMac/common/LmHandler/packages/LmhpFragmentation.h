/*!
 * \file  LmhpFragmentation.h
 *
 * \brief Implements the LoRa-Alliance fragmented data block transport package
 *        Specification: https://lora-alliance.org/sites/default/files/2018-09/fragmented_data_block_transport_v1.0.0.pdf
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
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
#ifndef __LMHP_FRAGMENTATION_H__
#define __LMHP_FRAGMENTATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "LoRaMac.h"
#include "LmHandlerTypes.h"
#include "LmhPackage.h"
#include "FragDecoder.h"

/*!
 * Fragmentation data block transport package identifier.
 *
 * \remark This value must be unique amongst the packages
 */
#define PACKAGE_ID_FRAGMENTATION                    3

/*!
 * Fragmentation package parameters
 */
typedef struct LmhpFragmentationParams_s
{
#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
    /*!
     * FragDecoder Write/Read function callbacks
     */
    FragDecoderCallbacks_t DecoderCallbacks;
#else
    /*!
     * Pointer to the un-fragmented received buffer.
     */
    uint8_t *Buffer;
    /*!
     * Size of the un-fragmented received buffer.
     */
    uint32_t BufferSize;
#endif
    /*!
     * Notifies the progress of the current fragmentation session
     *
     * \param [IN] fragCounter Fragment counter
     * \param [IN] fragNb      Number of fragments
     * \param [IN] fragSize    Size of fragments
     * \param [IN] fragNbLost  Number of lost fragments
     */
    void ( *OnProgress )( uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost );
#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
    /*!
     * Notifies that the fragmentation session is finished
     *
     * \param [IN] status Fragmentation session status [FRAG_SESSION_ONGOING,
     *                                                  FRAG_SESSION_FINISHED or
     *                                                  FragDecoder.Status.FragNbLost]
     * \param [IN] size   Received file size
     */
    void ( *OnDone )( int32_t status, uint32_t size );
#else
    /*!
     * Notifies that the fragmentation session is finished
     *
     * \param [IN] status Fragmentation session status [FRAG_SESSION_ONGOING,
     *                                                  FRAG_SESSION_FINISHED or
     *                                                  FragDecoder.Status.FragNbLost]
     * \param [IN] file   Pointer to the reception file buffer
     * \param [IN] size   Received file size
     */
    void ( *OnDone )( int32_t status, uint8_t *file, uint32_t size );
#endif
}LmhpFragmentationParams_t;

LmhPackage_t *LmhpFragmentationPackageFactory( void );

#ifdef __cplusplus
}
#endif

#endif // __LMHP_FRAGMENTATION_H__
