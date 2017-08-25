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
         CRYS CHACHA APIs, as well as the APIs themselves.
*/
#ifndef CRYS_CHACHA_H
#define CRYS_CHACHA_H


#include "ssi_pal_types.h"
#include "crys_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

/************************ Defines ******************************/

/*
brief - a definition describing the low level Engine type ( SW, Hardware, Etc )
*/

#define CRYS_CHACHA_USER_CTX_SIZE_IN_WORDS 17

#define CRYS_CHACHA_BLOCK_SIZE_IN_WORDS 16
#define CRYS_CHACHA_BLOCK_SIZE_IN_BYTES  (CRYS_CHACHA_BLOCK_SIZE_IN_WORDS * sizeof(uint32_t))

/*! The max size of the nonce buffer. */
#define CRYS_CHACHA_NONCE_MAX_SIZE_IN_WORDS   3
#define CRYS_CHACHA_NONCE_MAX_SIZE_IN_BYTES  (CRYS_CHACHA_NONCE_MAX_SIZE_IN_WORDS * sizeof(uint32_t))

/*! The maximum size of the CHACHA KEY in words and bytes. */
#define CRYS_CHACHA_KEY_MAX_SIZE_IN_WORDS 8
#define CRYS_CHACHA_KEY_MAX_SIZE_IN_BYTES (CRYS_CHACHA_KEY_MAX_SIZE_IN_WORDS * sizeof(uint32_t))

/************************ Enums ********************************/

/* Enum defining the Encrypt or Decrypt operation mode */
typedef enum {
	CRYS_CHACHA_Encrypt = 0,
	CRYS_CHACHA_Decrypt = 1,

	CRYS_CHACHA_EncryptNumOfOptions,

	CRYS_CHACHA_EncryptModeLast = 0x7FFFFFFF,

}CRYS_CHACHA_EncryptMode_t;

typedef enum {
        CRYS_CHACHA_Nonce64BitSize = 0,
        CRYS_CHACHA_Nonce96BitSize = 1,

        CRYS_CHACHA_NonceSizeNumOfOptions,

        CRYS_CHACHA_NonceSizeLast = 0x7FFFFFFF,

}CRYS_CHACHA_NonceSize_t;

/************************ Typedefs  ****************************/

/*! Defines the Nonce buffer 12 bytes array. */
typedef uint8_t CRYS_CHACHA_Nonce_t[CRYS_CHACHA_NONCE_MAX_SIZE_IN_BYTES];

/*! Defines the CHACHA key buffer. */
typedef uint8_t CRYS_CHACHA_Key_t[CRYS_CHACHA_KEY_MAX_SIZE_IN_BYTES];


/************************ context Structs  ******************************/

/* The user's context prototype - the argument type that is passed by the user
   to the APIs called */
typedef struct CRYS_CHACHAUserContext_t {
	/* Allocated buffer must be double the size of actual context
	 * + 1 word for offset management */
	uint32_t buff[CRYS_CHACHA_USER_CTX_SIZE_IN_WORDS];
}CRYS_CHACHAUserContext_t;

/************************ Public Variables **********************/


/************************ Public Functions **********************/

/****************************************************************************************************/

/*!
@brief This function is used to initialize the context for CHACHA operations.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_chacha_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_CHACHA_Init(
                        CRYS_CHACHAUserContext_t    *pContextID,        /*!< [in]  Pointer to the CHACHA context buffer that is allocated by the user
                                                                                   and is used for the CHACHA operation. */
                        CRYS_CHACHA_Nonce_t          pNonce,         	/*!< [in]  A buffer containing an nonce. */
                        CRYS_CHACHA_NonceSize_t      nonceSize,         /*!< [in]  Enumerator defining the nonce size (only 64 and 96 bit are valid). */
                        CRYS_CHACHA_Key_t            pKey,               /*!< [in]  A pointer to the user's key buffer. */
                        uint32_t                     initialCounter,     /*!< [in]  An initial counter. */
                        CRYS_CHACHA_EncryptMode_t    EncryptDecryptFlag  /*!< [in]  A flag specifying whether the CHACHA should perform an Encrypt operation
                                                                                   or a Decrypt operation. */
);


/*!
@brief This function is used to process aligned blocks of CHACHA.
The data in size should be a multiple of chacha block size.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_chacha_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_CHACHA_Block(
                        CRYS_CHACHAUserContext_t    *pContextID,        /*!< [in] Pointer to the context buffer. */
                        uint8_t                     *pDataIn,           /*!< [in]  A pointer to the buffer of the input data to the CHACHA.
                                                                                   The pointer does not need to be aligned. must not be null. */
                        uint32_t                    dataInSize,         /*!< [in]  The size of the input data.
                                                                                   Must be a multiple of ::CRYS_CHACHA_BLOCK_SIZE_IN_BYTES bytes and must not be 0. */
                        uint8_t                     *pDataOut           /*!< [out] A pointer to the buffer of the output data from the CHACHA.
                                                                                   The pointer does not need to be aligned. must not be null. */
);


/*!
@brief This function is used to process the remaining data of CHACHA.
The data in size should be smaller than chacha block size.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_chacha_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_CHACHA_Finish(
                        CRYS_CHACHAUserContext_t    *pContextID,        /*!< [in]  Pointer to the context buffer. */
                        uint8_t                     *pDataIn,           /*!< [in]  A pointer to the buffer of the input data to the CHACHA.
                                                                                   The pointer does not need to be aligned. If dataInSize = 0, input buffer is not required. */
                        uint32_t                    dataInSize,         /*!< [in]  The size of the input data.
                                                                                   zero and non multiple of ::CRYS_CHACHA_BLOCK_SIZE_IN_BYTES are valid. */
                        uint8_t                     *pDataOut           /*!< [out] A pointer to the buffer of the output data from the CHACHA.
                                                                                   The pointer does not need to be aligned. If dataInSize = 0, output buffer is not required. */
);


/*!
@brief This function is used to free the context of CHACHA operations.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_chacha_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_CHACHA_Free(
                        CRYS_CHACHAUserContext_t *pContextID    /*!< [in] Pointer to the context buffer. */
);


/*!
@brief This function is used to perform the CHACHA operation in one integrated process.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_chacha_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_CHACHA(
                            CRYS_CHACHA_Nonce_t       	  pNonce,           /*!< [in]  A buffer containing an nonce. */
                            CRYS_CHACHA_NonceSize_t       nonceSize,               /*!< [in]  Enumerator defining the nonce size (only 64 and 96 bit are valid). */
                            CRYS_CHACHA_Key_t             pKey,                 /*!< [in]  A pointer to the user's key buffer. */
                            uint32_t                      initialCounter,       /*!< [in]  An initial counter. */
                            CRYS_CHACHA_EncryptMode_t     encryptDecryptFlag,   /*!< [in]  A flag specifying whether the CHACHA should perform an Encrypt operation
                                                                                                or a Decrypt operation. */
                            uint8_t                      *pDataIn,              /*!< [in]  A pointer to the buffer of the input data to the CHACHA.
                                                                                           The pointer does not need to be aligned. must not be null. */
                            uint32_t                      dataInSize,           /*!< [in]  The size of the input data. must not be 0. */
                            uint8_t                      *pDataOut              /*!< [out] A pointer to the buffer of the output data from the CHACHA.
                                                                                           The pointer does not need to be aligned. must not be null. */
);


/***********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CRYS_CHACHA_H */





