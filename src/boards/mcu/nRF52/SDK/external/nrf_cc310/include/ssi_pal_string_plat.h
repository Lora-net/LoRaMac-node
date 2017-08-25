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



#ifndef _SSI_PAL_STRING_INT_H
#define _SSI_PAL_STRING_INT_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
/**
* @brief File Description:
*        This file contains wrapper functions for string manipulation.
*/


/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/**
 * @brief A wrapper function for strlen functionality. The function returns the size of a given string
 *
 */
#define  _SaSi_PalStrLen     strlen

/**
 * @brief A wrapper function for strstr functionality. The functions find a string in a string and
 *        return a pointer to it.
 *
 */
#define _SaSi_PalFindStr     strstr

/**
 * @brief A wrapper function for strchr functionality. The function finds a char in a given string.
 *
 */
#define _SaSi_PalStrChr      strchr

/**
 * @brief A wrapper function for strrchr functionality. The function finds a char inside a string
 *        (from the end) and returns a pointer to it
 *
 */
#define _SaSi_PalStrRChr     strrchr

/**
 * @brief A wrapper for strncpy functionality. The function copies a string.
 *
 */
#define _SaSi_PalStrNCopy    strncpy


#ifdef __cplusplus
}
#endif

#endif


