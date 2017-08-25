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



#ifndef _SSI_PAL_SEM_INT__H
#define _SSI_PAL_SEM_INT__H



#ifdef __cplusplus
extern "C"
{
#endif
#include "ssi_pal_types.h"
/**
* @brief File Description:
*        This file contains functions for resource management (semaphor operations).
*        The functions implementations are generally just wrappers to different operating system calls.
*        None of the described functions will check the input parameters so the behavior
*        of the APIs in illegal parameters case is dependent on the operating system behavior.
*
*/

typedef int DX_PAL_SEM;



/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/**
 * @brief This function purpose is to create a semaphore.
 *
 *
 * @param[out] aSemId - Pointer to created semaphor handle
 * @param[in] aInitialVal - Initial semaphore value
 *
 * @return The return values is according to operating system return values.
 */
SaSiError_t _SaSi_PalSemCreate( DX_PAL_SEM *aSemId, uint32_t aInitialVal );

#define _SaSi_PalSemCreate(aSemId,aInitialVal) SASI_SUCCESS
/**
 * @brief This function purpose is to delete a semaphore
 *
 *
 * @param[in] aSemId - Semaphore handle
 *
 * @return The return values is according to operating system return values.
 */
SaSiError_t _SaSi_PalSemDelete( DX_PAL_SEM *aSemId );

#define _SaSi_PalSemDelete( aSemId ) SASI_SUCCESS
/**
 * @brief This function purpose is to Wait for semaphore with aTimeOut. aTimeOut is
 *        specified in milliseconds.
 *
 *
 * @param[in] aSemId - Semaphore handle
 * @param[in] aTimeOut - timeout in mSec, or SASI_INFINITE
 *
 * @return The return values is according to operating system return values.
 */
SaSiError_t _SaSi_PalSemWait(DX_PAL_SEM aSemId, uint32_t aTimeOut);

#define _SaSi_PalSemWait(aSemId, aTimeOut) SASI_SUCCESS
/**
 * @brief This function purpose is to signal the semaphore.
 *
 *
 * @param[in] aSemId - Semaphore handle
 *
 * @return The return values is according to operating system return values.
 */
SaSiError_t _SaSi_PalSemGive(DX_PAL_SEM aSemId);

#define _SaSi_PalSemGive(aSemId) SASI_SUCCESS


#ifdef __cplusplus
}
#endif

#endif


