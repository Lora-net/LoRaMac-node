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

/*!
@file
@brief This file contains all of the enums and definitions that are used for the
       CRYS SRP APIs, as well as the APIs themselves.
*/
#ifndef CRYS_SRP_H
#define CRYS_SRP_H


#include "ssi_pal_types.h"
#include "crys_error.h"
#include "crys_pka_defs_hw.h"
#include "crys_hash.h"
#include "crys_rnd.h"


#ifdef __cplusplus
extern "C"
{
#endif


/************************ Defines ******************************/
/*! The SRP modulus sizes. */
#define CRYS_SRP_MODULUS_SIZE_1024_BITS   1024
#define CRYS_SRP_MODULUS_SIZE_1536_BITS   1536
#define CRYS_SRP_MODULUS_SIZE_2048_BITS   2048
#define CRYS_SRP_MODULUS_SIZE_3072_BITS   3072

#define CRYS_SRP_MAX_MODULUS_IN_BITS 		CRYS_SRP_MODULUS_SIZE_3072_BITS
#define CRYS_SRP_MAX_MODULUS	 		(CRYS_SRP_MAX_MODULUS_IN_BITS/SASI_BITS_IN_BYTE)
#define CRYS_SRP_MAX_MODULUS_IN_WORDS 		(CRYS_SRP_MAX_MODULUS_IN_BITS/SASI_BITS_IN_32BIT_WORD)

/*! The SRP private number size range. */
#define CRYS_SRP_PRIV_NUM_MIN_SIZE_IN_BITS 		(256)
#define CRYS_SRP_PRIV_NUM_MIN_SIZE	 		(CRYS_SRP_PRIV_NUM_MIN_SIZE_IN_BITS/SASI_BITS_IN_BYTE)
#define CRYS_SRP_PRIV_NUM_MIN_SIZE_IN_WORDS 		(CRYS_SRP_PRIV_NUM_MIN_SIZE_IN_BITS/SASI_BITS_IN_32BIT_WORD)
#define CRYS_SRP_PRIV_NUM_MAX_SIZE_IN_BITS 		(CRYS_SRP_MAX_MODULUS_IN_BITS)
#define CRYS_SRP_PRIV_NUM_MAX_SIZE	 		(CRYS_SRP_PRIV_NUM_MAX_SIZE_IN_BITS/SASI_BITS_IN_BYTE)
#define CRYS_SRP_PRIV_NUM_MAX_SIZE_IN_WORDS 		(CRYS_SRP_PRIV_NUM_MAX_SIZE_IN_BITS/SASI_BITS_IN_32BIT_WORD)

/*! The SRP HASH digest max size in bytes. */
#define CRYS_SRP_MAX_DIGEST_IN_WORDS 		CRYS_HASH_RESULT_SIZE_IN_WORDS
#define CRYS_SRP_MAX_DIGEST	 		(CRYS_SRP_MAX_DIGEST_IN_WORDS*SASI_32BIT_WORD_SIZE)

/*! The SRP salt size range. */
#define CRYS_SRP_MIN_SALT_SIZE		 	(8)
#define CRYS_SRP_MIN_SALT_SIZE_IN_WORDS 	(CRYS_SRP_MIN_SALT_SIZE/SASI_32BIT_WORD_SIZE)
#define CRYS_SRP_MAX_SALT_SIZE		 	(64)
#define CRYS_SRP_MAX_SALT_SIZE_IN_WORDS 	(CRYS_SRP_MAX_SALT_SIZE/SASI_32BIT_WORD_SIZE)

/************************ Typedefs  ****************************/
/*! Defines the SRP modulus sized buffer. */
typedef uint8_t CRYS_SRP_Modulus_t[CRYS_SRP_MAX_MODULUS];

/*! Defines the SRP digest sized buffer. */
typedef uint8_t CRYS_SRP_Digest_t[CRYS_SRP_MAX_DIGEST];

/*! Defines the SRP secret sized buffer. */
typedef uint8_t CRYS_SRP_Secret_t[2*CRYS_SRP_MAX_DIGEST];

/************************ Enums ********************************/

/*!
SRP supported versions
*/
typedef enum {
	CRYS_SRP_VER_3	= 0,    /*!< VER 3*/
	CRYS_SRP_VER_6   = 1,    /*!< VER 6 */
	CRYS_SRP_VER_6A  = 2,    /*!< VER 6A */
	CRYS_SRP_VER_HK  = 3,    /*!< VER 6A */

	CRYS_SRP_NumOfVersions,

	CRYS_SRP_VersionLast= 0x7FFFFFFF,

}CRYS_SRP_Version_t;

/*!
SRP entity type
*/
typedef enum {
	CRYS_SRP_HOST	= 1,    /*!< Host entity, called also server, verifier, or Accessory in Home-Kit */
	CRYS_SRP_USER   = 2,    /*!< User entity, called also client, or Device in Home-Kit */

	CRYS_SRP_NumOfEntityType,

	CRYS_SRP_EntityLast= 0x7FFFFFFF,

}CRYS_SRP_Entity_t;

/************************ Structs  ******************************/

/* The group parameters for the SRP - defines the modulus and the generatetoe used */
typedef struct CRYS_SRP_GroupParam_t {
	CRYS_SRP_Modulus_t	modulus;
	uint8_t			gen;
	size_t			modSizeInBits;
	uint32_t		validNp;
	uint32_t		Np[CRYS_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS];
}CRYS_SRP_GroupParam_t;

/************************ context Structs  ******************************/
/* The SRP context prototype */
typedef struct CRYS_SRP_Context_t {
	CRYS_SRP_Entity_t		srpType;
	CRYS_SRP_Version_t		srpVer;
	CRYS_SRP_GroupParam_t		groupParam;	// N, g, Np
	CRYS_HASH_OperationMode_t	hashMode;
	size_t				hashDigestSize;
	CRYS_RND_Context_t 		*pRndCtx;
	CRYS_SRP_Modulus_t		ephemPriv;	// a or b
	size_t				ephemPrivSize;
	CRYS_SRP_Digest_t		userNameDigest;	// M
	CRYS_SRP_Digest_t		credDigest;	// p
	CRYS_SRP_Digest_t		kMult;		// k multiplier
}CRYS_SRP_Context_t;


/************************ SRP common Functions **********************/
/****************************************************************************************************/
/*!
@brief This function initiates the SRP context.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_srp_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_SRP_Init(
				CRYS_SRP_Entity_t	srpType,	/*!< [in] SRP entity type. */
				CRYS_SRP_Version_t	srpVer,		/*!< [in] SRP version. */
				CRYS_SRP_Modulus_t	srpModulus,	/*!< [in] A pointer to the SRP modulus, BE byte buffer. */
				uint8_t			srpGen,		/*!< [in] The SRP generator param. */
				size_t			modSizeInBits,	/*!< [in] The SRP modulus size in bits:1024, 1536, 2048 & 3072 */
				CRYS_HASH_OperationMode_t	hashMode,       /*!< [in] Enumerator defining the HASH mode. */
				uint8_t			*pUserName,	/*!< [in] A Pointer to user name. */
				size_t                	userNameSize,	/*!< [in] The user name buffer size > 0. */
				uint8_t			*pPwd,		/*!< [in] A Pointer to user password. */
				size_t                	pwdSize,	/*!< [in] The user password buffer size > 0 if pPwd is valid. */
				CRYS_RND_Context_t 	*pRndCtx,	/*!< [in] A Pointer to RND context.*/
				CRYS_SRP_Context_t	*pCtx 		/*!< [out] A Pointer to the SRP host context.*/
);

#define CRYS_SRP_HK_INIT(srpType, srpModulus, srpGen, modSizeInBits, pUserName, userNameSize, pPwd, pwdSize, pRndCtx, pCtx) \
	CRYS_SRP_Init(srpType, CRYS_SRP_VER_HK, srpModulus, srpGen, modSizeInBits, CRYS_HASH_SHA512_mode, pUserName, userNameSize, pPwd, pwdSize, pRndCtx, pCtx)


/****************************************************************************************************/
/*!
@brief This function calculates pSalt &  password verifier

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_srp_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_SRP_PwdVerCreate(
				size_t                	saltSize,	/*!< [in] The size of the random salt to generate,
										The range is between CRYS_SRP_MIN_SALT_SIZE
										to CRYS_SRP_MAX_SALT_SIZE. */
				uint8_t			*pSalt,	   	/*!< [out] A Pointer to the pSalt number (s).*/
				CRYS_SRP_Modulus_t     	pwdVerifier,	/*!< [out] A Pointer to the password verifier (v). */
				CRYS_SRP_Context_t	*pCtx 		/*!< [out] A Pointer to the SRP context.*/
);


/****************************************************************************************************/
/*!
@brief Clears the SRP context.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_srp_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_SRP_Clear(
				CRYS_SRP_Context_t	*pCtx 		/*!< [in/out] A Pointer to the SRP context.*/
);


/************************ SRP Host Functions **********************/
/****************************************************************************************************/
/*!
@brief This function generates host public & private ephemeral key, known as B & b in RFC

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_srp_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_SRP_HostPubKeyCreate(
				size_t                	ephemPrivSize,	/*!< [in] The size of the generated ephemeral private key (b).
										The range is between CRYS_SRP_PRIV_NUM_MIN_SIZE to
										CRYS_SRP_PRIV_NUM_MAX_SIZE */
				CRYS_SRP_Modulus_t     	pwdVerifier,	/*!< [in] A Pointer to the verifier (v). */
				CRYS_SRP_Modulus_t     	hostPubKeyB,	/*!< [out] A Pointer to the host ephemeral public key (B). */
				CRYS_SRP_Context_t	*pCtx 		/*!< [in/out] A Pointer to the SRP context.*/
);


/*!
@brief Verifies the user Proof and calculates the Host message proof.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_srp_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_SRP_HostProofVerifyAndCalc(
				size_t                	saltSize,	/*!< [in] The size of the random salt,
										The range is between CRYS_SRP_MIN_SALT_SIZE
										to CRYS_SRP_MAX_SALT_SIZE. */
				uint8_t			*pSalt,	   	/*!< [in] A Pointer to the pSalt number.*/
				CRYS_SRP_Modulus_t     	pwdVerifier,	/*!< [in] A Pointer to the password verifier (v). */
				CRYS_SRP_Modulus_t     	userPubKeyA,	/*!< [in] A Pointer to the user ephemeral public key (A). */
				CRYS_SRP_Modulus_t     	hostPubKeyB,	/*!< [in] A Pointer to the host ephemeral public key (B). */
				CRYS_SRP_Digest_t	userProof,      /*!< [in] A Pointer to the SRP user proof buffer (M1).*/
				CRYS_SRP_Digest_t	hostProof,      /*!< [out] A Pointer to the SRP host proof buffer (M2).*/
				CRYS_SRP_Secret_t	sharedSecret,   /*!< [out] A Pointer to the SRP shared secret (K).*/
				CRYS_SRP_Context_t	*pCtx 		/*!< [in] A Pointer to the SRP context.*/
);



/************************ SRP User Functions **********************/
/****************************************************************************************************/
/*!
@brief This function generates user public & private ephemeral key, known as A & a in RFC

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_srp_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_SRP_UserPubKeyCreate(
				size_t                	ephemPrivSize,	/*!< [in] The size of the generated ephemeral private key (a).
										The range is between CRYS_SRP_PRIV_NUM_MIN_SIZE to
										CRYS_SRP_PRIV_NUM_MAX_SIZE */
				CRYS_SRP_Modulus_t     	userPubKeyA,	/*!< [out] A Pointer to the user ephemeral public key (A). */
				CRYS_SRP_Context_t	*pCtx 		/*!< [in/out] A Pointer to the SRP context.*/
);


/****************************************************************************************************/
/*!
@brief This function calculates the user proof.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_srp_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_SRP_UserProofCalc(
				size_t                	saltSize,	/*!< [in] The size of the random salt,
										The range is between CRYS_SRP_MIN_SALT_SIZE
										to CRYS_SRP_MAX_SALT_SIZE. */
				uint8_t			*pSalt,	   	/*!< [in] A Pointer to the pSalt number.*/
				CRYS_SRP_Modulus_t	userPubKeyA,  /*!< [in] A Pointer to the user public ephmeral key (A).*/
				CRYS_SRP_Modulus_t	hostPubKeyB,  /*!< [in] A Pointer to the host public ephmeral key (B).*/
				CRYS_SRP_Digest_t	userProof,      /*!< [out] A Pointer to the SRP user proof buffer (M1).*/
				CRYS_SRP_Secret_t	sharedSecret,   /*!< [out] A Pointer to the SRP shared secret (K).*/
				CRYS_SRP_Context_t	*pCtx 		/*!< [out] A Pointer to the SRP context.*/
);

/****************************************************************************************************/
/*!
@brief This function verifies the host proof

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_srp_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_SRP_UserProofVerify(
				CRYS_SRP_Secret_t	sharedSecret,   /*!< [in] A Pointer to the SRP shared secret (K).*/
				CRYS_SRP_Modulus_t	userPubKeyA,  /*!< [in] A Pointer to the user public ephmeral key (A).*/
				CRYS_SRP_Digest_t	userProof,      /*!< [in] A Pointer to the SRP user proof buffer (M1).*/
				CRYS_SRP_Digest_t	hostProof,     /*!< [in] A Pointer to the SRP host proof buffer (M2).*/
				CRYS_SRP_Context_t	*pCtx 		/*!< [out] A Pointer to the SRP user context.*/
);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef CRYS_SRP_H */





