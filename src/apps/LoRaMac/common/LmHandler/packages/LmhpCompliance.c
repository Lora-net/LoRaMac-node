/*!
 * \file      LmhpCompliance.c
 *
 * \brief     Implements the LoRa-Alliance certification protocol handling
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "NvmDataMgmt.h"
#include "LoRaMacTest.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"

/*!
 * LoRaWAN compliance certification protocol port number.
 *
 * LoRaWAN Specification V1.x.x, chapter 4.3.2
 */
#define COMPLIANCE_PORT 224

#define COMPLIANCE_ID 6
#define COMPLIANCE_VERSION 1

typedef struct ClassBStatus_s
{
    bool         IsBeaconRxOn;
    uint8_t      PingSlotPeriodicity;
    uint16_t     BeaconCnt;
    BeaconInfo_t Info;
} ClassBStatus_t;

/*!
 * LoRaWAN compliance tests support data
 */
typedef struct ComplianceTestState_s
{
    bool                Initialized;
    bool                IsTxPending;
    TimerTime_t         TxPendingTimestamp;
    LmHandlerMsgTypes_t IsTxConfirmed;
    uint8_t             DataBufferMaxSize;
    uint8_t             DataBufferSize;
    uint8_t*            DataBuffer;
    uint16_t            RxAppCnt;
    ClassBStatus_t      ClassBStatus;
    bool                IsResetCmdPending;
} ComplianceTestState_t;

typedef enum ComplianceMoteCmd_e
{
    COMPLIANCE_PKG_VERSION_ANS      = 0x00,
    COMPLIANCE_ECHO_PAYLOAD_ANS     = 0x08,
    COMPLIANCE_RX_APP_CNT_ANS       = 0x09,
    COMPLIANCE_BEACON_RX_STATUS_IND = 0x40,
    COMPLIANCE_BEACON_CNT_ANS       = 0x41,
    COMPLIANCE_DUT_VERSION_ANS      = 0x7F,
} ComplianceMoteCmd_t;

typedef enum ComplianceSrvCmd_e
{
    COMPLIANCE_PKG_VERSION_REQ              = 0x00,
    COMPLIANCE_DUT_RESET_REQ                = 0x01,
    COMPLIANCE_DUT_JOIN_REQ                 = 0x02,
    COMPLIANCE_SWITCH_CLASS_REQ             = 0x03,
    COMPLIANCE_ADR_BIT_CHANGE_REQ           = 0x04,
    COMPLIANCE_REGIONAL_DUTY_CYCLE_CTRL_REQ = 0x05,
    COMPLIANCE_TX_PERIODICITY_CHANGE_REQ    = 0x06,
    COMPLIANCE_TX_FRAMES_CTRL_REQ           = 0x07,
    COMPLIANCE_ECHO_PAYLOAD_REQ             = 0x08,
    COMPLIANCE_RX_APP_CNT_REQ               = 0x09,
    COMPLIANCE_RX_APP_CNT_RESET_REQ         = 0x0A,
    COMPLIANCE_LINK_CHECK_REQ               = 0x20,
    COMPLIANCE_DEVICE_TIME_REQ              = 0x21,
    COMPLIANCE_PING_SLOT_INFO_REQ           = 0x22,
    COMPLIANCE_BEACON_CNT_REQ               = 0x41,
    COMPLIANCE_BEACON_CNT_RESET_REQ         = 0x42,
    COMPLIANCE_TX_CW_REQ                    = 0x7D,
    COMPLIANCE_DUT_FPORT_224_DISABLE_REQ    = 0x7E,
    COMPLIANCE_DUT_VERSION_REQ              = 0x7F,
} ComplianceSrvCmd_t;

/*!
 * Holds the compliance test current context
 */
static ComplianceTestState_t ComplianceTestState = {
    .Initialized        = false,
    .IsTxPending        = false,
    .TxPendingTimestamp = 0,
    .IsTxConfirmed      = LORAMAC_HANDLER_UNCONFIRMED_MSG,
    .DataBufferMaxSize  = 0,
    .DataBufferSize     = 0,
    .DataBuffer         = NULL,
    .RxAppCnt           = 0,
    .ClassBStatus       = { 0 },
    .IsResetCmdPending  = false,
};

/*!
 * LoRaWAN compliance tests protocol handler parameters
 */
static LmhpComplianceParams_t* ComplianceParams;

/*!
 * Reset Beacon status structure
 */
static inline void ClassBStatusReset( void )
{
    memset1( ( uint8_t* ) &ComplianceTestState.ClassBStatus, 0, sizeof( ClassBStatus_t ) / sizeof( uint8_t ) );
}

/*!
 * Initializes the compliance tests with provided parameters
 *
 * \param [IN] params Structure containing the initial compliance
 *                    tests parameters.
 * \param [IN] dataBuffer        Pointer to main application buffer
 * \param [IN] dataBufferMaxSize Application buffer maximum size
 */
static void LmhpComplianceInit( void* params, uint8_t* dataBuffer, uint8_t dataBufferMaxSize );

/*!
 * Returns the current compliance certification protocol initialization status.
 *
 * \retval status Compliance certification protocol initialization status
 *                [true: Initialized, false: Not initialized]
 */
static bool LmhpComplianceIsInitialized( void );

/*!
 * Returns if a package transmission is pending or not.
 *
 * \retval status Package transmission status
 *                [true: pending, false: Not pending]
 */
static bool LmhpComplianceIsTxPending( void );

/*!
 * Processes the LoRaMac Compliance events.
 */
static void LmhpComplianceProcess( void );

/*!
 * Processes the MCPS Indication
 *
 * \param [IN] mcpsIndication     MCPS indication primitive data
 */
static void LmhpComplianceOnMcpsIndication( McpsIndication_t* mcpsIndication );

/*!
 * Processes the MLME Confirm
 *
 * \param [IN] mlmeConfirm MLME confirmation primitive data
 */
static void LmhpComplianceOnMlmeConfirm( MlmeConfirm_t *mlmeConfirm );

/*!
 * Processes the MLME Indication
 *
 * \param [IN] mlmeIndication     MLME indication primitive data
 */
static void LmhpComplianceOnMlmeIndication( MlmeIndication_t* mlmeIndication );

/*!
 * Helper function to send the BeaconRxStatusInd message
 */
static void SendBeaconRxStatusInd( void );

LmhPackage_t CompliancePackage = {
    .Port                    = COMPLIANCE_PORT,
    .Init                    = LmhpComplianceInit,
    .IsInitialized           = LmhpComplianceIsInitialized,
    .IsTxPending             = LmhpComplianceIsTxPending,
    .Process                 = LmhpComplianceProcess,
    .OnMcpsConfirmProcess    = NULL,  // Not used in this package
    .OnMcpsIndicationProcess = LmhpComplianceOnMcpsIndication,
    .OnMlmeConfirmProcess    = LmhpComplianceOnMlmeConfirm,
    .OnMlmeIndicationProcess = LmhpComplianceOnMlmeIndication,
    .OnMacMcpsRequest        = NULL,  // To be initialized by LmHandler
    .OnMacMlmeRequest        = NULL,  // To be initialized by LmHandler
    .OnJoinRequest           = NULL,  // To be initialized by LmHandler
    .OnDeviceTimeRequest     = NULL,  // To be initialized by LmHandler
    .OnSysTimeUpdate         = NULL,  // To be initialized by LmHandler
};

LmhPackage_t* LmphCompliancePackageFactory( void )
{
    return &CompliancePackage;
}

static void LmhpComplianceInit( void* params, uint8_t* dataBuffer, uint8_t dataBufferMaxSize )
{
    if( ( params != NULL ) && ( dataBuffer != NULL ) )
    {
        ComplianceParams                      = ( LmhpComplianceParams_t* ) params;
        ComplianceTestState.DataBuffer        = dataBuffer;
        ComplianceTestState.DataBufferMaxSize = dataBufferMaxSize;
        ComplianceTestState.Initialized       = true;
    }
    else
    {
        ComplianceParams                = NULL;
        ComplianceTestState.Initialized = false;
    }
    ComplianceTestState.RxAppCnt = 0;
    ClassBStatusReset( );
    ComplianceTestState.IsTxPending = false;
    ComplianceTestState.IsResetCmdPending = false;
}

static bool LmhpComplianceIsInitialized( void )
{
    return ComplianceTestState.Initialized;
}

static bool LmhpComplianceIsTxPending( void )
{
    return ComplianceTestState.IsTxPending;
}

static void LmhpComplianceProcess( void )
{
    if( ComplianceTestState.IsTxPending == true )
    {
        if( TimerGetCurrentTime( ) > ( ComplianceTestState.TxPendingTimestamp + LmHandlerGetDutyCycleWaitTime( ) ) )
        {
            ComplianceTestState.IsTxPending = false;
            ComplianceTestState.TxPendingTimestamp = TimerGetCurrentTime( );

            if( ComplianceTestState.DataBufferSize != 0 )
            {
                // Answer commands
                LmHandlerAppData_t appData = {
                    .Buffer     = ComplianceTestState.DataBuffer,
                    .BufferSize = ComplianceTestState.DataBufferSize,
                    .Port       = COMPLIANCE_PORT,
                };

                if( LmHandlerSend( &appData, ComplianceTestState.IsTxConfirmed ) != LORAMAC_HANDLER_SUCCESS )
                {
                    // try to send the message again
                    ComplianceTestState.IsTxPending = true;
                }
            }
        }
    }
    if( ComplianceTestState.IsResetCmdPending == true )
    {
        ComplianceTestState.IsResetCmdPending = false;

        // Call platform MCU reset API
        BoardResetMcu( );
    }
}

static void LmhpComplianceOnMcpsIndication( McpsIndication_t* mcpsIndication )
{
    uint8_t cmdIndex        = 0;
    MibRequestConfirm_t mibReq;

    if( ComplianceTestState.Initialized == false )
    {
        return;
    }

    // Increment the compliance certification protocol downlink counter
    // Not counting downlinks on FPort 0
    if( ( mcpsIndication->Port > 0 ) || ( mcpsIndication->AckReceived == true ) )
    {
        ComplianceTestState.RxAppCnt++;
    }

    if( mcpsIndication->RxData == false )
    {
        return;
    }

    if( mcpsIndication->Port != COMPLIANCE_PORT )
    {
        return;
    }

    ComplianceTestState.DataBufferSize = 0;

    switch( mcpsIndication->Buffer[cmdIndex++] )
    {
        case COMPLIANCE_PKG_VERSION_REQ:
        {
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = COMPLIANCE_PKG_VERSION_ANS;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = COMPLIANCE_ID;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = COMPLIANCE_VERSION;
            break;
        }
        case COMPLIANCE_DUT_RESET_REQ:
        {
            ComplianceTestState.IsResetCmdPending = true;
            break;
        }
        case COMPLIANCE_DUT_JOIN_REQ:
        {
            CompliancePackage.OnJoinRequest( true );
            break;
        }
        case COMPLIANCE_SWITCH_CLASS_REQ:
        {
            MibRequestConfirm_t mibReq;
            mibReq.Type = MIB_DEVICE_CLASS;
            // CLASS_A = 0, CLASS_B = 1, CLASS_C = 2
            mibReq.Param.Class = ( DeviceClass_t ) mcpsIndication->Buffer[cmdIndex++];

            LoRaMacMibSetRequestConfirm( &mibReq );
            break;
        }
        case COMPLIANCE_ADR_BIT_CHANGE_REQ:
        {
            MibRequestConfirm_t mibReq;
            mibReq.Type            = MIB_ADR;
            mibReq.Param.AdrEnable = mcpsIndication->Buffer[cmdIndex++];

            LoRaMacMibSetRequestConfirm( &mibReq );
            break;
        }
        case COMPLIANCE_REGIONAL_DUTY_CYCLE_CTRL_REQ:
        {
            LoRaMacTestSetDutyCycleOn( mcpsIndication->Buffer[cmdIndex++] );
            break;
        }
        case COMPLIANCE_TX_PERIODICITY_CHANGE_REQ:
        {
            // Periodicity in milli-seconds
            uint32_t periodicity[] = { 0, 5000, 10000, 20000, 30000, 40000, 50000, 60000, 120000, 240000, 480000 };
            uint8_t  index         = mcpsIndication->Buffer[cmdIndex++];

            if( index < ( sizeof( periodicity ) / sizeof( uint16_t ) ) )
            {
                if( ComplianceParams->OnTxPeriodicityChanged != NULL )
                {
                    ComplianceParams->OnTxPeriodicityChanged( periodicity[index] );
                }
            }
            break;
        }
        case COMPLIANCE_TX_FRAMES_CTRL_REQ:
        {
            uint8_t frameType = mcpsIndication->Buffer[cmdIndex++];

            if( ( frameType == 1 ) || ( frameType == 2 ) )
            {
                ComplianceTestState.IsTxConfirmed = ( frameType != 1 ) ? LORAMAC_HANDLER_CONFIRMED_MSG : LORAMAC_HANDLER_UNCONFIRMED_MSG;

                ComplianceParams->OnTxFrameCtrlChanged( ComplianceTestState.IsTxConfirmed );
            }
            break;
        }
        case COMPLIANCE_ECHO_PAYLOAD_REQ:
        {
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = COMPLIANCE_ECHO_PAYLOAD_ANS;

            for( uint8_t i = 1; i < MIN( mcpsIndication->BufferSize, ComplianceTestState.DataBufferMaxSize );
                 i++ )
            {
                ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = mcpsIndication->Buffer[cmdIndex++] + 1;
            }
            break;
        }
        case COMPLIANCE_RX_APP_CNT_REQ:
        {
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = COMPLIANCE_RX_APP_CNT_ANS;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceTestState.RxAppCnt;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceTestState.RxAppCnt >> 8;
            break;
        }
        case COMPLIANCE_RX_APP_CNT_RESET_REQ:
        {
            ComplianceTestState.RxAppCnt = 0;
            break;
        }
        case COMPLIANCE_LINK_CHECK_REQ:
        {
            MlmeReq_t mlmeReq;
            mlmeReq.Type = MLME_LINK_CHECK;

            CompliancePackage.OnMacMlmeRequest( LoRaMacMlmeRequest( &mlmeReq ), &mlmeReq,
                                                mlmeReq.ReqReturn.DutyCycleWaitTime );
            break;
        }
        case COMPLIANCE_DEVICE_TIME_REQ:
        {
            CompliancePackage.OnDeviceTimeRequest( );
            break;
        }
        case COMPLIANCE_PING_SLOT_INFO_REQ:
        {
            ComplianceTestState.ClassBStatus.PingSlotPeriodicity = mcpsIndication->Buffer[cmdIndex++];
            ComplianceParams->OnPingSlotPeriodicityChanged( ComplianceTestState.ClassBStatus.PingSlotPeriodicity );
            break;
        }
        case COMPLIANCE_BEACON_CNT_REQ:
        {
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = COMPLIANCE_BEACON_CNT_ANS;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceTestState.ClassBStatus.BeaconCnt;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceTestState.ClassBStatus.BeaconCnt >> 8;
            break;
        }
        case COMPLIANCE_BEACON_CNT_RESET_REQ:
        {
            ComplianceTestState.ClassBStatus.BeaconCnt = 0;
            break;
        }
        case COMPLIANCE_TX_CW_REQ:
        {
            MlmeReq_t mlmeReq;
            if( mcpsIndication->BufferSize == 7 )
            {
                mlmeReq.Type = MLME_TXCW;
                mlmeReq.Req.TxCw.Timeout =
                    ( uint16_t )( mcpsIndication->Buffer[cmdIndex] | ( mcpsIndication->Buffer[cmdIndex + 1] << 8 ) );
                cmdIndex += 2;
                mlmeReq.Req.TxCw.Frequency =
                    ( uint32_t )( mcpsIndication->Buffer[cmdIndex] | ( mcpsIndication->Buffer[cmdIndex + 1] << 8 ) |
                                  ( mcpsIndication->Buffer[cmdIndex + 2] << 16 ) ) *
                    100;
                cmdIndex += 3;
                mlmeReq.Req.TxCw.Power = mcpsIndication->Buffer[cmdIndex++];

                CompliancePackage.OnMacMlmeRequest( LoRaMacMlmeRequest( &mlmeReq ), &mlmeReq,
                                                    mlmeReq.ReqReturn.DutyCycleWaitTime );
            }
            break;
        }
        case COMPLIANCE_DUT_FPORT_224_DISABLE_REQ:
        {
            mibReq.Type = MIB_IS_CERT_FPORT_ON;
            mibReq.Param.IsCertPortOn = false;
            LoRaMacMibSetRequestConfirm( &mibReq );

            ComplianceTestState.IsResetCmdPending = true;
            break;
        }
        case COMPLIANCE_DUT_VERSION_REQ:
        {
            Version_t           lrwanVersion;
            Version_t           lrwanRpVersion;
            MibRequestConfirm_t mibReq;

            mibReq.Type = MIB_LORAWAN_VERSION;

            LoRaMacMibGetRequestConfirm( &mibReq );
            lrwanVersion   = mibReq.Param.LrWanVersion.LoRaWan;
            lrwanRpVersion = mibReq.Param.LrWanVersion.LoRaWanRegion;

            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = COMPLIANCE_DUT_VERSION_ANS;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceParams->FwVersion.Fields.Major;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceParams->FwVersion.Fields.Minor;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceParams->FwVersion.Fields.Patch;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceParams->FwVersion.Fields.Revision;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = lrwanVersion.Fields.Major;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = lrwanVersion.Fields.Minor;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = lrwanVersion.Fields.Patch;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = lrwanVersion.Fields.Revision;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = lrwanRpVersion.Fields.Major;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = lrwanRpVersion.Fields.Minor;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = lrwanRpVersion.Fields.Patch;
            ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = lrwanRpVersion.Fields.Revision;
            break;
        }
        default:
        {
            break;
        }
    }

    if( ComplianceTestState.DataBufferSize != 0 )
    {
        ComplianceTestState.IsTxPending = true;
    }
}

static void LmhpComplianceOnMlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    switch( mlmeConfirm->MlmeRequest )
    {
        case MLME_BEACON_ACQUISITION:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                ClassBStatusReset( );
                ComplianceTestState.ClassBStatus.IsBeaconRxOn = true;
            }
            else
            {
                ComplianceTestState.ClassBStatus.IsBeaconRxOn = false;
            }
            SendBeaconRxStatusInd( );
            break;
        }
        default:
            break;
    }
}

static void LmhpComplianceOnMlmeIndication( MlmeIndication_t* mlmeIndication )
{
    if( ComplianceTestState.Initialized == false )
    {
        return;
    }

    switch( mlmeIndication->MlmeIndication )
    {
        case MLME_BEACON_LOST:
        {
            ClassBStatusReset( );
            SendBeaconRxStatusInd( );
            break;
        }
        case MLME_BEACON:
        {
            if( mlmeIndication->Status == LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED )
            {
                // As we received a beacon ensure that IsBeaconRxOn is set to true
                if( ComplianceTestState.ClassBStatus.IsBeaconRxOn == false )
                {
                    ComplianceTestState.ClassBStatus.IsBeaconRxOn = true;
                }
                ComplianceTestState.ClassBStatus.BeaconCnt++;
            }
            ComplianceTestState.ClassBStatus.Info = mlmeIndication->BeaconInfo;
            SendBeaconRxStatusInd( );
            break;
        }
        default:
            break;
    }
}

static void SendBeaconRxStatusInd( void )
{
    uint32_t frequency = ComplianceTestState.ClassBStatus.Info.Frequency / 100;

    ComplianceTestState.DataBufferSize = 0;
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = COMPLIANCE_BEACON_RX_STATUS_IND;
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( ComplianceTestState.ClassBStatus.IsBeaconRxOn == true ) ? 1 : 0;
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.BeaconCnt );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.BeaconCnt >> 8 );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( frequency );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( frequency >> 8 );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( frequency >> 16 );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ComplianceTestState.ClassBStatus.Info.Datarate;
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.Rssi );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.Rssi >> 8 );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.Snr );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.Param );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.Time.Seconds );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.Time.Seconds >> 8 );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.Time.Seconds >> 16 );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.Time.Seconds >> 24 );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.GwSpecific.InfoDesc );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.GwSpecific.Info[0] );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.GwSpecific.Info[1] );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.GwSpecific.Info[2] );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.GwSpecific.Info[3] );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.GwSpecific.Info[4] );
    ComplianceTestState.DataBuffer[ComplianceTestState.DataBufferSize++] = ( uint8_t )( ComplianceTestState.ClassBStatus.Info.GwSpecific.Info[5] );

    ComplianceTestState.IsTxPending = true;
}
