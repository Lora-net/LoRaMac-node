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


#ifndef CRYS_ECPKI_TYPES_H
#define CRYS_ECPKI_TYPES_H

/*!
@file
@brief Contains all of the enums and definitions that are used for the CRYS ECPKI APIs.
*/

#include "ssi_pal_types_plat.h"
#include "crys_hash.h"
#include "crys_pka_defs_hw.h"
#include "ssi_pal_compiler.h"

#ifdef __cplusplus
extern "C"
{
#endif


/************************ Defines ******************************/

#define CRYS_PKA_DOMAIN_LLF_BUFF_SIZE_IN_WORDS (10 + 3*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS)

/* The type defines integer array of lengths of maximum lengths of EC modulus */
typedef  uint32_t  CRYS_ECPKI_ARRAY_t[CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];

/**************************************************************************************
 *	              Enumerators
 ***************************************************************************************/

/*------------------------------------------------------------------*/
/*! Enumerator for the EC Domain idetifier
   References: [13] - SEC 2: Recommended elliptic curve domain parameters.
                      Version 1.0. Certicom 2000.
               [8]  - WAP-261-WTLS-20010406-a, Version 06-April-2001.     */

typedef enum
{
	/* For prime field */
	CRYS_ECPKI_DomainID_secp160k1,   /*!< EC secp160r1 */
	CRYS_ECPKI_DomainID_secp160r1,   /*!< EC secp160k1 */
	CRYS_ECPKI_DomainID_secp160r2,   /*!< EC secp160r2 */
	CRYS_ECPKI_DomainID_secp192k1,   /*!< EC secp192k1 */
	CRYS_ECPKI_DomainID_secp192r1,   /*!< EC secp192r1 */
	CRYS_ECPKI_DomainID_secp224k1,   /*!< EC secp224k1 */
	CRYS_ECPKI_DomainID_secp224r1,   /*!< EC secp224r1 */
	CRYS_ECPKI_DomainID_secp256k1,   /*!< EC secp256k1 */
	CRYS_ECPKI_DomainID_secp256r1,   /*!< EC secp256r1 */
	CRYS_ECPKI_DomainID_secp384r1,   /*!< EC secp384r1 */
	CRYS_ECPKI_DomainID_secp521r1,   /*!< EC secp521r1 */

	CRYS_ECPKI_DomainID_Builded,     /*!< User given, not identified. */
	CRYS_ECPKI_DomainID_OffMode,

	CRYS_ECPKI_DomainIDLast      = 0x7FFFFFFF,

}CRYS_ECPKI_DomainID_t;


/*------------------------------------------------------------------*/
/*! Defines the enum for the HASH operation mode.
 *  The enumerator defines 6 HASH modes according to IEEE 1363.
 *
 */
typedef enum
{
	CRYS_ECPKI_HASH_SHA1_mode    = 0,		/*!< HASH SHA1 mode. */
	CRYS_ECPKI_HASH_SHA224_mode  = 1,		/*!< HASH SHA224 mode. */
	CRYS_ECPKI_HASH_SHA256_mode  = 2,		/*!< HASH SHA256 mode. */
	CRYS_ECPKI_HASH_SHA384_mode  = 3,		/*!< HASH SHA384 mode. */
	CRYS_ECPKI_HASH_SHA512_mode  = 4,		/*!< HASH SHA512 mode. */

	CRYS_ECPKI_AFTER_HASH_SHA1_mode    = 5, 	/*!< After HASH SHA1 mode (message was already hashed). */
	CRYS_ECPKI_AFTER_HASH_SHA224_mode  = 6,	/*!< After HASH SHA224 mode (message was already hashed). */
	CRYS_ECPKI_AFTER_HASH_SHA256_mode  = 7,	/*!< After HASH SHA256 mode (message was already hashed). */
	CRYS_ECPKI_AFTER_HASH_SHA384_mode  = 8,	/*!< After HASH SHA384 mode (message was already hashed). */
	CRYS_ECPKI_AFTER_HASH_SHA512_mode  = 9,	/*!< After HASH SHA512 mode (message was already hashed). */


	CRYS_ECPKI_HASH_NumOfModes,
	CRYS_ECPKI_HASH_OpModeLast        = 0x7FFFFFFF,

}CRYS_ECPKI_HASH_OpMode_t;


/*---------------------------------------------------*/
/*! Enumerator for the EC point compression idetifier. */
typedef enum
{
	CRYS_EC_PointCompressed     = 2,
	CRYS_EC_PointUncompressed   = 4,
	CRYS_EC_PointContWrong      = 5, /* wrong Point Control value */
	CRYS_EC_PointHybrid         = 6,

	CRYS_EC_PointCompresOffMode = 8,

	CRYS_ECPKI_PointCompressionLast= 0x7FFFFFFF,

}CRYS_ECPKI_PointCompression_t;

/*----------------------------------------------------*/
/*  Enumerator for compatibility of the DHC
   with cofactor multiplication to DH ordinary */
typedef enum
{
	CRYS_ECPKI_SVDP_DHC_CompatibleDH         = 0,   /* Requested compatiblity of SVDP_DHC with cofactor to SVDP_DH */
	CRYS_ECPKI_SVDP_DHC_NonCompatibleDH	    = 1,   /* Compatiblity of SVDP_DHC with cofactor to SVDP_DH is not requested */

	CRYS_ECPKI_SVDP_DHC_OffMode,

	CRYS_ECPKI_SVDP_DHC_OpModeLast         = 0x7FFFFFFF,

}CRYS_ECPKI_SVDP_DHC_OpMode_t;

/*----------------------------------------------------*/
/* Enumerator for indication what checking of EC public key must be performed */
typedef enum {
	CheckPointersAndSizesOnly = 0,   /* Only preliminary input parameters checking */
	ECpublKeyPartlyCheck      = 1,   /* In addition check that EC PubKey is point on curve */
	ECpublKeyFullCheck        = 2,   /* In addition check that EC_GeneratorOrder*PubKey = O */

	PublKeyChecingOffMode,
	EC_PublKeyCheckModeLast  = 0x7FFFFFFF,
}EC_PublKeyCheckMode_t;

/*----------------------------------------------------*/
/* This SCAP is related to included SCA_PROTECTION measures in   *
*  SW part of algoritthms but not in HW itself 			 */
typedef enum {
	SCAP_Inactive,
	SCAP_Active,
	SCAP_OFF_MODE,
	SCAP_LAST = 0x7FFFFFFF
} CRYS_ECPKI_ScaProtection_t;


/**************************************************************************************
 *	             EC  Domain structure definition
 ***************************************************************************************/

/*! The structure containing the EC domain parameters in little-endian form
    EC equation: Y^2 = X^3 + A*X + B over prime fild GFp. */
typedef  struct {

	/*! EC modulus: P. */
	uint32_t	ecP [CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
	/*! EC equation parameters a, b. */
	uint32_t	ecA [CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
	uint32_t	ecB [CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
	/*! Order of generator. */
	uint32_t	ecR [CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 1];
	/*! EC cofactor EC_Cofactor_K
	    Generator (EC base point) coordinates in projective form. */
	uint32_t	ecGx [CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
	uint32_t	ecGy [CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
	uint32_t  	ecH;
	/*! include the specific fields that are used by the low level.*/
	uint32_t      llfBuff[CRYS_PKA_DOMAIN_LLF_BUFF_SIZE_IN_WORDS];
	/*! Size of fields in bits. */
	uint32_t  	modSizeInBits;
	uint32_t  	ordSizeInBits;
	/*! Size of each inserted Barret tag in words; 0 - if not inserted.*/
	uint32_t 	barrTagSizeInWords;
	/*! EC Domain identifier.*/
	CRYS_ECPKI_DomainID_t	DomainID;
	int8_t name[20];

} CRYS_ECPKI_Domain_t;



/**************************************************************************************
 *	             EC  point structures definitions
 ***************************************************************************************/

/*! The structure containing the EC point in affine coordinates
   and little endian form. */
typedef  struct
{
	/*! Point coordinates. */
	uint32_t x[CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
	uint32_t y[CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];

}CRYS_ECPKI_PointAffine_t;


/**************************************************************************************
 *	              ECPKI public and private key  Structures
 ***************************************************************************************/

/* --------------------------------------------------------------------- */
/* .................. The public key structures definitions ............ */
/* --------------------------------------------------------------------- */

/*! The structure containing the Public Key in affine coordinates.*/

/*   Size = 2*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 1 +
   ( sizeof(LLF_ECPKI_publ_key_db_def.h) = 0 ).          */

typedef  struct
{
	/*! Public Key coordinates. */
	uint32_t x[CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
	uint32_t y[CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
	CRYS_ECPKI_Domain_t  domain;
	uint32_t pointType;
} CRYS_ECPKI_PublKey_t;


/*! The user structure containing EC public key data base form.*/

/*   Size = 2*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 2 +
   ( sizeof(LLF_ECPKI_publ_key_db_def.h) = 0 ).          */

typedef struct   CRYS_ECPKI_UserPublKey_t
{
	uint32_t 	valid_tag;
	uint32_t  	PublKeyDbBuff[(sizeof(CRYS_ECPKI_PublKey_t)+3)/4];

}  CRYS_ECPKI_UserPublKey_t;


/* --------------------------------------------------------------------- */
/* .................. The private key structures definitions ........... */
/* --------------------------------------------------------------------- */

/*! The EC private key structure in little endian form.*/

/*   Size = CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 2 +
   ( sizeof(LLF_ECPKI_priv_key_db_def.h) = 0 ).        */
typedef  struct
{
	/*! Private Key data. */
	uint32_t  PrivKey[CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 1];
	CRYS_ECPKI_Domain_t  domain;
	CRYS_ECPKI_ScaProtection_t  scaProtection;

}CRYS_ECPKI_PrivKey_t;


/*! The user structure containing EC private key data base in little endian form. */

/*   Size = CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 3 +
   ( sizeof(LLF_ECPKI_priv_key_db_def.h) = 0 )          */
typedef struct   CRYS_ECPKI_UserPrivKey_t
{
	uint32_t    valid_tag;
	uint32_t    PrivKeyDbBuff[(sizeof(CRYS_ECPKI_PrivKey_t)+3)/4];

}  CRYS_ECPKI_UserPrivKey_t;

/*!  The ECDH temporary data type  */
typedef struct CRYS_ECDH_TempData_t
{
	uint32_t crysEcdhIntBuff[CRYS_PKA_ECDH_BUFF_MAX_LENGTH_IN_WORDS];
}CRYS_ECDH_TempData_t;

typedef struct CRYS_ECPKI_BUILD_TempData_t
{
	uint32_t  crysBuildTmpIntBuff[CRYS_PKA_ECPKI_BUILD_TMP_BUFF_MAX_LENGTH_IN_WORDS];
}CRYS_ECPKI_BUILD_TempData_t;



/**************************************************************************
 *	              CRYS ECDSA context structures
 **************************************************************************/

/* --------------------------------------------------------------------- */
/*                CRYS ECDSA Signing context structure                   */
/* --------------------------------------------------------------------- */

typedef uint32_t CRYS_ECDSA_SignIntBuff[CRYS_PKA_ECDSA_SIGN_BUFF_MAX_LENGTH_IN_WORDS];
typedef  struct
{
	/* A user's buffer for the Private Key Object -*/
	CRYS_ECPKI_UserPrivKey_t     ECDSA_SignerPrivKey;

	/*HASH specific data and buffers */
	uint32_t hashUserCtxBuff[sizeof(CRYS_HASHUserContext_t)];
	CRYS_HASH_Result_t        hashResult;
	uint32_t            	 	 hashResultSizeWords;
	CRYS_ECPKI_HASH_OpMode_t	 hashMode;

	CRYS_ECDSA_SignIntBuff crysEcdsaSignIntBuff;
}ECDSA_SignContext_t;


/* --------------------------------------------------------------------- */
/*                CRYS ECDSA  Signing User context database              */
/* --------------------------------------------------------------------- */


typedef struct  CRYS_ECDSA_SignUserContext_t
{
	uint32_t  context_buff [(sizeof(ECDSA_SignContext_t)+3)/4];
	uint32_t  valid_tag;
} CRYS_ECDSA_SignUserContext_t;



/****************************************************************************/

/* --------------------------------------------------------------------- */
/*                CRYS ECDSA Verifying context structure                 */
/* --------------------------------------------------------------------- */

typedef uint32_t CRYS_ECDSA_VerifyIntBuff[CRYS_PKA_ECDSA_VERIFY_BUFF_MAX_LENGTH_IN_WORDS];

typedef  struct
{
	/* A user's buffer for the Private Key Object -*/
	CRYS_ECPKI_UserPublKey_t         ECDSA_SignerPublKey;

	/*HASH specific data and buffers */
	uint32_t hashUserCtxBuff[sizeof(CRYS_HASHUserContext_t)];
	CRYS_HASH_Result_t               hashResult;
	uint32_t                  	    hashResultSizeWords;
	CRYS_ECPKI_HASH_OpMode_t    	    hashMode;

	CRYS_ECDSA_VerifyIntBuff 	crysEcdsaVerIntBuff;

}ECDSA_VerifyContext_t;


/* --------------------------------------------------------------------- */
/*                CRYS ECDSA Verifying User context database             */
/* --------------------------------------------------------------------- */

typedef struct  CRYS_ECDSA_VerifyUserContext_t
{
	uint32_t  	context_buff[(sizeof(ECDSA_VerifyContext_t)+3)/4];
	uint32_t 	valid_tag;
}CRYS_ECDSA_VerifyUserContext_t;




/* --------------------------------------------------------------------- */
/* .................. key generation temp buffer   ........... */
/* --------------------------------------------------------------------- */

/*! The ECPKI KG temporary data type */
typedef struct CRYS_ECPKI_KG_TempData_t
{
	uint32_t crysKGIntBuff[CRYS_PKA_KG_BUFF_MAX_LENGTH_IN_WORDS];
}CRYS_ECPKI_KG_TempData_t;


typedef struct CRYS_ECIES_TempData_t {

	CRYS_ECPKI_UserPrivKey_t   PrivKey;
	CRYS_ECPKI_UserPublKey_t   PublKey;
	uint32_t  zz[3*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 1];
	union {
		CRYS_ECPKI_BUILD_TempData_t buildTempbuff;
		CRYS_ECPKI_KG_TempData_t    KgTempBuff;
		CRYS_ECDH_TempData_t        DhTempBuff;
	} tmp;

}CRYS_ECIES_TempData_t;


/* --------------------------------------------------------------------- */
/* .................. defines for FIPS      ........... */
/* --------------------------------------------------------------------- */

#define CRYS_ECPKI_FIPS_ORDER_LENGTH (256/SASI_BITS_IN_BYTE)  // the order of secp256r1 in bytes

/*! Required for internal FIPS verification for ECPKI key generation. */
typedef struct CRYS_ECPKI_KG_FipsContext_t
{
	union {
		CRYS_ECDSA_SignUserContext_t	signCtx;
		CRYS_ECDSA_VerifyUserContext_t  verifyCtx;
	}operationCtx;
	uint32_t	signBuff[2*CRYS_ECPKI_ORDER_MAX_LENGTH_IN_WORDS];
}CRYS_ECPKI_KG_FipsContext_t;



/*! Required for internal FIPS verification for ECDSA KAT.      *
*  The ECDSA KAT tests defined for domain 256r1.     */
typedef struct CRYS_ECDSAFipsKatContext_t{
	union {
		struct {
			CRYS_ECPKI_UserPrivKey_t   	PrivKey;
			CRYS_ECDSA_SignUserContext_t	signCtx;
		}userSignData;
		struct {
			CRYS_ECPKI_UserPublKey_t   	PublKey;
			union {
				CRYS_ECDSA_VerifyUserContext_t  verifyCtx;
				CRYS_ECPKI_BUILD_TempData_t	tempData;
			}buildOrVerify;
		}userVerifyData;
	}keyContextData;

	uint8_t			signBuff[2*CRYS_ECPKI_FIPS_ORDER_LENGTH];
}CRYS_ECDSAFipsKatContext_t;

/*! Required for internal FIPS verification for ECDH KAT. */
typedef struct CRYS_ECDHFipsKatContext_t{
	CRYS_ECPKI_UserPublKey_t  pubKey;
	CRYS_ECPKI_UserPrivKey_t  privKey;
	union {
		CRYS_ECPKI_BUILD_TempData_t  ecpkiTempData;
		CRYS_ECDH_TempData_t      ecdhTempBuff;
	}tmpData;
	uint8_t			  secretBuff[CRYS_ECPKI_FIPS_ORDER_LENGTH];
}CRYS_ECDHFipsKatContext_t;

#ifdef __cplusplus
}
#endif

#endif
