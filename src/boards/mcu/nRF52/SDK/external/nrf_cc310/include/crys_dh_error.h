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


#ifndef CRYS_DH_ERROR_H
#define CRYS_DH_ERROR_H


#include "crys_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This file contains error codes definitions for CRYS DH module.
*/
/************************ Defines ******************************/
/* DH module on the CRYS layer base address - 0x00F00500 */
/* The CRYS DH module errors */
#define CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR		(CRYS_DH_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_DH_INVALID_ARGUMENT_HASH_MODE_ERROR		(CRYS_DH_MODULE_ERROR_BASE + 0x3UL)

/*the derived secret key size needed is wrong*/
#define CRYS_DH_SECRET_KEYING_DATA_SIZE_ILLEGAL_ERROR		(CRYS_DH_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_DH_INVALID_L_ARGUMENT_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_DH_ARGUMENT_PRIME_SMALLER_THAN_GENERATOR_ERROR	(CRYS_DH_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_DH_ARGUMENT_GENERATOR_SMALLER_THAN_ZERO_ERROR    	(CRYS_DH_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_DH_ARGUMENT_PRV_SIZE_ERROR				(CRYS_DH_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_DH_ARGUMENT_BUFFER_SIZE_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x9UL)
#define CRYS_DH_INVALID_SHARED_SECRET_VALUE_ERROR		(CRYS_DH_MODULE_ERROR_BASE + 0xAUL)
#define CRYS_DH_IS_NOT_SUPPORTED				(CRYS_DH_MODULE_ERROR_BASE + 0xFUL)

#define CRYS_DH_X942_HYBRID_SIZE1_BUFFER_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x15UL)

/*The requested derived secret key size is invalid*/
#define CRYS_DH_SECRET_KEY_SIZE_NEEDED_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x16UL)
#define CRYS_DH_SECRET_KEY_SIZE_OUTPUT_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x17UL)
#define CRYS_DH_OTHERINFO_SIZE_ERROR                            (CRYS_DH_MODULE_ERROR_BASE + 0x18UL)

/* DH domain and key generation and checking errors */
#define CRYS_DH_INVALID_MODULUS_SIZE_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x20UL)
#define CRYS_DH_INVALID_ORDER_SIZE_ERROR			(CRYS_DH_MODULE_ERROR_BASE + 0x21UL)
#define CRYS_DH_INVALID_SEED_SIZE_ERROR  			(CRYS_DH_MODULE_ERROR_BASE + 0x22UL)
#define CRYS_DH_INVALID_J_FACTOR_PTR_OR_SIZE_ERROR 		(CRYS_DH_MODULE_ERROR_BASE + 0x23UL)
#define CRYS_DH_INVALID_GENERATOR_PTR_OR_SIZE_ERROR 		(CRYS_DH_MODULE_ERROR_BASE + 0x24UL)

#define CRYS_DH_CHECK_DOMAIN_PRIMES_NOT_VALID_ERROR         	(CRYS_DH_MODULE_ERROR_BASE + 0x25UL)
#define CRYS_DH_CHECK_DOMAIN_GENERATOR_NOT_VALID_ERROR         	(CRYS_DH_MODULE_ERROR_BASE + 0x26UL)
#define CRYS_DH_INVALID_PUBLIC_KEY_SIZE_ERROR                 	(CRYS_DH_MODULE_ERROR_BASE + 0x27UL)
#define CRYS_DH_CHECK_PUB_KEY_NOT_VALID_ERROR                 	(CRYS_DH_MODULE_ERROR_BASE + 0x28UL)
#define CRYS_DH_CHECK_GENERATOR_SIZE_OR_PTR_NOT_VALID_ERROR    	(CRYS_DH_MODULE_ERROR_BASE + 0x29UL)
#define CRYS_DH_CHECK_SEED_SIZE_OR_PTR_NOT_VALID_ERROR          (CRYS_DH_MODULE_ERROR_BASE + 0x2AUL)
#define CRYS_DH_CHECK_GENERATOR_NOT_VALID_ERROR               	(CRYS_DH_MODULE_ERROR_BASE + 0x2BUL)
#define CRYS_DH_PRIME_P_GENERATION_FAILURE_ERROR               	(CRYS_DH_MODULE_ERROR_BASE + 0x2CUL)
#define CRYS_DH_INVALID_PUBLIC_KEY_ERROR                    	(CRYS_DH_MODULE_ERROR_BASE + 0x2DUL)
#define CRYS_DH_PASSED_INVALID_SEED_ERROR  	            	(CRYS_DH_MODULE_ERROR_BASE + 0x2EUL)

#define CRYS_DH_HOST_MSG_GENERAL_RPC_A_ERROR                    (CRYS_DH_MODULE_ERROR_BASE + 0x30UL)
#define CRYS_DH_HOST_MSG_GENERAL_RPC_B_ERROR                    (CRYS_DH_MODULE_ERROR_BASE + 0x31UL)
#define CRYS_DH_HOST_MSG_GENERAL_RPC_C_ERROR                    (CRYS_DH_MODULE_ERROR_BASE + 0x32UL)





/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/




#ifdef __cplusplus
}
#endif

#endif

