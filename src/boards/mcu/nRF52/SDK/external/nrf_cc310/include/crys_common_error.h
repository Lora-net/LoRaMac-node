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


 #ifndef CRYS_COMMON_ERROR_H
#define CRYS_COMMON_ERROR_H

#include "crys_error.h"

#ifdef __cplusplus
extern "C"
{
#endif


/************************ Defines ******************************/

/* CRYS COMMON module errors. Base address - 0x00F00D00 */

#define CRYS_COMMON_INIT_HW_SEM_CREATION_FAILURE    (CRYS_COMMON_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_COMMON_DATA_IN_POINTER_INVALID_ERROR   (CRYS_COMMON_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_COMMON_DATA_SIZE_ILLEGAL		    (CRYS_COMMON_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_COMMON_DATA_OUT_DATA_IN_OVERLAP_ERROR  (CRYS_COMMON_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_COMMON_DATA_OUT_POINTER_INVALID_ERROR  (CRYS_COMMON_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_COMMON_OUTPUT_BUFF_SIZE_ILLEGAL	    (CRYS_COMMON_MODULE_ERROR_BASE + 0x9UL)

#define CRYS_COMMON_TST_UTIL_CHUNK_SIZE_SMALL_ERROR (CRYS_COMMON_MODULE_ERROR_BASE + 0x10UL)
#define CRYS_COMMON_ERROR_IN_SAVING_LLI_DATA_ERROR  (CRYS_COMMON_MODULE_ERROR_BASE + 0x11UL)


#define CRYS_COMMON_TST_UTIL_LLI_ENTRY_SIZE_TOO_SMALL_ERROR   (CRYS_COMMON_MODULE_ERROR_BASE + 0x12UL)
#define CRYS_COMMON_TST_CSI_DATA_SIZE_EXCEED_ERROR            (CRYS_COMMON_MODULE_ERROR_BASE + 0x13UL)
#define CRYS_COMMON_TST_CSI_MODULE_ID_OUT_OF_RANGE            (CRYS_COMMON_MODULE_ERROR_BASE + 0x14UL)
#define CRYS_COMMON_TST_CSI_MEMORY_MAPPING_ERROR              (CRYS_COMMON_MODULE_ERROR_BASE + 0x15UL)

#define CRYS_COMMON_TERM_HW_SEM_DELETE_FAILURE                (CRYS_COMMON_MODULE_ERROR_BASE + 0x16UL)

#define CRYS_COMMON_TST_UTIL_NOT_INTEGER_CHAR_ERROR           (CRYS_COMMON_MODULE_ERROR_BASE + 0x17UL)
#define CRYS_COMMON_TST_UTIL_BUFFER_IS_SMALL_ERROR            (CRYS_COMMON_MODULE_ERROR_BASE + 0x18UL)
#define CRYS_COMMON_POINTER_NOT_ALIGNED_ERROR                 (CRYS_COMMON_MODULE_ERROR_BASE + 0x19UL)


/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


