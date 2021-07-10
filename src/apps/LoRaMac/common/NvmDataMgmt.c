/*!
 * \file      NvmDataMgmt.c
 *
 * \brief     NVM context management implementation
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

#include <stdio.h>
#include "utilities.h"
#include "nvmm.h"
#include "LoRaMac.h"
#include "NvmDataMgmt.h"

/*!
 * Enables/Disables the context storage management storage.
 * Must be enabled for LoRaWAN 1.0.4 or later.
 */
#ifndef CONTEXT_MANAGEMENT_ENABLED
#define CONTEXT_MANAGEMENT_ENABLED         1
#endif


static uint16_t NvmNotifyFlags = 0;

void NvmDataMgmtEvent( uint16_t notifyFlags )
{
    NvmNotifyFlags = notifyFlags;
}

uint16_t NvmDataMgmtStore( void )
{
#if( CONTEXT_MANAGEMENT_ENABLED == 1 )
    uint16_t offset = 0;
    uint16_t dataSize = 0;
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm( &mibReq );
    LoRaMacNvmData_t* nvm = mibReq.Param.Contexts;

    // Input checks
    if( NvmNotifyFlags == LORAMAC_NVM_NOTIFY_FLAG_NONE )
    {
        // There was no update.
        return 0;
    }
    if( LoRaMacStop( ) != LORAMAC_STATUS_OK )
    {
        return 0;
    }

    // Crypto
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_CRYPTO ) ==
        LORAMAC_NVM_NOTIFY_FLAG_CRYPTO )
    {
        dataSize += NvmmWrite( ( uint8_t* ) &nvm->Crypto, sizeof( nvm->Crypto ),
                               offset );
    }
    offset += sizeof( nvm->Crypto );

    // MacGroup1
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP1 ) ==
        LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP1 )
    {
        dataSize += NvmmWrite( ( uint8_t* ) &nvm->MacGroup1,
                               sizeof( nvm->MacGroup1 ), offset );
    }
    offset += sizeof( nvm->MacGroup1 );

    // MacGroup2
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP2 ) ==
        LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP2 )
    {
        dataSize += NvmmWrite( ( uint8_t* ) &nvm->MacGroup2,
                               sizeof( nvm->MacGroup2 ), offset );
    }
    offset += sizeof( nvm->MacGroup2 );

    // Secure element
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_SECURE_ELEMENT ) ==
        LORAMAC_NVM_NOTIFY_FLAG_SECURE_ELEMENT )
    {
        dataSize += NvmmWrite( ( uint8_t* ) &nvm->SecureElement, sizeof( nvm->SecureElement ),
                               offset );
    }
    offset += sizeof( nvm->SecureElement );

    // Region group 1
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP1 ) ==
        LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP1 )
    {
        dataSize += NvmmWrite( ( uint8_t* ) &nvm->RegionGroup1,
                               sizeof( nvm->RegionGroup1 ), offset );
    }
    offset += sizeof( nvm->RegionGroup1 );

    // Region group 2
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP2 ) ==
        LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP2 )
    {
        dataSize += NvmmWrite( ( uint8_t* ) &nvm->RegionGroup2,
                               sizeof( nvm->RegionGroup2 ), offset );
    }
    offset += sizeof( nvm->RegionGroup2 );

    // Class b
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_CLASS_B ) ==
        LORAMAC_NVM_NOTIFY_FLAG_CLASS_B )
    {
        dataSize += NvmmWrite( ( uint8_t* ) &nvm->ClassB, sizeof( nvm->ClassB ),
                               offset );
    }
    offset += sizeof( nvm->ClassB );

    // Reset notification flags
    NvmNotifyFlags = LORAMAC_NVM_NOTIFY_FLAG_NONE;

    // Resume LoRaMac
    LoRaMacStart( );
    return dataSize;
#else
    return 0;
#endif
}

uint16_t NvmDataMgmtRestore( void )
{
#if( CONTEXT_MANAGEMENT_ENABLED == 1 )
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm( &mibReq );
    LoRaMacNvmData_t* nvm = mibReq.Param.Contexts;
    uint16_t offset = 0;

    // Crypto
    if( NvmmCrc32Check( sizeof( LoRaMacCryptoNvmData_t ), offset ) == false )
    {
        return 0;
    }
    offset += sizeof( LoRaMacCryptoNvmData_t );

    // Mac Group 1
    if( NvmmCrc32Check( sizeof( LoRaMacNvmDataGroup1_t ), offset ) == false )
    {
        return 0;
    }
    offset += sizeof( LoRaMacNvmDataGroup1_t );

    // Mac Group 2
    if( NvmmCrc32Check( sizeof( LoRaMacNvmDataGroup2_t ), offset ) == false )
    {
        return 0;
    }
    offset += sizeof( LoRaMacNvmDataGroup2_t );

    // Secure element
    if( NvmmCrc32Check( sizeof( SecureElementNvmData_t ), offset ) == false )
    {
        return 0;
    }
    offset += sizeof( SecureElementNvmData_t );

    // Region group 1
    if( NvmmCrc32Check( sizeof( RegionNvmDataGroup1_t ), offset ) == false )
    {
        return 0;
    }
    offset += sizeof( RegionNvmDataGroup1_t );

    // Region group 2
    if( NvmmCrc32Check( sizeof( RegionNvmDataGroup2_t ), offset ) == false )
    {
        return 0;
    }
    offset += sizeof( RegionNvmDataGroup2_t );

    // Class b
    if( NvmmCrc32Check( sizeof( LoRaMacClassBNvmData_t ), offset ) == false )
    {
        return 0;
    }
    offset += sizeof( LoRaMacClassBNvmData_t );

    if( NvmmRead( ( uint8_t* ) nvm, sizeof( LoRaMacNvmData_t ), 0 ) ==
                  sizeof( LoRaMacNvmData_t ) )
    {
        return sizeof( LoRaMacNvmData_t );
    }
#endif
    return 0;
}

bool NvmDataMgmtFactoryReset( void )
{
    uint16_t offset = 0;
#if( CONTEXT_MANAGEMENT_ENABLED == 1 )
    // Crypto
    if( NvmmReset( sizeof( LoRaMacCryptoNvmData_t ), offset ) == false )
    {
        return false;
    }
    offset += sizeof( LoRaMacCryptoNvmData_t );

    // Mac Group 1
    if( NvmmReset( sizeof( LoRaMacNvmDataGroup1_t ), offset ) == false )
    {
        return false;
    }
    offset += sizeof( LoRaMacNvmDataGroup1_t );

    // Mac Group 2
    if( NvmmReset( sizeof( LoRaMacNvmDataGroup2_t ), offset ) == false )
    {
        return false;
    }
    offset += sizeof( LoRaMacNvmDataGroup2_t );

    // Secure element
    if( NvmmReset( sizeof( SecureElementNvmData_t ), offset ) == false )
    {
        return false;
    }
    offset += sizeof( SecureElementNvmData_t );

    // Region group 1
    if( NvmmReset( sizeof( RegionNvmDataGroup1_t ), offset ) == false )
    {
        return false;
    }
    offset += sizeof( RegionNvmDataGroup1_t );

    // Region group 2
    if( NvmmReset( sizeof( RegionNvmDataGroup2_t ), offset ) == false )
    {
        return false;
    }
    offset += sizeof( RegionNvmDataGroup2_t );

    // Class b
    if( NvmmReset( sizeof( LoRaMacClassBNvmData_t ), offset ) == false )
    {
        return false;
    }
    offset += sizeof( LoRaMacClassBNvmData_t );
#endif
    return true;
}
