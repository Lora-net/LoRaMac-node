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


#ifndef CRYS_ECPKI_DH_H
#define CRYS_ECPKI_DH_H

/*! @file
@brief Defines the API that supports EC Diffie-Hellman shared secret value derivation primitives.
*/


#include "crys_ecpki_types.h"
#include "crys_ecpki_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 *		         CRYS_ECDH_SVDP_DH function                    *
 ***********************************************************************/
/*!
@brief Creates the shared secret value according to [IEEE1363, ANS X9.63]:

<ol><li> Checks input-parameter pointers and EC Domain in public and private
keys.</li>
<li> Derives the partner public key and calls the EcWrstDhDeriveSharedSecret
function, which performs EC SVDP operations.</li></ol>
\note The term "User"
refers to any party that calculates a shared secret value using this primitive.
The term "Partner" refers to any other party of shared secret value calculation.
Partner's public key shall be validated before using in this primitive.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_ecpki_error.h.
*/
CIMPORT_C CRYSError_t CRYS_ECDH_SVDP_DH(
                        CRYS_ECPKI_UserPublKey_t *PartnerPublKey_ptr,       /*!< [in]  Pointer to a partner public key. */
                        CRYS_ECPKI_UserPrivKey_t *UserPrivKey_ptr,          /*!< [in]  Pointer to a user private key. */
                        uint8_t		         *SharedSecretValue_ptr,    /*!< [out] Pointer to an output buffer that will contain the shared
										       secret value. */
                        uint32_t                 *SharedSecrValSize_ptr,    /*!< [in/out] Pointer to the size of user-passed buffer (in) and
                                                                                          actual size of output of calculated shared secret value
											  (out). */
                        CRYS_ECDH_TempData_t     *TempBuff_ptr              /*!< [in]  Pointer to a temporary buffer. */);

#ifdef __cplusplus
}
#endif

#endif
