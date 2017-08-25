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

#ifndef CRYS_CHACHA_POLY_ERROR_H
#define CRYS_CHACHA_POLY_ERROR_H


#include "crys_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module contains the definitions of the CRYS CHACHA POLY errors.
*/



/************************ Defines ******************************/

/* The CRYS CHACHA POLY module errors base address - 0x00F02400 */
#define CRYS_CHACHA_POLY_ADATA_INVALID_ERROR                  	(CRYS_CHACHA_POLY_MODULE_ERROR_BASE + 0x01UL)
#define CRYS_CHACHA_POLY_DATA_INVALID_ERROR                 	(CRYS_CHACHA_POLY_MODULE_ERROR_BASE + 0x02UL)
#define CRYS_CHACHA_POLY_ENC_MODE_INVALID_ERROR              	(CRYS_CHACHA_POLY_MODULE_ERROR_BASE + 0x03UL)
#define CRYS_CHACHA_POLY_DATA_SIZE_INVALID_ERROR             	(CRYS_CHACHA_POLY_MODULE_ERROR_BASE + 0x04UL)
#define CRYS_CHACHA_POLY_GEN_KEY_ERROR          		(CRYS_CHACHA_POLY_MODULE_ERROR_BASE + 0x05UL)
#define CRYS_CHACHA_POLY_ENCRYPTION_ERROR         		(CRYS_CHACHA_POLY_MODULE_ERROR_BASE + 0x06UL)
#define CRYS_CHACHA_POLY_AUTH_ERROR          			(CRYS_CHACHA_POLY_MODULE_ERROR_BASE + 0x07UL)
#define CRYS_CHACHA_POLY_MAC_ERROR                             	(CRYS_CHACHA_POLY_MODULE_ERROR_BASE + 0x08UL)

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  *****************************/

/************************ Public Variables *********************/

/************************ Public Functions *********************/

#ifdef __cplusplus
}
#endif

#endif


