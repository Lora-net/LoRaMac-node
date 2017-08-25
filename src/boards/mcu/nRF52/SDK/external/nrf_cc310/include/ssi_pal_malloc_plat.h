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



#ifndef _SSI_PAL_MEMALLOC_INT_H
#define _SSI_PAL_MEMALLOC_INT_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
/**
* @brief File Description:
*        This file contains wrappers for memory operations APIs.
*/


/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/**
 * @brief A wrapper over malloc functionality. The function allocates a buffer according to given size
 *
 */
 #define _SaSi_PalMemMalloc    malloc


/**
 * @brief A wrapper over realloc functionality. The function allocates and copy a buffer
 *        according to size
 *
 */
#define _SaSi_PalMemRealloc    realloc


/**
 * @brief A wrapper over free functionality/ The function will free allocated memory.
 *
 */
#define _SaSi_PalMemFree       free



#ifdef __cplusplus
}
#endif

#endif


