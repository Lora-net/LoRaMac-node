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

/*! @file
@brief This file contains basic type definitions that are platform dependent.
*/
#ifndef SSI_PAL_TYPES_PLAT_H
#define SSI_PAL_TYPES_PLAT_H
/* Host specific types for standard (ISO-C99) compilant platforms */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uintptr_t	    SaSiVirtAddr_t;
typedef uint32_t            SaSiBool_t;
typedef uint32_t            SaSiStatus;

#define SaSiError_t         SaSiStatus
#define SASI_INFINITE       0xFFFFFFFF

#define CEXPORT_C
#define CIMPORT_C

#endif /*SSI_PAL_TYPES_PLAT_H*/
