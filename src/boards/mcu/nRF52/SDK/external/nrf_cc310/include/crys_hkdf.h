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

#ifndef CRYS_HKDF_H
#define CRYS_HKDF_H

#include "crys_hash.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module defines the API that supports HMAC Key derivation function as defined by RFC5869.
*/

#define CRYS_HKDF_MAX_HASH_KEY_SIZE_IN_BYTES        512


#define CRYS_HKDF_MAX_HASH_DIGEST_SIZE_IN_BYTES     CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES

/************************ Defines ******************************/

/************************ Enums ********************************/

typedef enum
{
    CRYS_HKDF_HASH_SHA1_mode      = 0,
    CRYS_HKDF_HASH_SHA224_mode  = 1,
    CRYS_HKDF_HASH_SHA256_mode  = 2,
    CRYS_HKDF_HASH_SHA384_mode  = 3,
    CRYS_HKDF_HASH_SHA512_mode  = 4,

    CRYS_HKDF_HASH_NumOfModes,

    CRYS_HKDF_HASH_OpModeLast    = 0x7FFFFFFF,

}CRYS_HKDF_HASH_OpMode_t;

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/

/****************************************************************/


/*********************************************************************************************************/
/**
 * @brief CRYS_HKDF_KeyDerivFunc performs the HMAC-based key derivation, according to RFC5869

  @return CRYSError_t - On success the value CRYS_OK is returned,
            and on failure an ERROR as defined in CRYS_KDF_error.h:
            CRYS_HKDF_INVALID_ARGUMENT_POINTER_ERROR
            CRYS_HKDF_INVALID_ARGUMENT_SIZE_ERROR
            CRYS_HKDF_INVALID_ARGUMENT_HASH_MODE_ERROR
*/
CEXPORT_C CRYSError_t  CRYS_HKDF_KeyDerivFunc(
                        CRYS_HKDF_HASH_OpMode_t HKDFhashMode,   /*!< [in]   The HKDF identifier of hash function to be used. */
                        uint8_t*                Salt_ptr,       /*!< [in]   A pointer to a non secret random value. can be NULL. */
                        size_t	                SaltLen,        /*!< [in]   The size of the salt_ptr. */
                        uint8_t*                Ikm_ptr,        /*!< [in]    A pointer to a input key message. */
                        uint32_t                IkmLen,         /*!< [in]   The size of the input key message */
                        uint8_t*                Info,           /*!< [in]   A pointer to an optional context and application specific information. can be NULL */
                        uint32_t                InfoLen,        /*!< [in]   The size of the info. */
                        uint8_t*                Okm,            /*!< [in]   A pointer to a output key material. */
                        uint32_t                OkmLen,         /*!< [in]   The size of the output key material. */
                        SaSiBool                IsStrongKey    /*!< [in]    if TRUE , then no need to perform the extraction phase. */
                        );

#ifdef __cplusplus
}
#endif

#endif

