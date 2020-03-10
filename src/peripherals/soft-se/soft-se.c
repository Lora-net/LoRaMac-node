/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: Secure Element software implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ),
            Daniel Jaeckle ( STACKFORCE ),  Johannes Bruder ( STACKFORCE )
*/
#include <stdlib.h>
#include <stdint.h>

#include "utilities.h"
#include "aes.h"
#include "cmac.h"
#include "radio.h"

#include "LoRaMacHeaderTypes.h"

#include "secure-element.h"

/*!
 * Number of supported crypto keys
 */
#define NUM_OF_KEYS 24

/*!
 * Crypto keys size in bytes
 */
#define KEY_SIZE 16

/*!
 * Size of JoinReqType is field for integrity check
 * \remark required for 1.1.x support
 */
#define JOIN_REQ_TYPE_SIZE 1

/*!
 * Size of DevNonce is field for integrity check
 * \remark required for 1.1.x support
 */
#define DEV_NONCE_SIZE 2

/*!
 * MIC computation offset
 * \remark required for 1.1.x support
 */
#define CRYPTO_MIC_COMPUTATION_OFFSET ( JOIN_REQ_TYPE_SIZE + LORAMAC_JOIN_EUI_FIELD_SIZE + DEV_NONCE_SIZE + LORAMAC_MHDR_FIELD_SIZE )

/*!
 * Identifier value pair type for Keys
 */
typedef struct sKey
{
    /*
     * Key identifier
     */
    KeyIdentifier_t KeyID;
    /*
     * Key value
     */
    uint8_t KeyValue[KEY_SIZE];
} Key_t;

/*
 * Secure Element Non Volatile Context structure
 */
typedef struct sSecureElementNvCtx
{
    /*
     * DevEUI storage
     */
    uint8_t DevEui[SE_EUI_SIZE];
    /*
     * Join EUI storage
     */
    uint8_t JoinEui[SE_EUI_SIZE];
    /*
     * AES computation context variable
     */
    aes_context AesContext;
    /*
     * CMAC computation context variable
     */
    AES_CMAC_CTX AesCmacCtx[1];
    /*
     * Key List
     */
    Key_t KeyList[NUM_OF_KEYS];
}SecureElementNvCtx_t;

/*
 * Module context
 */
static SecureElementNvCtx_t SeNvmCtx;

static SecureElementNvmEvent SeNvmCtxChanged;

/*
 * Local functions
 */

/*
 * Gets key item from key list.
 *
 * \param[IN]  keyID          - Key identifier
 * \param[OUT] keyItem        - Key item reference
 * \retval                    - Status of the operation
 */
static SecureElementStatus_t GetKeyByID( KeyIdentifier_t keyID, Key_t** keyItem )
{
    for( uint8_t i = 0; i < NUM_OF_KEYS; i++ )
    {
        if( SeNvmCtx.KeyList[i].KeyID == keyID )
        {
            *keyItem = &( SeNvmCtx.KeyList[i] );
            return SECURE_ELEMENT_SUCCESS;
        }
    }
    return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
}

/*
 * Dummy callback in case if the user provides NULL function pointer
 */
static void DummyCB( void )
{
    return;
}
/*
 * Derives K1 or K2 key for CMAC encryption
 * \param[IN/OUT] K	Initial vector modified as K1 or K2 result
 * \remark	All keys are big endian
 * \remark 	This function shall be called with all zeros buffer encrypted using AESCBC128Bits
 * to derive initial K1, or with K1 to derive K2 from K1.
 */
static void ComputeK1K2(uint8_t *K) {
uint8_t XOR = (K[0] & 0x80) ? 0x87 : 0;
	for (int k=KEY_SIZE-1; k ; k--, K++) {
		*K <<= 1;
		*K |= (*(K+1)) >> 7;
	}
	*K <<= 1;
	*K ^= XOR;
}
/*
 * Performs an XOR between b1 and b2, giving the result in b1
 * \param[IN/OUT] 	b1	Input/Output buffer 1
 * \param[IN] 		b2	Input buffer 2
 * \param[IN] 		len	Length of the buffers
 */
static void XORBuffers( uint8_t* b1, const uint8_t *b2, int len ) {
	while ( len-- )
		*( b1++ ) ^= *( b2++ );
}

#define USE_SOFT_AES	( 1 )
#if USE_SOFT_AES
#include "aes.h"
/*
 * AES computation context variable
 */
aes_context AesContext;

void LORAMAC_AESStart(void) {
    memset1( AesContext.ksch, '\0', 240 );
}
void LORAMAC_AESStop(void) {

}

void LORAMAC_AES_128bits(uint8_t *encBuffer, const uint8_t *buffer, uint16_t size, const uint8_t *key, const uint8_t *iv ) {
uint8_t tmp[KEY_SIZE];
	if (key) aes_set_key( key, KEY_SIZE, &AesContext );

	if (iv) memcpy1(tmp, iv, KEY_SIZE);		// Introduce iv in context for first round (used by AES-CBC)

	uint8_t block = 0;

    while( size != 0 )
    {
    	if (iv) XORBuffers(tmp, &buffer[block], KEY_SIZE);
    	else memcpy1(tmp,&buffer[block],KEY_SIZE);
        aes_encrypt( tmp, &encBuffer[block], &AesContext );
        block = block + KEY_SIZE;
        size = size - KEY_SIZE;
    }
}

#endif

/*
 * Computes a CMAC of a message using provided initial Bx block
 *
 *  cmac = aes128_cmac(keyID, blocks[i].Buffer)
 *
 * \param[IN]  micBxBuffer    - Buffer containing the initial Bx block
 * \param[IN]  buffer         - Data buffer
 * \param[IN]  size           - Data buffer size
 * \param[IN]  keyID          - Key identifier to determine the AES key to be used
 * \param[OUT] cmac           - Computed cmac
 * \retval                    - Status of the operation
 */
static SecureElementStatus_t ComputeCmac( uint8_t *micBxBuffer, uint8_t *buffer, uint16_t size, KeyIdentifier_t keyID, uint32_t* cmac )
{
    if( ( buffer == NULL ) || ( cmac == NULL ) )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    uint8_t Cmac[KEY_SIZE];
    memset1(Cmac,0,sizeof(Cmac));
    Key_t* keyItem;
    SecureElementStatus_t retval = GetKeyByID( keyID, &keyItem );

    if( retval == SECURE_ELEMENT_SUCCESS )
    {
    	LORAMAC_AESStart();
    	bool bFirstPass = true;
        uint8_t K[KEY_SIZE];
        memset1(K,0,sizeof(K));
		LORAMAC_AES_128bits( K, K, KEY_SIZE, keyItem->KeyValue, K );
		if( micBxBuffer != NULL )
        {
			// WARNING: We assume micBxBuffer is KEY_SIZE length
    		LORAMAC_AES_128bits( Cmac, micBxBuffer, KEY_SIZE, keyItem->KeyValue, Cmac );
    		bFirstPass = false;
        }
        while ( size ) {
    		if ( size <= KEY_SIZE ) {
    			// If last block is exactly KEY_SIZE, XOR last block remaining bytes with K1
    			ComputeK1K2( K );
    			if (size < KEY_SIZE) {
    				// Recompute K1 as K2 and prepare it for 0x80..00 buffer append
    				ComputeK1K2( K );
    				K[size] ^= 0x80;
    			}
    			XORBuffers( K, buffer, size ); // Modify K to keep source buffer intact
    			LORAMAC_AES_128bits( Cmac, K, KEY_SIZE, ( bFirstPass ) ? keyItem->KeyValue : NULL, Cmac );
    			break;	// We have finished
    		}
   			else {
   				// Make CBC chain by re-injecting ciphered output
   				LORAMAC_AES_128bits( Cmac, buffer, KEY_SIZE,  ( bFirstPass ) ? keyItem->KeyValue : NULL, Cmac );
   			}
    		bFirstPass = false;
    		size -= KEY_SIZE;
    		buffer += KEY_SIZE;
        }
        LORAMAC_AESStop();
        // Bring into the required format
        *cmac = ( uint32_t )( ( uint32_t ) Cmac[3] << 24 | ( uint32_t ) Cmac[2] << 16 | ( uint32_t ) Cmac[1] << 8 | ( uint32_t ) Cmac[0] );
    }

    return retval;
}


/*
 * API functions
 */

SecureElementStatus_t SecureElementInit( SecureElementNvmEvent seNvmCtxChanged )
{
    uint8_t itr = 0;
    uint8_t zeroKey[16] = { 0 };

    // Initialize with defaults
    SeNvmCtx.KeyList[itr++].KeyID = APP_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = GEN_APP_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = NWK_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = J_S_INT_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = J_S_ENC_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = F_NWK_S_INT_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = S_NWK_S_INT_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = NWK_S_ENC_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = APP_S_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = MC_ROOT_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KE_KEY;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KEY_0;
    SeNvmCtx.KeyList[itr++].KeyID = MC_APP_S_KEY_0;
    SeNvmCtx.KeyList[itr++].KeyID = MC_NWK_S_KEY_0;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KEY_1;
    SeNvmCtx.KeyList[itr++].KeyID = MC_APP_S_KEY_1;
    SeNvmCtx.KeyList[itr++].KeyID = MC_NWK_S_KEY_1;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KEY_2;
    SeNvmCtx.KeyList[itr++].KeyID = MC_APP_S_KEY_2;
    SeNvmCtx.KeyList[itr++].KeyID = MC_NWK_S_KEY_2;
    SeNvmCtx.KeyList[itr++].KeyID = MC_KEY_3;
    SeNvmCtx.KeyList[itr++].KeyID = MC_APP_S_KEY_3;
    SeNvmCtx.KeyList[itr++].KeyID = MC_NWK_S_KEY_3;
    SeNvmCtx.KeyList[itr].KeyID = SLOT_RAND_ZERO_KEY;

    // Set standard keys
    memcpy1( SeNvmCtx.KeyList[itr].KeyValue, zeroKey, KEY_SIZE );

    memset1( SeNvmCtx.DevEui, 0, SE_EUI_SIZE );
    memset1( SeNvmCtx.JoinEui, 0, SE_EUI_SIZE );

    // Assign callback
    if( seNvmCtxChanged != 0 )
    {
        SeNvmCtxChanged = seNvmCtxChanged;
    }
    else
    {
        SeNvmCtxChanged = DummyCB;
    }

    return SECURE_ELEMENT_SUCCESS;
}

SecureElementStatus_t SecureElementRestoreNvmCtx( void* seNvmCtx )
{
    // Restore nvm context
    if( seNvmCtx != 0 )
    {
        memcpy1( ( uint8_t* ) &SeNvmCtx, ( uint8_t* ) seNvmCtx, sizeof( SeNvmCtx ) );
        return SECURE_ELEMENT_SUCCESS;
    }
    else
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }
}

void* SecureElementGetNvmCtx( size_t* seNvmCtxSize )
{
    *seNvmCtxSize = sizeof( SeNvmCtx );
    return &SeNvmCtx;
}

SecureElementStatus_t SecureElementSetKey( KeyIdentifier_t keyID, uint8_t* key )
{
    if( key == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    for( uint8_t i = 0; i < NUM_OF_KEYS; i++ )
    {
        if( SeNvmCtx.KeyList[i].KeyID == keyID )
        {
            if( ( keyID == MC_KEY_0 ) || ( keyID == MC_KEY_1 ) || ( keyID == MC_KEY_2 ) || ( keyID == MC_KEY_3 ) )
            {  // Decrypt the key if its a Mckey
                SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
                uint8_t decryptedKey[16] = { 0 };

                retval = SecureElementAesEncrypt( key, 16, MC_KE_KEY, decryptedKey );

                memcpy1( SeNvmCtx.KeyList[i].KeyValue, decryptedKey, KEY_SIZE );
                SeNvmCtxChanged( );

                return retval;
            }
            else
            {
                memcpy1( SeNvmCtx.KeyList[i].KeyValue, key, KEY_SIZE );
                SeNvmCtxChanged( );
                return SECURE_ELEMENT_SUCCESS;
            }
        }
    }

    return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
}

SecureElementStatus_t SecureElementComputeAesCmac( uint8_t *micBxBuffer, uint8_t *buffer, uint16_t size, KeyIdentifier_t keyID, uint32_t* cmac )
{
    if( keyID >= LORAMAC_CRYPTO_MULTICAST_KEYS )
    {
        // Never accept multicast key identifier for cmac computation
        return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
    }

    return ComputeCmac( micBxBuffer, buffer, size, keyID, cmac );
}

SecureElementStatus_t SecureElementVerifyAesCmac( uint8_t* buffer, uint16_t size, uint32_t expectedCmac, KeyIdentifier_t keyID )
{
    if( buffer == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
    uint32_t compCmac = 0;
    retval = ComputeCmac( NULL, buffer, size, keyID, &compCmac );
    if( retval != SECURE_ELEMENT_SUCCESS )
    {
        return retval;
    }

    if( expectedCmac != compCmac )
    {
        retval = SECURE_ELEMENT_FAIL_CMAC;
    }

    return retval;
}

SecureElementStatus_t SecureElementAesEncrypt( uint8_t* buffer, uint16_t size, KeyIdentifier_t keyID, uint8_t* encBuffer )
{
    if( buffer == NULL || encBuffer == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    // Check if the size is divisible by 16,
    if( ( size % 16 ) != 0 )
    {
        return SECURE_ELEMENT_ERROR_BUF_SIZE;
    }

    memset1( SeNvmCtx.AesContext.ksch, '\0', 240 );

    Key_t* pItem;
    SecureElementStatus_t retval = GetKeyByID( keyID, &pItem );

    if( retval == SECURE_ELEMENT_SUCCESS )
    {
        LORAMAC_AESStart();
		LORAMAC_AES_128bits( encBuffer, buffer, size, pItem->KeyValue, NULL );
	    LORAMAC_AESStop();
    }
    return retval;
}

SecureElementStatus_t SecureElementDeriveAndStoreKey( Version_t version, uint8_t* input, KeyIdentifier_t rootKeyID, KeyIdentifier_t targetKeyID )
{
    if( input == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    SecureElementStatus_t retval = SECURE_ELEMENT_ERROR;
    uint8_t key[16] = { 0 };

    // In case of MC_KE_KEY, prevent other keys than NwkKey or AppKey for LoRaWAN 1.1 or later
    if( targetKeyID == MC_KE_KEY )
    {
        if( ( ( rootKeyID == APP_KEY ) && ( version.Fields.Minor == 0 ) ) || ( rootKeyID == NWK_KEY ) )
        {
            return SECURE_ELEMENT_ERROR_INVALID_KEY_ID;
        }
    }

    // Derive key
    retval = SecureElementAesEncrypt( input, 16, rootKeyID, key );
    if( retval != SECURE_ELEMENT_SUCCESS )
    {
        return retval;
    }

    // Store key
    retval = SecureElementSetKey( targetKeyID, key );
    if( retval != SECURE_ELEMENT_SUCCESS )
    {
        return retval;
    }

    return SECURE_ELEMENT_SUCCESS;
}

SecureElementStatus_t SecureElementProcessJoinAccept( KeyIdentifier_t encKeyID, KeyIdentifier_t micKeyID,
                                                      uint8_t versionMinor, uint8_t* micHeader, uint8_t* encJoinAccept,
                                                      uint8_t encJoinAcceptSize, uint8_t* decJoinAccept )
{
    if( ( micHeader == NULL ) || ( encJoinAccept == NULL ) || ( decJoinAccept == NULL ) )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }

    // Decrypt header, skip MHDR
    memcpy1( decJoinAccept, encJoinAccept, encJoinAcceptSize );

    if( SecureElementAesEncrypt( encJoinAccept + LORAMAC_MHDR_FIELD_SIZE, encJoinAcceptSize - LORAMAC_MHDR_FIELD_SIZE,
                                 encKeyID, decJoinAccept + LORAMAC_MHDR_FIELD_SIZE ) != SECURE_ELEMENT_SUCCESS )
    {
        return SECURE_ELEMENT_FAIL_ENCRYPT;
    }

    uint32_t mic = 0;

    mic = ( ( uint32_t ) decJoinAccept[encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE] << 0 );
    mic |= ( ( uint32_t ) decJoinAccept[encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE + 1] << 8 );
    mic |= ( ( uint32_t ) decJoinAccept[encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE + 2] << 16 );
    mic |= ( ( uint32_t ) decJoinAccept[encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE + 3] << 24 );

    // Verify mic
    if( versionMinor == 0 )
    {
        // For LoRaWAN 1.0.x
        //   cmac = aes128_cmac(NwkKey, MHDR |  JoinNonce | NetID | DevAddr | DLSettings | RxDelay | CFList | CFListType)
        if( SecureElementVerifyAesCmac( decJoinAccept, ( encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE ), mic, micKeyID ) != SECURE_ELEMENT_SUCCESS )
        {
            return SECURE_ELEMENT_FAIL_CMAC;
        }
    }
    else if( versionMinor == 1 )
    {
        // For LoRaWAN 1.1.x and later:
        //   cmac = aes128_cmac(JSIntKey, JoinReqType | JoinEUI | DevNonce | MHDR | JoinNonce | NetID | DevAddr | DLSettings | RxDelay | CFList | CFListType)
        // Prepare the msg for integrity check (adding JoinReqType, JoinEUI and DevNonce)
        uint8_t localBuffer[32 + CRYPTO_MIC_COMPUTATION_OFFSET] = { 0 };
        memcpy1( localBuffer, micHeader, CRYPTO_MIC_COMPUTATION_OFFSET );
        memcpy1( localBuffer + CRYPTO_MIC_COMPUTATION_OFFSET, decJoinAccept, encJoinAcceptSize - LORAMAC_MIC_FIELD_SIZE );

        if( SecureElementVerifyAesCmac( localBuffer, encJoinAcceptSize + CRYPTO_MIC_COMPUTATION_OFFSET - LORAMAC_MIC_FIELD_SIZE, mic, micKeyID ) != SECURE_ELEMENT_SUCCESS )
        {
            return SECURE_ELEMENT_FAIL_CMAC;
        }
    }
    else
    {
        return SECURE_ELEMENT_ERROR_INVALID_LORAWAM_SPEC_VERSION;
    }

    return SECURE_ELEMENT_SUCCESS;
}

SecureElementStatus_t SecureElementRandomNumber( uint32_t* randomNum )
{
    if( randomNum == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }
    *randomNum = Radio.Random( );
    return SECURE_ELEMENT_SUCCESS;
}

SecureElementStatus_t SecureElementSetDevEui( uint8_t* devEui )
{
    if( devEui == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }
    memcpy1( SeNvmCtx.DevEui, devEui, SE_EUI_SIZE );
    SeNvmCtxChanged( );
    return SECURE_ELEMENT_SUCCESS;
}

uint8_t* SecureElementGetDevEui( void )
{
    return SeNvmCtx.DevEui;
}

SecureElementStatus_t SecureElementSetJoinEui( uint8_t* joinEui )
{
    if( joinEui == NULL )
    {
        return SECURE_ELEMENT_ERROR_NPE;
    }
    memcpy1( SeNvmCtx.JoinEui, joinEui, SE_EUI_SIZE );
    SeNvmCtxChanged( );
    return SECURE_ELEMENT_SUCCESS;
}

uint8_t* SecureElementGetJoinEui( void )
{
    return SeNvmCtx.JoinEui;
}
