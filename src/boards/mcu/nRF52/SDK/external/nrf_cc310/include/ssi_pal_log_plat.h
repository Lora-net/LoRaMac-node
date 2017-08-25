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

#ifndef _SSI_PAL_LOG_PLAT_H_
#define _SSI_PAL_LOG_PLAT_H_

#include "dx_log_mask.h"
#include <stdio.h>

/************** PRINTF rules ******************/
#if defined(DEBUG)

#define PRINTF printf

#else /* Disable all prints */

#define PRINTF(...)  do {} while (0)

#endif

#define __SASI_PAL_LOG_PLAT(level, format, ...) PRINTF(format, ##__VA_ARGS__)

#endif /*_SSI_PAL_LOG_PLAT_H_*/
