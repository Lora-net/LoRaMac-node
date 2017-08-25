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


#ifndef _SSI_PAL_MUTEX_H
#define _SSI_PAL_MUTEX_H

#include "ssi_pal_mutex_plat.h"
#include "ssi_pal_types_plat.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
* @file
* @brief This file contains functions for resource management (mutex operations).
*        The functions implementations are generally just wrappers to different operating system calls.
*        None of the described functions will check the input parameters so the behavior
*        of the APIs in illegal parameters case is dependent on the operating system behavior.
*
*/




/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/**
 * @brief This function purpose is to create a mutex.
 *
 *
 * @return Zero on success.
 * @return A non-zero value on failure.
 */
SaSiError_t SaSi_PalMutexCreate(SaSi_PalMutex *pMutexId /*!< [out] Pointer to created mutex handle. */);


/**
 * @brief This function purpose is to destroy a mutex.
 *
 *
 * @return Zero on success.
 * @return A non-zero value on failure.
 */
SaSiError_t SaSi_PalMutexDestroy(SaSi_PalMutex *pMutexId /*!< [in] Pointer to mutex handle. */);


/**
 * @brief This function purpose is to Wait for Mutex with aTimeOut. aTimeOut is
 *        specified in milliseconds (SASI_INFINITE is blocking).
 *
 * @return Zero on success.
 * @return A non-zero value on failure.
 */
SaSiError_t SaSi_PalMutexLock (SaSi_PalMutex *pMutexId, /*!< [in] Pointer to Mutex handle. */
			       uint32_t aTimeOut	/*!< [in] Timeout in mSec, or SASI_INFINITE. */);


/**
 * @brief This function purpose is to release the mutex.
 *
 * @return Zero on success.
 * @return A non-zero value on failure.
 */
SaSiError_t SaSi_PalMutexUnlock (SaSi_PalMutex *pMutexId/*!< [in] Pointer to Mutex handle. */);





#ifdef __cplusplus
}
#endif

#endif


