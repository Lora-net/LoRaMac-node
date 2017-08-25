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



#ifndef CRYS_HASH_ERROR_H
#define CRYS_HASH_ERROR_H


#include "crys_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module contains the definitions of the CRYS HASH errors.
*/




/************************ Defines ******************************/
/*HASH module on the CRYS layer base address - 0x00F00200*/
/* The CRYS HASH module errors */
#define CRYS_HASH_INVALID_USER_CONTEXT_POINTER_ERROR    	(CRYS_HASH_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_HASH_ILLEGAL_OPERATION_MODE_ERROR          	(CRYS_HASH_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_HASH_USER_CONTEXT_CORRUPTED_ERROR          	(CRYS_HASH_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_HASH_DATA_IN_POINTER_INVALID_ERROR         	(CRYS_HASH_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_HASH_DATA_SIZE_ILLEGAL                     	(CRYS_HASH_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_HASH_INVALID_RESULT_BUFFER_POINTER_ERROR   	(CRYS_HASH_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_HASH_DATA_SIZE_IS_ILLEGAL_FOR_CSI_ERROR    	(CRYS_HASH_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_HASH_LAST_BLOCK_ALREADY_PROCESSED_ERROR		(CRYS_HASH_MODULE_ERROR_BASE + 0xCUL)
#define CRYS_HASH_ILLEGAL_PARAMS_ERROR 				(CRYS_HASH_MODULE_ERROR_BASE + 0xDUL)

#define CRYS_HASH_CTX_SIZES_ERROR   	                	(CRYS_HASH_MODULE_ERROR_BASE + 0xEUL)

#define CRYS_HASH_IS_NOT_SUPPORTED                      	(CRYS_HASH_MODULE_ERROR_BASE + 0xFUL)



/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


