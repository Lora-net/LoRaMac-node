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

#ifndef _SSI_PAL_ERROR_H
#define _SSI_PAL_ERROR_H

/*!
@file
@brief This file contains the platform dependent error definitions.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#define SASI_PAL_BASE_ERROR                0x0F000000

/* Memory error returns */
#define SASI_PAL_MEM_BUF1_GREATER          SASI_PAL_BASE_ERROR + 0x01UL
#define SASI_PAL_MEM_BUF2_GREATER          SASI_PAL_BASE_ERROR + 0x02UL

/* Semaphore error returns */
#define SASI_PAL_SEM_CREATE_FAILED         SASI_PAL_BASE_ERROR + 0x03UL
#define SASI_PAL_SEM_DELETE_FAILED         SASI_PAL_BASE_ERROR + 0x04UL
#define SASI_PAL_SEM_WAIT_TIMEOUT          SASI_PAL_BASE_ERROR + 0x05UL
#define SASI_PAL_SEM_WAIT_FAILED           SASI_PAL_BASE_ERROR + 0x06UL
#define SASI_PAL_SEM_RELEASE_FAILED        SASI_PAL_BASE_ERROR + 0x07UL

#define SASI_PAL_ILLEGAL_ADDRESS	   SASI_PAL_BASE_ERROR + 0x08UL

#ifdef __cplusplus
}
#endif

#endif


