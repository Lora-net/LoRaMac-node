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


#ifndef CRYS_RSA_PRIM_H
#define CRYS_RSA_PRIM_H

#include "crys_rsa_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module defines the API that implements the [PKCS1_2.1] primitive functions.

\note Direct use of primitive functions, rather than schemes to protect data, is strongly discouraged as primitive functions are
susceptible to well-known attacks.
*/



/**********************************************************************************/
/*!
@brief Implements the RSAEP algorithm, as defined in [PKCS1_2.1] - 6.1.1.

@return CRYS_OK on success.
@return A non-zero value from crys_rsa_error.h on failure.
 */
CIMPORT_C CRYSError_t CRYS_RSA_PRIM_Encrypt(
                                CRYS_RSAUserPubKey_t *UserPubKey_ptr,       /*!< [in]  Pointer to the public key data structure. */
                                CRYS_RSAPrimeData_t  *PrimeData_ptr,        /*!< [in]  Pointer to a temporary structure containing internal buffers. */
                                uint8_t              *Data_ptr,             /*!< [in]  Pointer to the data to encrypt. */
                                uint16_t              DataSize,             /*!< [in]  The size (in bytes) of the data to encrypt. Data size must be &le; Modulus size.
										       It can be smaller than the modulus size but it is not recommended.
                                                                                       If smaller, the data is zero-padded up to the modulus size.
                                                                                       Since the result of decryption is always the size of the modulus,
                                                                                       this will cause the size of the decrypted data to be larger than the
										       originally encrypted data. */
                                uint8_t              *Output_ptr            /*!< [out] Pointer to the encrypted data. The buffer size must be &ge; the modulus size. */
);


/**********************************************************************************/
/*!
@brief Implements the RSADP algorithm, as defined in [PKCS1_2.1] - 6.1.2.

@return CRYS_OK on success.
@return A non-zero value from crys_rsa_error.h on failure.

*/
CIMPORT_C CRYSError_t CRYS_RSA_PRIM_Decrypt(
                            CRYS_RSAUserPrivKey_t *UserPrivKey_ptr,     /*!< [in]  Pointer to the private key data structure.
                                                                                    The representation (pair or quintuple) and hence the algorithm (CRT or not-CRT)
                                                                                    is determined by the Private Key data structure - using
										    ::CRYS_RSA_Build_PrivKey or ::CRYS_RSA_Build_PrivKeyCRT
                                                                                    to determine which algorithm is used.*/
                            CRYS_RSAPrimeData_t   *PrimeData_ptr,       /*!< [in]  Pointer to a temporary structure containing internal buffers required for
										   the RSA operation. */
                            uint8_t     *Data_ptr,                      /*!< [in]  Pointer to the data to be decrypted. */
                            uint16_t     DataSize,                      /*!< [in]  The size (in bytes) of the data to decrypt. Must be equal to the modulus size. */
                            uint8_t     *Output_ptr                     /*!< [out] Pointer to the decrypted data. The buffer size must be &le; the modulus size. */
);


/*!
@brief Implements the RSASP1 algorithm, as defined in [PKCS1_2.1] - 6.2.1, as a call to ::CRYS_RSA_PRIM_Decrypt,
since the signature primitive is identical to the decryption primitive.
*/
#define CRYS_RSA_PRIM_Sign CRYS_RSA_PRIM_Decrypt

/*!
@brief Implements the RSAVP1 algorithm, as defined in [PKCS1_2.1] - 6.2.2, as a call to ::CRYS_RSA_PRIM_Encrypt.
*/
#define CRYS_RSA_PRIM_Verify CRYS_RSA_PRIM_Encrypt

#ifdef __cplusplus
}
#endif

#endif
