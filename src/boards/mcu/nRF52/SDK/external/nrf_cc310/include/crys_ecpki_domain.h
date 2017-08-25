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


#ifndef CRYS_ECPKI_DOMAIN_H
#define CRYS_ECPKI_DOMAIN_H


/*!
@file
@brief Defines the ecpki build domain API.
*/


#include "crys_error.h"
#include "crys_ecpki_types.h"

#ifdef __cplusplus
extern "C"
{
#endif




/**********************************************************************************
 *      	      CRYS_ECPKI_GetEcDomain function 			  	  *
 **********************************************************************************/

/*!
 * @brief  The function returns a pointer to an ECDSA saved domain (one of the supported domains).
 *
 * @return Domain pointer on success.
 * @return NULL on failure.
 */

const CRYS_ECPKI_Domain_t *CRYS_ECPKI_GetEcDomain(CRYS_ECPKI_DomainID_t domainId /*!< [in] Index of one of the domain Id (must be one of the supported domains). */);

#ifdef __cplusplus
}
#endif

#endif
