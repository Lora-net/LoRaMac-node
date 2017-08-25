// Copyright (c) 2016-2017, ARM Limited or its affiliates. All rights reserved 
// 
// This file and the related binary are licensed under the ARM Object Code and 
// Headers License; you may not use these files except in compliance with this 
// license. 
// 
// You may obtain a copy of the License at <.../external/nrf_cc310/license.txt> 
// 
// See the License for the specific language governing permissions and 
// limitations under the License.


#ifndef CRYS_ECPKI_ECDSA_H
#define CRYS_ECPKI_ECDSA_H

/*!
@file
@brief Defines the APIs that support the ECDSA functions.
*/

#include "crys_error.h"
#include "crys_ecpki_types.h"
#include "crys_hash.h"
#include "crys_rnd.h"

#ifdef __cplusplus
extern "C"
{
#endif



/**************************************************************************
 *	              CRYS_ECDSA_Sign - integrated function
 **************************************************************************/
/*!
@brief This function performs an ECDSA sign operation in integrated form.

\note
Using of HASH functions with HASH size greater than EC modulus size, is not recommended!.
Algorithm according [ANS X9.62] standard

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_ecpki_error.h.
**/
CIMPORT_C CRYSError_t CRYS_ECDSA_Sign(
				     CRYS_RND_Context_t             *pRndContext,        /*!< [in/out] Pointer to the RND context buffer. */
				     CRYS_ECDSA_SignUserContext_t   *pSignUserContext,   /*!< [in/out] Pointer to the user buffer for signing the database. */
				     CRYS_ECPKI_UserPrivKey_t       *pSignerPrivKey,    /*!< [in]  A pointer to a user private key structure. */
				     CRYS_ECPKI_HASH_OpMode_t        hashMode,           /*!< [in]  One of the supported SHA-x HASH modes, as defined in
												    ::CRYS_ECPKI_HASH_OpMode_t.
												    \note MD5 is not supported. */
				     uint8_t                         *pMessageDataIn,    /*!< [in] Pointer to the input data to be signed.
												   The size of the scatter/gather list representing the data buffer
												   is limited to 128 entries, and the size of each entry is limited
												   to 64KB (fragments larger than 64KB are broken into
												   fragments <= 64KB). */
				     uint32_t                        messageSizeInBytes, /*!< [in]  Size of message data in bytes. */
				     uint8_t                        *pSignatureOut,      /*!< [in]  Pointer to a buffer for output of signature. */
				     uint32_t                       *pSignatureOutSize   /*!< [in/out] Pointer to the signature size. Used to pass the size of
												       the SignatureOut buffer (in), which must be >= 2
												       * OrderSizeInBytes. When the API returns,
												       it is replaced with the size of the actual signature (out). */
				     );



/**************************************************************************
 *	              CRYS_ECDSA_Verify integrated function
 **************************************************************************/
/*!
@brief This function performs an ECDSA verify operation in integrated form.
Algorithm according [ANS X9.62] standard

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_ecpki_error.h.
*/
CIMPORT_C CRYSError_t CRYS_ECDSA_Verify (
					CRYS_ECDSA_VerifyUserContext_t *pVerifyUserContext, /*!< [in] Pointer to the user buffer for signing the database. */
					CRYS_ECPKI_UserPublKey_t       *pUserPublKey,       /*!< [in] Pointer to a user public key structure. */
					CRYS_ECPKI_HASH_OpMode_t        hashMode,           /*!< [in] One of the supported SHA-x HASH modes, as defined in
												      ::CRYS_ECPKI_HASH_OpMode_t.
												      \note MD5 is not supported. */
					uint8_t                        *pSignatureIn,       /*!< [in] Pointer to the signature to be verified. */
					uint32_t                        SignatureSizeBytes, /*!< [in] Size of the signature (in bytes).  */
					uint8_t                        *pMessageDataIn,     /*!< [in] Pointer to the input data that was signed (same as given to
												      the signing function). The size of the scatter/gather list representing
												      the data buffer is limited to 128 entries, and the size of each entry is
												      limited to 64KB (fragments larger than 64KB are broken into fragments <= 64KB). */
					uint32_t                        messageSizeInBytes  /*!< [in] Size of the input data (in bytes). */
					);


/**********************************************************************************************************/


#ifdef __cplusplus
}
#endif

#endif
