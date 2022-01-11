/*!
 * \file  LoRaMacAdr.c
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

#include "region/Region.h"
#include "LoRaMacAdr.h"

bool LoRaMacAdrCalcNext( CalcNextAdrParams_t* adrNext, int8_t* drOut, int8_t* txPowOut,
                         uint8_t* nbTransOut, uint32_t* adrAckCounter )
{
    bool adrAckReq = false;
    int8_t datarate = adrNext->Datarate;
    int8_t txPower = adrNext->TxPower;
    uint8_t nbTrans = adrNext->NbTrans;
    int8_t minTxDatarate;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    // Report back the adr ack counter
    *adrAckCounter = adrNext->AdrAckCounter;

    if( adrNext->AdrEnabled == true )
    {
        // Query minimum TX Datarate
        getPhy.Attribute = PHY_MIN_TX_DR;
        getPhy.UplinkDwellTime = adrNext->UplinkDwellTime;
        phyParam = RegionGetPhyParam( adrNext->Region, &getPhy );
        minTxDatarate = phyParam.Value;
        datarate = MAX( datarate, minTxDatarate );

        // Verify if ADR ack req bit needs to be set.
        if( adrNext->AdrAckCounter >= adrNext->AdrAckLimit )
        {
            adrAckReq = true;
        }

        // Verify, if we need to set the TX power to default
        if( adrNext->AdrAckCounter >= ( adrNext->AdrAckLimit + adrNext->AdrAckDelay ) )
        {
            // Set TX Power to default
            getPhy.Attribute = PHY_DEF_TX_POWER;
            phyParam = RegionGetPhyParam( adrNext->Region, &getPhy );
            txPower = phyParam.Value;
        }

        // Verify, if we need to decrease the data rate
        if( adrNext->AdrAckCounter >= ( uint32_t )( adrNext->AdrAckLimit + ( adrNext->AdrAckDelay << 1 ) ) )
        {
            // Perform actions with every adrNext->AdrAckDelay only
            if( ( ( adrNext->AdrAckCounter - adrNext->AdrAckLimit ) % adrNext->AdrAckDelay ) == 0 )
            {
                if( datarate == minTxDatarate )
                {
                    // Restore the channel mask
                    if( adrNext->UpdateChanMask == true )
                    {
                        InitDefaultsParams_t params;
                        params.Type = INIT_TYPE_ACTIVATE_DEFAULT_CHANNELS;
                        RegionInitDefaults( adrNext->Region, &params );
                    }

                    // Restore NbTrans
                    nbTrans = 1;
                }

                // Decrease the datarate
                getPhy.Attribute = PHY_NEXT_LOWER_TX_DR;
                getPhy.Datarate = datarate;
                getPhy.UplinkDwellTime = adrNext->UplinkDwellTime;
                phyParam = RegionGetPhyParam( adrNext->Region, &getPhy );
                datarate = phyParam.Value;
            }
        }
    }

    *drOut = datarate;
    *txPowOut = txPower;
    *nbTransOut = nbTrans;
    return adrAckReq;
}
