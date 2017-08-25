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



#ifndef CRYS_RSA_KG_H
#define CRYS_RSA_KG_H

#include "crys_rsa_types.h"
#include "crys_rnd.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief Generates a RSA pair of public and private keys.
*/

/************************ Defines ******************************/

/* Max allowed size and values of public exponent for key generation in CRYS */
#define CRYS_RSA_KG_PUB_EXP_MAX_SIZE_BITS    17
#define CRYS_RSA_KG_PUB_EXP_ALLOW_VAL_1      0x000003
#define CRYS_RSA_KG_PUB_EXP_ALLOW_VAL_2      0x000011
#define CRYS_RSA_KG_PUB_EXP_ALLOW_VAL_3      0x010001




/***********************************************************************************************/

/*!
@brief CRYS_RSA_KG_GenerateKeyPair generates a Pair of public and private keys on non CRT mode according to [ANS X9.31].

\note    To be FIPS 186-4 [5.1] compliant use only the following key sizes (in bits): 1024, 2048 and 3072.

@return CRYS_OK on success.
@return A non-zero value from crys_rsa_error.h on failure.

*/
CIMPORT_C CRYSError_t CRYS_RSA_KG_GenerateKeyPair(
                                        CRYS_RND_Context_t *rndContext_ptr,         /*!< [in/out] Pointer to the RND context buffer. */
                                        uint8_t             *pubExp_ptr,            /*!< [in]  The pointer to the public exponent (public key). */
                                        uint16_t             pubExpSizeInBytes,     /*!< [in]  The public exponent size in bytes. */
                                        uint32_t             keySize,               /*!< [in]  The size of the key, in bits. Supported sizes are
											       512, 1024, 2048, 3072 and 4096 bit. */
                                        CRYS_RSAUserPrivKey_t *userPrivKey_ptr,     /*!< [out] Pointer to the private key structure. */
                                        CRYS_RSAUserPubKey_t  *userPubKey_ptr,      /*!< [out] Pointer to the public key structure. */
					CRYS_RSAKGData_t      *keyGenData_ptr,      /*!< [in]  Pointer to a temporary structure required for the KeyGen operation. */
                                        CRYS_RSAKGFipsContext_t    *pFipsCtx        /*!< [in]  Pointer to temporary buffer used in case FIPS certification if required. */
);

/***********************************************************************************************/
/*!
@brief Generates a pair of public and private keys on CRT mode according to [ANS X9.31].

\note    To be FIPS 186-4 [5.1] compliant use only the following key sizes (in bits): 1024, 2048 and 3072.

@return CRYS_OK on success.
@return A non-zero value from crys_rsa_error.h on failure.
*/

CIMPORT_C CRYSError_t CRYS_RSA_KG_GenerateKeyPairCRT(
                                        CRYS_RND_Context_t *rndContext_ptr,        /*!< [in/out] Pointer to the RND context buffer. */
                                        uint8_t             *pubExp_ptr,           /*!< [in]  The pointer to the public exponent (public key). */
                                        uint16_t             pubExpSizeInBytes,    /*!< [in]  The public exponent size in bytes. */
                                        uint32_t             keySize,              /*!< [in]  The size of the key, in bits. Supported sizes are
                                                                                              512, 1024, 2048, 3072 and 4096 bit. */
                                        CRYS_RSAUserPrivKey_t *userPrivKey_ptr,    /*!< [out] Pointer to the private key structure. */
                                        CRYS_RSAUserPubKey_t  *userPubKey_ptr,     /*!< [out] Pointer to the public key structure. */
                                        CRYS_RSAKGData_t      *keyGenData_ptr,     /*!< [in] Pointer to a temporary structure required for the KeyGen operation. */
                                        CRYS_RSAKGFipsContext_t    *pFipsCtx       /*!< [in] Pointer to temporary buffer used in case FIPS certification if required. */
);


#ifdef __cplusplus
}
#endif

#endif


