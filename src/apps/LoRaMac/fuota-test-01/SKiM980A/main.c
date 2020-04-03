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

/*! \file fuota-test-01/SKiM980A/main.c */

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
#include "LmHandlerMsgDisplay.h"

#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif

/*!
 * LoRaWAN default end-device class
 */
#define LORAWAN_DEFAULT_CLASS                       CLASS_A

/*!
 * Defines the application data transmission duty cycle. 30s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            40000

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
#define LORAWAN_DEFAULT_DATARATE                    DR_3

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
 * Timer to handle the state of LED4
 */
static TimerEvent_t Led4Timer;

/*!
 * Timer to handle the state of LED2
 */
static TimerEvent_t Led2Timer;

static void OnMacProcessNotify( void );
static void OnNvmContextChange( LmHandlerNvmContextStates_t state );
static void OnNetworkParametersChange( CommissioningParams_t* params );
static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn );
static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn );
static void OnJoinRequest( LmHandlerJoinParams_t* params );
static void OnTxData( LmHandlerTxParams_t* params );
static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params );
static void OnClassChange( DeviceClass_t deviceClass );
static void OnBeaconStatusChange( LoRaMAcHandlerBeaconParams_t* params );
#if( LMH_SYS_TIME_UPDATE_NEW_API == 1 )
static void OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection );
#else
static void OnSysTimeUpdate( void );
#endif
#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
static uint8_t FragDecoderWrite( uint32_t addr, uint8_t *data, uint32_t size );
static uint8_t FragDecoderRead( uint32_t addr, uint8_t *data, uint32_t size );
#endif
static void OnFragProgress( uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost );
#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
static void OnFragDone( int32_t status, uint32_t size );
#else
static void OnFragDone( int32_t status, uint8_t *file, uint32_t size );
#endif
static void StartTxProcess( LmHandlerTxEvents_t txEvent );
static void UplinkProcess( void );

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
 * Function executed on Led 4 Timeout event
 */
static void OnLed4TimerEvent( void* context );

/*!
 * Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent( void* context );

static LmHandlerCallbacks_t LmHandlerCallbacks =
{
    .GetBatteryLevel = BoardGetBatteryLevel,
    .GetTemperature = NULL,
    .GetRandomSeed = BoardGetRandomSeed,
    .OnMacProcess = OnMacProcessNotify,
    .OnNvmContextChange = OnNvmContextChange,
    .OnNetworkParametersChange = OnNetworkParametersChange,
    .OnMacMcpsRequest = OnMacMcpsRequest,
    .OnMacMlmeRequest = OnMacMlmeRequest,
    .OnJoinRequest = OnJoinRequest,
    .OnTxData = OnTxData,
    .OnRxData = OnRxData,
    .OnClassChange= OnClassChange,
    .OnBeaconStatusChange = OnBeaconStatusChange,
    .OnSysTimeUpdate = OnSysTimeUpdate
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

/*!
 * Defines the maximum size for the buffer receiving the fragmentation result.
 *
 * \remark By default FragDecoder.h defines:
 *         \ref FRAG_MAX_NB   21
 *         \ref FRAG_MAX_SIZE 50
 *
 *         FileSize = FRAG_MAX_NB * FRAG_MAX_SIZE
 *
 *         If bigger file size is to be received or is fragmented differently
 *         one must update those parameters.
 */
#define UNFRAGMENTED_DATA_SIZE                     ( 21 * 50 )

/*
 * Un-fragmented data storage.
 */
static uint8_t UnfragmentedData[UNFRAGMENTED_DATA_SIZE];

static LmhpFragmentationParams_t FragmentationParams =
{
#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
    .DecoderCallbacks = 
    {
        .FragDecoderWrite = FragDecoderWrite,
        .FragDecoderRead = FragDecoderRead,
    },
#else
    .Buffer = UnfragmentedData,
    .BufferSize = UNFRAGMENTED_DATA_SIZE,
#endif
    .OnProgress = OnFragProgress,
    .OnDone = OnFragDone
};

/*!
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

static volatile uint8_t IsTxFramePending = 0;

/*
 * Indicates if the system time has been synchronized
 */
static volatile bool IsClockSynched = false;

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
extern Gpio_t Led4; // Tx
extern Gpio_t Led2; // Rx
extern Gpio_t Led3; // App

/*!
 * Main application entry point.
 */
int main( void )
{
    BoardInitMcu( );
    BoardInitPeriph( );

    TimerInit( &Led4Timer, OnLed4TimerEvent );
    TimerSetValue( &Led4Timer, 25 );

    TimerInit( &Led2Timer, OnLed2TimerEvent );
    TimerSetValue( &Led2Timer, 100 );

    const Version_t appVersion = { .Fields.Major = 1, .Fields.Minor = 0, .Fields.Patch = 0 };
    const Version_t gitHubVersion = { .Fields.Major = 4, .Fields.Minor = 4, .Fields.Patch = 3 };
    DisplayAppInfo( "fuota-test-01", 
                    &appVersion,
                    &gitHubVersion );

    if ( LmHandlerInit( &LmHandlerCallbacks, &LmHandlerParams ) != LORAMAC_HANDLER_SUCCESS )
    {
        printf( "LoRaMac wasn't properly initialized" );
        // Fatal error, endless loop.
        while ( 1 )
        {
        }
    }

    // Set system maximum tolerated rx error in milliseconds
    LmHandlerSetSystemMaxRxError( 20 );

    // The LoRa-Alliance Compliance protocol package should always be
    // initialized and activated.
    LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams );
    LmHandlerPackageRegister( PACKAGE_ID_CLOCK_SYNC, NULL );
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

        // Process application uplinks management
        UplinkProcess( );

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

static void OnMacProcessNotify( void )
{
    IsMacProcessPending = 1;
}

static void OnNvmContextChange( LmHandlerNvmContextStates_t state )
{
    DisplayNvmContextChange( state );
}

static void OnNetworkParametersChange( CommissioningParams_t* params )
{
    DisplayNetworkParametersUpdate( params );
}

static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn )
{
    DisplayMacMcpsRequestUpdate( status, mcpsReq, nextTxIn );
}

static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn )
{
    DisplayMacMlmeRequestUpdate( status, mlmeReq, nextTxIn );
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
            break;
        }
        case LORAMAC_HANDLER_BEACON_LOST:
        case LORAMAC_HANDLER_BEACON_NRX:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    DisplayBeaconUpdate( params );
}

#if( LMH_SYS_TIME_UPDATE_NEW_API == 1 )
static void OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection )
{
    IsClockSynched = isSynchronized;
}
#else
static void OnSysTimeUpdate( void )
{
    IsClockSynched = true;
}
#endif

#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
static uint8_t FragDecoderWrite( uint32_t addr, uint8_t *data, uint32_t size )
{
    if( size >= UNFRAGMENTED_DATA_SIZE )
    {
        return -1; // Fail
    }
    for(uint32_t i = 0; i < size; i++ )
    {
        UnfragmentedData[addr + i] = data[i];
    }
    return 0; // Success
}

static uint8_t FragDecoderRead( uint32_t addr, uint8_t *data, uint32_t size )
{
    if( size >= UNFRAGMENTED_DATA_SIZE )
    {
        return -1; // Fail
    }
    for(uint32_t i = 0; i < size; i++ )
    {
        data[i] = UnfragmentedData[addr + i];
    }
    return 0; // Success
}
#endif

static void OnFragProgress( uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost )
{
    // Switch LED 2 OFF for each received downlink
    GpioWrite( &Led2, 0 );
    TimerStart( &Led2Timer );

    printf( "\n###### =========== FRAG_DECODER ============ ######\n" );
    printf( "######               PROGRESS                ######\n");
    printf( "###### ===================================== ######\n");
    printf( "RECEIVED    : %5d / %5d Fragments\n", fragCounter, fragNb );
    printf( "              %5d / %5d Bytes\n", fragCounter * fragSize, fragNb * fragSize );
    printf( "LOST        :       %7d Fragments\n\n", fragNbLost );
}

#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
static void OnFragDone( int32_t status, uint32_t size )
{
    FileRxCrc = Crc32( UnfragmentedData, size );
    IsFileTransferDone = true;
    // Switch LED 2 OFF
    GpioWrite( &Led2, 0 );

    printf( "\n###### =========== FRAG_DECODER ============ ######\n" );
    printf( "######               FINISHED                ######\n");
    printf( "###### ===================================== ######\n");
    printf( "STATUS      : %ld\n", status );
    printf( "CRC         : %08lX\n\n", FileRxCrc );
}
#else
static void OnFragDone( int32_t status, uint8_t *file, uint32_t size )
{
    FileRxCrc = Crc32( file, size );
    IsFileTransferDone = true;
    // Switch LED 2 OFF
    GpioWrite( &Led2, 0 );

    printf( "\n###### =========== FRAG_DECODER ============ ######\n" );
    printf( "######               FINISHED                ######\n");
    printf( "###### ===================================== ######\n");
    printf( "STATUS      : %ld\n", status );
    printf( "CRC         : %08lX\n\n", FileRxCrc );
}
#endif

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

static void UplinkProcess( void )
{
    LmHandlerErrorStatus_t status = LORAMAC_HANDLER_ERROR;

    if( LmHandlerIsBusy( ) == true )
    {
        return;
    }

    uint8_t isPending = 0;
    CRITICAL_SECTION_BEGIN( );
    isPending = IsTxFramePending;
    IsTxFramePending = 0;
    CRITICAL_SECTION_END( );
    if( isPending == 1 )
    {
        if( IsMcSessionStarted == false )
        {
            if( IsFileTransferDone == false )
            {
                if( IsClockSynched == false )
                {
                    status = LmhpClockSyncAppTimeReq( );
                }
                else
                {
                    AppDataBuffer[0] = randr( 0, 255 );
                    // Send random packet
                    LmHandlerAppData_t appData =
                    {
                        .Buffer = AppDataBuffer,
                        .BufferSize = 1,
                        .Port = 1
                    };
                    status = LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
                }
            }
            else
            {
                AppDataBuffer[0] = 0x05; // FragDataBlockAuthReq
                AppDataBuffer[1] = FileRxCrc & 0x000000FF;
                AppDataBuffer[2] = ( FileRxCrc >> 8 ) & 0x000000FF;
                AppDataBuffer[3] = ( FileRxCrc >> 16 ) & 0x000000FF;
                AppDataBuffer[4] = ( FileRxCrc >> 24 ) & 0x000000FF;

                // Send FragAuthReq
                LmHandlerAppData_t appData =
                {
                    .Buffer = AppDataBuffer,
                    .BufferSize = 5,
                    .Port = 201
                };
                status = LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
            }
            if( status == LORAMAC_HANDLER_SUCCESS )
            {
                // Switch LED 1 ON
                GpioWrite( &Led4, 1 );
                TimerStart( &Led4Timer );
            }
        }
    }
}

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent( void* context )
{
    TimerStop( &TxTimer );

    IsTxFramePending = 1;

    // Schedule next transmission
    TimerSetValue( &TxTimer, APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
    TimerStart( &TxTimer );
}

/*!
 * Function executed on Led 4 Timeout event
 */
static void OnLed4TimerEvent( void* context )
{
    TimerStop( &Led4Timer );
    // Switch LED 4 OFF
    GpioWrite( &Led4, 0 );
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
