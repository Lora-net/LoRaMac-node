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

#ifndef CRYS_HKDF_ERROR_H
#define CRYS_HKDF_ERROR_H

#include "crys_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module contains the definitions of the CRYS HKDF errors.
 */


/************************ Defines *******************************/

/* The CRYS KDF module errors / base address - 0x00F01100*/
#define CRYS_HKDF_INVALID_ARGUMENT_POINTER_ERROR				(CRYS_HKDF_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_HKDF_INVALID_ARGUMENT_SIZE_ERROR					(CRYS_HKDF_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_HKDF_INVALID_ARGUMENT_HASH_MODE_ERROR				(CRYS_HKDF_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_HKDF_IS_NOT_SUPPORTED                              (CRYS_HKDF_MODULE_ERROR_BASE + 0xFFUL)

/************************ Enums *********************************/

/************************ Typedefs  *****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/




#ifdef __cplusplus
}
#endif

#endif




