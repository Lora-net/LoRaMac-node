/*!
 * \file  LoRaMacAdr.h
 *
 * \brief LoRa MAC ADR implementation
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
 * \defgroup  LORAMACADR LoRa MAC ADR implementation
 *            Implementation of the ADR algorithm for LoRa.
 * \{
 */
#ifndef __LORAMACADR_H__
#define __LORAMACADR_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*! \} defgroup LORAMACADR */

/*
 * Parameter structure for the function CalcNextAdr.
 */
typedef struct sCalcNextAdrParams
{
    /*!
     * Set to true, if the function should update the channels mask.
     */
    bool UpdateChanMask;
    /*!
     * Set to true, if ADR is enabled.
     */
    bool AdrEnabled;
    /*!
     * ADR ack counter.
     */
    uint32_t AdrAckCounter;
    /*!
     * ADR Ack limit
     */
    uint16_t AdrAckLimit;
    /*!
     * ADR Ack delay
     */
    uint16_t AdrAckDelay;
    /*!
     * Datarate used currently.
     */
    int8_t Datarate;
    /*!
     * TX power used currently.
     */
    int8_t TxPower;
    /*!
     * NbTrans counter used currently.
     */
    uint8_t NbTrans;
    /*!
     * UplinkDwellTime
     */
    uint8_t UplinkDwellTime;
    /*!
     * Region
     */
    LoRaMacRegion_t Region;
}CalcNextAdrParams_t;

/*!
 * \brief Calculates the next datarate to set, when ADR is on or off.
 *
 * \details Here is a summary of the actions:
 *
 * | ADR_ACK_CNT | Action                                                    |
 * | ----------- | --------------------------------------------------------- |
 * | 0... 63     | Do nothing                                                |
 * | 64...95     | Set ADR ack bit                                           |
 * | 96...127    | Set TX power to default (if already default, do nothing)  |
 * | 128...159   | Set data rate to default (if already default, do nothing) |
 * | >=160       | Set NbTrans to 1, re-enable default channels              |
 *
 * \param [IN] adrNext Pointer to the function parameters.
 *
 * \param [OUT] drOut The calculated datarate for the next TX.
 *
 * \param [OUT] txPowOut The TX power for the next TX.
 *
 * \param [OUT] nbTransOut The NbTrans counter.
 *
 * \param [OUT] adrAckCounter The calculated ADR acknowledgement counter.
 *
 * \retval Returns true, if an ADR request should be performed.
 */
bool LoRaMacAdrCalcNext( CalcNextAdrParams_t* adrNext, int8_t* drOut, int8_t* txPowOut,
                         uint8_t* nbTransOut, uint32_t* adrAckCounter );

#ifdef __cplusplus
}
#endif

#endif // __LORAMACADR_H__
