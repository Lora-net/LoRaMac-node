/*!
 * \file      LmhpFragmentation.c
 *
 * \brief     Implements the LoRa-Alliance fragmented data block transport package
 *            Specification: https://lora-alliance.org/sites/default/files/2018-09/fragmented_data_block_transport_v1.0.0.pdf
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
#include "LmHandler.h"
#include "LmhpFragmentation.h"
#include "FragDecoder.h"

/*!
 * LoRaWAN Application Layer Fragmented Data Block Transport Specification
 */
#define FRAGMENTATION_PORT                          201

#define FRAGMENTATION_ID                            3
#define FRAGMENTATION_VERSION                       1

#define FRAGMENTATION_MAX_SESSIONS                  4

/*!
 * Package current context
 */
typedef struct LmhpFragmentationState_s
{
    bool Initialized;
    bool IsRunning;
    uint8_t DataBufferMaxSize;
    uint8_t *DataBuffer;
    uint8_t *file;
}LmhpFragmentationState_t;

typedef enum LmhpFragmentationMoteCmd_e
{
    FRAGMENTATION_PKG_VERSION_ANS         = 0x00,
    FRAGMENTATION_FRAG_STATUS_ANS         = 0x01,
    FRAGMENTATION_FRAG_SESSION_SETUP_ANS  = 0x02,
    FRAGMENTATION_FRAG_SESSION_DELETE_ANS = 0x03,
}LmhpFragmentationMoteCmd_t;

typedef enum LmhpFragmentationSrvCmd_e
{
    FRAGMENTATION_PKG_VERSION_REQ         = 0x00,
    FRAGMENTATION_FRAG_STATUS_REQ         = 0x01,
    FRAGMENTATION_FRAG_SESSION_SETUP_REQ  = 0x02,
    FRAGMENTATION_FRAG_SESSION_DELETE_REQ = 0x03,
    FRAGMENTATION_DATA_FRAGMENT           = 0x08,
}LmhpFragmentationSrvCmd_t;

/*!
 * LoRaWAN fragmented data block transport handler parameters
 */
static LmhpFragmentationParams_t* LmhpFragmentationParams;

/*!
 * Initializes the package with provided parameters
 *
 * \param [IN] params            Pointer to the package parameters
 * \param [IN] dataBuffer        Pointer to main application buffer
 * \param [IN] dataBufferMaxSize Main application buffer maximum size
 */
static void LmhpFragmentationInit( void *params, uint8_t *dataBuffer, uint8_t dataBufferMaxSize );

/*!
 * Returns the current package initialization status.
 *
 * \retval status Package initialization status
 *                [true: Initialized, false: Not initialized]
 */
static bool LmhpFragmentationIsInitialized( void );

/*!
 * Returns the package operation status.
 *
 * \retval status Package operation status
 *                [true: Running, false: Not running]
 */
static bool LmhpFragmentationIsRunning( void );

/*!
 * Processes the internal package events.
 */
static void LmhpFragmentationProcess( void );

/*!
 * Processes the MCPS Indication
 *
 * \param [IN] mcpsIndication     MCPS indication primitive data
 */
static void LmhpFragmentationOnMcpsIndication( McpsIndication_t *mcpsIndication );

static LmhpFragmentationState_t LmhpFragmentationState =
{
    .Initialized = false,
    .IsRunning = false
};

typedef struct FragGroupData_s
{
    bool IsActive;
    union
    {
        uint8_t Value;
        struct
        {
            uint8_t McGroupBitMask:   4;
            uint8_t FragIndex:        2;
            uint8_t RFU:              2;
        }Fields;
    }FragSession;
    uint16_t FragNb;
    uint8_t FragSize;
    union
    {
        uint8_t Value;
        struct
        {
            uint8_t BlockAckDelay:    3;
            uint8_t FragAlgo:         3;
            uint8_t RFU:              2;
        }Fields;
    }Control;
    uint8_t Padding;
    uint32_t Descriptor;
}FragGroupData_t;

typedef struct FragSessionData_s
{
    FragGroupData_t FragGroupData;
    FragDecoderStatus_t FragDecoderStatus;
    int32_t FragDecoderPorcessStatus;
}FragSessionData_t;

FragSessionData_t FragSessionData[FRAGMENTATION_MAX_SESSIONS];


static LmhPackage_t LmhpFragmentationPackage =
{
    .Port = FRAGMENTATION_PORT,
    .Init = LmhpFragmentationInit,
    .IsInitialized = LmhpFragmentationIsInitialized,
    .IsRunning = LmhpFragmentationIsRunning,
    .Process = LmhpFragmentationProcess,
    .OnMcpsConfirmProcess = NULL,                              // Not used in this package
    .OnMcpsIndicationProcess = LmhpFragmentationOnMcpsIndication,
    .OnMlmeConfirmProcess = NULL,                              // Not used in this package
    .OnMlmeIndicationProcess = NULL,                           // Not used in this package
    .OnMacMcpsRequest = NULL,                                  // To be initialized by LmHandler
    .OnMacMlmeRequest = NULL,                                  // To be initialized by LmHandler
    .OnJoinRequest = NULL,                                     // To be initialized by LmHandler
    .OnSendRequest = NULL,                                     // To be initialized by LmHandler
    .OnDeviceTimeRequest = NULL,                               // To be initialized by LmHandler
    .OnSysTimeUpdate = NULL,                                   // To be initialized by LmHandler
};

LmhPackage_t *LmhpFragmentationPackageFactory( void )
{
    return &LmhpFragmentationPackage;
}

static void LmhpFragmentationInit( void *params, uint8_t *dataBuffer, uint8_t dataBufferMaxSize )
{
    if( ( params != NULL ) && ( dataBuffer != NULL ) )
    {
        LmhpFragmentationParams = ( LmhpFragmentationParams_t* )params;
        LmhpFragmentationState.DataBuffer = dataBuffer;
        LmhpFragmentationState.DataBufferMaxSize = dataBufferMaxSize;
        LmhpFragmentationState.Initialized = true;
        LmhpFragmentationState.IsRunning = true;
    }
    else
    {
        LmhpFragmentationParams = NULL;
        LmhpFragmentationState.IsRunning = false;
        LmhpFragmentationState.Initialized = false;
    }
}

static bool LmhpFragmentationIsInitialized( void )
{
    return LmhpFragmentationState.Initialized;
}

static bool LmhpFragmentationIsRunning( void )
{
    if( LmhpFragmentationState.Initialized == false )
    {
        return false;
    }

    return LmhpFragmentationState.IsRunning;
}

static void LmhpFragmentationProcess( void )
{
    // TODO: Start a timer to randomly delay the answer

}

static void LmhpFragmentationOnMcpsIndication( McpsIndication_t *mcpsIndication )
{
    uint8_t cmdIndex = 0;
    uint8_t dataBufferIndex = 0;
    bool isAnswerDelayed = false;

    while( cmdIndex < mcpsIndication->BufferSize )
    {
        switch( mcpsIndication->Buffer[cmdIndex++] )
        {
            case FRAGMENTATION_PKG_VERSION_REQ:
            {
                if( mcpsIndication->Multicast == 1 )
                {
                    // Multicast channel. Don't process command.
                    break;
                }
                LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FRAGMENTATION_PKG_VERSION_ANS;
                LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FRAGMENTATION_ID;
                LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FRAGMENTATION_VERSION;
                break;
            }
            case FRAGMENTATION_FRAG_STATUS_REQ:
            {
                uint8_t fragIndex = mcpsIndication->Buffer[cmdIndex++];
                uint8_t participants = fragIndex & 0x01;

                fragIndex >>= 1;
                FragSessionData[fragIndex].FragDecoderStatus = FragDecoderGetStatus( );

                if( ( participants == 1 ) ||
                    ( ( participants == 0 ) && ( FragSessionData[fragIndex].FragDecoderStatus.FragNbLost > 0 ) ) )
                {
                    LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FRAGMENTATION_FRAG_STATUS_ANS;
                    LmhpFragmentationState.DataBuffer[dataBufferIndex++] = ( fragIndex << 14 ) |
                                                                           ( ( FragSessionData[fragIndex].FragDecoderStatus.FragNbRx >> 8 ) & 0x3F );
                    LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FragSessionData[fragIndex].FragDecoderStatus.FragNbRx & 0xFF;
                    LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FragSessionData[fragIndex].FragDecoderStatus.FragNbLost;
                    LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FragSessionData[fragIndex].FragDecoderStatus.MatrixError & 0x01;
                    isAnswerDelayed = true;
                }
                break;
            }
            case FRAGMENTATION_FRAG_SESSION_SETUP_REQ:
            {
                if( mcpsIndication->Multicast == 1 )
                {
                    // Multicast channel. Don't process command.
                    break;
                }
                FragSessionData_t fragSessionData;
                uint8_t status = 0x00;

                fragSessionData.FragGroupData.FragSession.Value = mcpsIndication->Buffer[cmdIndex++];
                
                fragSessionData.FragGroupData.FragNb =  ( mcpsIndication->Buffer[cmdIndex++] << 0 ) & 0x00FF;
                fragSessionData.FragGroupData.FragNb |= ( mcpsIndication->Buffer[cmdIndex++] << 8 ) & 0xFF00;

                fragSessionData.FragGroupData.FragSize = mcpsIndication->Buffer[cmdIndex++];

                fragSessionData.FragGroupData.Control.Value = mcpsIndication->Buffer[cmdIndex++];

                fragSessionData.FragGroupData.Padding = mcpsIndication->Buffer[cmdIndex++];

                fragSessionData.FragGroupData.Descriptor =  ( mcpsIndication->Buffer[cmdIndex++] << 0  ) & 0x000000FF;
                fragSessionData.FragGroupData.Descriptor += ( mcpsIndication->Buffer[cmdIndex++] << 8  ) & 0x0000FF00;
                fragSessionData.FragGroupData.Descriptor += ( mcpsIndication->Buffer[cmdIndex++] << 16 ) & 0x00FF0000;
                fragSessionData.FragGroupData.Descriptor += ( mcpsIndication->Buffer[cmdIndex++] << 24 ) & 0xFF000000;

                if( fragSessionData.FragGroupData.Control.Fields.FragAlgo > 0 )
                {
                    status |= 0x01; // Encoding unsupported
                }

#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
                if( ( fragSessionData.FragGroupData.FragNb * fragSessionData.FragGroupData.FragSize ) > FragDecoderGetMaxFileSize( ) )
                {
                    status |= 0x02; // Not enough Memory
                }
#else
                if( ( fragSessionData.FragGroupData.FragNb * fragSessionData.FragGroupData.FragSize ) > LmhpFragmentationParams->BufferSize )
                {
                    status |= 0x02; // Not enough Memory
                }
#endif
                status |= ( fragSessionData.FragGroupData.FragSession.Fields.FragIndex << 6 ) & 0xC0;
                if( fragSessionData.FragGroupData.FragSession.Fields.FragIndex >= FRAGMENTATION_MAX_SESSIONS )
                {
                    status |= 0x04; // FragSession index not supported
                }

                // Descriptor is not really defined in the specification
                // Not clear how to handle this.
                // Currently the descriptor is always correct
                if( fragSessionData.FragGroupData.Descriptor != 0x01020304 )
                {
                    //status |= 0x08; // Wrong Descriptor
                }

                if( ( status & 0x0F ) == 0 )
                {
                    // The FragSessionSetup is accepted
                    fragSessionData.FragGroupData.IsActive = true;
                    fragSessionData.FragDecoderPorcessStatus = FRAG_SESSION_ONGOING;
                    FragSessionData[fragSessionData.FragGroupData.FragSession.Fields.FragIndex] = fragSessionData;
#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
                    FragDecoderInit( fragSessionData.FragGroupData.FragNb,
                                     fragSessionData.FragGroupData.FragSize,
                                     &LmhpFragmentationParams->DecoderCallbacks );
#else
                    FragDecoderInit( fragSessionData.FragGroupData.FragNb,
                                     fragSessionData.FragGroupData.FragSize,
                                     LmhpFragmentationParams->Buffer,
                                     LmhpFragmentationParams->BufferSize );
#endif
                }
                LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FRAGMENTATION_FRAG_SESSION_SETUP_ANS;
                LmhpFragmentationState.DataBuffer[dataBufferIndex++] = status;
                isAnswerDelayed = false;
                break;
            }
            case FRAGMENTATION_FRAG_SESSION_DELETE_REQ:
            {
                if( mcpsIndication->Multicast == 1 )
                {
                    // Multicast channel. Don't process command.
                    break;
                }
                uint8_t status = 0x00;
                uint8_t id = mcpsIndication->Buffer[cmdIndex++] & 0x03;

                status |= id;
                if( ( id >= FRAGMENTATION_MAX_SESSIONS ) || ( FragSessionData[id].FragGroupData.IsActive == false ) )
                {
                    status |= 0x04; // Session does not exist
                }
                else
                {
                    // Delete session
                    FragSessionData[id].FragGroupData.IsActive = false;
                }
                LmhpFragmentationState.DataBuffer[dataBufferIndex++] = FRAGMENTATION_FRAG_SESSION_DELETE_ANS;
                LmhpFragmentationState.DataBuffer[dataBufferIndex++] = status;
                isAnswerDelayed = false;
                break;
            }
            case FRAGMENTATION_DATA_FRAGMENT:
            {
                uint8_t fragIndex = 0;
                uint8_t fragCounter = 0;

                fragCounter = ( mcpsIndication->Buffer[cmdIndex++] << 0 ) & 0x00FF;
                fragCounter |= ( mcpsIndication->Buffer[cmdIndex++] << 8 ) & 0xFF00;

                fragIndex = ( fragCounter >> 14 ) & 0x03;
                fragCounter &= 0x3FFF;

                if( mcpsIndication->Multicast == 1 )
                {
                    // Message received on a multicast address
                    //
                    // TODO: Not working yet
                    //
                    // Check McGroupBitMask
                    //uint8_t groupId = LoRaMacMcChannelGetGroupId( mcpsIndication->DevAddress );
                    //if( ( groupId == 0xFF ) ||
                    //    ( ( FragSessionData[fragIndex].FragGroupData.FragSession.Fields.McGroupBitMask & ( 1 << groupId ) ) == 0 ) )
                    //{
                    //    // Ignore message
                    //    break;
                    //}
                }

                if( FragSessionData[fragIndex].FragDecoderPorcessStatus == FRAG_SESSION_ONGOING )
                {
                    FragSessionData[fragIndex].FragDecoderPorcessStatus = FragDecoderProcess( fragCounter, &mcpsIndication->Buffer[cmdIndex] );
                    FragSessionData[fragIndex].FragDecoderStatus = FragDecoderGetStatus( );
                    if( LmhpFragmentationParams->OnProgress != NULL )
                    {
                        LmhpFragmentationParams->OnProgress( FragSessionData[fragIndex].FragDecoderStatus.FragNbRx,
                                                             FragSessionData[fragIndex].FragGroupData.FragNb,
                                                             FragSessionData[fragIndex].FragGroupData.FragSize,
                                                             FragSessionData[fragIndex].FragDecoderStatus.FragNbLost );
                    }
                }
                else
                {
                    if( FragSessionData[fragIndex].FragDecoderPorcessStatus >= 0 )
                    {
                        // Fragmentation successfully done
                        FragSessionData[fragIndex].FragDecoderPorcessStatus = FRAG_SESSION_NOT_STARTED;
                        if( LmhpFragmentationParams->OnDone != NULL )
                        {
#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
                            LmhpFragmentationParams->OnDone( FragSessionData[fragIndex].FragDecoderPorcessStatus,
                                                            ( FragSessionData[fragIndex].FragGroupData.FragNb * FragSessionData[fragIndex].FragGroupData.FragSize ) - FragSessionData[fragIndex].FragGroupData.Padding );
#else
                            LmhpFragmentationParams->OnDone( FragSessionData[fragIndex].FragDecoderPorcessStatus,
                                                            LmhpFragmentationParams->Buffer,
                                                            ( FragSessionData[fragIndex].FragGroupData.FragNb * FragSessionData[fragIndex].FragGroupData.FragSize ) - FragSessionData[fragIndex].FragGroupData.Padding );
#endif
                        }
                    }
                }
                cmdIndex += FragSessionData[fragIndex].FragGroupData.FragSize;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    if( isAnswerDelayed == true )
    {
        // TODO: Start a timer to randomly delay the answer
    }
    else
    {
        if( dataBufferIndex != 0 )
        {
            // Answer commands
            LmHandlerAppData_t appData =
            {
                .Buffer = LmhpFragmentationState.DataBuffer,
                .BufferSize = dataBufferIndex,
                .Port = FRAGMENTATION_PORT
            };
            LmhpFragmentationPackage.OnSendRequest( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
        }
    }
}
