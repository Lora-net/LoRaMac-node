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
       CRYS POLY APIs, as well as the APIs themselves.
*/
#ifndef CRYS_POLY_H
#define CRYS_POLY_H


#include "ssi_pal_types.h"
#include "crys_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

/************************ Defines ******************************/
/*! The POLY KEY size in bytes. */
#define CRYS_POLY_KEY_SIZE_IN_WORDS		8
#define CRYS_POLY_KEY_SIZE_IN_BYTES		(CRYS_POLY_KEY_SIZE_IN_WORDS*SASI_32BIT_WORD_SIZE)

/*! The POLY MAC size in bytes. */
#define CRYS_POLY_MAC_SIZE_IN_WORDS		4
#define CRYS_POLY_MAC_SIZE_IN_BYTES		(CRYS_POLY_MAC_SIZE_IN_WORDS*SASI_32BIT_WORD_SIZE)

/************************ Typedefs  ****************************/

/*! Defines the CHACHA key buffer. */
typedef uint32_t CRYS_POLY_Mac_t[CRYS_POLY_MAC_SIZE_IN_WORDS];

/*! Defines the CHACHA key buffer. */
typedef uint32_t CRYS_POLY_Key_t[CRYS_POLY_KEY_SIZE_IN_WORDS];

/************************ Public Functions **********************/

/****************************************************************************************************/
/*!
@brief This function is used to perform the POLY MAC Calculation.

@return CRYS_OK on success.
@return A non-zero value on failure as defined crys_poly_error.h.
*/
CIMPORT_C CRYSError_t  CRYS_POLY(
                            CRYS_POLY_Key_t       pKey,            /*!< [in] A pointer to the user's key buffer. */
			    uint8_t              *pDataIn,        /*!< [in] A pointer to the buffer of the input data to the CHACHA.
                                                                             must not be null. */
                            size_t                dataInSize,      /*!< [in]  The size of the input data. must not be 0. */
			    CRYS_POLY_Mac_t	  macRes	   /*!< [in/out] Pointer to the MAC result buffer.*/
);



#ifdef __cplusplus
}
#endif

#endif /* #ifndef CRYS_POLY_H */





