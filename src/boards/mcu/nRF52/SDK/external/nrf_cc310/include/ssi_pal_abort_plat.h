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

#ifndef _SSI_PAL_ABORT_PLAT_H
#define _SSI_PAL_ABORT_PLAT_H

#include "ssi_pal_log.h"
#include <stdlib.h>

#define _SaSi_PalAbort(exp)	do {					\
	SASI_PAL_LOG_ERR("ASSERT:%s:%d: %s", __FILE__, __LINE__, (#exp));	\
	abort();							\
	} while (0)

#endif
