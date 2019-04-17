/*!
 * \file      LmHandlerMsgDisplay.h
 *
 * \brief     Common set of functions to display default messages from
 *            LoRaMacHandler.
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
 *              (C)2013-2019 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "utilities.h"
#include "timer.h"

#include "LmHandlerMsgDisplay.h"

/*!
 * MAC status strings
 */
const char* MacStatusStrings[] =
{
    "OK",                            // LORAMAC_STATUS_OK
    "Busy",                          // LORAMAC_STATUS_BUSY
    "Service unknown",               // LORAMAC_STATUS_SERVICE_UNKNOWN
    "Parameter invalid",             // LORAMAC_STATUS_PARAMETER_INVALID
    "Frequency invalid",             // LORAMAC_STATUS_FREQUENCY_INVALID
    "Datarate invalid",              // LORAMAC_STATUS_DATARATE_INVALID
    "Frequency or datarate invalid", // LORAMAC_STATUS_FREQ_AND_DR_INVALID
    "No network joined",             // LORAMAC_STATUS_NO_NETWORK_JOINED
    "Length error",                  // LORAMAC_STATUS_LENGTH_ERROR
    "Region not supported",          // LORAMAC_STATUS_REGION_NOT_SUPPORTED
    "Skipped APP data",              // LORAMAC_STATUS_SKIPPED_APP_DATA
    "Duty-cycle restricted",         // LORAMAC_STATUS_DUTYCYCLE_RESTRICTED
    "No channel found",              // LORAMAC_STATUS_NO_CHANNEL_FOUND
    "No free channel found",         // LORAMAC_STATUS_NO_FREE_CHANNEL_FOUND
    "Busy beacon reserved time",     // LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME
    "Busy ping-slot window time",    // LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME
    "Busy uplink collision",         // LORAMAC_STATUS_BUSY_UPLINK_COLLISION
    "Crypto error",                  // LORAMAC_STATUS_CRYPTO_ERROR
    "FCnt handler error",            // LORAMAC_STATUS_FCNT_HANDLER_ERROR
    "MAC command error",             // LORAMAC_STATUS_MAC_COMMAD_ERROR
    "ClassB error",                  // LORAMAC_STATUS_CLASS_B_ERROR
    "Confirm queue error",           // LORAMAC_STATUS_CONFIRM_QUEUE_ERROR
    "Multicast group undefined",     // LORAMAC_STATUS_MC_GROUP_UNDEFINED
    "Unknown error",                 // LORAMAC_STATUS_ERROR
};

/*!
 * MAC event info status strings.
 */
const char* EventInfoStatusStrings[] =
{ 
    "OK",                            // LORAMAC_EVENT_INFO_STATUS_OK
    "Error",                         // LORAMAC_EVENT_INFO_STATUS_ERROR
    "Tx timeout",                    // LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT
    "Rx 1 timeout",                  // LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT
    "Rx 2 timeout",                  // LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT
    "Rx1 error",                     // LORAMAC_EVENT_INFO_STATUS_RX1_ERROR
    "Rx2 error",                     // LORAMAC_EVENT_INFO_STATUS_RX2_ERROR
    "Join failed",                   // LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL
    "Downlink repeated",             // LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED
    "Tx DR payload size error",      // LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR
    "Downlink too many frames loss", // LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS
    "Address fail",                  // LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL
    "MIC fail",                      // LORAMAC_EVENT_INFO_STATUS_MIC_FAIL
    "Multicast fail",                // LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL
    "Beacon locked",                 // LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED
    "Beacon lost",                   // LORAMAC_EVENT_INFO_STATUS_BEACON_LOST
    "Beacon not found"               // LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND
};

/*!
 * Prints the provided buffer in HEX
 * 
 * \param buffer Buffer to be printed
 * \param size   Buffer size to be printed
 */
void PrintHexBuffer( uint8_t *buffer, uint8_t size )
{
    uint8_t newline = 0;

    for( uint8_t i = 0; i < size; i++ )
    {
        if( newline != 0 )
        {
            printf( "\r\n" );
            newline = 0;
        }

        printf( "%02X ", buffer[i] );

        if( ( ( i + 1 ) % 16 ) == 0 )
        {
            newline = 1;
        }
    }
    printf( "\r\n" );
}

void DisplayNvmContextChange( LmHandlerNvmContextStates_t state )
{
    if( state == LORAMAC_HANDLER_NVM_STORE )
    {
        printf( "\r\n###### ============ CTXS STORED ============ ######\r\n\r\n" );
    }
    else
    {
        printf( "\r\n###### =========== CTXS RESTORED =========== ######\r\n\r\n" );
    }
}

void DisplayNetworkParametersUpdate( CommissioningParams_t *commissioningParams )
{
    printf( "DevEui      : %02X", commissioningParams->DevEui[0] );
    for( int i = 1; i < 8; i++ )
    {
        printf( "-%02X", commissioningParams->DevEui[i] );
    }
    printf( "\r\n" );
    printf( "AppEui      : %02X", commissioningParams->JoinEui[0] );
    for( int i = 1; i < 8; i++ )
    {
        printf( "-%02X", commissioningParams->JoinEui[i] );
    }
    printf( "\r\n" );
    // For 1.0.x devices the AppKey corresponds to NwkKey
    printf( "AppKey      : %02X", commissioningParams->NwkKey[0] );
    for( int i = 1; i < 16; i++ )
    {
        printf( " %02X", commissioningParams->NwkKey[i] );
    }
    printf( "\n\r\n" );
}

void DisplayMacMcpsRequestUpdate( LoRaMacStatus_t status, McpsReq_t *mcpsReq )
{
    switch( mcpsReq->Type )
    {
        case MCPS_CONFIRMED:
        {
            printf( "\r\n###### =========== MCPS-Request ============ ######\r\n" );
            printf( "######            MCPS_CONFIRMED             ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
        case MCPS_UNCONFIRMED:
        {
            printf( "\r\n###### =========== MCPS-Request ============ ######\r\n" );
            printf( "######           MCPS_UNCONFIRMED            ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
        case MCPS_PROPRIETARY:
        {
            printf( "\r\n###### =========== MCPS-Request ============ ######\r\n" );
            printf( "######           MCPS_PROPRIETARY            ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
        default:
        {
            printf( "\r\n###### =========== MCPS-Request ============ ######\r\n" );
            printf( "######                MCPS_ERROR             ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
    }
    printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
}

void DisplayMacMlmeRequestUpdate( LoRaMacStatus_t status, MlmeReq_t *mlmeReq )
{
    switch( mlmeReq->Type )
    {
        case MLME_JOIN:
        {
            printf( "\r\n###### =========== MLME-Request ============ ######\r\n" );
            printf( "######               MLME_JOIN               ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
        case MLME_LINK_CHECK:
        {
            printf( "\r\n###### =========== MLME-Request ============ ######\r\n" );
            printf( "######            MLME_LINK_CHECK            ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
        case MLME_DEVICE_TIME:
        {
            printf( "\r\n###### =========== MLME-Request ============ ######\r\n" );
            printf( "######            MLME_DEVICE_TIME           ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
        case MLME_TXCW:
        {
            printf( "\r\n###### =========== MLME-Request ============ ######\r\n" );
            printf( "######               MLME_TXCW               ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
        case MLME_TXCW_1:
        {
            printf( "\r\n###### =========== MLME-Request ============ ######\r\n" );
            printf( "######               MLME_TXCW_1             ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
        default:
        {
            printf( "\r\n###### =========== MLME-Request ============ ######\r\n" );
            printf( "######              MLME_UNKNOWN             ######\r\n");
            printf( "###### ===================================== ######\r\n");
            break;
        }
    }
    printf( "STATUS      : %s\r\n", MacStatusStrings[status] );
}

void DisplayJoinRequestUpdate( LmHandlerJoinParams_t *params )
{
    if( params->CommissioningParams->IsOtaaActivation == true )
    {
        if( params->Status == LORAMAC_HANDLER_SUCCESS )
        {
            printf( "###### ===========   JOINED     ============ ######\r\n" );
            printf( "\r\nOTAA\r\n\r\n" );
            printf( "DevAddr     :  %08lX\r\n", params->CommissioningParams->DevAddr );
            printf( "\r\n\r\n" );
            printf( "DATA RATE   : DR_%d\r\n\r\n", params->Datarate );
        }
    }
#if ( OVER_THE_AIR_ACTIVATION == 0 )
    else
    {
        printf( "###### ===========   JOINED     ============ ######\r\n" );
        printf( "\r\nABP\r\n\r\n" );
        printf( "DevAddr     : %08lX\r\n", params->CommissioningParams->DevAddr );
        printf( "NwkSKey     : %02X", params->CommissioningParams->FNwkSIntKey[0] );
        for( int i = 1; i < 16; i++ )
        {
            printf( " %02X", params->CommissioningParams->FNwkSIntKey[i] );
        }
        printf( "\r\n" );
        printf( "AppSKey     : %02X", params->CommissioningParams->AppSKey[0] );
        for( int i = 1; i < 16; i++ )
        {
            printf( " %02X", params->CommissioningParams->AppSKey[i] );
        }
        printf( "\n\r\n" );
    }
#endif
}

void DisplayTxUpdate( LmHandlerTxParams_t *params )
{
    MibRequestConfirm_t mibGet;

    if( params->IsMcpsConfirm == 0 )
    {
        printf( "\r\n###### =========== MLME-Confirm ============ ######\r\n" );
        printf( "STATUS      : %s\r\n", EventInfoStatusStrings[params->Status] );
        return;
    }

    printf( "\r\n###### =========== MCPS-Confirm ============ ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[params->Status] );

    printf( "\r\n###### =====   UPLINK FRAME %8lu   ===== ######\r\n", params->UplinkCounter );
    printf( "\r\n" );

    printf( "CLASS       : %c\r\n", "ABC"[LmHandlerGetCurrentClass( )] );
    printf( "\r\n" );
    printf( "TX PORT     : %d\r\n", params->AppData.Port );

    if( params->AppData.BufferSize != 0 )
    {
        printf( "TX DATA     : " );
        if( params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG )
        {
            printf( "CONFIRMED - %s\r\n", ( params->AckReceived != 0 ) ? "ACK" : "NACK" );
        }
        else
        {
            printf( "UNCONFIRMED\r\n" );
        }
        PrintHexBuffer( params->AppData.Buffer, params->AppData.BufferSize );
    }

    printf( "\r\n" );
    printf( "DATA RATE   : DR_%d\r\n", params->Datarate );

    mibGet.Type  = MIB_CHANNELS;
    if( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK )
    {
        printf( "U/L FREQ    : %lu\r\n", mibGet.Param.ChannelList[params->Channel].Frequency );
    }

    printf( "TX POWER    : %d\r\n", params->TxPower );

    mibGet.Type  = MIB_CHANNELS_MASK;
    if( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK )
    {
        printf("CHANNEL MASK: ");
        switch( LmHandlerGetActiveRegion( ) )
        {
            case LORAMAC_REGION_AS923:
            case LORAMAC_REGION_CN779:
            case LORAMAC_REGION_EU868:
            case LORAMAC_REGION_IN865:
            case LORAMAC_REGION_KR920:
            case LORAMAC_REGION_EU433:
            case LORAMAC_REGION_RU864:
            {
                printf( "%04X ", mibGet.Param.ChannelsMask[0] );
                break;
            }
            case LORAMAC_REGION_AU915:
            case LORAMAC_REGION_CN470:
            case LORAMAC_REGION_US915:
            {
                for( uint8_t i = 0; i < 5; i++)
                {
                    printf( "%04X ", mibGet.Param.ChannelsMask[i] );
                }
                break;
            }
            default:
            {
                printf( "\r\n###### ========= Unknown Region ============ ######" );
                break;
            }
        }
        printf("\r\n");
    }

    printf( "\r\n" );
}

void DisplayRxUpdate( LmHandlerAppData_t *appData, LmHandlerRxParams_t *params )
{
    const char *slotStrings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };

    if( params->IsMcpsIndication == 0 )
    {
        printf( "\r\n###### ========== MLME-Indication ========== ######\r\n" );
        printf( "STATUS      : %s\r\n", EventInfoStatusStrings[params->Status] );
        return;
    }

    printf( "\r\n###### ========== MCPS-Indication ========== ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[params->Status] );

    printf( "\r\n###### =====  DOWNLINK FRAME %8lu  ===== ######\r\n", params->DownlinkCounter );

    printf( "RX WINDOW   : %s\r\n", slotStrings[params->RxSlot] );
    
    printf( "RX PORT     : %d\r\n", appData->Port );

    if( appData->BufferSize != 0 )
    {
        printf( "RX DATA     : \r\n" );
        PrintHexBuffer( appData->Buffer, appData->BufferSize );
    }

    printf( "\r\n" );
    printf( "DATA RATE   : DR_%d\r\n", params->Datarate );
    printf( "RX RSSI     : %d\r\n", params->Rssi );
    printf( "RX SNR      : %d\r\n", params->Snr );

    printf( "\r\n" );
}

void DisplayBeaconUpdate( LoRaMAcHandlerBeaconParams_t *params )
{
    switch( params->State )
    {
        default:
        case LORAMAC_HANDLER_BEACON_ACQUIRING:
        {
            printf( "\r\n###### ========= BEACON ACQUIRING ========== ######\r\n" );
            break;
        }
        case LORAMAC_HANDLER_BEACON_LOST:
        {
            printf( "\r\n###### ============ BEACON LOST ============ ######\r\n" );
            break;
        }
        case LORAMAC_HANDLER_BEACON_RX:
        {
            printf( "\r\n###### ===== BEACON %8lu ==== ######\r\n", params->Info.Time.Seconds );
            printf( "GW DESC     : %d\r\n", params->Info.GwSpecific.InfoDesc );
            printf( "GW INFO     : " );
            PrintHexBuffer( params->Info.GwSpecific.Info, 6 );
            printf( "\r\n" );
            printf( "FREQ        : %lu\r\n", params->Info.Frequency );
            printf( "DATA RATE   : DR_%d\r\n", params->Info.Datarate );
            printf( "RX RSSI     : %d\r\n", params->Info.Rssi );
            printf( "RX SNR      : %d\r\n", params->Info.Snr );
            printf( "\r\n" );
            break;
        }
        case LORAMAC_HANDLER_BEACON_NRX:
        {
            printf( "\r\n###### ======== BEACON NOT RECEIVED ======== ######\r\n" );
            break;
        }
    }
}

void DisplayClassUpdate( DeviceClass_t deviceClass )
{
    printf( "\r\n\r\n###### ===== Switch to Class %c done.  ===== ######\r\n\r\n", "ABC"[deviceClass] );
}

void DisplayAppInfo( const char* appName, const Version_t* appVersion, const Version_t* gitHubVersion )
{
    printf( "\r\n###### ===================================== ######\r\n\r\n" );
    printf( "Application name   : %s\r\n", appName );
    printf( "Application version: %d.%d.%d\r\n", appVersion->Fields.Major, appVersion->Fields.Minor, appVersion->Fields.Revision );
    printf( "GitHub base version: %d.%d.%d\r\n", gitHubVersion->Fields.Major, gitHubVersion->Fields.Minor, gitHubVersion->Fields.Revision );
    printf( "\r\n###### ===================================== ######\r\n\r\n" );
}
