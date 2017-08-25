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

#ifndef CRYS_KDF_ERROR_H
#define CRYS_KDF_ERROR_H

#include "crys_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module contains the definitions of the CRYS KDF errors.
 */


/************************ Defines *******************************/

/* The CRYS KDF module errors / base address - 0x00F01100*/
#define CRYS_KDF_INVALID_ARGUMENT_POINTER_ERROR			(CRYS_KDF_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_KDF_INVALID_ARGUMENT_SIZE_ERROR			(CRYS_KDF_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_KDF_INVALID_ARGUMENT_OPERATION_MODE_ERROR		(CRYS_KDF_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_KDF_INVALID_ARGUMENT_HASH_MODE_ERROR		(CRYS_KDF_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_KDF_INVALID_KEY_DERIVATION_MODE_ERROR              (CRYS_KDF_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_KDF_INVALID_SHARED_SECRET_VALUE_SIZE_ERROR         (CRYS_KDF_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_KDF_INVALID_OTHER_INFO_SIZE_ERROR                  (CRYS_KDF_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_KDF_INVALID_KEYING_DATA_SIZE_ERROR                 (CRYS_KDF_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_KDF_INVALID_ALGORITHM_ID_POINTER_ERROR             (CRYS_KDF_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_KDF_INVALID_ALGORITHM_ID_SIZE_ERROR                (CRYS_KDF_MODULE_ERROR_BASE + 0x9UL)

#define CRYS_KDF_HOST_MSG_GENERAL_RPC_A_ERROR                   (CRYS_KDF_MODULE_ERROR_BASE + 0xAUL)
#define CRYS_KDF_HOST_MSG_GENERAL_RPC_B_ERROR                   (CRYS_KDF_MODULE_ERROR_BASE + 0xBUL)
#define CRYS_KDF_HOST_MSG_GENERAL_RPC_C_ERROR                   (CRYS_KDF_MODULE_ERROR_BASE + 0xCUL)

#define CRYS_KDF_IS_NOT_SUPPORTED                               (CRYS_KDF_MODULE_ERROR_BASE + 0xFFUL)

/************************ Enums *********************************/

/************************ Typedefs  *****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/




#ifdef __cplusplus
}
#endif

#endif




