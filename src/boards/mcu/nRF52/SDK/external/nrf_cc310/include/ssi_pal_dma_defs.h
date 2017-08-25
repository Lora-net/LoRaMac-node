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


/*!
@file
@brief This file contains the platform dependent DMA definitions.
*/

#ifndef _SSI_PAL_DMA_DEFS_H
#define _SSI_PAL_DMA_DEFS_H


#ifdef __cplusplus
extern "C"
{
#endif

typedef void * SaSi_PalDmaBufferHandle;

/* DMA directions configuration */
typedef enum {
	SASI_PAL_DMA_DIR_NONE = 0, /*!< No direction. */
	SASI_PAL_DMA_DIR_TO_DEVICE = 1,	/*!< The original buffer is the input to the operation, and should be copied/mapped to a temp buffer,
					     prior to activating the HW on the temp buffer. */
	SASI_PAL_DMA_DIR_FROM_DEVICE = 2, /*!< The temp buffer holds the output of the HW, and this API should copy/map it to the original output buffer.*/
	SASI_PAL_DMA_DIR_BI_DIRECTION = 3, /*!< Used when the result is written over the original data at the same address. should be treated as 1 & 2.*/
	SASI_PAL_DMA_DIR_MAX,
	SASI_PAL_DMA_DIR_RESERVE32 = 0x7FFFFFFF
}SaSi_PalDmaBufferDirection_t;


#ifdef __cplusplus
}
#endif

#endif


