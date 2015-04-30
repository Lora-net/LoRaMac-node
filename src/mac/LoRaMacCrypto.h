/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __LORAMAC_CRYPTO_H__
#define __LORAMAC_CRYPTO_H__

/*!
 * Copies size elements of src array to dst array
 * 
 * \remark STM32 Standard memcpy function only works on pointers that are aligned
 *
 * \param [IN]  src  Source array
 * \param [OUT] dst  Destination array
 * \param [IN]  size Number of bytes to be copied
 */
#define LoRaMacMemCpy( src, dst, size ) memcpy1( dst, src, size )

/*!
 * Computes the LoRaMAC frame MIC field  
 *
 * \param [IN]  buffer          Data buffer
 * \param [IN]  size            Data buffer size
 * \param [IN]  key             AES key to be used
 * \param [IN]  address         Frame address
 * \param [IN]  dir             Frame direction [0: uplink, 1: downlink]
 * \param [IN]  sequenceCounter Frame sequence counter
 * \param [OUT] mic             Computed MIC field
 */
void LoRaMacComputeMic( uint8_t *buffer, uint16_t size, uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint32_t *mic );

/*!
 * Computes the LoRaMAC payload encryption 
 *
 * \param [IN]  buffer          Data buffer
 * \param [IN]  size            Data buffer size
 * \param [IN]  key             AES key to be used
 * \param [IN]  address         Frame address
 * \param [IN]  dir             Frame direction [0: uplink, 1: downlink]
 * \param [IN]  sequenceCounter Frame sequence counter
 * \param [OUT] encBuffer       Encrypted buffer
 */
void LoRaMacPayloadEncrypt( uint8_t *buffer, uint16_t size, uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *encBuffer );

/*!
 * Computes the LoRaMAC payload decryption 
 *
 * \param [IN]  buffer          Data buffer
 * \param [IN]  size            Data buffer size
 * \param [IN]  key             AES key to be used
 * \param [IN]  address         Frame address
 * \param [IN]  dir             Frame direction [0: uplink, 1: downlink]
 * \param [IN]  sequenceCounter Frame sequence counter
 * \param [OUT] decBuffer       Decrypted buffer
 */
void LoRaMacPayloadDecrypt( uint8_t *buffer, uint16_t size, uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *decBuffer );

/*!
 * Computes the LoRaMAC Join Request frame MIC field  
 *
 * \param [IN]  buffer          Data buffer
 * \param [IN]  size            Data buffer size
 * \param [IN]  key             AES key to be used
 * \param [OUT] mic             Computed MIC field
 */
void LoRaMacJoinComputeMic( uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t *mic );

/*!
 * Computes the LoRaMAC join frame decryption 
 *
 * \param [IN]  buffer          Data buffer
 * \param [IN]  size            Data buffer size
 * \param [IN]  key             AES key to be used
 * \param [OUT] decBuffer       Decrypted buffer
 */
void LoRaMacJoinDecrypt( uint8_t *buffer, uint16_t size, const uint8_t *key, uint8_t *decBuffer );

/*!
 * Computes the LoRaMAC join frame decryption 
 *
 * \param [IN]  key             AES key to be used
 * \param [IN]  appNonce        Application nonce
 * \param [IN]  devNonce        Device nonce
 * \param [OUT] nwkSKey         Network session key
 * \param [OUT] appSKey         Application session key
 */
void LoRaMacJoinComputeSKeys( const uint8_t *key, uint8_t *appNonce, uint16_t devNonce, uint8_t *nwkSKey, uint8_t *appSKey );

#endif // __LORAMAC_CRYPTO_H__
