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


#ifndef PKA_HW_PLAT_DEFS_H
#define PKA_HW_PLAT_DEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief Contains the enums and definitions that are used in the PKA code (definitions that are platform dependent).
*/

#define SASI_PKA_WORD_SIZE_IN_BITS	64

#define CRYS_SRP_MAX_MODULUS_SIZE_IN_BITS       3072

#define CRYS_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS       2048

#define CRYS_RSA_MAX_KEY_GENERATION_HW_SIZE_BITS        2048

/* PKA operations maximal count of extra bits: */
#define PKA_EXTRA_BITS  8
#define PKA_MAX_COUNT_OF_PHYS_MEM_REGS  32


#ifdef __cplusplus
}
#endif

#endif //PKA_HW_PLAT_DEFS_H


