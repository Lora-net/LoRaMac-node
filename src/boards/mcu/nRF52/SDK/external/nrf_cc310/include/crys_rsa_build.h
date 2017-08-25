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


#ifndef CRYS_RSA_BUILD_H
#define CRYS_RSA_BUILD_H


#include "crys_error.h"
#include "crys_rsa_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module defines some utility functions for working with RSA cryptography.
*/

/******************************************************************************************/
/*!
@brief Builds a ::CRYSRSAPubKey_t public key structure with the provided modulus and exponent.

@return CRYS_OK on success.
@return A non-zero value from crys_rsa_error.h on failure.
*/
CIMPORT_C CRYSError_t CRYS_RSA_Build_PubKey(
                                    CRYS_RSAUserPubKey_t *UserPubKey_ptr,   /*!< [out] Pointer to the public key structure. */
                                    uint8_t *Exponent_ptr,                  /*!< [in]  Pointer to the exponent stream of bytes (Big-Endian format). */
                                    uint16_t ExponentSize,                  /*!< [in]  The size of the exponent (in bytes). */
                                    uint8_t *Modulus_ptr,                   /*!< [in]  Pointer to the modulus stream of bytes (Big-Endian format).
										       The most significant bit (MSB) must be set to '1'. */
                                    uint16_t ModulusSize                    /*!< [in]  The modulus size in bytes. Supported sizes are 64, 128, 256, 384 and 512. */
);


/******************************************************************************************/
/*!
@brief Builds a ::CRYSRSAPrivKey_t private key structure with the provided modulus and exponent, marking the key as a non-CRT key.

@return CRYS_OK on success.
@return A non-zero value from crys_rsa_error.h on failure.
*/
CIMPORT_C CRYSError_t CRYS_RSA_Build_PrivKey(
                                    CRYS_RSAUserPrivKey_t   *UserPrivKey_ptr,   /*!< [out] Pointer to the public key structure.*/
                                    uint8_t               *PrivExponent_ptr,    /*!< [in]  Pointer to the private exponent stream of bytes (Big-Endian format). */
                                    uint16_t               PrivExponentSize,    /*!< [in]  The size of the private exponent (in bytes). */
                                    uint8_t               *PubExponent_ptr,     /*!< [in]  Pointer to the public exponent stream of bytes (Big-Endian format). */
                                    uint16_t               PubExponentSize,     /*!< [in]  The size of the public exponent (in bytes). */
                                    uint8_t               *Modulus_ptr,         /*!< [in]  Pointer to the modulus stream of bytes (Big-Endian format).
											   The most significant bit must be set to '1'. */
                                    uint16_t               ModulusSize          /*!< [in]  The modulus size in bytes. Supported sizes are 64, 128, 256, 384 and 512. */
);

/******************************************************************************************/
/*!
@brief Builds a ::CRYSRSAPrivKey_t private key structure with the provided parameters, marking the key as a CRT key.

@return CRYS_OK on success.
@return A non-zero value from crys_rsa_error.h on failure.
*/
CIMPORT_C CRYSError_t CRYS_RSA_Build_PrivKeyCRT(
                                      CRYS_RSAUserPrivKey_t *UserPrivKey_ptr,   /*!< [out] Pointer to the public key structure. */
                                      uint8_t *P_ptr,                           /*!< [in]  Pointer to the first factor stream of bytes (Big-Endian format). */
                                      uint16_t PSize,                           /*!< [in]  The size of the first factor (in bytes). */
                                      uint8_t *Q_ptr,                           /*!< [in]  Pointer to the second factor stream of bytes (Big-Endian format). */
                                      uint16_t QSize,                           /*!< [in]  The size of the second factor (in bytes). */
                                      uint8_t *dP_ptr,                          /*!< [in]  Pointer to the first factor's CRT exponent stream of bytes
											   (Big-Endian format). */
                                      uint16_t dPSize,                          /*!< [in]  The size of the first factor's CRT exponent (in bytes). */
                                      uint8_t *dQ_ptr,                          /*!< [in]  Pointer to the second factor's CRT exponent stream of bytes
											   (Big-Endian format). */
                                      uint16_t dQSize,                          /*!< [in]  The size of the second factor's CRT exponent (in bytes). */
                                      uint8_t *qInv_ptr,                        /*!< [in]  Pointer to the first CRT coefficient stream of bytes (Big-Endian format). */
                                      uint16_t qInvSize                         /*!< [in]  The size of the first CRT coefficient (in bytes). */
);


/******************************************************************************************/
/*!
@brief The function gets the e,n public key parameters from the input
CRYS_RSAUserPubKey_t structure. The function can also be used to retrieve the
modulus and exponent sizes only (Exponent_ptr AND Modulus_ptr must be set to
NULL).

\note All members of input UserPubKey_ptr structure must be initialized.

@return CRYS_OK on success.
@return A non-zero value from crys_rsa_error.h on failure.
*/
CIMPORT_C CRYSError_t CRYS_RSA_Get_PubKey(
                                CRYS_RSAUserPubKey_t *UserPubKey_ptr,   /*!< [in] A pointer to the public key structure. */
                                uint8_t  *Exponent_ptr,                 /*!< [out] A pointer to the exponent stream of bytes (Big-Endian format). */
                                uint16_t *ExponentSize_ptr,             /*!< [in/out] the size of the exponent buffer in bytes,
										      it is updated to the actual size of the exponent, in bytes. */
                                uint8_t  *Modulus_ptr,                  /*!< [out] A pointer to the modulus stream of bytes (Big-Endian format).
										   The MS (most significant) bit must be set to '1'. */
                                uint16_t *ModulusSize_ptr               /*!< [in/out] the size of the modulus buffer in bytes, it is updated to the actual
										      size of the modulus, in bytes. */
);


#ifdef __cplusplus
}
#endif

#endif
