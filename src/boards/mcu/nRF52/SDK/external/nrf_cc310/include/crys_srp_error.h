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

#ifndef CRYS_SRP_ERROR_H
#define CRYS_SRP_ERROR_H


#include "crys_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This module contains the definitions of the CRYS SRP errors.
*/



/************************ Defines ******************************/

/* The CRYS SRP module errors base address - 0x00F02600 */
#define CRYS_SRP_PARAM_INVALID_ERROR               (CRYS_SRP_MODULE_ERROR_BASE + 0x01UL)
#define CRYS_SRP_MOD_SIZE_INVALID_ERROR            (CRYS_SRP_MODULE_ERROR_BASE + 0x02UL)
#define CRYS_SRP_STATE_UNINITIALIZED_ERROR         (CRYS_SRP_MODULE_ERROR_BASE + 0x03UL)
#define CRYS_SRP_RESULT_ERROR         		   (CRYS_SRP_MODULE_ERROR_BASE + 0x04UL)
#define CRYS_SRP_PARAM_ERROR         		   (CRYS_SRP_MODULE_ERROR_BASE + 0x05UL)

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  *****************************/

/************************ Public Variables *********************/

/************************ Public Functions *********************/

#ifdef __cplusplus
}
#endif

#endif


