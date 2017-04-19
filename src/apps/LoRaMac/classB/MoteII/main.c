/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: LoRaMac classB device implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

#include <string.h>
#include <math.h>
#include "board.h"

#include "LoRaMac.h"
#include "Commissioning.h"

#include "buttons.h"
#include "screen.h"

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            5000

/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND                        1000

/*!
 * Default datarate
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_0

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG_ON                    false

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON                              1

#if defined( USE_BAND_868 )

#include "LoRaMacTest.h"

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                        true

#define USE_SEMTECH_DEFAULT_CHANNEL_LINEUP          1

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )

#define LC4                { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10               { 868300000, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

#endif

#endif

/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT                            2

/*!
 * User application data buffer size
 */
#if defined( USE_BAND_868 )

#define LORAWAN_APP_DATA_SIZE                       16

#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )

#define LORAWAN_APP_DATA_SIZE                       11

#endif

static uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

#if( OVER_THE_AIR_ACTIVATION == 0 )

static uint8_t NwkSKey[] = LORAWAN_NWKSKEY;
static uint8_t AppSKey[] = LORAWAN_APPSKEY;

#endif

/*!
 * Application port
 */
static uint8_t AppPort = LORAWAN_APP_PORT;

/*!
 * User application data size
 */
static uint8_t AppDataSize = LORAWAN_APP_DATA_SIZE;

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_MAX_SIZE                           64

/*!
 * User application data
 */
static uint8_t AppData[LORAWAN_APP_DATA_MAX_SIZE];

/*!
 * Indicates if the node is sending confirmed or unconfirmed messages
 */
static uint8_t IsTxConfirmed = false;

/*!
 * Defines the application data transmission duty cycle
 */
static uint32_t TxDutyCycleTime;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxNextPacketTimer;

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;

/*!
 * Timer to handle the state of LED1
 */
static TimerEvent_t Led1Timer;

/*!
 * Timer to handle the state of LED2
 */
static TimerEvent_t Led2Timer;

/*!
 * Indicates if a new packet can be sent
 */
static bool NextTx = true;

/*
 * Application Settings and Status
 */

static AppSettings_t AppSettings;

/*!
 * Indicates if the MAC layer network join status has changed.
 */
static bool IsNetworkJoinedStatusUpdate = false;

LoRaMacUplinkStatus_t LoRaMacUplinkStatus;

LoRaMacDownlinkStatus_t LoRaMacDownlinkStatus;

/*!
 * Device states
 */
static enum eDeviceState
{
    DEVICE_STATE_INIT,
    DEVICE_STATE_JOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_SLEEP
}DeviceState;

/*!
 * LoRaWAN compliance tests support data
 */
struct ComplianceTest_s
{
    bool Running;
    uint8_t State;
    bool IsTxConfirmed;
    uint8_t AppPort;
    uint8_t AppDataSize;
    uint8_t *AppDataBuffer;
    uint16_t DownLinkCounter;
    bool LinkCheck;
    uint8_t DemodMargin;
    uint8_t NbGateways;
}ComplianceTest;

/*!
 * \brief   Prepares the payload of the frame
 */
static void PrepareTxFrame( uint8_t port )
{
    switch( port )
    {
    case 2:
        {
#if defined( USE_BAND_868 )
            LoRaMacUplinkStatus.Pressure = ( uint16_t )( MPL3115ReadPressure( ) / 10 );             // in hPa / 10
            LoRaMacUplinkStatus.Temperature = ( int16_t )( MPL3115ReadTemperature( ) * 100 );       // in °C * 100
            LoRaMacUplinkStatus.AltitudeBar = ( int16_t )( MPL3115ReadAltitude( ) * 10 );           // in m * 10
            LoRaMacUplinkStatus.BatteryLevel = BoardGetBatteryLevel( );                             // 1 (very low) to 254 (fully charged)
            GpsGetLatestGpsPositionBinary( &LoRaMacUplinkStatus.Latitude, &LoRaMacUplinkStatus.Longitude );
            LoRaMacUplinkStatus.AltitudeGps = GpsGetLatestGpsAltitude( );                           // in m
            LoRaMacUplinkStatus.GpsHaxFix = GpsHasFix( );

            AppData[0] = AppLedStateOn;
            AppData[1] = ( LoRaMacUplinkStatus.Pressure >> 8 ) & 0xFF;
            AppData[2] = LoRaMacUplinkStatus.Pressure & 0xFF;
            AppData[3] = ( LoRaMacUplinkStatus.Temperature >> 8 ) & 0xFF;
            AppData[4] = LoRaMacUplinkStatus.Temperature & 0xFF;
            AppData[5] = ( LoRaMacUplinkStatus.AltitudeBar >> 8 ) & 0xFF;
            AppData[6] = LoRaMacUplinkStatus.AltitudeBar & 0xFF;
            AppData[7] = LoRaMacUplinkStatus.BatteryLevel;
            AppData[8] = ( LoRaMacUplinkStatus.Latitude >> 16 ) & 0xFF;
            AppData[9] = ( LoRaMacUplinkStatus.Latitude >> 8 ) & 0xFF;
            AppData[10] = LoRaMacUplinkStatus.Latitude & 0xFF;
            AppData[11] = ( LoRaMacUplinkStatus.Longitude >> 16 ) & 0xFF;
            AppData[12] = ( LoRaMacUplinkStatus.Longitude >> 8 ) & 0xFF;
            AppData[13] = LoRaMacUplinkStatus.Longitude & 0xFF;
            AppData[14] = ( LoRaMacUplinkStatus.AltitudeGps >> 8 ) & 0xFF;
            AppData[15] = LoRaMacUplinkStatus.AltitudeGps & 0xFF;
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
            LoRaMacUplinkStatus.Temperature = ( int16_t )( MPL3115ReadTemperature( ) * 100 );       // in °C * 100

            LoRaMacUplinkStatus.BatteryLevel = BoardGetBatteryLevel( );                             // 1 (very low) to 254 (fully charged)
            GpsGetLatestGpsPositionBinary( &LoRaMacUplinkStatus.Latitude, &LoRaMacUplinkStatus.Longitude );
            LoRaMacUplinkStatus.AltitudeGps = GpsGetLatestGpsAltitude( );                           // in m
            LoRaMacUplinkStatus.GpsHaxFix = GpsHasFix( );

            AppData[0] = AppLedStateOn;
            AppData[1] = LoRaMacUplinkStatus.Temperature;                                           // Signed degrees celsius in half degree units. So,  +/-63 C
            AppData[2] = LoRaMacUplinkStatus.BatteryLevel;                                          // Per LoRaWAN spec; 0=Charging; 1...254 = level, 255 = N/A
            AppData[3] = ( LoRaMacUplinkStatus.Latitude >> 16 ) & 0xFF;
            AppData[4] = ( LoRaMacUplinkStatus.Latitude >> 8 ) & 0xFF;
            AppData[5] = LoRaMacUplinkStatus.Latitude & 0xFF;
            AppData[6] = ( LoRaMacUplinkStatus.Longitude >> 16 ) & 0xFF;
            AppData[7] = ( LoRaMacUplinkStatus.Longitude >> 8 ) & 0xFF;
            AppData[8] = LoRaMacUplinkStatus.Longitude & 0xFF;
            AppData[9] = ( LoRaMacUplinkStatus.AltitudeGps >> 8 ) & 0xFF;
            AppData[10] = LoRaMacUplinkStatus.AltitudeGps & 0xFF;
#endif
        }
        break;
    case 224:
        if( ComplianceTest.LinkCheck == true )
        {
            ComplianceTest.LinkCheck = false;
            AppDataSize = 3;
            AppData[0] = 5;
            AppData[1] = ComplianceTest.DemodMargin;
            AppData[2] = ComplianceTest.NbGateways;
            ComplianceTest.State = 1;
        }
        else
        {
            switch( ComplianceTest.State )
            {
            case 4:
                ComplianceTest.State = 1;
                break;
            case 1:
                AppDataSize = 2;
                AppData[0] = ComplianceTest.DownLinkCounter >> 8;
                AppData[1] = ComplianceTest.DownLinkCounter;
                break;
            }
        }
        break;
    default:
        break;
    }
}

/*!
 * \brief   Prepares the payload of the frame
 *
 * \retval  [0: frame could be send, 1: error]
 */
static bool SendFrame( void )
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    LoRaMacUplinkStatus.Acked = false;
    LoRaMacUplinkStatus.NbTrials = 0;

    if( LoRaMacQueryTxPossible( AppDataSize, &txInfo ) != LORAMAC_STATUS_OK )
    {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;

        LoRaMacUplinkStatus.Confirmed = false;
        LoRaMacUplinkStatus.Port = 0;
        LoRaMacUplinkStatus.Buffer = NULL;
        LoRaMacUplinkStatus.BufferSize = 0;
    }
    else
    {
        LoRaMacUplinkStatus.Confirmed = IsTxConfirmed;
        LoRaMacUplinkStatus.Acked = false;
        LoRaMacUplinkStatus.Port = AppPort;
        LoRaMacUplinkStatus.Buffer = AppData;
        LoRaMacUplinkStatus.BufferSize = AppDataSize;

        if( IsTxConfirmed == false )
        {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = AppPort;
            mcpsReq.Req.Unconfirmed.fBuffer = AppData;
            mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
        else
        {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = AppPort;
            mcpsReq.Req.Confirmed.fBuffer = AppData;
            mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Confirmed.NbTrials = 8;
            mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
    }

    if( LoRaMacMcpsRequest( &mcpsReq ) == LORAMAC_STATUS_OK )
    {
        LoRaMacUplinkStatus.UplinkInProgress = true;
        LoRaMacDownlinkStatus.DownlinkInProgress = false;
        return false;
    }
    return true;
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    TimerStop( &TxNextPacketTimer );

    mibReq.Type = MIB_NETWORK_JOINED;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK )
    {
        if( mibReq.Param.IsNetworkJoined == true )
        {
            DeviceState = DEVICE_STATE_SEND;
            NextTx = true;
        }
        else
        {
            DeviceState = DEVICE_STATE_JOIN;
        }
    }
}

/*!
 * \brief Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent( void )
{
    TimerStop( &Led1Timer );
    // Switch LED 1 OFF
    GpioWrite( &Led1, 1 );
}

/*!
 * \brief Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent( void )
{
    TimerStop( &Led2Timer );
    // Switch LED 2 OFF
    GpioWrite( &Led2, 1 );
}

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    if( mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
    {
        switch( mcpsConfirm->McpsRequest )
        {
            case MCPS_UNCONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                break;
            }
            case MCPS_CONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                // Check AckReceived
                // Check NbTrials
                break;
            }
            case MCPS_PROPRIETARY:
            {
                break;
            }
            default:
                break;
        }

        // Switch LED 1 ON
        GpioWrite( &Led1, 0 );
        TimerStart( &Led1Timer );
    }

    LoRaMacUplinkStatus.Acked = mcpsConfirm->AckReceived;
    LoRaMacUplinkStatus.NbTrials = mcpsConfirm->NbRetries;
    LoRaMacUplinkStatus.Datarate = mcpsConfirm->Datarate;
    LoRaMacUplinkStatus.TxPower = mcpsConfirm->TxPower;
    LoRaMacUplinkStatus.UplinkCounter = mcpsConfirm->UpLinkCounter;

    LoRaMacUplinkStatus.StatusUpdated = true;
    LoRaMacUplinkStatus.UplinkInProgress = false;
    LoRaMacDownlinkStatus.DownlinkInProgress = false;
    NextTx = true;
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        return;
    }

    switch( mcpsIndication->McpsIndication )
    {
        case MCPS_UNCONFIRMED:
        {
            break;
        }
        case MCPS_CONFIRMED:
        {
            break;
        }
        case MCPS_PROPRIETARY:
        {
            break;
        }
        case MCPS_MULTICAST:
        {
            break;
        }
        default:
            break;
    }

    // Check Multicast
    // Check Port
    // Check Datarate
    // Check FramePending
    // Check Buffer
    // Check BufferSize
    // Check Rssi
    // Check Snr
    // Check RxSlot
    LoRaMacDownlinkStatus.DownlinkInProgress = true;
    LoRaMacDownlinkStatus.RxSlot = mcpsIndication->RxSlot;
    LoRaMacDownlinkStatus.Rssi = mcpsIndication->Rssi;
    if( mcpsIndication->Snr & 0x80 ) // The SNR sign bit is 1
    {
        // Invert and divide by 4
        LoRaMacDownlinkStatus.Snr = ( ( ~mcpsIndication->Snr + 1 ) & 0xFF ) >> 2;
        LoRaMacDownlinkStatus.Snr = -LoRaMacDownlinkStatus.Snr;
    }
    else
    {
        // Divide by 4
        LoRaMacDownlinkStatus.Snr = ( mcpsIndication->Snr & 0xFF ) >> 2;
    }
    LoRaMacDownlinkStatus.DownlinkCounter++;
    LoRaMacDownlinkStatus.RxData = mcpsIndication->RxData;
    LoRaMacDownlinkStatus.Port = mcpsIndication->Port;
    LoRaMacDownlinkStatus.Buffer = mcpsIndication->Buffer;
    LoRaMacDownlinkStatus.BufferSize = mcpsIndication->BufferSize;

    if( ComplianceTest.Running == true )
    {
        ComplianceTest.DownLinkCounter++;
    }

    if( mcpsIndication->RxData == true )
    {
        switch( mcpsIndication->Port )
        {
        case 1: // The application LED can be controlled on port 1 or 2
        case 2:
            if( mcpsIndication->BufferSize == 1 )
            {
                AppLedStateOn = mcpsIndication->Buffer[0] & 0x01;
                GpioWrite( &Led3, ( ( AppLedStateOn & 0x01 ) != 0 ) ? 0 : 1 );
            }
            break;
        case 3:
            ScreenSetCurrent( SCREEN_4 );
            break;
        case 224:
            if( ComplianceTest.Running == false )
            {
                // Check compliance test enable command (i)
                if( ( mcpsIndication->BufferSize == 4 ) &&
                    ( mcpsIndication->Buffer[0] == 0x01 ) &&
                    ( mcpsIndication->Buffer[1] == 0x01 ) &&
                    ( mcpsIndication->Buffer[2] == 0x01 ) &&
                    ( mcpsIndication->Buffer[3] == 0x01 ) )
                {
                    IsTxConfirmed = false;
                    AppPort = 224;
                    AppDataSize = 2;
                    ComplianceTest.DownLinkCounter = 0;
                    ComplianceTest.LinkCheck = false;
                    ComplianceTest.DemodMargin = 0;
                    ComplianceTest.NbGateways = 0;
                    ComplianceTest.Running = true;
                    ComplianceTest.State = 1;

                    MibRequestConfirm_t mibReq;
                    mibReq.Type = MIB_ADR;
                    mibReq.Param.AdrEnable = true;
                    LoRaMacMibSetRequestConfirm( &mibReq );

#if defined( USE_BAND_868 )
                    LoRaMacTestSetDutyCycleOn( false );
#endif
                    GpsStop( );
                }
            }
            else
            {
                ComplianceTest.State = mcpsIndication->Buffer[0];
                switch( ComplianceTest.State )
                {
                case 0: // Check compliance test disable command (ii)
                    IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
                    AppPort = LORAWAN_APP_PORT;
                    AppDataSize = LORAWAN_APP_DATA_SIZE;
                    ComplianceTest.DownLinkCounter = 0;
                    ComplianceTest.Running = false;

                    MibRequestConfirm_t mibReq;
                    mibReq.Type = MIB_ADR;
                    mibReq.Param.AdrEnable = AppSettings.AdrOn;
                    LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( USE_BAND_868 )
                    LoRaMacTestSetDutyCycleOn( AppSettings.DutyCycleOn );
#endif
                    GpsStart( );
                    break;
                case 1: // (iii, iv)
                    AppDataSize = 2;
                    break;
                case 2: // Enable confirmed messages (v)
                    IsTxConfirmed = true;
                    ComplianceTest.State = 1;
                    break;
                case 3:  // Disable confirmed messages (vi)
                    IsTxConfirmed = false;
                    ComplianceTest.State = 1;
                    break;
                case 4: // (vii)
                    AppDataSize = mcpsIndication->BufferSize;

                    AppData[0] = 4;
                    for( uint8_t i = 1; i < AppDataSize; i++ )
                    {
                        AppData[i] = mcpsIndication->Buffer[i] + 1;
                    }
                    break;
                case 5: // (viii)
                    {
                        MlmeReq_t mlmeReq;
                        mlmeReq.Type = MLME_LINK_CHECK;
                        LoRaMacMlmeRequest( &mlmeReq );
                    }
                    break;
                case 6: // (ix)
                    {
                        MlmeReq_t mlmeReq;

                        // Disable TestMode and revert back to normal operation
                        IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
                        AppPort = LORAWAN_APP_PORT;
                        AppDataSize = LORAWAN_APP_DATA_SIZE;
                        ComplianceTest.DownLinkCounter = 0;
                        ComplianceTest.Running = false;

                        MibRequestConfirm_t mibReq;
                        mibReq.Type = MIB_ADR;
                        mibReq.Param.AdrEnable = AppSettings.AdrOn;
                        LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( USE_BAND_868 )
                        LoRaMacTestSetDutyCycleOn( AppSettings.DutyCycleOn );
#endif
                        GpsStart( );

                        mlmeReq.Type = MLME_JOIN;

                        mlmeReq.Req.Join.DevEui = AppSettings.DevEui;
                        mlmeReq.Req.Join.AppEui = AppSettings.AppEui;
                        mlmeReq.Req.Join.AppKey = AppSettings.AppKey;
                        mlmeReq.Req.Join.NbTrials = 3;

                        LoRaMacMlmeRequest( &mlmeReq );
                        DeviceState = DEVICE_STATE_SLEEP;
                    }
                    break;
                case 7: // (x)
                    {
                        if( mcpsIndication->BufferSize == 3 )
                        {
                            MlmeReq_t mlmeReq;
                            mlmeReq.Type = MLME_TXCW;
                            mlmeReq.Req.TxCw.Timeout = ( uint16_t )( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
                            LoRaMacMlmeRequest( &mlmeReq );
                        }
                        else if( mcpsIndication->BufferSize == 7 )
                        {
                            MlmeReq_t mlmeReq;
                            mlmeReq.Type = MLME_TXCW_1;
                            mlmeReq.Req.TxCw.Timeout = ( uint16_t )( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
                            mlmeReq.Req.TxCw.Frequency = ( uint32_t )( ( mcpsIndication->Buffer[3] << 16 ) | ( mcpsIndication->Buffer[4] << 8 ) | mcpsIndication->Buffer[5] ) * 100;
                            mlmeReq.Req.TxCw.Power = mcpsIndication->Buffer[6];
                            LoRaMacMlmeRequest( &mlmeReq );
                        }
                        ComplianceTest.State = 1;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            break;
        }
    }

    // Switch LED 2 ON for each received downlink
    GpioWrite( &Led2, 0 );
    TimerStart( &Led2Timer );
    LoRaMacUplinkStatus.StatusUpdated = true;
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    switch( mlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                // Status is OK, node has joined the network
                IsNetworkJoinedStatusUpdate = true;
                DeviceState = DEVICE_STATE_SEND;
            }
            else
            {
                // Join was not successful. Try to join again
                DeviceState = DEVICE_STATE_JOIN;
            }
            break;
        }
        case MLME_LINK_CHECK:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                // Check DemodMargin
                // Check NbGateways
                if( ComplianceTest.Running == true )
                {
                    ComplianceTest.LinkCheck = true;
                    ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
                    ComplianceTest.NbGateways = mlmeConfirm->NbGateways;
                }
            }
            break;
        }
        default:
            break;
    }
    NextTx = true;
    LoRaMacUplinkStatus.StatusUpdated = true;
}

/**
 * Main application entry point.
 */
int main( void )
{
    LoRaMacPrimitives_t LoRaMacPrimitives;
    LoRaMacCallback_t LoRaMacCallbacks;
    MibRequestConfirm_t mibReq;

    BoardInitMcu( );
    BoardInitPeriph( );

    AppSettings.DevEui = ( uint8_t* )DevEui;
    AppSettings.AppEui = ( uint8_t* )AppEui;
    AppSettings.AppKey = ( uint8_t* )AppKey;
#if( OVER_THE_AIR_ACTIVATION == 0 )
    AppSettings.NwkSKey = ( uint8_t* )NwkSKey;
    AppSettings.AppSKey = ( uint8_t* )AppSKey;
    AppSettings.DevAddr = LORAWAN_DEVICE_ADDRESS;
#else
    AppSettings.NwkSKey = NULL;
    AppSettings.AppSKey = NULL;
    AppSettings.DevAddr = 0;
#endif
    AppSettings.Otaa = OVER_THE_AIR_ACTIVATION;
    IsTxConfirmed = AppSettings.IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
#if defined( USE_BAND_868 )
    AppSettings.DutyCycleOn = LORAWAN_DUTYCYCLE_ON;
#endif
    AppSettings.DeviceClass = CLASS_B;

    LoRaMacUplinkStatus.Acked = false;
    LoRaMacUplinkStatus.NbTrials = 0;
    LoRaMacUplinkStatus.Datarate = LORAWAN_DEFAULT_DATARATE;
    LoRaMacUplinkStatus.TxPower = LORAMAC_DEFAULT_TX_POWER;
    LoRaMacUplinkStatus.UplinkCounter = 0;

    LoRaMacDownlinkStatus.DownlinkCounter = 0;
    LoRaMacDownlinkStatus.Rssi = -130;
    LoRaMacDownlinkStatus.Snr = 0;

    LoRaMacUplinkStatus.StatusUpdated = true;
    LoRaMacUplinkStatus.UplinkInProgress = false;
    LoRaMacDownlinkStatus.DownlinkInProgress = false;

    ButtonsInit( );
    ScreenInit( );

    DeviceState = DEVICE_STATE_INIT;

    while( 1 )
    {
        ScreenProcess( &AppSettings, &LoRaMacUplinkStatus, &LoRaMacDownlinkStatus );

        switch( DeviceState )
        {
            case DEVICE_STATE_INIT:
            {
                LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
                LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
                LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
                LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
                LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks );

                TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );

                TimerInit( &Led1Timer, OnLed1TimerEvent );
                TimerSetValue( &Led1Timer, 25 );

                TimerInit( &Led2Timer, OnLed2TimerEvent );
                TimerSetValue( &Led2Timer, 25 );

                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = AppSettings.AdrOn = LORAWAN_ADR_ON;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_PUBLIC_NETWORK;
                mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
                LoRaMacMibSetRequestConfirm( &mibReq );

#if defined( USE_BAND_868 )
                LoRaMacTestSetDutyCycleOn( AppSettings.DutyCycleOn );

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )
                LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4 );
                LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5 );
                LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6 );
                LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7 );
                LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8 );
                LoRaMacChannelAdd( 8, ( ChannelParams_t )LC9 );
                LoRaMacChannelAdd( 9, ( ChannelParams_t )LC10 );

                mibReq.Type = MIB_RX2_DEFAULT_CHANNEL;
                mibReq.Param.Rx2DefaultChannel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_RX2_CHANNEL;
                mibReq.Param.Rx2Channel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
                LoRaMacMibSetRequestConfirm( &mibReq );
#endif

#endif
                LoRaMacDownlinkStatus.DownlinkCounter = 0;
                LoRaMacDownlinkStatus.Rssi = -130;
                LoRaMacDownlinkStatus.Snr = 0;

                DeviceState = DEVICE_STATE_JOIN;
                break;
            }
            case DEVICE_STATE_JOIN:
            {
                // Initialize LoRaMac device unique ID
                BoardGetUniqueId( AppSettings.DevEui );

#if( OVER_THE_AIR_ACTIVATION != 0 )
                MlmeReq_t mlmeReq;

                mlmeReq.Type = MLME_JOIN;

                mlmeReq.Req.Join.DevEui = AppSettings.DevEui;
                mlmeReq.Req.Join.AppEui = AppSettings.AppEui;
                mlmeReq.Req.Join.AppKey = AppSettings.AppKey;
                mlmeReq.Req.Join.NbTrials = 3;

                if( NextTx == true )
                {
                    LoRaMacMlmeRequest( &mlmeReq );
                }
                DeviceState = DEVICE_STATE_SLEEP;
#else
                // Choose a random device address if not already defined in Commissioning.h
                if( AppSettings.DevAddr == 0 )
                {
                    // Random seed initialization
                    srand1( BoardGetRandomSeed( ) );

                    // Choose a random device address
                    AppSettings.DevAddr = randr( 0, 0x01FFFFFF );
                }

                mibReq.Type = MIB_NET_ID;
                mibReq.Param.NetID = LORAWAN_NETWORK_ID;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_DEV_ADDR;
                mibReq.Param.DevAddr = AppSettings.DevAddr;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NWK_SKEY;
                mibReq.Param.NwkSKey = AppSettings.NwkSKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_APP_SKEY;
                mibReq.Param.AppSKey = AppSettings.AppSKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NETWORK_JOINED;
                mibReq.Param.IsNetworkJoined = true;
                LoRaMacMibSetRequestConfirm( &mibReq );

                DeviceState = DEVICE_STATE_SEND;
#endif
                ScreenSetCurrent( SCREEN_JOIN );
                IsNetworkJoinedStatusUpdate = true;
                break;
            }
            case DEVICE_STATE_SEND:
            {
                if( NextTx == true )
                {
                    PrepareTxFrame( AppPort );

                    NextTx = SendFrame( );
                }
                if( ComplianceTest.Running == true )
                {
                    // Schedule next packet transmission
                    TxDutyCycleTime = 5000; // 5000 ms
                }
                else
                {
                    // Schedule next packet transmission
                    TxDutyCycleTime = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
                }
                DeviceState = DEVICE_STATE_CYCLE;
                break;
            }
            case DEVICE_STATE_CYCLE:
            {
                DeviceState = DEVICE_STATE_SLEEP;

                // Schedule next packet transmission
                TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
                TimerStart( &TxNextPacketTimer );
                break;
            }
            case DEVICE_STATE_SLEEP:
            {
                // Wake up through events
                TimerLowPowerHandler( );

                if( IsNetworkJoinedStatusUpdate == true )
                {
                    IsNetworkJoinedStatusUpdate = false;
                    mibReq.Type = MIB_NETWORK_JOINED;
                    LoRaMacMibGetRequestConfirm( &mibReq );
                    if( mibReq.Param.IsNetworkJoined == true )
                    {
                        ScreenSetCurrent( ( Screen_t )( ScreenGetCurrent( ) + 1 ) );
                    }
                    else
                    {
                        ScreenSetCurrent( SCREEN_JOIN );
                    }
                }
                GpsProcess( );
                break;
            }
            default:
            {
                DeviceState = DEVICE_STATE_INIT;
                break;
            }
        }
    }
}
