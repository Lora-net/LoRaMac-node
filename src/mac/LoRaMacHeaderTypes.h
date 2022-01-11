/*!
 * \file  LoRaMacHeaderTypes.h
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
 * \addtogroup LORAMAC
 * \{
 *
 */
#ifndef __LORAMAC_HEADER_TYPES_H__
#define __LORAMAC_HEADER_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/*! MAC header field size */
#define LORAMAC_MHDR_FIELD_SIZE             1

/*! ReJoinType field size */
#define LORAMAC_JOIN_TYPE_FIELD_SIZE        1

/*! Join EUI field size */
#define LORAMAC_JOIN_EUI_FIELD_SIZE         8

/*! Device EUI field size */
#define LORAMAC_DEV_EUI_FIELD_SIZE          8

/*! End-device nonce field size */
#define LORAMAC_DEV_NONCE_FIELD_SIZE        2

/*! Join-server nonce field size */
#define LORAMAC_JOIN_NONCE_FIELD_SIZE       3

/*! RJcount0 field size */
#define LORAMAC_RJCOUNT_0_FIELD_SIZE        2

/*! RJcount1 field size */
#define LORAMAC_RJCOUNT_1_FIELD_SIZE        2

/*! Network ID field size */
#define LORAMAC_NET_ID_FIELD_SIZE           3

/*! Device address field size */
#define LORAMAC_DEV_ADDR_FIELD_SIZE         4

/*! DLSettings field size */
#define LORAMAC_DL_SETTINGS_FIELD_SIZE      1

/*! RxDelay field size */
#define LORAMAC_RX_DELAY_FIELD_SIZE         1

/*! CFList field size */
#define LORAMAC_CF_LIST_FIELD_SIZE          16

/*! FHDR Device address field size */
#define LORAMAC_FHDR_DEV_ADDR_FIELD_SIZE    LORAMAC_DEV_ADDR_FIELD_SIZE

/*! FHDR Frame control field size */
#define LORAMAC_FHDR_F_CTRL_FIELD_SIZE      1

/*! FHDR Frame control field size */
#define LORAMAC_FHDR_F_CNT_FIELD_SIZE       2

/*! FOpts maximum field size */
#define LORAMAC_FHDR_F_OPTS_MAX_FIELD_SIZE  15

/*! Port field size */
#define LORAMAC_F_PORT_FIELD_SIZE           1

/*! Port field size */
#define LORAMAC_MAC_PAYLOAD_FIELD_MAX_SIZE  242

/*! MIC field size */
#define LORAMAC_MIC_FIELD_SIZE              4

/*!
 * JoinRequest frame size
 *
 * MHDR(1) + JoinEUI(8) + DevEUI(8) + DevNonce(2) + MIC(4)
 */
#define LORAMAC_JOIN_REQ_MSG_SIZE           ( LORAMAC_MHDR_FIELD_SIZE + LORAMAC_JOIN_EUI_FIELD_SIZE + \
                                              LORAMAC_DEV_EUI_FIELD_SIZE + LORAMAC_DEV_NONCE_FIELD_SIZE + \
                                              LORAMAC_MIC_FIELD_SIZE )

/*!
 * ReJoinRequest type 1 frame size
 *
 * MHDR(1) + ReJoinType(1) + JoinEUI(8) + DevEUI(8) + RJcount1(2) + MIC(4)
 */
#define LORAMAC_RE_JOIN_1_MSG_SIZE          ( LORAMAC_MHDR_FIELD_SIZE + LORAMAC_JOIN_TYPE_FIELD_SIZE + \
                                              LORAMAC_JOIN_EUI_FIELD_SIZE + LORAMAC_DEV_EUI_FIELD_SIZE + \
                                              LORAMAC_RJCOUNT_1_FIELD_SIZE + \
                                              LORAMAC_MIC_FIELD_SIZE )

/*!
 * ReJoinRequest type 0 or 2 frame size
 *
 * MHDR(1) + ReJoinType(1) + NetID(3) + DevEUI(8) + RJcount0(2) + MIC(4)
 */
#define LORAMAC_RE_JOIN_0_2_MSG_SIZE        ( LORAMAC_MHDR_FIELD_SIZE + LORAMAC_JOIN_TYPE_FIELD_SIZE + \
                                              LORAMAC_NET_ID_FIELD_SIZE + LORAMAC_DEV_EUI_FIELD_SIZE + \
                                              LORAMAC_RJCOUNT_0_FIELD_SIZE + \
                                              LORAMAC_MIC_FIELD_SIZE )

/*!
 * JoinAccept frame minimum size
 *
 * MHDR(1) + AppNonce(3) + NetID(3) + DevAddr(4) + DLSettings(1) + RxDelay(1) + MIC(4)
 */
#define LORAMAC_JOIN_ACCEPT_FRAME_MIN_SIZE  ( LORAMAC_MHDR_FIELD_SIZE + LORAMAC_JOIN_NONCE_FIELD_SIZE + \
                                              LORAMAC_NET_ID_FIELD_SIZE + LORAMAC_DEV_ADDR_FIELD_SIZE + \
                                              LORAMAC_DL_SETTINGS_FIELD_SIZE + LORAMAC_RX_DELAY_FIELD_SIZE + \
                                              LORAMAC_MIC_FIELD_SIZE )

/*!
 * JoinAccept frame maximum size
 *
 * MHDR(1) + AppNonce(3) + NetID(3) + DevAddr(4) + DLSettings(1) + RxDelay(1) + CFList(16) + MIC(4)
 */
#define LORAMAC_JOIN_ACCEPT_FRAME_MAX_SIZE  ( LORAMAC_MHDR_FIELD_SIZE + LORAMAC_JOIN_NONCE_FIELD_SIZE + \
                                              LORAMAC_NET_ID_FIELD_SIZE + LORAMAC_DEV_ADDR_FIELD_SIZE + \
                                              LORAMAC_DL_SETTINGS_FIELD_SIZE + LORAMAC_RX_DELAY_FIELD_SIZE + \
                                              LORAMAC_CF_LIST_FIELD_SIZE + LORAMAC_MIC_FIELD_SIZE )

/*!
 * MIC computation offset
 * \remark required for 1.1.x support
 */
#define JOIN_ACCEPT_MIC_COMPUTATION_OFFSET                                                   \
    ( LORAMAC_MHDR_FIELD_SIZE + LORAMAC_JOIN_TYPE_FIELD_SIZE + LORAMAC_JOIN_EUI_FIELD_SIZE + \
      LORAMAC_DEV_NONCE_FIELD_SIZE )

/*!
 * FRMPayload overhead to be used when setting the Radio.SetMaxPayloadLength
 * 
 * Overhead to be used when setting the Radio.SetMaxPayloadLength in RxWindowSetup function.
 *
 * MHDR(1) + FHDR(7) + Port(1) + MIC(4)
 *
 * Maximum PHYPayload = MaxPayloadOfDatarate + LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE
 */
#define LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE ( LORAMAC_MHDR_FIELD_SIZE + ( LORAMAC_FHDR_DEV_ADDR_FIELD_SIZE + \
                                              LORAMAC_FHDR_F_CTRL_FIELD_SIZE + LORAMAC_FHDR_F_CNT_FIELD_SIZE ) + \
                                              LORAMAC_F_PORT_FIELD_SIZE + LORAMAC_MIC_FIELD_SIZE )

/*!
 * FRMPayload minimum size
 * 
 * MHDR(1) + FHDR(7) + MIC(4)
 */
#define LORAMAC_FRAME_PAYLOAD_MIN_SIZE      ( LORAMAC_MHDR_FIELD_SIZE + ( LORAMAC_FHDR_DEV_ADDR_FIELD_SIZE + \
                                              LORAMAC_FHDR_F_CTRL_FIELD_SIZE + LORAMAC_FHDR_F_CNT_FIELD_SIZE ) + \
                                              LORAMAC_MIC_FIELD_SIZE )
/*!
 * FRMPayload maximum possible size
 *
 * MHDR(1) + FHDR(7) + Port(1) + MACPayload(242) + MIC(4)
 */
#define LORAMAC_FRAME_PAYLOAD_MAX_SIZE      ( LORAMAC_MHDR_FIELD_SIZE + ( LORAMAC_FHDR_DEV_ADDR_FIELD_SIZE + \
                                              LORAMAC_FHDR_F_CTRL_FIELD_SIZE + LORAMAC_FHDR_F_CNT_FIELD_SIZE ) + \
                                              LORAMAC_F_PORT_FIELD_SIZE + LORAMAC_MAC_PAYLOAD_FIELD_MAX_SIZE + \
                                              LORAMAC_MIC_FIELD_SIZE )

/*!
 * LoRaMAC field definition of DLSettings
 *
 * LoRaWAN Specification V1.0.2, chapter 5.4
 */
typedef union uLoRaMacDLSettings
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to header bits
     */
    struct sDLSettingsBits
    {
        /*!
         * Data rate of a downlink using the second receive window
         */
        uint8_t RX2DataRate     : 4;
        /*!
         * Offset between up and downlink datarate of first reception slot
         */
        uint8_t RX1DRoffset     : 3;
        /*!
         * Indicates network server LoRaWAN implementation version 1.1 or later.
         */
        uint8_t OptNeg          : 1;
    }Bits;
}LoRaMacDLSettings_t;

/*!
 * LoRaMAC header field definition (MHDR field)
 *
 * LoRaWAN Specification V1.0.2, chapter 4.2
 */
typedef union uLoRaMacHeader
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to header bits
     */
    struct sMacHeaderBits
    {
        /*!
         * Major version
         */
        uint8_t Major           : 2;
        /*!
         * RFU
         */
        uint8_t RFU             : 3;
        /*!
         * Message type
         */
        uint8_t MType           : 3;
    }Bits;
}LoRaMacHeader_t;

/*!
 * LoRaMAC frame control field definition (FCtrl)
 *
 * LoRaWAN Specification V1.0.2, chapter 4.3.1
 */
typedef union uLoRaMacFrameCtrl
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to bits
     */
    struct sCtrlBits
    {
        /*!
         * Frame options length
         */
        uint8_t FOptsLen        : 4;
        /*!
         * Frame pending bit
         */
        uint8_t FPending        : 1;
        /*!
         * Message acknowledge bit
         */
        uint8_t Ack             : 1;
        /*!
         * ADR acknowledgment request bit
         */
        uint8_t AdrAckReq       : 1;
        /*!
         * ADR control in frame header
         */
        uint8_t Adr             : 1;
    }Bits;
}LoRaMacFrameCtrl_t;

/*!
 * LoRaMac Frame header (FHDR)
 *
 * LoRaWAN Specification V1.0.2, chapter 4.3.1
 */
typedef struct sLoRaMacFrameHeader
{
    /*!
     * Device address
     */
    uint32_t DevAddr;
    /*!
     * Frame control field
     */
    LoRaMacFrameCtrl_t FCtrl;
    /*!
     * Frame counter
     */
    uint16_t FCnt;
    /*!
     * FOpts field may transport  MAC commands (opt. 0-15 Bytes)
     */
    uint8_t FOpts[LORAMAC_FHDR_F_OPTS_MAX_FIELD_SIZE];
}LoRaMacFrameHeader_t;

/*! \} addtogroup LORAMAC */

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_HEADER_TYPES_H__
