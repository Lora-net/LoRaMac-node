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

#ifndef CRYS_KDF_H
#define CRYS_KDF_H


#include "crys_hash.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module defines the API that supports Key derivation function in modes
as defined in PKCS#3, ANSI X9.42-2001, and ANSI X9.63-1999.
*/

#include "crys_hash.h"

/************************ Defines ******************************/

/* Shared secret value max size in bytes: must be defined according to      *
*  DH max. modulus size definition!     				    */
#define  CRYS_KDF_MAX_SIZE_OF_SHARED_SECRET_VALUE  1024 /*!< Size is in bytes*/

/* Count and max. sizeof OtherInfo entries (pointers to data buffers) */
#define  CRYS_KDF_COUNT_OF_OTHER_INFO_ENTRIES   5
#define  CRYS_KDF_MAX_SIZE_OF_OTHER_INFO_ENTRY  64 /*!< Size is in bytes*/
/* Max size of keying data in bytes to generate */
#define  CRYS_KDF_MAX_SIZE_OF_KEYING_DATA  2048

/************************ Enums ********************************/

typedef enum
{
	CRYS_KDF_HASH_SHA1_mode      = 0,
	CRYS_KDF_HASH_SHA224_mode	= 1,
	CRYS_KDF_HASH_SHA256_mode	= 2,
	CRYS_KDF_HASH_SHA384_mode	= 3,
	CRYS_KDF_HASH_SHA512_mode	= 4,

	CRYS_KDF_HASH_NumOfModes,

	CRYS_KDF_HASH_OpModeLast    = 0x7FFFFFFF,

}CRYS_KDF_HASH_OpMode_t;

typedef enum
{
	CRYS_KDF_ASN1_DerivMode    = 0,
	CRYS_KDF_ConcatDerivMode   = 1,
	CRYS_KDF_X963_DerivMode    = CRYS_KDF_ConcatDerivMode,
	CRYS_KDF_ISO18033_KDF1_DerivMode = 3,
	CRYS_KDF_ISO18033_KDF2_DerivMode = 4,

	CRYS_KDF_DerivFunc_NumOfModes = 5,

	CRYS_KDF_DerivFuncModeLast= 0x7FFFFFFF,

}CRYS_KDF_DerivFuncMode_t;

/************************ Typedefs  ****************************/

/* Structure, containing the optional data for KDF,
   if any data is not needed, then the pointer value and
   the size must be set to NULL */
typedef struct
{
	/* a unique object identifier (OID), indicating algorithm(s)
	for which the keying data is used*/
	uint8_t    AlgorithmID[CRYS_KDF_MAX_SIZE_OF_OTHER_INFO_ENTRY];
	uint32_t   SizeOfAlgorithmID;
	/* Public information contributed by the initiator */
	uint8_t    PartyUInfo[CRYS_KDF_MAX_SIZE_OF_OTHER_INFO_ENTRY];
	uint32_t   SizeOfPartyUInfo;
	/* Public information contributed by the responder */
	uint8_t    PartyVInfo[CRYS_KDF_MAX_SIZE_OF_OTHER_INFO_ENTRY];
	uint32_t   SizeOfPartyVInfo;
	/* Mutually-known private information, e.g. shared information
	communicated throgh a separate channel */
	uint8_t    SuppPrivInfo[CRYS_KDF_MAX_SIZE_OF_OTHER_INFO_ENTRY];
	uint32_t   SizeOfSuppPrivInfo;
	/* Mutually-known public information, */
	uint8_t    SuppPubInfo[CRYS_KDF_MAX_SIZE_OF_OTHER_INFO_ENTRY];
	uint32_t   SizeOfSuppPubInfo;

}CRYS_KDF_OtherInfo_t;

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/

/****************************************************************/


/*********************************************************************************************************/
/*!
 @brief CRYS_KDF_KeyDerivFunc performs key derivation according to one of the modes defined in standards:
		ANS X9.42-2001, ANS X9.63, ISO/IEC 18033-2.

The present implementation of the function allows the following operation modes:
<ul><li> CRYS_KDF_ASN1_DerivMode - mode based on  ASN.1 DER encoding; </li>
<li> CRYS_KDF_ConcatDerivMode - mode based on concatenation;</li>
<li> CRYS_KDF_X963_DerivMode = CRYS_KDF_ConcatDerivMode;</li>
<li> CRYS_KDF_ISO18033_KDF1_DerivMode - specific mode according to ECIES-KEM algorithm (ISO/IEC 18033-2).</li></ul>

The purpose of this function is to derive a keying data from the shared secret value and some
other optional shared information (SharedInfo).

\note
<ul id="noteb"><li> The length in Bytes of the hash result buffer is denoted by "hashlen".</li>
<li> All buffers arguments are represented in Big-Endian format.</li>

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_kdf_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_KDF_KeyDerivFunc(
                    uint8_t                  *ZZSecret_ptr,         /*!< [in]  A pointer to shared secret value octet string. */
                    uint32_t                  ZZSecretSize,         /*!< [in]  The size of the shared secret value in bytes.
                                                                               The maximal size is defined as: CRYS_KDF_MAX_SIZE_OF_SHARED_SECRET_VALUE. */
                    CRYS_KDF_OtherInfo_t     *OtherInfo_ptr,        /*!< [in]  The pointer to structure, containing the data, shared by two entities of
									       agreement and the data sizes. This argument may be optional in several modes
									       (if it is not needed - set NULL).
                                                                               On two ISO/IEC 18033-2 modes - set NULL.
                                                                               On KDF ASN1 mode the OtherInfo and its AlgorithmID entry are mandatory. */
                    CRYS_KDF_HASH_OpMode_t    KDFhashMode,          /*!< [in]  The KDF identifier of hash function to be used. The hash function output
									       must be at least 160 bits. */
                    CRYS_KDF_DerivFuncMode_t  derivation_mode,      /*!< [in]  Specifies one of above described derivation modes. */
                    uint8_t                  *KeyingData_ptr,       /*!< [out] A pointer to the buffer for derived keying data. */
                    uint32_t                  KeyingDataSizeBytes   /*!< [in]  The size in bytes of the keying data to be derived.
                                                                               The maximal size is defined as: CRYS_KDF_MAX_SIZE_OF_KEYING_DATA. */
);

/* alias for backward compatibility */
#define _DX_KDF_KeyDerivFunc   CRYS_KDF_KeyDerivFunc

/*********************************************************************************************************/
/*!
 CRYS_KDF_ASN1_KeyDerivFunc is A MACRO that performs key derivation according to ASN1 DER encoding method defined
 in standard  ANS X9.42-2001, 7.2.1. For a description of the parameters see ::CRYS_KDF_KeyDerivFunc.
*/
#define CRYS_KDF_ASN1_KeyDerivFunc(ZZSecret_ptr,ZZSecretSize,OtherInfo_ptr,KDFhashMode,KeyingData_ptr,KeyLenInBytes)\
		CRYS_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(OtherInfo_ptr),(KDFhashMode),CRYS_KDF_ASN1_DerivMode,(KeyingData_ptr),(KeyLenInBytes))


/*********************************************************************************************************/
/*!
 CRYS_KDF_ConcatKeyDerivFunc is a MACRO that performs key derivation according to concatenation mode defined
 in standard  ANS X9.42-2001, 7.2.2. For a description of the parameters see
 ::CRYS_KDF_KeyDerivFunc.
*/
#define CRYS_KDF_ConcatKeyDerivFunc(ZZSecret_ptr,ZZSecretSize,OtherInfo_ptr,KDFhashMode,KeyingData_ptr,KeyLenInBytes)\
		CRYS_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(OtherInfo_ptr),(KDFhashMode),CRYS_KDF_ConcatDerivMode,(KeyingData_ptr),(KeyLenInBytes))

#ifdef __cplusplus
}
#endif

#endif

