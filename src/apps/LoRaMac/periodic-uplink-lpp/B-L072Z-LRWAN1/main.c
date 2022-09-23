/*!
 * \file      main.c
 *
 * \brief     Performs a periodic uplink
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

/*! \file periodic-uplink/B-L072Z-LRWAN1/main.c */

#include <stdio.h>
#include "board.h"

#include "cli.h"
#include "Commissioning.h"

#include "config.h"
#include "ublox.h"
#include "geofence.h"
#include "iwdg.h"
#include "print_utils.h"
#include "NvmDataMgmt.h"
#include "RegionAS923.h"
#include "callbacks.h"
#include "message_sender.h"
#include "eeprom_settings_manager.h"
#include "nvmm.h"

#ifdef UNITTESTING_LORA
#include "rtc_mock.h"
#endif

/*!
 * User application data
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * User application data structure
 */
static LmHandlerAppData_t AppData =
{
    .Buffer = AppDataBuffer,
    .BufferSize = 0,
    .Port = 0
};

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxTimer;

static void OnMacProcessNotify( void );
static void timer_init( void );
static void UplinkProcess( void );
static void transmit_n_times_on_this_credential( void );
static void setup_next_tx_alarm( uint32_t interval );

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent( void* context );

static LmHandlerCallbacks_t LmHandlerCallbacks =
{
    .GetBatteryLevel = BoardGetBatteryLevel,
    .GetTemperature = NULL,
    .GetRandomSeed = BoardGetRandomSeed,
    .OnMacProcess = OnMacProcessNotify,
    .OnNvmDataChange = OnNvmDataChange,
    .OnNetworkParametersChange = OnNetworkParametersChange,
    .OnMacMcpsRequest = OnMacMcpsRequest,
    .OnMacMlmeRequest = OnMacMlmeRequest,
    .OnJoinRequest = OnJoinRequest,
    .OnTxData = OnTxData,
    .OnRxData = OnRxData,
    .OnClassChange= OnClassChange,
    .OnBeaconStatusChange = OnBeaconStatusChange,
    .OnSysTimeUpdate = OnSysTimeUpdate,
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
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

static volatile uint8_t IsTxFramePending = 0;

/*!
 * UART object used for command line interface handling
 */
extern Uart_t Uart1;


uint32_t tx_count_on_this_credential = 0;

/*!
 * Main application entry point.
 */
int main( void )
{
#if( USE_WATCHDOG )
    IWDG_Init( );
#endif

    setup_board( );

#if PRINT_EEPROM_DEBUG
    printf( "Dumping EEPROM:\n" );
    EEPROM_Dump( );
    printf( "\n" );
#endif

    /* Initialise timer */
    timer_init( );

    /* Transmit immediately (10 milliseconds later) */
    setup_next_tx_alarm( 10 );

    while( 1 )
    {
        switch_to_next_registered_credentials( ); // Switch to the next set of
                                                  // credentials
        transmit_n_times_on_this_credential( );   // do 2 transmissions on the
                                                  // same credential
    }
}

static void transmit_n_times_on_this_credential( void )
{
    print_current_region( );

    /* Configure the subband settings for AS923 BEFORE initing it. Only needed
     * to be done for AS923 - does not do anything for other regions */
    as923_subbands_t current_subband = get_as923_subband( );
    set_as923_region_specific_frequencies( current_subband );

    /* Set region and datarate */
    LoRaMacRegion_t current_loramac_region = get_current_loramac_region( );
    picotracker_lorawan_settings_t settings = get_lorawan_setting( current_loramac_region );
    LmHandlerParams.Region = current_loramac_region;
    LmHandlerParams.TxDatarate = settings.datarate;

    if ( LmHandlerInit( &LmHandlerCallbacks, &LmHandlerParams ) != LORAMAC_HANDLER_SUCCESS )
    {
        printf( "LoRaMac wasn't properly initialized\n" );
        // Fatal error, endless loop.
        while ( 1 )
        {
        }
    }

    // Set system maximum tolerated rx error in milliseconds
    LmHandlerSetSystemMaxRxError( 500 );

    // The LoRa-Alliance Compliance protocol package should always be
    // initialized and activated.
    LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams );

    LmHandlerJoin( );

    IWDG_reset( );

    tx_count_on_this_credential = 0; // reset tx count for this network back to 0.

    while( tx_count_on_this_credential < N_TRANMISSIONS_PER_CREDENTIAL )
    {
#ifdef UNITTESTING_LORA
        /* simulate 1 millisecond per loop */
        bump_rtc_tick( );
#endif
        // Process characters sent over the command line interface
        CliProcess( &Uart1 );

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

/*!
 * Prepares the payload of the frame and transmits it.
 */
static void PrepareTxFrame( void )
{
    IWDG_reset( );

    if( LmHandlerIsBusy( ) == true )
    {
        return;
    }

    tx_count_on_this_credential ++;
    sensor_read_and_send( &AppData, LmHandlerParams.Region );
    uint32_t interval = read_tx_interval_in_eeprom( TX_INTERVAL_EEPROM_ADDRESS,
                                                    TX_INTERVAL_GPS_FIX_OK );
    setup_next_tx_alarm( interval );
}

static void UplinkProcess( void )
{
    uint8_t isPending = 0;
    CRITICAL_SECTION_BEGIN( );
    isPending = IsTxFramePending;
    IsTxFramePending = 0;
    CRITICAL_SECTION_END( );
    if( isPending == 1 )
    {
        IWDG_reset( );
        PrepareTxFrame( );
        IWDG_reset( );
    }
}

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent( void* context )
{
    IWDG_reset( );
    TimerStop( &TxTimer ); // stop the timer now - it will be restarted after a
                           // tranmission has been completed.

    IsTxFramePending = 1;
}

static void setup_next_tx_alarm( uint32_t interval )
{
    IWDG_reset( );
    TimerStop( &TxTimer ); /* Stop tx timer. Requirement before starting it back
                              up again */
    TimerSetValue( &TxTimer, interval );
    TimerStart( &TxTimer ); /* Restart tx interval timer */
}

static void timer_init( )
{
    TimerInit( &TxTimer, OnTxTimerEvent );
    TimerStop( &TxTimer );
}
