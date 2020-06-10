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
#include "utilities.h"
#include "timer.h"
#include "LoRaMac.h"
#include "LoRaMacTest.h"
#include "Region.h"
#include "LmhPackage.h"
#include "LmhpCompliance.h"

/*!
 * LoRaWAN compliance certification protocol port number.
 *
 * LoRaWAN Specification V1.x.x, chapter 4.3.2
 */
#define COMPLIANCE_PORT 224

#define COMPLIANCE_ID 6
#define COMPLIANCE_VERSION 1

/*!
 * LoRaWAN compliance tests support data
 */
typedef struct ComplianceTestState_s
{
    bool     Initialized;
    bool     IsTxConfirmed;
    uint8_t  DataBufferMaxSize;
    uint8_t  DataBufferSize;
    uint8_t* DataBuffer;
    uint16_t RxAppCnt;
} ComplianceTestState_t;

typedef enum ComplianceMoteCmd_e
{
    COMPLIANCE_PKG_VERSION_ANS  = 0x00,
    COMPLIANCE_ECHO_PAYLOAD_ANS = 0x08,
    COMPLIANCE_RX_APP_CNT_ANS   = 0x09,
    COMPLIANCE_DUT_VERSION_ANS  = 0x7F,
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
    COMPLIANCE_TX_CW_REQ                    = 0x7D,
    COMPLIANCE_DUT_FPORT_224_DISABLE_REQ    = 0x7E,
    COMPLIANCE_DUT_VERSION_REQ              = 0x7F,
} ComplianceSrvCmd_t;

/*!
 * Holds the compliance test current context
 */
static ComplianceTestState_t ComplianceTestState = {
    .Initialized       = false,
    .IsTxConfirmed     = false,
    .DataBufferMaxSize = 0,
    .DataBufferSize    = 0,
    .DataBuffer        = NULL,
    .RxAppCnt          = 0,
};

/*!
 * LoRaWAN compliance tests protocol handler parameters
 */
static LmhpComplianceParams_t* ComplianceParams;

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
 * Processes the LoRaMac Compliance events.
 */
static void LmhpComplianceProcess( void );

/*!
 * Processes the MCPS Indication
 *
 * \param [IN] mcpsIndication     MCPS indication primitive data
 */
static void LmhpComplianceOnMcpsIndication( McpsIndication_t* mcpsIndication );

LmhPackage_t CompliancePackage = {
    .Port                    = COMPLIANCE_PORT,
    .Init                    = LmhpComplianceInit,
    .IsInitialized           = LmhpComplianceIsInitialized,
    .Process                 = LmhpComplianceProcess,
    .OnMcpsConfirmProcess    = NULL,  // Not used in this package
    .OnMcpsIndicationProcess = LmhpComplianceOnMcpsIndication,
    .OnMlmeConfirmProcess    = NULL,  // Not used in this package
    .OnMlmeIndicationProcess = NULL,  // Not used in this package
    .OnMacMcpsRequest        = NULL,  // To be initialized by LmHandler
    .OnMacMlmeRequest        = NULL,  // To be initialized by LmHandler
    .OnJoinRequest           = NULL,  // To be initialized by LmHandler
    .OnSendRequest           = NULL,  // To be initialized by LmHandler
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
        ComplianceTestState.RxAppCnt          = 0;
    }
    else
    {
        ComplianceParams                = NULL;
        ComplianceTestState.Initialized = false;
        ComplianceTestState.RxAppCnt    = 0;
    }
}

static bool LmhpComplianceIsInitialized( void )
{
    return ComplianceTestState.Initialized;
}

static void LmhpComplianceProcess( void )
{
}

static void LmhpComplianceOnMcpsIndication( McpsIndication_t* mcpsIndication )
{
    uint8_t cmdIndex        = 0;
    uint8_t dataBufferIndex = 0;

    if( ComplianceTestState.Initialized == false )
    {
        return;
    }

    if( ComplianceParams->IsDutFPort224On == false )
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

    switch( mcpsIndication->Buffer[cmdIndex++] )
    {
        case COMPLIANCE_PKG_VERSION_REQ:
        {
            ComplianceTestState.DataBuffer[dataBufferIndex++] = COMPLIANCE_PKG_VERSION_ANS;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = COMPLIANCE_ID;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = COMPLIANCE_VERSION;
            break;
        }
        case COMPLIANCE_DUT_RESET_REQ:
        {
            // TODO: Call platform MCU reset API
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
                ComplianceTestState.IsTxConfirmed = ( frameType != 1 ) ? true : false;

                ComplianceParams->OnTxFrameCtrlChanged( ComplianceTestState.IsTxConfirmed );
            }
            break;
        }
        case COMPLIANCE_ECHO_PAYLOAD_REQ:
        {
            ComplianceTestState.DataBuffer[dataBufferIndex++] = COMPLIANCE_ECHO_PAYLOAD_ANS;
            ComplianceTestState.DataBufferSize                = mcpsIndication->BufferSize;

            for( uint8_t i = 1; i < MIN( ComplianceTestState.DataBufferSize, ComplianceTestState.DataBufferMaxSize );
                 i++ )
            {
                ComplianceTestState.DataBuffer[dataBufferIndex++] = mcpsIndication->Buffer[cmdIndex++] + 1;
            }
            break;
        }
        case COMPLIANCE_RX_APP_CNT_REQ:
        {
            ComplianceTestState.DataBuffer[dataBufferIndex++] = COMPLIANCE_RX_APP_CNT_ANS;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = ComplianceTestState.RxAppCnt >> 8;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = ComplianceTestState.RxAppCnt;
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
            MlmeReq_t mlmeReq;
            mlmeReq.Type = MLME_DEVICE_TIME;

            CompliancePackage.OnMacMlmeRequest( LoRaMacMlmeRequest( &mlmeReq ), &mlmeReq,
                                                mlmeReq.ReqReturn.DutyCycleWaitTime );
            break;
        }
        case COMPLIANCE_PING_SLOT_INFO_REQ:
        {
            MlmeReq_t mlmeReq;
            mlmeReq.Type                            = MLME_PING_SLOT_INFO;
            mlmeReq.Req.PingSlotInfo.PingSlot.Value = mcpsIndication->Buffer[cmdIndex++];

            CompliancePackage.OnMacMlmeRequest( LoRaMacMlmeRequest( &mlmeReq ), &mlmeReq,
                                                mlmeReq.ReqReturn.DutyCycleWaitTime );
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
            ComplianceParams->IsDutFPort224On = false;
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

            ComplianceTestState.DataBuffer[dataBufferIndex++] = COMPLIANCE_DUT_VERSION_ANS;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = ComplianceParams->FwVersion.Fields.Major;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = ComplianceParams->FwVersion.Fields.Minor;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = ComplianceParams->FwVersion.Fields.Patch;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = lrwanVersion.Fields.Major;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = lrwanVersion.Fields.Minor;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = lrwanVersion.Fields.Patch;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = lrwanRpVersion.Fields.Major;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = lrwanRpVersion.Fields.Minor;
            ComplianceTestState.DataBuffer[dataBufferIndex++] = lrwanRpVersion.Fields.Patch;
            break;
        }
        default:
        {
            break;
        }
    }

    if( dataBufferIndex != 0 )
    {
        // Answer commands
        LmHandlerAppData_t appData = {
            .Buffer     = ComplianceTestState.DataBuffer,
            .BufferSize = dataBufferIndex,
            .Port       = COMPLIANCE_PORT,
        };

        CompliancePackage.OnSendRequest( &appData, ( ComplianceTestState.IsTxConfirmed == true )
                                                       ? LORAMAC_HANDLER_CONFIRMED_MSG
                                                       : LORAMAC_HANDLER_UNCONFIRMED_MSG );
    }
}
