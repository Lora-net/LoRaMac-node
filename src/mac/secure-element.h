/*!
 * \file      secure-element.h
 *
 * \brief     Secure Element driver API
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
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 *
 * \defgroup  SECUREELEMENT Secure Element API Definition
 *
 * \{
 *
 */
#ifndef __SECURE_ELEMENT_H__
#define __SECURE_ELEMENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "LoRaMacCrypto.h"

#define SE_EUI_SIZE             8

/*!
 * Return values.
 */
typedef enum eSecureElementStatus
{
    /*!
     * No error occurred
     */
    SECURE_ELEMENT_SUCCESS = 0,
    /*!
     * Failed to encrypt
     */
    SECURE_ELEMENT_FAIL_ENCRYPT,
    /*!
     * CMAC does not match
     */
    SECURE_ELEMENT_FAIL_CMAC,
    /*!
     * Null pointer exception
     */
    SECURE_ELEMENT_ERROR_NPE,
    /*!
     * Invalid key identifier exception
     */
    SECURE_ELEMENT_ERROR_INVALID_KEY_ID,
    /*!
     * Invalid LoRaWAN specification version
     */
    SECURE_ELEMENT_ERROR_INVALID_LORAWAM_SPEC_VERSION,
    /*!
     * Incompatible buffer size
     */
    SECURE_ELEMENT_ERROR_BUF_SIZE,
    /*!
     * Undefined Error occurred
     */
    SECURE_ELEMENT_ERROR,
}SecureElementStatus_t;

/*!
 * Signature of callback function to be called by the Secure Element driver when the
 * non volatile context have to be stored.
 *
 */
typedef void ( *SecureElementNvmEvent )( void );

/*!
 * Initialization of Secure Element driver
 *
 * \param[IN]     seNvmCtxChanged           - Callback function which will be called  when the
 *                                            non-volatile context have to be stored.
 * \retval                                  - Status of the operation
 */
SecureElementStatus_t SecureElementInit( SecureElementNvmEvent seNvmCtxChanged );

/*!
 * Restores the internal nvm context from passed pointer.
 *
 * \param[IN]     seNvmCtx         - Pointer to non-volatile module context to be restored.
 * \retval                         - Status of the operation
 */
SecureElementStatus_t SecureElementRestoreNvmCtx( void* seNvmCtx );

/*!
 * Returns a pointer to the internal non-volatile context.
 *
 * \param[IN]     seNvmCtxSize    - Size of the module non volatile context
 * \retval                        - Points to a structure where the module store its non volatile context
 */
void* SecureElementGetNvmCtx( size_t* seNvmCtxSize );

/*!
 * Sets a key
 *
 * \param[IN]  keyID          - Key identifier
 * \param[IN]  key            - Key value
 * \retval                    - Status of the operation
 */
SecureElementStatus_t SecureElementSetKey( KeyIdentifier_t keyID, uint8_t* key );

/*!
 * Computes a CMAC of a message using provided initial Bx block
 *
 * \param[IN]  micBxBuffer    - Buffer containing the initial Bx block
 * \param[IN]  buffer         - Data buffer
 * \param[IN]  size           - Data buffer size
 * \param[IN]  keyID          - Key identifier to determine the AES key to be used
 * \param[OUT] cmac           - Computed cmac
 * \retval                    - Status of the operation
 */
SecureElementStatus_t SecureElementComputeAesCmac( uint8_t* micBxBuffer, uint8_t* buffer, uint16_t size, KeyIdentifier_t keyID, uint32_t* cmac );

/*!
 * Verifies a CMAC (computes and compare with expected cmac)
 *
 * \param[IN]  buffer         - Data buffer
 * \param[IN]  size           - Data buffer size
 * \param[in]  expectedCmac   - Expected cmac
 * \param[IN]  keyID          - Key identifier to determine the AES key to be used
 * \retval                    - Status of the operation
 */
SecureElementStatus_t SecureElementVerifyAesCmac( uint8_t* buffer, uint16_t size, uint32_t expectedCmac, KeyIdentifier_t keyID );

/*!
 * Encrypt a buffer
 *
 * \param[IN]  buffer         - Data buffer
 * \param[IN]  size           - Data buffer size
 * \param[IN]  keyID          - Key identifier to determine the AES key to be used
 * \param[OUT] encBuffer      - Encrypted buffer
 * \retval                    - Status of the operation
 */
SecureElementStatus_t SecureElementAesEncrypt( uint8_t* buffer, uint16_t size, KeyIdentifier_t keyID, uint8_t* encBuffer );

/*!
 * Derives and store a key
 *
 * \param[IN]  version        - LoRaWAN specification version currently in use.
 * \param[IN]  input          - Input data from which the key is derived ( 16 byte )
 * \param[IN]  rootKeyID      - Key identifier of the root key to use to perform the derivation
 * \param[IN]  targetKeyID    - Key identifier of the key which will be derived
 * \retval                    - Status of the operation
 */
SecureElementStatus_t SecureElementDeriveAndStoreKey( Version_t version, uint8_t* input, KeyIdentifier_t rootKeyID, KeyIdentifier_t targetKeyID );

/*!
 * Process JoinAccept message.
 *
 * \param[IN]  encKeyID          - Key identifier of the key which will be used to decrypt the JoinAccept message
 * \param[IN]  micKeyID          - Key identifier of the key which will be used to compute the JoinAccept message MIC
 * \param[IN]  versionMinor      - LoRaWAN specification version minor field which will be used to perform the processing.
 *                                     - 0 -> LoRaWAN 1.0.x
 *                                     - 1 -> LoRaWAN 1.1.x
 * \param[IN]  micHeader         - Header buffer to be used for MIC computation
 *                                     - LoRaWAN 1.0.x : micHeader = [MHDR(1)]
 *                                     - LoRaWAN 1.1.x : micHeader = [JoinReqType(1), JoinEUI(8), DevNonce(2), MHDR(1)]
 * \param[IN]  encJoinAccept     - Received encrypted JoinAccept message
 * \param[IN]  encJoinAcceptSize - Received encrypted JoinAccept message Size
 * \param[IN]  decJoinAccept     - Decrypted and validated JoinAccept message
 * \retval                       - Status of the operation
 */
SecureElementStatus_t SecureElementProcessJoinAccept( KeyIdentifier_t encKeyID, KeyIdentifier_t micKeyID, uint8_t versionMinor,
                                                      uint8_t *micHeader, uint8_t *encJoinAccept, uint8_t encJoinAcceptSize,
                                                      uint8_t *decJoinAccept );

/*!
 * Generates a random number
 *
 * \param[OUT] randomNum      - 32 bit random number
 * \retval                    - Status of the operation
 */
SecureElementStatus_t SecureElementRandomNumber( uint32_t* randomNum );

/*!
 * Sets the DevEUI
 *
 * \param[IN] devEui          - Pointer to the 16-byte devEUI
 * \retval                    - Status of the operation
 */
SecureElementStatus_t SecureElementSetDevEui( uint8_t* devEui );

/*!
 * Gets the DevEUI
 *
 * \retval                    - Pointer to the 16-byte devEUI
 */
uint8_t* SecureElementGetDevEui( void );

/*!
 * Sets the JoinEUI
 *
 * \param[IN] joinEui         - Pointer to the 16-byte joinEui
 * \retval                    - Status of the operation
 */
SecureElementStatus_t SecureElementSetJoinEui( uint8_t* joinEui );

/*!
 * Gets the DevEUI
 *
 * \retval                    - Pointer to the 16-byte joinEui
 */
uint8_t* SecureElementGetJoinEui( void );

/*!
 * Initialization of AES driver
 */
extern void LORAMAC_AESStart(void);
/*!
 * De-initialization/Cleanup of AES driver
 */
extern void LORAMAC_AESStop(void);

/*!
 * Process AES encryption of a block of memory using with 128 bits key
 * \param[OUT]	encBuffer		Encrypted output buffer
 * \param[IN] 	buffer			Input buffer to encrypt
 * \param[IN] 	size			Size of buffer to encrypt
 * \param[IN] 	key				AES Key used to encrypt (NULL to continue with current key context for CBC)
 * \param[IN] 	iv				Initialization Vector to XOR with input buffer before first round (for CBC)
 */
extern void LORAMAC_AES_128bits(uint8_t *encBuffer, const uint8_t *buffer, uint16_t size, const uint8_t *key, const uint8_t *iv );

/*! \} defgroup SECUREELEMENT */

#ifdef __cplusplus
}
#endif

#endif //  __SECURE_ELEMENT_H__
