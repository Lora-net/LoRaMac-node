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


#ifndef _SSI_PAL_INIT_H
#define _SSI_PAL_INIT_H

#include "ssi_pal_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This file contains the PAL layer entry point, it includes the definitions and APIs for PAL initialization and termination.
*/

/**
 * @brief This function Performs all initializations that may be required by the customer's PAL implementation, specifically by the DMA-able buffer
 *	scheme. The existing implementation allocates a contiguous memory pool that is later used by the ARM TrustZone CryptoCell TEE implementation.
 * 	In case no initializations are needed in the customer's environment, the function can be minimized to return OK.
 *      It is called by ::SaSi_LibInit.
 *
 * @return A non-zero value in case of failure.
 */
int SaSi_PalInit(void);



/**
 * @brief This function is used to terminate the PAL implementation and free the resources that were taken by ::SaSi_PalInit.
 *
 * @return Void.
 */
void SaSi_PalTerminate(void);



#ifdef __cplusplus
}
#endif

#endif


