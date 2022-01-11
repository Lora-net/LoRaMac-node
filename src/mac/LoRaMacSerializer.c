/*!
 * \file  LoRaMacSerializer.h
 *
 * \brief LoRa MAC layer message serializer functionality implementation
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
#include "LoRaMacSerializer.h"
#include "utilities.h"

LoRaMacSerializerStatus_t LoRaMacSerializerJoinRequest( LoRaMacMessageJoinRequest_t* macMsg )
{
    if( ( macMsg == 0 ) || ( macMsg->Buffer == 0 ) )
    {
        return LORAMAC_SERIALIZER_ERROR_NPE;
    }

    uint16_t bufItr = 0;

    // Check macMsg->BufSize
    if( macMsg->BufSize < LORAMAC_JOIN_REQ_MSG_SIZE )
    {
        return LORAMAC_SERIALIZER_ERROR_BUF_SIZE;
    }

    macMsg->Buffer[bufItr++] = macMsg->MHDR.Value;

    memcpyr( &macMsg->Buffer[bufItr], macMsg->JoinEUI, LORAMAC_JOIN_EUI_FIELD_SIZE );
    bufItr += LORAMAC_JOIN_EUI_FIELD_SIZE;

    memcpyr( &macMsg->Buffer[bufItr], macMsg->DevEUI, LORAMAC_DEV_EUI_FIELD_SIZE );
    bufItr += LORAMAC_DEV_EUI_FIELD_SIZE;

    macMsg->Buffer[bufItr++] = macMsg->DevNonce & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->DevNonce >> 8 ) & 0xFF;

    macMsg->Buffer[bufItr++] = macMsg->MIC & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 8 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 16 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 24 ) & 0xFF;

    macMsg->BufSize = bufItr;

    return LORAMAC_SERIALIZER_SUCCESS;
}

LoRaMacSerializerStatus_t LoRaMacSerializerReJoinType1( LoRaMacMessageReJoinType1_t* macMsg )
{
    if( ( macMsg == 0 ) || ( macMsg->Buffer == 0 ) )
    {
        return LORAMAC_SERIALIZER_ERROR_NPE;
    }

    uint16_t bufItr = 0;

    // Check macMsg->BufSize
    if( macMsg->BufSize < LORAMAC_RE_JOIN_1_MSG_SIZE )
    {
        return LORAMAC_SERIALIZER_ERROR_BUF_SIZE;
    }

    macMsg->Buffer[bufItr++] = macMsg->MHDR.Value;
    macMsg->Buffer[bufItr++] = macMsg->ReJoinType;

    memcpyr( &macMsg->Buffer[bufItr], macMsg->JoinEUI, LORAMAC_JOIN_EUI_FIELD_SIZE );
    bufItr += LORAMAC_JOIN_EUI_FIELD_SIZE;

    memcpyr( &macMsg->Buffer[bufItr], macMsg->DevEUI, LORAMAC_DEV_EUI_FIELD_SIZE );
    bufItr += LORAMAC_DEV_EUI_FIELD_SIZE;

    macMsg->Buffer[bufItr++] = macMsg->RJcount1 & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->RJcount1 >> 8 ) & 0xFF;

    macMsg->Buffer[bufItr++] = macMsg->MIC & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 8 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 16 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 24 ) & 0xFF;

    macMsg->BufSize = bufItr;

    return LORAMAC_SERIALIZER_SUCCESS;
}

LoRaMacSerializerStatus_t LoRaMacSerializerReJoinType0or2( LoRaMacMessageReJoinType0or2_t* macMsg )
{
    if( ( macMsg == 0 ) || ( macMsg->Buffer == 0 ) )
    {
        return LORAMAC_SERIALIZER_ERROR_NPE;
    }

    uint16_t bufItr = 0;

    // Check macMsg->BufSize
    if( macMsg->BufSize < LORAMAC_RE_JOIN_0_2_MSG_SIZE )
    {
        return LORAMAC_SERIALIZER_ERROR_BUF_SIZE;
    }

    macMsg->Buffer[bufItr++] = macMsg->MHDR.Value;
    macMsg->Buffer[bufItr++] = macMsg->ReJoinType;

    memcpy1( &macMsg->Buffer[bufItr], macMsg->NetID, LORAMAC_NET_ID_FIELD_SIZE );
    bufItr += LORAMAC_NET_ID_FIELD_SIZE;

    memcpyr( &macMsg->Buffer[bufItr], macMsg->DevEUI, LORAMAC_DEV_EUI_FIELD_SIZE );
    bufItr += LORAMAC_DEV_EUI_FIELD_SIZE;

    macMsg->Buffer[bufItr++] = macMsg->RJcount0 & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->RJcount0 >> 8 ) & 0xFF;

    macMsg->Buffer[bufItr++] = macMsg->MIC & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 8 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 16 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 24 ) & 0xFF;

    macMsg->BufSize = bufItr;

    return LORAMAC_SERIALIZER_SUCCESS;
}

LoRaMacSerializerStatus_t LoRaMacSerializerData( LoRaMacMessageData_t* macMsg )
{
    if( ( macMsg == 0 ) || ( macMsg->Buffer == 0 ) )
    {
        return LORAMAC_SERIALIZER_ERROR_NPE;
    }

    uint16_t bufItr = 0;

    // Check macMsg->BufSize
    uint16_t computedBufSize =   LORAMAC_MHDR_FIELD_SIZE
                               + LORAMAC_FHDR_DEV_ADDR_FIELD_SIZE
                               + LORAMAC_FHDR_F_CTRL_FIELD_SIZE
                               + LORAMAC_FHDR_F_CNT_FIELD_SIZE;

    computedBufSize += macMsg->FHDR.FCtrl.Bits.FOptsLen;

    if( macMsg->FRMPayloadSize > 0 )
    {
        computedBufSize += LORAMAC_F_PORT_FIELD_SIZE;
    }

    computedBufSize += macMsg->FRMPayloadSize;
    computedBufSize += LORAMAC_MIC_FIELD_SIZE;

    if( macMsg->BufSize < computedBufSize )
    {
        return LORAMAC_SERIALIZER_ERROR_BUF_SIZE;
    }

    macMsg->Buffer[bufItr++] = macMsg->MHDR.Value;

    macMsg->Buffer[bufItr++] = ( macMsg->FHDR.DevAddr ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->FHDR.DevAddr >> 8 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->FHDR.DevAddr >> 16 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->FHDR.DevAddr >> 24 ) & 0xFF;

    macMsg->Buffer[bufItr++] = macMsg->FHDR.FCtrl.Value;

    macMsg->Buffer[bufItr++] = macMsg->FHDR.FCnt & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->FHDR.FCnt >> 8 ) & 0xFF;

    memcpy1( &macMsg->Buffer[bufItr], macMsg->FHDR.FOpts, macMsg->FHDR.FCtrl.Bits.FOptsLen );
    bufItr = bufItr + macMsg->FHDR.FCtrl.Bits.FOptsLen;

    if( macMsg->FRMPayloadSize > 0 )
    {
        macMsg->Buffer[bufItr++] = macMsg->FPort;
    }

    memcpy1( &macMsg->Buffer[bufItr], macMsg->FRMPayload, macMsg->FRMPayloadSize );
    bufItr = bufItr + macMsg->FRMPayloadSize;

    macMsg->Buffer[bufItr++] = macMsg->MIC & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 8 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 16 ) & 0xFF;
    macMsg->Buffer[bufItr++] = ( macMsg->MIC >> 24 ) & 0xFF;

    macMsg->BufSize = bufItr;

    return LORAMAC_SERIALIZER_SUCCESS;
}
