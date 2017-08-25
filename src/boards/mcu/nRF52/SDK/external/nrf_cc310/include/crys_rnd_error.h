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



#ifndef CRYS_RND_ERROR_H
#define CRYS_RND_ERROR_H

#include "crys_error.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*!
@file
@brief This module contains the definitions of the CRYS RND errors.
*/




/************************ Defines ******************************/
/* RND module on the CRYS layer base address - 0x00F00C00 */

#define CRYS_RND_DATA_OUT_POINTER_INVALID_ERROR        (CRYS_RND_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_RND_CAN_NOT_GENERATE_RAND_IN_RANGE        (CRYS_RND_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_RND_CPRNG_TEST_FAIL_ERROR		       (CRYS_RND_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_RND_ADDITIONAL_INPUT_BUFFER_NULL	       (CRYS_RND_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_RND_ADDITIONAL_INPUT_SIZE_ERROR	       (CRYS_RND_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_RND_DATA_SIZE_OVERFLOW_ERROR	       (CRYS_RND_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_RND_VECTOR_SIZE_ERROR			(CRYS_RND_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_RND_RESEED_COUNTER_OVERFLOW_ERROR		(CRYS_RND_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_RND_INSTANTIATION_NOT_DONE_ERROR		(CRYS_RND_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_RND_TRNG_LOSS_SAMPLES_ERROR               (CRYS_RND_MODULE_ERROR_BASE + 0x9UL)
#define CRYS_RND_TRNG_TIME_EXCEED_ERROR         	(CRYS_RND_MODULE_ERROR_BASE + 0xAUL)
#define CRYS_RND_TRNG_LOSS_SAMPLES_AND_TIME_EXCEED_ERROR (CRYS_RND_MODULE_ERROR_BASE + 0xBUL)
#define CRYS_RND_IS_KAT_MODE_ERROR                     (CRYS_RND_MODULE_ERROR_BASE + 0xCUL)
#define CRYS_RND_OPERATION_IS_NOT_SUPPORTED_ERROR      (CRYS_RND_MODULE_ERROR_BASE + 0xDUL)
#define CRYS_RND_STATE_VALIDATION_TAG_ERROR            (CRYS_RND_MODULE_ERROR_BASE + 0xEUL)

#define CRYS_RND_IS_NOT_SUPPORTED                      (CRYS_RND_MODULE_ERROR_BASE + 0xFUL)

#define CRYS_RND_GEN_A_ERROR			                (CRYS_RND_MODULE_ERROR_BASE + 0x10UL)
#define CRYS_RND_GEN_B_ERROR			                (CRYS_RND_MODULE_ERROR_BASE + 0x11UL)
#define CRYS_RND_GEN_C_ERROR			                (CRYS_RND_MODULE_ERROR_BASE + 0x12UL)
#define CRYS_RND_ILLEGAL_PARAMS_ACCORDING_TO_PRIV_ERROR (CRYS_RND_MODULE_ERROR_BASE + 0x13UL)
#define CRYS_RND_GEN_VECTOR_FUNC_ERROR			        (CRYS_RND_MODULE_ERROR_BASE + 0x14UL)

#define CRYS_RND_WORK_BUFFER_PTR_INVALID_ERROR          (CRYS_RND_MODULE_ERROR_BASE + 0x20UL)
#define CRYS_RND_ILLEGAL_AES_KEY_SIZE_ERROR             (CRYS_RND_MODULE_ERROR_BASE + 0x21UL)
#define CRYS_RND_ILLEGAL_DATA_PTR_ERROR                 (CRYS_RND_MODULE_ERROR_BASE + 0x22UL)
#define CRYS_RND_ILLEGAL_DATA_SIZE_ERROR                (CRYS_RND_MODULE_ERROR_BASE + 0x23UL)
#define CRYS_RND_ILLEGAL_PARAMETER_ERROR                (CRYS_RND_MODULE_ERROR_BASE + 0x24UL)
#define CRYS_RND_STATE_PTR_INVALID_ERROR                (CRYS_RND_MODULE_ERROR_BASE + 0x25UL)
#define CRYS_RND_TRNG_ERRORS_ERROR                      (CRYS_RND_MODULE_ERROR_BASE + 0x26UL)
#define CRYS_RND_CONTEXT_PTR_INVALID_ERROR              (CRYS_RND_MODULE_ERROR_BASE + 0x27UL)

#define CRYS_RND_VECTOR_OUT_PTR_ERROR                   (CRYS_RND_MODULE_ERROR_BASE + 0x30UL)
#define CRYS_RND_VECTOR_OUT_SIZE_ERROR			(CRYS_RND_MODULE_ERROR_BASE + 0x31UL)
#define CRYS_RND_MAX_VECTOR_IS_TOO_SMALL_ERROR		(CRYS_RND_MODULE_ERROR_BASE + 0x32UL)
#define CRYS_RND_KAT_DATA_PARAMS_ERROR                 	(CRYS_RND_MODULE_ERROR_BASE + 0x33UL)
#define CRYS_RND_TRNG_KAT_NOT_SUPPORTED_ERROR         	(CRYS_RND_MODULE_ERROR_BASE + 0x34UL)
#define CRYS_RND_SRAM_NOT_SUPPORTED_ERROR               (CRYS_RND_MODULE_ERROR_BASE + 0x35UL)


/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


