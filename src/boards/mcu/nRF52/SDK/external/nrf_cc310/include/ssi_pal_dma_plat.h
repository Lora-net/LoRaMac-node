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



#ifndef _SSI_PAL_DMA_PLAT_H
#define _SSI_PAL_DMA_PLAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   stub function, the function should initialize the DMA mapping of the platform (if needed)
 *
 * @param[in] buffSize - buffer size in Bytes
 * @param[in] physBuffAddr - physical start address of the memory to map
 *
 * @return Virtual start address of contiguous memory
 */
extern uint32_t SaSi_PalDmaInit(uint32_t  buffSize,
			       uint32_t  physBuffAddr);

/**
 * @brief   free system resources created in PD_PAL_DmaInit()
 *
 *
 * @return void
 */
extern void SaSi_PalDmaTerminate(void);
#ifdef __cplusplus
}
#endif

#endif


