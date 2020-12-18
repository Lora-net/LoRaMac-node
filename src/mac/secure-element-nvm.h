/*!
 * \file      secure-element-nvm.h
 *
 * \brief     Secure Element non-volatile data.
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
 *              (C)2013 Semtech
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
 * \addtogroup  SECUREELEMENT
 *
 * \{
 *
 */
#ifndef __SECURE_ELEMENT_NVM_H__
#define __SECURE_ELEMENT_NVM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "LoRaMacTypes.h"

/*!
 * Secure-element keys size in bytes
 */
#define SE_KEY_SIZE             16

/*!
 * Secure-element EUI size in bytes
 */
#define SE_EUI_SIZE             8

/*!
 * Secure-element pin size in bytes
 */
#define SE_PIN_SIZE             4

#ifdef SOFT_SE
/*!
 * Number of supported crypto keys for the soft-se
 */
#define NUM_OF_KEYS             23

/*!
 * Key structure definition for the soft-se
 */
typedef struct sKey
{
    /*!
     * Key identifier
     */
    KeyIdentifier_t KeyID;
    /*!
     * Key value
     */
    uint8_t KeyValue[SE_KEY_SIZE];
} Key_t;
#endif

typedef struct sSecureElementNvCtx
{
    /*!
     * DevEUI storage
     */
    uint8_t DevEui[SE_EUI_SIZE];
    /*!
     * Join EUI storage
     */
    uint8_t JoinEui[SE_EUI_SIZE];
    /*!
     * Pin storage
     */
    uint8_t Pin[SE_PIN_SIZE];
#ifdef SOFT_SE
    /*!
     * The key list is required for the soft-se only. All other secure-elements
     * handle the storage on their own.
     */
    Key_t KeyList[NUM_OF_KEYS];
#endif
    /*!
     * CRC32 value of the SecureElement data structure.
     */
    uint32_t Crc32;
} SecureElementNvmData_t;


/*! \} addtogroup SECUREELEMENT */

#ifdef __cplusplus
}
#endif

#endif //  __SECURE_ELEMENT_NVM_H__
