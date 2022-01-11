/*!
 * \file  nvmm.c
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

#include <stdint.h>

#include "utilities.h"
#include "eeprom-board.h"
#include "nvmm.h"

uint16_t NvmmWrite( uint8_t* src, uint16_t size, uint16_t offset )
{
    if( EepromMcuWriteBuffer( offset, src, size ) == LMN_STATUS_OK )
    {
        return size;
    }
    return 0;
}

uint16_t NvmmRead( uint8_t* dest, uint16_t size, uint16_t offset )
{
    if( EepromMcuReadBuffer( offset, dest, size ) == LMN_STATUS_OK )
    {
        return size;
    }
    return 0;
}

bool NvmmCrc32Check( uint16_t size, uint16_t offset )
{
    uint8_t data = 0;
    uint32_t calculatedCrc32 = 0;
    uint32_t readCrc32 = 0;

    if( NvmmRead( ( uint8_t* ) &readCrc32, sizeof( readCrc32 ),
                  ( offset + ( size - sizeof( readCrc32 ) ) ) ) == sizeof( readCrc32 ) )
    {
        // Calculate crc
        calculatedCrc32 = Crc32Init( );
        for( uint16_t i = 0; i < ( size - sizeof( readCrc32 ) ); i++ )
        {
            if( NvmmRead( &data, 1, offset + i ) != 1 )
            {
                return false;
            }
            calculatedCrc32 = Crc32Update( calculatedCrc32, &data, 1 );
        }
        calculatedCrc32 = Crc32Finalize( calculatedCrc32 );

        if( calculatedCrc32 != readCrc32 )
        {
            return false;
        }
    }
    return true;
}

bool NvmmReset( uint16_t size, uint16_t offset )
{
    uint32_t crc32 = 0;

    if( EepromMcuWriteBuffer( offset + size - sizeof( crc32 ),
                              ( uint8_t* ) &crc32, sizeof( crc32 ) ) == LMN_STATUS_OK )
    {
        return true;
    }
    return false;
}
