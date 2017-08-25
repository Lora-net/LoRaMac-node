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


#ifndef CRYS_ECPKI_KG_H
#define CRYS_ECPKI_KG_H

/*! @file
@brief Defines the API for generation of ECC private and public keys.
*/


#include "crys_error.h"
#include "crys_rnd.h"
#include "crys_ecpki_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************  CRYS_ECPKI_GenKeyPair function   **********************/
/*!
@brief Generates a pair of private and public keys in internal representation according to [ANS X9.62].

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_ecpki_error.h.
*/
CIMPORT_C CRYSError_t CRYS_ECPKI_GenKeyPair(
                        CRYS_RND_Context_t         *pRndContext,      /*!< [in/out] Pointer to the RND context buffer. */
                        const CRYS_ECPKI_Domain_t  *pDomain,          /*!< [in]  Pointer to EC domain (curve). */
                        CRYS_ECPKI_UserPrivKey_t   *pUserPrivKey,     /*!< [out] Pointer to the private key structure. This structure is used as input to the
										 ECPKI cryptographic primitives. */
                        CRYS_ECPKI_UserPublKey_t   *pUserPublKey,     /*!< [out] Pointer to the public key structure. This structure is used as input to the
										 ECPKI cryptographic primitives. */
			CRYS_ECPKI_KG_TempData_t   *pTempData,        /*!< [in] Temporary buffers for internal use, defined in ::CRYS_ECPKI_KG_TempData_t. */
                        CRYS_ECPKI_KG_FipsContext_t   *pFipsCtx       /*!< [in] Pointer to temporary buffer used in case FIPS certification if required. */
);



#ifdef __cplusplus
}
#endif

#endif




