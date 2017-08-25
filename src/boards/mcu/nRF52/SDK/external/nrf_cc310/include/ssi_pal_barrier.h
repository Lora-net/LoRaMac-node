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


#ifndef _SSI_PAL_BARRIER_H
#define _SSI_PAL_BARRIER_H

/*!
@file
@brief This file contains the definitions and APIs for memory barrier implementation.
* This is a place holder for platform specific memory barrier implementation
* The secure core driver should include a memory barrier before and after the last word of the descriptor
* to allow correct order between the words and different descriptors.
*/

/*!
 * This MACRO is responsible to put the memory barrier after the write operation.
 *
 * @return None
 */

void SaSi_PalWmb(void);

/*!
 * This MACRO is responsible to put the memory barrier before the read operation.
 *
 * @return None
 */
void SaSi_PalRmb(void);


#endif

