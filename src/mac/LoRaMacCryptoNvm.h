/*!
 * \file      LoRaMacCryptoNvm.h
 *
 * \brief     LoRa MAC layer cryptographic NVM data.
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
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * addtogroup LORAMAC
 * \{
 *
 */
#ifndef __LORAMAC_CRYPTO_NVM_H__
#define __LORAMAC_CRYPTO_NVM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "utilities.h"
#include "LoRaMacTypes.h"


/*!
 * LoRaWAN Frame counter list.
 */
typedef struct sFCntList
{
    /*!
     * Uplink frame counter which is incremented with each uplink.
     */
    uint32_t FCntUp;
    /*!
     * Network downlink frame counter which is incremented with each downlink on FPort 0
     * or when the FPort field is missing.
     */
    uint32_t NFCntDown;
    /*!
     * Application downlink frame counter which is incremented with each downlink
     * on a port different than 0.
     */
    uint32_t AFCntDown;
    /*!
     * In case if the device is connected to a LoRaWAN 1.0 Server,
     * this counter is used for every kind of downlink frame.
     */
    uint32_t FCntDown;
    /*!
     * Multicast downlink counters
     */
    uint32_t McFCntDown[LORAMAC_MAX_MC_CTX];
#if( USE_LRWAN_1_1_X_CRYPTO == 1 )
    /*!
     * RJcount1 is a counter incremented with every Rejoin request Type 1 frame transmitted.
     */
    uint16_t RJcount1;
#endif
}FCntList_t;

/*!
 * LoRaMac Crypto Non Volatile Context structure
 */
typedef struct sLoRaMacCryptoNvmData
{
    /*!
     * Stores the information if the device is connected to a LoRaWAN network
     * server with prior to 1.1.0 implementation.
     */
    Version_t LrWanVersion;
    /*!
     * Device nonce is a counter starting at 0 when the device is initially
     * powered up and incremented with every JoinRequest.
     */
    uint16_t DevNonce;
    /*!
     * JoinNonce is a device specific counter value (that never repeats itself)
     * provided by the join server and incremented with every JoinAccept message.
     */
    uint32_t JoinNonce;
    /*!
     * Frame counter list
     */
    FCntList_t FCntList;
    /*!
     * LastDownFCnt stores the information which frame counter was used to
     * decrypt the last frame. This information is needed to compute ConfFCnt in
     * B1 block for the MIC.
     */
    uint32_t LastDownFCnt;
    /*!
     * CRC32 value of the Crypto data structure.
     */
    uint32_t Crc32;
}LoRaMacCryptoNvmData_t;

/*! \} addtogroup LORAMAC */

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_CRYPTO_NVM_H__
