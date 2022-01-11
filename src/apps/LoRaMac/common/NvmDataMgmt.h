/*!
 * \file  NvmDataMgmt.h
 *
 * \brief NVM context management implementation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright Stackforce 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*!
 * \defgroup  NVMDATAMGMT NVM context management implementation
 *            This module implements the NVM context handling
 * \{
 */
#ifndef __NVMDATAMGMT_H__
#define __NVMDATAMGMT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief NVM Management event.
 *
 * \param [IN] notifyFlags Bitmap which contains the information about modules that
 *                         changed.
 */
void NvmDataMgmtEvent( uint16_t notifyFlags );

/*!
 * \brief Function which stores the MAC data into NVM, if required.
 *
 * \retval Number of bytes which were stored.
 */
uint16_t NvmDataMgmtStore( void );

/*!
 * \brief Function which restores the MAC data from NVM, if required.
 *
 * \retval Number of bytes which were restored.
 */
uint16_t NvmDataMgmtRestore(void );

/*!
 * \brief Resets the NVM data.
 *
 * \retval Returns true, if successful.
 */
bool NvmDataMgmtFactoryReset( void );

/* \} */

#ifdef __cplusplus
}
#endif

#endif // __NVMDATAMGMT_H__
