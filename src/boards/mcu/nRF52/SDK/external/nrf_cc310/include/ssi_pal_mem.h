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



#ifndef _SSI_PAL_MEM_H
#define _SSI_PAL_MEM_H


#ifdef __cplusplus
extern "C"
{
#endif

#include "ssi_pal_types.h"
#include "ssi_pal_mem_plat.h"
#include "ssi_pal_malloc_plat.h"

/*!
@file
@brief This file contains functions for memory operations. The functions implementations
*     are generally just wrappers to different operating system calls.
*     None of the described functions will check the input parameters so the behavior
*     of the APIs in illegal parameters case is dependent on the operating system behavior.
*/


/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/**** ----- Memory Operations APIs ----- ****/

/*!
* @brief This function purpose is to compare between two given buffers according to given size.
*
* @return The return values is according to operating system return values.
*/


int32_t SaSi_PalMemCmp(	const void* aTarget, /*!< [in] The target buffer to compare. */
	                const void* aSource, /*!< [in] The Source buffer to compare to. */
		        uint32_t  aSize	     /*!< [in] Number of bytes to compare. */);

/* Definition for MemCmp */
#define  SaSi_PalMemCmp    _SaSi_PalMemCmp

/*!
 * @brief This function purpose is to perform secured memory comparison between two given
 *        buffers according to given size. The function will compare each byte till aSize
 *        number of bytes was compared even if the bytes are different.
 *        The function should be used to avoid security timing attacks.
 *
 * @return SASI_SUCCESS in case of success,
 * @return value on failure as defined in  ssi_pal_error.h.
 */
SaSiError_t SaSi_PalSecMemCmp(	const uint8_t* aTarget,	 /*!< [in] The target buffer to compare. */
	                  	const uint8_t* aSource,	 /*!< [in] The Source buffer to compare to. */
		                uint32_t  aSize		 /*!< [in] Number of bytes to compare. */);

/*!
 * @brief This function purpose is to copy aSize bytes from source buffer to destination buffer.
 *
 * @return void.
 */
void SaSi_PalMemCopy(	const void* aDestination, /*!< [out] The destination buffer to copy bytes to. */
	                const void* aSource,	  /*!< [in] The Source buffer to copy from. */
		        uint32_t  aSize		  /*!< [in] Number of bytes to copy. */ );

/*!
 * @brief This function purpose is to copy aSize bytes from source buffer to destination buffer.
 * This function Supports overlapped buffers.
 *
 * @return void.
 */
void SaSi_PalMemMove(	const void* aDestination, /*!< [out] The destination buffer to copy bytes to. */
	              	const void* aSource,	  /*!< [in] The Source buffer to copy from. */
		        uint32_t  aSize		  /*!< [in] Number of bytes to copy. */);

/* Definition for MemCopy */
#define SaSi_PalMemCopy    _SaSi_PalMemCopy
#define SaSi_PalMemMove    _SaSi_PalMemMove


/*!
 * @brief This function purpose is to set aSize bytes in the given buffer with aChar.
 *
 * @return void.
 */
void SaSi_PalMemSet(	const void* aTarget, /*!< [out]  The target buffer to set. */
	                const uint8_t aChar, /*!< [in] The char to set into aTarget. */
		        uint32_t  aSize	     /*!< [in] Number of bytes to set. */);

/* Definition for MemSet */
#define SaSi_PalMemSet(aTarget, aChar, aSize)   _SaSi_PalMemSet(aTarget, aChar, aSize)

/*!
 * @brief This function purpose is to set aSize bytes in the given buffer with zeroes.
 *
 * @return void.
 */
void SaSi_PalMemSetZero(	const void* aTarget, /*!< [out]  The target buffer to set. */
		                uint32_t  aSize	     /*!< [in] Number of bytes to set. */);

#define SaSi_PalMemSetZero(aTarget, aSize)   _SaSi_PalMemSetZero(aTarget, aSize)

/**** ----- Memory Allocation APIs ----- ****/

/*!
 * @brief This function purpose is to allocate a memory buffer according to aSize.
 *
 *
 * @return The function will return a pointer to allocated buffer or NULL if allocation failed.
 */
void* SaSi_PalMemMalloc(uint32_t aSize /*!< [in] Number of bytes to allocate. */);

/* Definition for MemMalloc */
#define SaSi_PalMemMalloc  _SaSi_PalMemMalloc

/*!
 * @brief This function purpose is to reallocate a memory buffer according to aNewSize.
 *        The content of the old buffer is moved to the new location.
 *
 * @return The function will return a pointer to the newly allocated buffer or NULL if allocation failed.
 */
void* SaSi_PalMemRealloc(  void* aBuffer, 	/*!< [in] Pointer to allocated buffer. */
                           uint32_t aNewSize 	/*!< [in] Number of bytes to reallocate. */);

/* Definition for MemRealloc */
#define SaSi_PalMemRealloc  _SaSi_PalMemRealloc

/*!
 * @brief This function purpose is to free allocated buffer.
 *
 *
 * @return void.
 */
void SaSi_PalMemFree(void* aBuffer /*!< [in] Pointer to allocated buffer.*/);

/* Definition for MemFree */
#define SaSi_PalMemFree  _SaSi_PalMemFree

#ifdef __cplusplus
}
#endif

#endif


