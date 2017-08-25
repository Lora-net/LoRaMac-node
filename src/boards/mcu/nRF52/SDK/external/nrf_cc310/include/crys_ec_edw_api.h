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


#ifndef CRYS_EC_EDW_API_H
#define CRYS_EC_EDW_API_H

#include "ssi_pal_types.h"
#include "crys_hash.h"
#include "crys_rnd.h"
#include "crys_pka_defs_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
@file
@brief This file contains the CRYS APIs used for EC EDW (Edwards) ed25519 algorithms.

\note  Algorithms of Montgomery and Edwards elliptic curves cryptography were developed by
       Daniel.J.Bernstein.
*/

/*! EC Edwards ed25519 modulus and order sizes in bits, words and bytes  */
#define CRYS_ECEDW_MOD_SIZE_IN_BITS  255U   /*!<\internal MOD - EC Edw modulus size*/
#define CRYS_ECEDW_ORD_SIZE_IN_BITS  255U   /*!<\internal ORD - EC Edw generator order size*/
#define CRYS_ECEDW_MOD_SIZE_IN_32BIT_WORDS \
        ((CRYS_ECEDW_MOD_SIZE_IN_BITS + SASI_BITS_IN_32BIT_WORD - 1) / SASI_BITS_IN_32BIT_WORD)
#define CRYS_ECEDW_MOD_SIZE_IN_BYTES  (CRYS_ECEDW_MOD_SIZE_IN_32BIT_WORDS * SASI_32BIT_WORD_SIZE)
#define CRYS_ECEDW_ORD_SIZE_IN_32BIT_WORDS \
        ((CRYS_ECEDW_ORD_SIZE_IN_BITS + SASI_BITS_IN_32BIT_WORD - 1) / SASI_BITS_IN_32BIT_WORD)
#define CRYS_ECEDW_ORD_SIZE_IN_BYTES  (CRYS_ECEDW_ORD_SIZE_IN_32BIT_WORDS * SASI_32BIT_WORD_SIZE)

/*! Constant sizes of special EC_MONT buffers and arrays  */
#define CRYS_ECEDW_SEED_BYTES       CRYS_ECEDW_MOD_SIZE_IN_BYTES
#define CRYS_ECEDW_SECRET_KEY_BYTES (2 * CRYS_ECEDW_MOD_SIZE_IN_BYTES)
#define CRYS_ECEDW_SIGNATURE_BYTES  (2 * CRYS_ECEDW_ORD_SIZE_IN_BYTES)
#define CRYS_ECEDW_SCALAR_BYTES     CRYS_ECEDW_ORD_SIZE_IN_BYTES
#define CRYS_ECEDW_SCALARMULT_BYTES CRYS_ECEDW_MOD_SIZE_IN_BYTES

/*! EC_EDW  temp buffer size definition */
#define CRYS_ECEDW_TEMP_BUFF_SIZE_IN_32BIT_WORD \
     (10*CRYS_EC_MONT_EDW_MODULUS_MAX_SIZE_IN_WORDS + \
     (sizeof(CRYS_HASHUserContext_t)+SASI_32BIT_WORD_SIZE-1)/SASI_32BIT_WORD_SIZE)

/*! EC_EDW temp buffer type definition */
typedef struct {
	uint32_t buff[CRYS_ECEDW_TEMP_BUFF_SIZE_IN_32BIT_WORD];
} CRYS_ECEDW_TempBuff_t;


/******************************************************************************/
/*!
@brief The function creates EC Edwards signature on the message.
\note Used detached form of signature, separated from the message.
      Implemented algorithm of Bernstein D. etc. sign ed25519.

@return CRYS_OK on success,
@return A non-zero value on failure as defined crys_ec_mont_edw_error.h.
*/
CIMPORT_C CRYSError_t CRYS_ECEDW_Sign (
				      uint8_t       *pSign,                /*!< [out] Pointer to the detached signature. */
				      size_t        *pSignSize,            /*!< [in/out] Pointer to the total size of the signature ;
										    In  - the buffer size, which (must be at least 2*EC order size);
										    Out - the actual size of output data. */
				      const uint8_t *pMsg,                 /*!< [in] Pointer to the message. */
				      size_t         msgSize,              /*!< [in] Message size in bytes: must be less, than
										    (CRYS_HASH_UPDATE_DATA_MAX_SIZE_IN_BYTES - 2*(EC_EDW modulus size)). */
				      const uint8_t *pSignSecrKey,         /*!< [in] Pointer to the signer secret key (seed || pulKey) */
				      size_t         secrKeySize,          /*!< [in] Size of signer secret key in bytes: (must be 2*EC order size). */
				      CRYS_ECEDW_TempBuff_t *pTempBuff);   /*!< [in] Pointer to the temp buffer. */



/******************************************************************************/
/*!
@brief The function verifies the EC Edwards ed25519 signature on the message.
\note The input signature is in detached form, i.e. separated from the message.

@return CRYS_OK on success,
@return A non-zero value on failure as defined crys_ec_mont_edw_error.h.
*/
CIMPORT_C CRYSError_t CRYS_ECEDW_Verify(
				       const uint8_t *pSign,                /*!< [in] Pointer to detached signature, i.e. the
										 signature is separated from the message. */
				       size_t         signSize,             /*!< [in] Size of the signature in bytes, it must be
										 equal to two EC Order size in bytes. */
				       const uint8_t *pSignPublKey,         /*!< [in] Pointer to signer public key. */
				       size_t         publKeySize,          /*!< [in] Size of the signer public key in bytes; must be
										 equal to EC modulus size. */
				       uint8_t       *pMsg,                 /*!< [in] Pointer to the message. */
				       size_t         msgSize,              /*!< [in] Pointer to the message size in bytes. Must be less than
										 (CRYS_HASH_UPDATE_DATA_MAX_SIZE_IN_BYTES - 2*(EC_EDW modulus size)). */
				       CRYS_ECEDW_TempBuff_t *pTempBuff);   /*!< [in] Pointer to the temp buffer, for internal use. */


/******************************************************************************/
/*!
@brief The function randomly generates Ec ed25519 private and public keys
       using given seed.
       The generation is performed using EC Edwards ed25519 algorithm.

@return CRYS_OK on success,
@return A non-zero value on failure as defined crys_ec_mont_edw_error.h.
*/
CIMPORT_C CRYSError_t CRYS_ECEDW_SeedKeyPair (
					     const uint8_t *pSeed,                  /*!< [in] Pointer to the given seed. */
					     size_t         seedSize,               /*!< [in] Size of the seed in bytes, must be equal the EC order size
											 in bytes. */
					     uint8_t       *pSecrKey,               /*!< [out] Pointer to the secret key, including the seed, concatenated
											 with the public key. */
					     size_t        *pSecrKeySize,           /*!< [in/out] Pointer to the size of the secret key buffer in bytes
											 (must be at least 2*EC order size). */
					     uint8_t       *pPublKey,               /*!< [out] Pointer to the public key. */
					     size_t        *pPublKeySize,           /*!< [in/out] Pointer to the size of the public key in bytes.
											    In  - the size of buffer must be at least EC modulus size;
											    Out - the actual size. */
					     CRYS_ECEDW_TempBuff_t *pTempBuff);     /*!< [in] Pointer to the temp buffer, for internal use. */

/*******************************************************************/
/*!
 @brief The function randomly generates the EC Edwards ed25519 private and
	public keys.
	The generation is performed using EC Edwards ed25519 algorithm.

@return CRYS_OK on success,
@return A non-zero value on failure as defined crys_ec_mont_edw_error.h.
*/
CIMPORT_C CRYSError_t CRYS_ECEDW_KeyPair (
					 uint8_t       *pSecrKey,               /*!< [out] Pointer to the secret key (including seed and public key). */
					 size_t        *pSecrKeySize,           /*!< [in/out] Pointer to the size of the secret key in bytes,
										      (must be at least 2*EC order size). */
					 uint8_t       *pPublKey,               /*!< [out] Pointer to the public key. */
					 size_t        *pPublKeySize,           /*!< [in/out] - Pointer to the size of the public key in bytes.
											In  - the size of buffer must be at least EC modulus size;
											Out - the actual size. */
					 CRYS_RND_Context_t *pRndContext,       /*!< [in/out] Pointer to the RND context buffer. */
					 CRYS_ECEDW_TempBuff_t *pTempBuff);     /*!< [in] Pointer to the temp buffer. */


#ifdef __cplusplus
}
#endif

#endif



