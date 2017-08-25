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


#ifndef CRYS_HMAC_ERROR_H
#define CRYS_HMAC_ERROR_H

#include "crys_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*! @file
*  @brief This module contains the definitions of the CRYS HMAC errors.
*
*/



/************************ Defines ******************************/

/* The CRYS HASH module errors */
#define CRYS_HMAC_INVALID_USER_CONTEXT_POINTER_ERROR     (CRYS_HMAC_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_HMAC_ILLEGAL_OPERATION_MODE_ERROR           (CRYS_HMAC_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_HMAC_USER_CONTEXT_CORRUPTED_ERROR           (CRYS_HMAC_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_HMAC_DATA_IN_POINTER_INVALID_ERROR          (CRYS_HMAC_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_HMAC_DATA_SIZE_ILLEGAL                      (CRYS_HMAC_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_HMAC_INVALID_RESULT_BUFFER_POINTER_ERROR    (CRYS_HMAC_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_HMAC_INVALID_KEY_POINTER_ERROR              (CRYS_HMAC_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_HMAC_UNVALID_KEY_SIZE_ERROR                 (CRYS_HMAC_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_HMAC_LAST_BLOCK_ALREADY_PROCESSED_ERROR	 (CRYS_HMAC_MODULE_ERROR_BASE + 0xBUL)
#define CRYS_HMAC_ILLEGAL_PARAMS_ERROR 			 (CRYS_HMAC_MODULE_ERROR_BASE + 0xCUL)

#define CRYS_HMAC_CTX_SIZES_ERROR   	                 (CRYS_HMAC_MODULE_ERROR_BASE + 0xEUL)

#define CRYS_HMAC_IS_NOT_SUPPORTED                       (CRYS_HMAC_MODULE_ERROR_BASE + 0xFUL)



/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


