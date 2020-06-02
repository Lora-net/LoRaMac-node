/*!
 * \file      LoRaMacAdr.h
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
 *
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
