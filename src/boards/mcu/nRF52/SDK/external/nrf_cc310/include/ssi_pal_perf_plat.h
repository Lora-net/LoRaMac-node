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

#ifndef _SSI_PAL_PERF_PLAT_H__
#define _SSI_PAL_PERF_PLAT_H__




typedef unsigned int SaSi_PalPerfData_t;

/**
 * @brief   DSM environment bug - sometimes very long write operation.
 * 	   to overcome this bug we added while to make sure write opeartion is completed
 *
 * @param[in]
 * *
 * @return None
 */
void SaSi_PalDsmWorkarround();


#endif /*_SSI_PAL_PERF_PLAT_H__*/
