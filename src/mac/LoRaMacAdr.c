/*!
 * \file      LoRaMacAdr.c
 *
 * \brief     LoRa MAC ADR implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
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
