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



#ifndef _SSI_PAL_MEM_PLAT_H
#define _SSI_PAL_MEM_PLAT_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <string.h>
/**
* @brief File Description:
*        This file contains the implementation for memory operations APIs.
*        The functions implementations are generally just wrappers to different operating system calls.
*/


/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/**
 * @brief A wrapper over memcmp functionality. The function compares two given buffers
 *        according to size.
 */
#define _SaSi_PalMemCmp        memcmp

/**
 * @brief A wrapper over memmove functionality, the function copies from one
 *        buffer to another according to given size
 *
 */
#define _SaSi_PalMemCopy       memmove

#define	_SaSi_PalMemMove	      memmove

/**
 * @brief A wrapper over memset functionality, the function sets a buffer with given value
 *        according to size
 *
 */
#define _SaSi_PalMemSet(aTarget, aChar, aSize)        memset(aTarget, aChar, aSize)

/**
 * @brief A wrapper over memset functionality, the function sets a buffer with zeroes
 *        according to size
 *
 */
#define _SaSi_PalMemSetZero(aTarget, aSize)    _SaSi_PalMemSet(aTarget,0x00, aSize)



#ifdef __cplusplus
}
#endif

#endif


