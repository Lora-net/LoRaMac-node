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
#include "print_utils.h"
#include "config.h"
#include "region_nvm.h"
#include "geofence.h"
#include "compression_structs.h"

#include <string.h>  // For memcmp()
#include <stdlib.h>  // For exit()
#include "lz4.h"     // This is all that is required to expose the prototypes for basic compression and decompression.
/*!
 * Enables/Disables the context storage management storage.
 * Must be enabled for LoRaWAN 1.0.4 or later.
 */
bool context_management_enabled = true;


void run_screaming(const char* message, const int code);

uint32_t compress_nvm(LoRaMacNvmData_t *nvm_data);
uint32_t decompress_nvm(LoRaMacNvmData_t *nvm_data);


#define PRINT_TIME( )                                                                \
    do                                                                               \
    {                                                                                \
        SysTime_t time = SysTimeGet( );                                              \
        printf( "%ld.%03d: ", time.Seconds-UNIX_GPS_EPOCH_OFFSET, time.SubSeconds ); \
    } while( 0 )

#define PRINT( ... )            \
    do                          \
    {                           \
        PRINT_TIME( );          \
        printf( "[DBG    ] " ); \
        printf( __VA_ARGS__ );  \
    } while( 0 )



static uint16_t NvmNotifyFlags = 0;

/* Size of buffer to hold the compressed nvm data before writing to EEPROM.
 * It must be greater than the max possible compressed size
 */




nvm_data_t nvm_data_struct;

void NvmDataMgmtEvent( uint16_t notifyFlags )
{
    NvmNotifyFlags = notifyFlags;
}

uint16_t NvmDataMgmtStore( void )
{
    if (context_management_enabled == true)
    {
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

        SysTime_t start = SysTimeGetMcuTime( );
        SysTime_t stop = {.Seconds = 0, .SubSeconds = 0};

#if PRINT_EEPROM_DEBUG
        printf("NVM Bytes:\n");
        print_bytes(nvm, sizeof(LoRaMacNvmData_t));
        printf("\n");
#endif

        printf("Start Compression........\n");
        /* Compress NVM data */
        compress_nvm(nvm);
        /* Now update CRC in the struct */
        nvm_data_struct.Crc32 = Crc32((uint8_t *)&nvm_data_struct, sizeof(nvm_data_struct) - sizeof(nvm_data_struct.Crc32));
        printf("End Compression........\n");

#if PRINT_EEPROM_DEBUG
        printf("Writing these bytes\n");
        print_bytes(&nvm_data_struct, sizeof(nvm_data_struct));
        printf("End these bytes\n");
#endif

        /* Save compressed NVM to eeprom */
        uint16_t eeprom_address = get_eeprom_location_for_region(get_current_loramac_region());
        uint16_t ret = NvmmUpdate((void *)&nvm_data_struct, sizeof(nvm_data_struct), eeprom_address);

        // Reset notification flags
        NvmNotifyFlags = LORAMAC_NVM_NOTIFY_FLAG_NONE;

        // Resume LoRaMac
        LoRaMacStart( );

        stop = SysTimeGetMcuTime( );
        SysTime_t diff = SysTimeSub( stop, start );
        PRINT( "NVM updated in %ld ms\n", diff.Seconds * 1000 + diff.SubSeconds );     
           
        return ret;
    }
    else
    {
        return 0;
    }
}

uint16_t NvmDataMgmtRestore( void )
{
    if (context_management_enabled == true)
    {
        MibRequestConfirm_t mibReq;
        mibReq.Type = MIB_NVM_CTXS;
        LoRaMacMibGetRequestConfirm( &mibReq );
        LoRaMacNvmData_t* nvm = mibReq.Param.Contexts;

        /* Read nvm into compressed data buffer(not yet decompressed) */
        uint16_t eeprom_address = get_eeprom_location_for_region(get_current_loramac_region());
        NvmmRead((void *)&nvm_data_struct, sizeof(nvm_data_struct), eeprom_address);

#if PRINT_EEPROM_DEBUG
        printf("Reading these bytes\n");
        print_bytes(&nvm_data_struct, sizeof(nvm_data_struct));
        printf("End these bytes\n");
#endif

        printf("src size: %d compressed_datasize: %d \n", sizeof(LoRaMacNvmData_t), nvm_data_struct.compressed_data_size);

        printf("Start Deompression........\n");
        uint32_t ret = decompress_nvm(nvm);
        printf("End Decompression........\n");

        return ret;
    }
    return 0;
}


bool NvmDataMgmtFactoryReset( void )
{
    uint16_t offset = 0;
    if (context_management_enabled == true)
    {
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
    }
    return true;
}


/*
 * Simple show-error-and-bail function.
 */
void run_screaming(const char* message, const int code) {
  printf("%s \n", message);
//   exit(code);
}



/**
 * @brief Compress nvm data to buffer

 * 
 * @param nvm_data 
 * @return int 
 */
uint32_t compress_nvm(LoRaMacNvmData_t *nvm_data)
{
    memcpy((void *)&nvm_data_struct.Crypto, (void *)&nvm_data->Crypto, sizeof(LoRaMacCryptoNvmData_t));
    memcpy((void *)&nvm_data_struct.MacGroup1, (void *)&nvm_data->MacGroup1, sizeof(LoRaMacNvmDataGroup1_t));
    memcpy((void *)&nvm_data_struct.RegionGroup1, (void *)&nvm_data->RegionGroup1, sizeof(RegionNvmDataGroup1_t));

    const char *src = (char *)(rarely_changing_t *)nvm_data;

    int src_size = sizeof(rarely_changing_t);

    const int max_dst_size = LZ4_compressBound(src_size);

    if (nvm_data_struct.compressed_data == NULL)
        run_screaming("Failed to allocate memory for *compressed_data.", 1);

    nvm_data_struct.compressed_data_size = LZ4_compress_default(src, nvm_data_struct.compressed_data, src_size, max_dst_size);

    if (nvm_data_struct.compressed_data_size <= 0)
        run_screaming("A 0 or negative result from LZ4_compress_default() indicates a failure trying to compress the data. ", 1);

    if (nvm_data_struct.compressed_data_size > 0)
        printf("We successfully compressed some data! Ratio: %.2f\n",
               (float)nvm_data_struct.compressed_data_size / src_size);

    printf("Compressed %d bytes to %d bytes\n", src_size, nvm_data_struct.compressed_data_size);

    return nvm_data_struct.compressed_data_size;
}

/**
 * @brief Decompress NVM data from eeprom
 * 
 * @param nvm_data 
 * @return int 
 */

uint32_t decompress_nvm(LoRaMacNvmData_t *nvm_data)
{

    /* Check if CRC is correct */
    if (is_crc_correct(sizeof(nvm_data_t), &nvm_data_struct) == false)
    {
        return 0;
    }

    char *regen_buffer = (char *)(rarely_changing_t *)nvm_data;

    int src_size = sizeof(rarely_changing_t);

    if (regen_buffer == NULL)
        run_screaming("Failed to allocate memory for *regen_buffer.", 1);

    const int decompressed_size = LZ4_decompress_safe(nvm_data_struct.compressed_data, regen_buffer, nvm_data_struct.compressed_data_size, src_size);

    printf("Decompressed %d bytes to %d bytes\n", nvm_data_struct.compressed_data_size, decompressed_size);

    if (decompressed_size < 0)
    {
        run_screaming("A negative result from LZ4_decompress_safe indicates a failure trying to decompress the data.  See exit code (echo $?) for value returned.", decompressed_size);
    }

    if (decompressed_size >= 0)
    {
        printf("We successfully decompressed some data!\n");
    }

    if (decompressed_size != src_size)
    {
        run_screaming("Decompressed data is different from original! \n", 1);
    }

    if ((decompressed_size >= 0) && (decompressed_size == src_size))
    {

        memcpy((void *)&nvm_data->Crypto, (void *)&nvm_data_struct.Crypto, sizeof(LoRaMacCryptoNvmData_t));
        memcpy((void *)&nvm_data->MacGroup1, (void *)&nvm_data_struct.MacGroup1, sizeof(LoRaMacNvmDataGroup1_t));
        memcpy((void *)&nvm_data->RegionGroup1, (void *)&nvm_data_struct.RegionGroup1, sizeof(RegionNvmDataGroup1_t));

        return sizeof(LoRaMacNvmData_t);
    }
    return 0;
}
