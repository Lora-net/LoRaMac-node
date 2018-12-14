/*!
 * \file      main.c
 *
 * \brief     FUOTA interop tests - test 01
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
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */

/*! \file fuota-test-01/NucleoL476/main.c */

#include <stdio.h>
#include "utilities.h"
#include "board.h"
#include "gpio.h"

#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "LmhpClockSync.h"
#include "LmhpRemoteMcastSetup.h"
#include "LmhpFragmentation.h"
#include "Display.h"

#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif

/*!
 * Time synchronization method choice
 */
#define TIME_APP_TIME_REQ                           0
#define TIME_DEVICE_TIME_REQ                        1 // DO NOT USE. TODO add a notification from the MAC layer

#define TIME_SYNCH_REQ                              TIME_APP_TIME_REQ

/*!
 * LoRaWAN default end-device class
 */
#define LORAWAN_DEFAULT_CLASS                       CLASS_A

/*!
 * Defines the application data transmission duty cycle. 30s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            30000

/*!
 * Defines a random delay for application data transmission duty cycle. 5s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND                        5000

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE                           LORAMAC_HANDLER_ADR_ON

/*!
 * Default datarate
 *
 * \remark Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_5

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE         LORAMAC_HANDLER_UNCONFIRMED_MSG

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                        false

/*!
 *
 */
typedef enum
{
    LORAMAC_HANDLER_TX_ON_TIMER,
    LORAMAC_HANDLER_TX_ON_EVENT,
}LmHandlerTxEvents_t;

/*!
 * User application data
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxTimer;

/*!
 * Timer to handle the state of LED1
 */
static TimerEvent_t Led1Timer;

/*!
 * Timer to handle the state of LED2
 */
static TimerEvent_t Led2Timer;

/*!
 * Timer to handle the state of LED beacon indicator
 */
static TimerEvent_t LedBeaconTimer;

static void OnMacProcessNotify( void );
static void OnNetworkParametersChange( CommissioningParams_t* params );
static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq );
static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq );
static void OnJoinRequest( LmHandlerJoinParams_t* params );
static void OnTxData( LmHandlerTxParams_t* params );
static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params );
static void OnClassChange( DeviceClass_t deviceClass );
static void OnBeaconStatusChange( LoRaMAcHandlerBeaconParams_t* params );

#if( TIME_SYNCH_REQ == TIME_APP_TIME_REQ )
static void OnClockSync( int32_t timeCorrection );
#endif

static void OnFragProgress( uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost );
static void OnFragDone( int32_t status, uint8_t *file, uint32_t size );
static void StartTxProcess( LmHandlerTxEvents_t txEvent );

/*!
 * Computes a CCITT 32 bits CRC
 *
 * \param [IN] buffer   Data buffer used to compute the CRC
 * \param [IN] length   Data buffer length
 * 
 * \retval crc          The computed buffer of length CRC
 */
static uint32_t Crc32( uint8_t *buffer, uint16_t length );

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent( void* context );

/*!
 * Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent( void* context );

/*!
 * Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent( void* context );

/*!
 * \brief Function executed on Beacon timer Timeout event
 */
static void OnLedBeaconTimerEvent( void* context );

static LmHandlerCallbacks_t LmHandlerCallbacks =
{
    .GetBatteryLevel = BoardGetBatteryLevel,
    .GetTemperature = NULL,
    .GetUniqueId = BoardGetUniqueId,
    .GetRandomSeed = BoardGetRandomSeed,
    .OnMacProcess = OnMacProcessNotify,
    .OnNetworkParametersChange = OnNetworkParametersChange,
    .OnMacMcpsRequest = OnMacMcpsRequest,
    .OnMacMlmeRequest = OnMacMlmeRequest,
    .OnJoinRequest = OnJoinRequest,
    .OnTxData = OnTxData,
    .OnRxData = OnRxData,
    .OnClassChange= OnClassChange,
    .OnBeaconStatusChange = OnBeaconStatusChange
};

static LmHandlerParams_t LmHandlerParams =
{
    .Region = ACTIVE_REGION,
    .AdrEnable = LORAWAN_ADR_STATE,
    .TxDatarate = LORAWAN_DEFAULT_DATARATE,
    .PublicNetworkEnable = LORAWAN_PUBLIC_NETWORK,
    .DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
    .DataBufferMaxSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
    .DataBuffer = AppDataBuffer
};

static LmhpComplianceParams_t LmhpComplianceParams =
{
    .AdrEnabled = LORAWAN_ADR_STATE,
    .DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
    .StopPeripherals = NULL,
    .StartPeripherals = NULL,
};

#if( TIME_SYNCH_REQ == TIME_APP_TIME_REQ )
static LmhpClockSyncParams_t LmhpClockSyncParams =
{
    .OnSync = OnClockSync
};
#endif

/*!
 * Defines the maximum size for the buffer receiving the fragmentation result.
 */
#define UNFRAGMENTED_DATA_SIZE                     1024

/*
 * Un-fragmented data storage.
 */
static uint8_t UnfragmentedData[UNFRAGMENTED_DATA_SIZE];

static LmhpFragmentationParams_t FragmentationParams =
{
    .Buffer = UnfragmentedData,
    .BufferSize = UNFRAGMENTED_DATA_SIZE,
    .OnProgress = OnFragProgress,
    .OnDone = OnFragDone
};

/*!
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

#if( TIME_SYNCH_REQ == TIME_APP_TIME_REQ )
/*
 * Indicates if the system time has been synchronized
 */
static volatile bool IsClockSynched = false;
#endif

/*
 * MC Session Started
 */
static volatile bool IsMcSessionStarted = false;

/*
 * Indicates if the file transfer is done
 */
static volatile bool IsFileTransferDone = false;

/*
 *  Received file computed CRC32
 */
static volatile uint32_t FileRxCrc = 0;

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1; // Tx
extern Gpio_t Led2; // Rx

static void OnMacProcessNotify( void )
{
    IsMacProcessPending = 1;
}

/*!
 * Main application entry point.
 */
int main( void )
{
    BoardInitMcu( );
    BoardInitPeriph( );

    TimerInit( &Led1Timer, OnLed1TimerEvent );
    TimerSetValue( &Led1Timer, 25 );

    TimerInit( &Led2Timer, OnLed2TimerEvent );
    TimerSetValue( &Led2Timer, 100 );

    TimerInit( &LedBeaconTimer, OnLedBeaconTimerEvent );
    TimerSetValue( &LedBeaconTimer, 5000 );

    DisplayVersion( );

    LmHandlerInit( &LmHandlerCallbacks, &LmHandlerParams );

    // Manually setup Multicast Keys.
    MibRequestConfirm_t mibReq;

    // MC key encryption key setup
    const uint8_t mcKeKey[] = { 0x2C, 0x57, 0x8F, 0x79, 0x27, 0xA9, 0x49, 0xD3, 0xB5, 0x11, 0xAE, 0x8F, 0xB6, 0x91, 0x45, 0xC6 };
    mibReq.Type = MIB_MC_KE_KEY;
    mibReq.Param.McKEKey = ( uint8_t* )mcKeKey;
    LoRaMacMibSetRequestConfirm( &mibReq );

    // The LoRa-Alliance Compliance protocol package should always be
    // initialized and activated.
    LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams );
#if( TIME_SYNCH_REQ == TIME_APP_TIME_REQ )
    LmHandlerPackageRegister( PACKAGE_ID_CLOCK_SYNC, &LmhpClockSyncParams );
#endif
    LmHandlerPackageRegister( PACKAGE_ID_REMOTE_MCAST_SETUP, NULL );
    LmHandlerPackageRegister( PACKAGE_ID_FRAGMENTATION, &FragmentationParams );

    IsClockSynched = false;
    IsFileTransferDone = false;

    LmHandlerJoin( );

    StartTxProcess( LORAMAC_HANDLER_TX_ON_TIMER );

    while( 1 )
    {
        // Processes the LoRaMac events
        LmHandlerProcess( );

        CRITICAL_SECTION_BEGIN( );
        if( IsMacProcessPending == 1 )
        {
            // Clear flag and prevent MCU to go into low power modes.
            IsMacProcessPending = 0;
        }
        else
        {
            // The MCU wakes up through events
            BoardLowPowerHandler( );
        }
        CRITICAL_SECTION_END( );
    }
}

static void OnNetworkParametersChange( CommissioningParams_t* params )
{
    DisplayNetworkParametersUpdate( params );
}

static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq )
{
    DisplayMacMcpsRequestUpdate( status, mcpsReq );
}

static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq )
{
    DisplayMacMlmeRequestUpdate( status, mlmeReq );
}

static void OnJoinRequest( LmHandlerJoinParams_t* params )
{
    DisplayJoinRequestUpdate( params );
    if( params->Status == LORAMAC_HANDLER_ERROR )
    {
        LmHandlerJoin( );
    }
    else
    {
        LmHandlerRequestClass( LORAWAN_DEFAULT_CLASS );
    }
}

static void OnTxData( LmHandlerTxParams_t* params )
{
    DisplayTxUpdate( params );
}

static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params )
{
    DisplayRxUpdate( appData, params );
}

static void OnClassChange( DeviceClass_t deviceClass )
{
    DisplayClassUpdate( deviceClass );

    switch( deviceClass )
    {
        default:
        case CLASS_A:
        {
            IsMcSessionStarted = false;
            break;
        }
        case CLASS_B:
        {
            // Inform the server as soon as possible that the end-device has switched to ClassB
            LmHandlerAppData_t appData =
            {
                .Buffer = NULL,
                .BufferSize = 0,
                .Port = 0
            };
            LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
            IsMcSessionStarted = true;
            break;
        }
        case CLASS_C:
        {
            IsMcSessionStarted = true;
            // Switch LED 2 ON
            GpioWrite( &Led2, 1 );
            break;
        }
    }
}

static void OnBeaconStatusChange( LoRaMAcHandlerBeaconParams_t* params )
{
    switch( params->State )
    {
        case LORAMAC_HANDLER_BEACON_RX:
        {
            TimerStart( &LedBeaconTimer );
            break;
        }
        case LORAMAC_HANDLER_BEACON_LOST:
        case LORAMAC_HANDLER_BEACON_NRX:
        {
            TimerStop( &LedBeaconTimer );
            break;
        }
        default:
        {
            break;
        }
    }

    DisplayBeaconUpdate( params );
}

static void OnClockSync( int32_t timeCorrection )
{
    if( timeCorrection >= -1 && timeCorrection <= 1 )
    {
        IsClockSynched = true;
    }
}

static void OnFragProgress( uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost )
{
    // Switch LED 2 OFF for each received downlink
    GpioWrite( &Led2, 0 );
    TimerStart( &Led2Timer );

    printf( "\r\n###### =========== FRAG_DECODER ============ ######\r\n" );
    printf( "######               PROGRESS                ######\r\n");
    printf( "###### ===================================== ######\r\n");
    printf( "RECEIVED    : %d/%d Fragments\r\n", fragCounter, fragNb );
    printf( "              %d/%d Bytes\r\n", fragCounter * fragSize, fragNb * fragSize );
    printf( "LOST        :    %d Fragments\n\n", fragNbLost );
}

static void OnFragDone( int32_t status, uint8_t *file, uint32_t size )
{
    FileRxCrc = Crc32( file, size );
    IsFileTransferDone = true;
    // Switch LED 2 OFF
    GpioWrite( &Led2, 0 );

    printf( "\r\n###### =========== FRAG_DECODER ============ ######\r\n" );
    printf( "######               FINISHED                ######\r\n");
    printf( "###### ===================================== ######\r\n");
    printf( "STATUS      : %ld\r\n", status );
    printf( "CRC         : %08lX\r\n", FileRxCrc );
}

static void StartTxProcess( LmHandlerTxEvents_t txEvent )
{
    switch( txEvent )
    {
    default:
        // Intentional fall through
    case LORAMAC_HANDLER_TX_ON_TIMER:
        {
            // Schedule 1st packet transmission
            TimerInit( &TxTimer, OnTxTimerEvent );
            TimerSetValue( &TxTimer, APP_TX_DUTYCYCLE  + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
            OnTxTimerEvent( NULL );
        }
        break;
    case LORAMAC_HANDLER_TX_ON_EVENT:
        {
        }
        break;
    }
}

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent( void* context )
{
    TimerStop( &TxTimer );

    if( IsMcSessionStarted == false )
    {
        if( IsFileTransferDone == false )
        {
#if( TIME_SYNCH_REQ == TIME_APP_TIME_REQ )
            LmhpClockSyncAppTimeReq( );
#else
            // Use MAC command to synchronize the time.
            LmHandlerDeviceTimeReq( );
#endif
        }
        else
        {
            uint8_t fragAuthReq[] = { 0, FileRxCrc & 0x000000FF, ( FileRxCrc >> 8 ) & 0x000000FF, ( FileRxCrc >> 16 ) & 0x000000FF, ( FileRxCrc >> 24 ) & 0x000000FF };
            // Send FragAuthReq
            LmHandlerAppData_t appData =
            {
                .Buffer = fragAuthReq,
                .BufferSize = sizeof( fragAuthReq ),
                .Port = 201
            };
            LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
        }
        // Switch LED 1 ON
        GpioWrite( &Led1, 1 );
        TimerStart( &Led1Timer );
    }

    // Schedule next transmission
    TimerSetValue( &TxTimer, APP_TX_DUTYCYCLE  + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
    TimerStart( &TxTimer );
}

/*!
 * Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent( void* context )
{
    TimerStop( &Led1Timer );
    // Switch LED 1 OFF
    GpioWrite( &Led1, 0 );
}

/*!
 * Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent( void* context )
{
    TimerStop( &Led2Timer );
    // Switch LED 2 ON
    GpioWrite( &Led2, 1 );
}

/*!
 * \brief Function executed on Beacon timer Timeout event
 */
static void OnLedBeaconTimerEvent( void* context )
{
    GpioWrite( &Led2, 1 );
    TimerStart( &Led2Timer );

    TimerStart( &LedBeaconTimer );
}

static uint32_t Crc32( uint8_t *buffer, uint16_t length )
{
    // The CRC calculation follows CCITT - 0x04C11DB7
    const uint32_t reversedPolynom = 0xEDB88320;

    // CRC initial value
    uint32_t crc = 0xFFFFFFFF;

    if( buffer == NULL )
    {
        return 0;
    }

    for( uint16_t i = 0; i < length; ++i )
    {
        crc ^= ( uint32_t )buffer[i];
        for( uint16_t i = 0; i < 8; i++ )
        {
            crc = ( crc >> 1 ) ^ ( reversedPolynom & ~( ( crc & 0x01 ) - 1 ) );
        }
    }

    return ~crc;
}
