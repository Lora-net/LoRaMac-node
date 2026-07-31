/*!
 * \file  main.c
 *
 * \brief Performs a periodic uplink
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*! \file periodic-uplink/NAMote72/main.c */
#include <stdio.h>
#include <string.h>
#include "../firmwareVersion.h"
#include "../../common/githubVersion.h"
#include "utilities.h"
#include "board-config.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "RegionCommon.h"
#include "gps.h"
#include "mpl3115.h"

#include "cli.h"
#include "Commissioning.h"
#include "LmHandler.h"
#include "secure-element.h"
#include "LmhpCompliance.h"
#include "CayenneLpp.h"
#include "LmHandlerMsgDisplay.h"
#include "labscim_platform_socket.h"

#include "labscim_helper.h"
#include <sys/time.h>


#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif

/*!
 * LoRaWAN default end-device class
 */
#ifndef LORAWAN_DEFAULT_CLASS
#define LORAWAN_DEFAULT_CLASS                       CLASS_A
#endif

extern uint64_t gCurrentTime;


uint64_t gUpstreamPacketGeneratedSignal;
uint64_t gUpstreamPacketLatencySignal;
uint64_t gDownstreamLatencySignal;
uint64_t gNodeJoinSignal;
uint64_t gUpstreamAoIMaxSignal;
uint64_t gUpstreamAoIMinSignal;
uint64_t gAoIAreaSignal;
uint64_t gPacketReceivedSignal;
uint64_t gDownstreamPacketSentOnGateway;
uint64_t gDownstreamPacketSentToMe;

uint64_t gSignature=0;
extern uint8_t mac_addr[];

struct signal_info
{
	uint64_t signature;	
    double error;
	double latency;
	double aoi_max;
	double aoi_min;
	double aoi_area;
} __attribute__((packed));

struct packet_generated_info
{
	uint64_t signature;	
    uint64_t labscim_time;	
} __attribute__((packed));

extern uint8_t gAPP_KEY[32];
/*!
 * Defines the application data transmission duty cycle. 60s, value in [ms].
 * Matches the reference Class A application: APP_TX_DUTYCYCLE = 60000. It had been set
 * to 5000 here, i.e. twelve times the intended offered load.
 */
#define APP_TX_DUTYCYCLE                            60000

/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND                        1000

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
#define LORAWAN_DEFAULT_DATARATE                    DR_0

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
#define LORAWAN_DUTYCYCLE_ON                        true

/*!
 * LoRaWAN application port
 * @remark The allowed port range is from 1 up to 223. Other values are reserved.
 */
#define LORAWAN_APP_PORT                            2

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
 * User application data structure
 */
static LmHandlerAppData_t AppData =
{
    .Buffer = AppDataBuffer,
    .BufferSize = 0,
    .Port = 0,
};

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;

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
static void OnNvmDataChange( LmHandlerNvmContextStates_t state, uint16_t size );
static void OnNetworkParametersChange( CommissioningParams_t* params );
static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn );
static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn );
static void OnJoinRequest( LmHandlerJoinParams_t* params );
static void OnTxData( LmHandlerTxParams_t* params );
static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params );
static void OnClassChange( DeviceClass_t deviceClass );
static void OnBeaconStatusChange( LoRaMacHandlerBeaconParams_t* params );
#if( LMH_SYS_TIME_UPDATE_NEW_API == 1 )
static void OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection );
#else
static void OnSysTimeUpdate( void );
#endif
static void PrepareTxFrame( void );
static void StartTxProcess( LmHandlerTxEvents_t txEvent );
static void UplinkProcess( void );

static void OnTxPeriodicityChanged( uint32_t periodicity );
static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed );
static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity );

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
    .GetTemperature = MPL3115ReadTemperature,
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
    .IsTxConfirmed = LORAWAN_DEFAULT_CONFIRMED_MSG_STATE,
    .TxDatarate = LORAWAN_DEFAULT_DATARATE,
    .PublicNetworkEnable = LORAWAN_PUBLIC_NETWORK,
    .DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
    .DataBufferMaxSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
    .DataBuffer = AppDataBuffer,
    .PingSlotPeriodicity = REGION_COMMON_DEFAULT_PING_SLOT_PERIODICITY,
};

static LmhpComplianceParams_t LmhpComplianceParams =
{
    .FwVersion.Value = FIRMWARE_VERSION,
    .OnTxPeriodicityChanged = OnTxPeriodicityChanged,
    .OnTxFrameCtrlChanged = OnTxFrameCtrlChanged,
    .OnPingSlotPeriodicityChanged = OnPingSlotPeriodicityChanged,
};

/*!
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

static volatile uint8_t IsTxFramePending = 0;

static volatile uint32_t TxPeriodicity = 0;

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1; // Tx
extern Gpio_t Led2; // Rx and blinks every 5 seconds when beacon is acquired
extern Gpio_t Led3; // App

/*!
 * UART object used for command line interface handling
 */
//extern Uart_t Uart2;

/*!
 * Main application entry point.
 */
int main(int argc, char const *argv[])
{
    char* mac_inv = (char*)(&gSignature);

    //command line arguments to this node
    platform_process_args(argc,argv);

    BoardInitMcu( );
    BoardInitPeriph( );

    TimerInit( &Led1Timer, OnLed1TimerEvent );
    TimerSetValue( &Led1Timer, 25 );

    TimerInit( &Led2Timer, OnLed2TimerEvent );
    TimerSetValue( &Led2Timer, 25 );

    TimerInit( &LedBeaconTimer, OnLedBeaconTimerEvent );
    TimerSetValue( &LedBeaconTimer, 5000 );

    // Initialize transmission periodicity variable
    TxPeriodicity = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );

    const Version_t appVersion = { .Value = FIRMWARE_VERSION };
    const Version_t gitHubVersion = { .Value = GITHUB_VERSION };
    DisplayAppInfo( "periodic-uplink-lpp", 
                    &appVersion,
                    &gitHubVersion );

    gUpstreamPacketGeneratedSignal = LabscimSignalRegister("LoRaUpstreamPacketGenerated");
    gUpstreamPacketLatencySignal = LabscimSignalRegister("LoRaUpstreamPacketLatency");    
    gNodeJoinSignal = LabscimSignalRegister("LoRaNodeJoin"); 

    gUpstreamAoIMaxSignal = LabscimSignalRegister("LoRaUpstreamAoIMax");
    gUpstreamAoIMinSignal = LabscimSignalRegister("LoRaUpstreamAoIMin");
    gAoIAreaSignal = LabscimSignalRegister("LoRaUpstreamAoIArea");

    gUpstreamPacketLatencySignal = LabscimSignalRegister("LoRaUpstreamPacketLatency");    
    gNodeJoinSignal = LabscimSignalRegister("LoRaNodeJoin");    
    
    gDownstreamLatencySignal = LabscimSignalRegister("LoRaDownstreamPacketLatency"); 
    gDownstreamPacketSentToMe = LabscimSignalRegister("LoRaDownstreamPacketGenerated"); 
       

    for(uint32_t i=0;i<8;i++)
    {
        mac_inv[i] = mac_addr[7-i];
    }

    gPacketReceivedSignal = LabscimSignalRegister("LoRaPacketReceived");
	LabscimSignalSubscribe(gPacketReceivedSignal);

    gDownstreamPacketSentOnGateway = LabscimSignalRegister("LoRaDownstreamPacket");
	LabscimSignalSubscribe(gDownstreamPacketSentOnGateway);


    if ( LmHandlerInit( &LmHandlerCallbacks, &LmHandlerParams ) != LORAMAC_HANDLER_SUCCESS )
    {
        labscim_printf( "LoRaMac wasn't properly initialized\n" );
        // Fatal error, endless loop.
        while ( 1 )
        {
        }
    }

    /*
     * Install the per-node root key the model hands us at boot: SHA1("node-<mac>"), in
     * gAPP_KEY. Without this every device keeps the soft-se demo key (2B7E1516...), and
     * since a JoinAccept carries no device identifier while its MIC is computed with the
     * NwkKey, a device happily validates and accepts the JoinAccept addressed to another
     * one, adopts its DevAddr and transmits with the wrong session keys -- the server then
     * drops those uplinks on MIC failure (generated > 0, delivered = 0).
     * Must run after LmHandlerInit, which initialises the secure element, and before the
     * join. The server needs the same key: see provision_devices.py.
     */
    SecureElementSetKey( NWK_KEY, gAPP_KEY );
    SecureElementSetKey( APP_KEY, gAPP_KEY );

    // Set system maximum tolerated rx error in milliseconds
    LmHandlerSetSystemMaxRxError( 20 );

    // The LoRa-Alliance Compliance protocol package should always be
    // initialized and activated.
    LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams );

    LmHandlerJoin( );

    StartTxProcess( LORAMAC_HANDLER_TX_ON_TIMER );

    while( 1 )
    {
        // Process characters sent over the command line interface
        //CliProcess( &Uart2 );

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

static void OnNvmDataChange( LmHandlerNvmContextStates_t state, uint16_t size )
{
    DisplayNvmDataChange( state, size );
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

        //20250612
        // Emit join signal
        MibRequestConfirm_t mibReq;
        mibReq.Type = MIB_DEV_ADDR;
        LoRaMacMibGetRequestConfirm( &mibReq );
        LabscimSignalEmitDouble(gNodeJoinSignal, (double)mibReq.Param.DevAddr);
    }
}

static void OnTxData( LmHandlerTxParams_t* params )
{
    DisplayTxUpdate( params );
}

static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params )
{
    DisplayRxUpdate( appData, params );

    switch( appData->Port )
    {
    case 1: // The application LED can be controlled on port 1 or 2
    case LORAWAN_APP_PORT:
        {
            //uint64_t *tx_time = (uint64_t *)appData->Buffer;
            //LabscimSignalEmitDouble(gPacketLatencySignal, (double)(gCurrentTime - tx_time[1]) / 1e6);
            //LabscimSignalEmitDouble(gRTTSignal, (double)(gCurrentTime - tx_time[0]) / 1e6);
            
            //20250612
            if (appData->BufferSize >= 2 * sizeof(uint64_t)) {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                long long timestamp_us = (long long)tv.tv_sec * 1000000LL + tv.tv_usec;                
                uint64_t *tx_time = (uint64_t *)appData->Buffer;                
                LabscimSignalEmitDouble(gDownstreamLatencySignal, (double)(timestamp_us - tx_time[1]) / 1e6);
                // Deliberately NOT emitting gUpstreamPacketLatencySignal here.
                //
                // There are two emitters of LoRaUpstreamPacketLatency. The one the
                // reference results use is the gateway path: lora_pkt_fwd subscribes to the
                // ChirpStack uplink event itself, computes the latency and the AoI in
                // loragw_hal.c, and feeds LoRaPacketReceived back into the simulator, where
                // labscim_signal_arrived() (below, ~line 720) stamps it. There "delivered"
                // means the uplink reached the server -- no downlink involved, and AoI is
                // emitted alongside.
                //
                // Emitting here instead makes "delivered" mean "the uplink reached the
                // server AND an application echo came back", which silently folds downlink
                // loss into the uplink numerator and reports a round-trip wall-clock time in
                // place of a simulated-time uplink latency. It was re-enabled as a
                // workaround for PDR 0, but the real cause of that was
                // MQTTLoggerApplicationTopic still carrying the v3 form "application/1/..."
                // while ChirpStack v4 publishes under an application UUID, so the gateway
                // subscription matched nothing. That is fixed in the .ini files now.
                //
                // Checked against the reference .sca files: LoRaUpstreamPacketLatency equals
                // the AoI count exactly in every run, and LoRaDownstreamPacketGenerated is 0
                // everywhere -- there was never an application downlink.
            }
            //AppLedStateOn = appData->Buffer[0] & 0x01;
            //GpioWrite( &Led3, ( ( AppLedStateOn & 0x01 ) != 0 ) ? 1 : 0 );
        }
        break;
    default:
        break;
    }

    // Switch LED 2 ON for each received downlink
    GpioWrite( &Led2, 0 );
    TimerStart( &Led2Timer );
}

static void OnClassChange( DeviceClass_t deviceClass )
{
    DisplayClassUpdate( deviceClass );

    // Inform the server as soon as possible that the end-device has switched to ClassB
    LmHandlerAppData_t appData =
    {
        .Buffer = NULL,
        .BufferSize = 0,
        .Port = 0,
    };
    LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
}

static void OnBeaconStatusChange( LoRaMacHandlerBeaconParams_t* params )
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

#if( LMH_SYS_TIME_UPDATE_NEW_API == 1 )
static void OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection )
{

}
#else
static void OnSysTimeUpdate( void )
{

}
#endif

/*!
 * Prepares the payload of the frame and transmits it.
 */
static void PrepareTxFrame( void )
{
    if( LmHandlerIsBusy( ) == true )
    {
        return;
    }

#if defined( REGION_US915 )
    MibRequestConfirm_t mibReq;

    if( BoardGetBatteryVoltage( ) < LOW_BAT_THRESHOLD )
    {
        mibReq.Type = MIB_CHANNELS_TX_POWER;
        LoRaMacMibGetRequestConfirm( &mibReq );
        // 30 dBm = TX_POWER_0, 28 dBm = TX_POWER_1, ..., 20 dBm = TX_POWER_5, ..., 10 dBm = TX_POWER_10
        // The if condition is then "less than" to check if the power is greater than 20 dBm
        if( mibReq.Param.ChannelsTxPower < TX_POWER_5 )
        {
            mibReq.Param.ChannelsTxPower = TX_POWER_5;
            LoRaMacMibSetRequestConfirm( &mibReq );
        }
    }
#endif

    static uint8_t TxGpsData = 1; // GPS data transmission control
    uint64_t *data = (uint64_t *)AppDataBuffer;

    AppData.Port = LORAWAN_APP_PORT;    
    
    
    data[0] = gCurrentTime;
    AppData.BufferSize = sizeof(uint64_t);

    if( LmHandlerSend( &AppData, LmHandlerParams.IsTxConfirmed ) == LORAMAC_HANDLER_SUCCESS )
    {
        LabscimSignalEmitDouble(gUpstreamPacketGeneratedSignal,(double)(gCurrentTime)/1e6);        
        // Switch LED 1 ON
        GpioWrite( &Led1, 0 );
        TimerStart( &Led1Timer );
    }
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
            TimerSetValue( &TxTimer, TxPeriodicity );
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
    uint8_t isPending = 0;
    CRITICAL_SECTION_BEGIN( );
    isPending = IsTxFramePending;
    IsTxFramePending = 0;
    CRITICAL_SECTION_END( );
    if( isPending == 1 )
    {
        PrepareTxFrame( );
    }
}

static void OnTxPeriodicityChanged( uint32_t periodicity )
{
    TxPeriodicity = periodicity;

    if( TxPeriodicity == 0 )
    { // Revert to application default periodicity
        TxPeriodicity = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
    }

    // Update timer periodicity
    TimerStop( &TxTimer );
    TimerSetValue( &TxTimer, TxPeriodicity );
    TimerStart( &TxTimer );
}

static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed )
{
    LmHandlerParams.IsTxConfirmed = isTxConfirmed;
}

static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity )
{
    LmHandlerParams.PingSlotPeriodicity = pingSlotPeriodicity;
}

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent( void* context )
{
    TimerStop( &TxTimer );

    IsTxFramePending = 1;

    // Schedule next transmission
    TimerSetValue( &TxTimer, TxPeriodicity );
    TimerStart( &TxTimer );
}

/*!
 * Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent( void* context )
{
    TimerStop( &Led1Timer );
    // Switch LED 1 OFF
    GpioWrite( &Led1, 1 );
}

/*!
 * Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent( void* context )
{
    TimerStop( &Led2Timer );
    // Switch LED 2 OFF
    GpioWrite( &Led2, 1 );
}

/*!
 * \brief Function executed on Beacon timer Timeout event
 */
static void OnLedBeaconTimerEvent( void* context )
{
    GpioWrite( &Led2, 0 );
    TimerStart( &Led2Timer );

    TimerStart( &LedBeaconTimer );
}

void labscim_signal_arrived(struct labscim_signal* sig)
{
	if (sig->signal_id == gPacketReceivedSignal)
	{
		struct signal_info* si = (struct signal_info*)(sig->signal);		
		if (si->signature == gSignature)
		{
			LabscimSignalEmitDouble(gUpstreamPacketLatencySignal, si->latency);                        
			LabscimSignalEmitDouble(gUpstreamAoIMinSignal, si->aoi_min);
			LabscimSignalEmitDouble(gUpstreamAoIMaxSignal, si->aoi_max);
			LabscimSignalEmitDouble(gAoIAreaSignal, si->aoi_area);
		}
	} else if (sig->signal_id == gDownstreamPacketSentOnGateway)
	{
        //*
		struct packet_generated_info* pg = (struct packet_generated_info*)(sig->signal);		
		if (pg->signature == gSignature)
		{
			LabscimSignalEmitDouble(gDownstreamPacketSentToMe, pg->labscim_time);                        			
		}
        //*/
        /*
		struct signal_info* si = (struct signal_info*)(sig->signal);		
		if (si->signature == gSignature)
		{
			LabscimSignalEmitDouble(gDownstreamPacketSentToMe, si->error);  //error is just a value. Might be anything the gateway sent.
		}
        //*/

	}
	free(sig);
}