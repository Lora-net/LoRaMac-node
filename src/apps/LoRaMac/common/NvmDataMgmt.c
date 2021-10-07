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
#include "LoRaWAN_config_switcher.h"


/*!
 * Enables/Disables the context storage management storage.
 * Must be enabled for LoRaWAN 1.0.4 or later.
 */
bool context_management_enabled = true;




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


typedef struct
{
    uint32_t fcount;
} selected_loramac_params_t;

selected_loramac_params_t lormac_params;




void NvmDataMgmtEvent( uint16_t notifyFlags )
{
    NvmNotifyFlags = notifyFlags;
}

uint16_t NvmDataMgmtStore( void )
{
    if (context_management_enabled == true)
    {

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

        // Store frame count here

        MibRequestConfirm_t mibReq;
        mibReq.Type = MIB_NVM_CTXS;
        LoRaMacMibGetRequestConfirm( &mibReq );
        LoRaMacNvmData_t* nvm = mibReq.Param.Contexts;

        uint16_t bytes_written;

        lormac_params.fcount = nvm->Crypto.FCntList.FCntUp;

        bytes_written = NvmmWrite((uint8_t *)&lormac_params, sizeof(selected_loramac_params_t), 0);

        // Reset notification flags
        NvmNotifyFlags = LORAMAC_NVM_NOTIFY_FLAG_NONE;

        // Resume LoRaMac
        LoRaMacStart( );

        return 0;
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
        LoRaMacMibGetRequestConfirm(&mibReq);
        LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

        // Read from eeprom the region/networks fcount
        uint16_t bytes_read;
        bytes_read = NvmmRead((uint8_t *)&lormac_params, sizeof(selected_loramac_params_t), 0);

        nvm->Crypto.FCntList.FCntUp = lormac_params.fcount;

        return 0;
    }
    return 0;
}


