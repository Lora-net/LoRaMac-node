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

#ifndef _SSI_PAL_ABORT_H
#define _SSI_PAL_ABORT_H


#include "ssi_pal_abort_plat.h"

/*!
@file
@brief This file contains definitions for PAL Abort API.
*/

/*!
This function performs the "Abort" operation, should be implemented according to platform and OS.
*/

#define SaSi_PalAbort(msg) _SaSi_PalAbort(msg)

#endif


