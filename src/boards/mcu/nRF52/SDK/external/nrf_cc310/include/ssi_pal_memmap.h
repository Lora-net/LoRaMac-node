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



#ifndef _SSI_PAL_MEMMAP_H
#define _SSI_PAL_MEMMAP_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "ssi_pal_types.h"
/*!
* @file
* @brief This file contains functions for memory mapping
*        None of the described functions will check the input parameters so the behavior
*        of the APIs in illegal parameters case is dependent on the operating system behavior.
*
*/


/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/**
 * @brief This function purpose is to return the base virtual address that maps the
 *        base physical address
 *
 * @return Zero on success.
 * @return A non-zero value in case of failure.
 */
uint32_t SaSi_PalMemMap(uint32_t physicalAddress, /*!< [in] Start physical address of the I/O range to be mapped. */
	                uint32_t mapSize,	  /*!< [in] Number of bytes that were mapped. */
		        uint32_t **ppVirtBuffAddr /*!< [out] Pointer to the base virtual address to which the physical pages were mapped. */ );


/**
 * @brief This function purpose is to Unmap a specified address range previously mapped
 *        by SaSi_PalMemMap.
 *
 * @return Zero on success.
 * @return A non-zero value in case of failure.
 */
uint32_t SaSi_PalMemUnMap(uint32_t *pVirtBuffAddr, /*!< [in] Pointer to the base virtual address to which the physical pages were mapped. */
	                  uint32_t mapSize	   /*!< [in] Number of bytes that were mapped. */);

#ifdef __cplusplus
}
#endif

#endif


