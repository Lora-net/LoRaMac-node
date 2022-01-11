/*!
 * \file  nvmm.h
 *
 * \brief Non volatile memory management module
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
#ifndef __NVMM_H__
#define __NVMM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/*!
 * \brief Writes data to given data block.
 *
 * \param[IN] src    Pointer to the source of data to be copied.
 * \param[IN] size   Number of bytes to copy.
 * \param[IN] offset Relative NVM offset.
 *
 * \retval           Status of the operation
 */
uint16_t NvmmWrite( uint8_t* src, uint16_t size, uint16_t offset );

/*!
 * \brief Reads from data block to destination pointer.
 *
 * \param[IN] dst    Pointer to the destination array where the content is to be copied.
 * \param[IN] size   Number of bytes to copy.
 * \param[IN] offset Relative NVM offset.
 *
 * \retval           Status of the operation
 */
uint16_t NvmmRead( uint8_t* dest, uint16_t size, uint16_t offset );

/*!
 * \brief Verfies the CRC 32 of a data block. The function assumes that the
 *        crc32 is at the end of the block with 4 bytes.
 *
 * \param[IN] size   Length of the block.
 * \param[IN] offset Address offset of the NVM.
 *
 * \retval           Status of the operation
 */
bool NvmmCrc32Check( uint16_t size, uint16_t offset );

/*!
 * \brief Invalidates the CRC 32 of a data block. The function assumes that the
 *        crc32 is at the end of the block with 4 bytes.
 *
 * \param[IN] size   Length of the block.
 * \param[IN] offset Address offset of the NVM.
 *
 * \retval           Status of the operation
 */
bool NvmmReset( uint16_t size, uint16_t offset );

#ifdef __cplusplus
}
#endif

#endif // __NVMM_H__
