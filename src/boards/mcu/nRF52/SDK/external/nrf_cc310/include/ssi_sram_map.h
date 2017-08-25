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

#ifndef _SSI_SRAM_MAP_H_
#define _SSI_SRAM_MAP_H_

/*!
@file
@brief This file contains internal SRAM mapping definitions.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#define SASI_SRAM_PKA_BASE_ADDRESS                                0x0
#define SASI_PKA_SRAM_SIZE_IN_KBYTES				  4


#define SASI_SRAM_RND_HW_DMA_ADDRESS                              0x0
#define SASI_SRAM_RND_MAX_SIZE                                    0x800    /*!< Addresses 0K-4KB in SRAM reserved for RND operations - same as the PKA. */

#define SASI_SRAM_MAX_SIZE                                       0x1000    /*!< 4KB in SRAM.  */

#ifdef __cplusplus
}
#endif

#endif /*_SSI_SRAM_MAP_H_*/
