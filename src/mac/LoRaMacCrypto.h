/*!
 * \file      LoRaMacCrypto.h
 *
 * \brief     LoRa MAC layer cryptographic functionality implementation
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
 * addtogroup LORAMAC
 * \{
 *
 */
#ifndef __LORAMAC_CRYPTO_H__
#define __LORAMAC_CRYPTO_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "utilities.h"
#include "LoRaMacTypes.h"
#include "LoRaMacMessageTypes.h"

/*!
 * LoRaMac Cryto Status
 */
typedef enum eLoRaMacCryptoStatus
{
    /*!
     * No error occurred
     */
    LORAMAC_CRYPTO_SUCCESS = 0,
    /*!
     * MIC does not match
     */
    LORAMAC_CRYPTO_FAIL_MIC,
    /*!
     * Address does not match
     */
    LORAMAC_CRYPTO_FAIL_ADDRESS,
    /*!
     * JoinNonce was not greater than previous one.
     */
    LORAMAC_CRYPTO_FAIL_JOIN_NONCE,
    /*!
     * RJcount0 reached 2^16-1
     */
    LORAMAC_CRYPTO_FAIL_RJCOUNT0_OVERFLOW,
    /*!
     * FCntUp/Down check failed
     */
    LORAMAC_CRYPTO_FAIL_FCNT,
    /*!
     * Not allowed parameter value
     */
    LORAMAC_CRYPTO_FAIL_PARAM,
    /*!
     * Null pointer exception
     */
    LORAMAC_CRYPTO_ERROR_NPE,
    /*!
     * Invalid key identifier exception
     */
    LORAMAC_CRYPTO_ERROR_INVALID_KEY_ID,
    /*!
     * Invalid address identifier exception
     */
    LORAMAC_CRYPTO_ERROR_INVALID_ADDR_ID,
    /*!
     * Invalid LoRaWAN specification version
     */
    LORAMAC_CRYPTO_ERROR_INVALID_VERSION,
    /*!
     * Incompatible buffer size
     */
    LORAMAC_CRYPTO_ERROR_BUF_SIZE,
    /*!
     * The secure element reports an error
     */
    LORAMAC_CRYPTO_ERROR_SECURE_ELEMENT_FUNC,
    /*!
     * Error from parser reported
     */
    LORAMAC_CRYPTO_ERROR_PARSER,
    /*!
     * Error from serializer reported
     */
    LORAMAC_CRYPTO_ERROR_SERIALIZER,
    /*!
     * RJcount1 reached 2^16-1 which should never happen
     */
    LORAMAC_CRYPTO_ERROR_RJCOUNT1_OVERFLOW,
    /*!
     * Undefined Error occurred
     */
    LORAMAC_CRYPTO_ERROR,
}LoRaMacCryptoStatus_t;

/*!
 * Signature of callback function to be called by the LoRaMac Crypto module when the
 * non-volatile context have to be stored. It is also possible to save the entire
 * crypto module context.
 *
 */
typedef void ( *EventNvmCtxChanged )( void );

/*!
 * Initialization of LoRaMac Crypto module
 * It sets initial values of volatile variables and assigns the non-volatile context.
 *
 * \param[IN]     cryptoNvmCtxChanged - Callback function which will be called  when the
 *                                      non-volatile context have to be stored.
 * \retval                            - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoInit( EventNvmCtxChanged cryptoNvmCtxChanged );

/*!
 * Sets the LoRaWAN specification version to be used.
 * 
 * \warning This function should be used for ABP only. In case of OTA the version will be set automatically.
 *
 * \param[IN]     version             - LoRaWAN specification version to be used.
 *
 * \retval                            - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoSetLrWanVersion( Version_t version );

/*!
 * Restores the internal nvm context from passed pointer.
 *
 * \param[IN]     cryptoNmvCtx     - Pointer to non-volatile crypto module context to be restored.
 * \retval                         - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoRestoreNvmCtx( void* cryptoNvmCtx );

/*!
 * Returns a pointer to the internal non-volatile context.
 *
 * \param[IN]     cryptoNvmCtxSize - Size of the module non-volatile context
 * \retval                         - Points to a structure where the module store its non-volatile context
 */
void* LoRaMacCryptoGetNvmCtx( size_t* cryptoNvmCtxSize );

/*!
 * Sets a key
 *
 * \param[IN]     keyID          - Key identifier
 * \param[IN]     key            - Key value (16 byte), if its a multicast key it must be encrypted with McKEKey
 * \retval                       - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoSetKey( KeyIdentifier_t keyID, uint8_t* key );

/*!
 * Prepares the join-request message.
 * It computes the mic and add it to the message.
 *
 * \param[IN/OUT] macMsg         - Join-request message object
 * \retval                       - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoPrepareJoinRequest( LoRaMacMessageJoinRequest_t* macMsg );

/*!
 * Prepares a rejoin-request type 1 message.
 * It computes the mic and add it to the message.
 *
 * \param[IN/OUT] macMsg         - Rejoin message object
 * \retval                       - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoPrepareReJoinType1( LoRaMacMessageReJoinType1_t* macMsg );

/*!
 * Prepares a rejoin-request type 0 or 2 message.
 * It computes the mic and add it to the message.
 *
 * \param[IN/OUT] macMsg         - Rejoin message object
 * \retval                       - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoPrepareReJoinType0or2( LoRaMacMessageReJoinType0or2_t* macMsg );

/*!
 * Handles the join-accept message.
 * It decrypts the message, verifies the MIC and if successful derives the session keys.
 *
 * \param[IN]     joinReqType    - Type of last join-request or rejoin which triggered the join-accept response
 * \param[IN]     joinEUI        - Join server EUI (8 byte)
 * \param[IN/OUT] macMsg         - Join-accept message object
 * \retval                       - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoHandleJoinAccept( JoinReqIdentifier_t joinReqType, uint8_t* joinEUI, LoRaMacMessageJoinAccept_t* macMsg );

/*!
 * Secures a message (encryption + integrity).
 *
 * \param[IN]     fCntUp          - Uplink sequence counter
 * \param[IN]     txDr            - Data rate used for the transmission
 * \param[IN]     txCh            - Index of the channel used for the transmission
 * \param[IN/OUT] macMsg          - Data message object
 * \retval                        - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoSecureMessage( uint32_t fCntUp, uint8_t txDr, uint8_t txCh, LoRaMacMessageData_t* macMsg );

/*!
 * Unsecures a message (decryption + integrity verification).
 *
 * \param[IN]     addrID          - Address identifier
 * \param[IN]     address         - Address
 * \param[IN]     fCntID          - Frame counter identifier
 * \param[IN]     fCntDown        - Downlink sequence counter
 * \param[IN/OUT] macMsg          - Data message object
 * \retval                        - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoUnsecureMessage( AddressIdentifier_t addrID, uint32_t address, FCntIdentifier_t fCntID, uint32_t fCntDown, LoRaMacMessageData_t* macMsg );

/*!
 * Derives the McKEKey from the AppKey or NwkKey.
 *
 * McKEKey = aes128_encrypt(NwkKey or AppKey , nonce | DevEUI  | pad16)
 *
 * \param[IN]     keyID           - Key identifier of the root key to use to perform the derivation ( NwkKey or AppKey )
 * \param[IN]     nonce           - Nonce value ( nonce <= 15)
 * \param[IN]     devEUI          - DevEUI Value
 * \retval                        - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoDeriveMcKEKey( KeyIdentifier_t keyID, uint16_t nonce, uint8_t* devEUI );

/*!
 * Derives a Multicast group key pair ( McAppSKey, McNwkSKey ) from McKey
 *
 * McAppSKey = aes128_encrypt(McKey, 0x01 | McAddr | pad16)
 * McNwkSKey = aes128_encrypt(McKey, 0x02 | McAddr | pad16)
 *
 * \param[IN]     addrID          - Address identifier to select the multicast group
 * \param[IN]     mcAddr          - Multicast group address (4 bytes)
 * \retval                        - Status of the operation
 */
LoRaMacCryptoStatus_t LoRaMacCryptoDeriveMcSessionKeyPair( AddressIdentifier_t addrID, uint32_t mcAddr );

/*! \} addtogroup LORAMAC */

#endif // __LORAMAC_CRYPTO_H__
