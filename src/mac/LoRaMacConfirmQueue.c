/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: LoRa MAC confirm queue implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jaeckle ( STACKFORCE )
*/

#include "LoRaMac.h"
#include "LoRaMacConfirmQueue.h"


static LoRaMacPrimitives_t* Primitives;

/*!
 * MlmeConfirm queue data structure
 */
static MlmeConfirmQueue_t MlmeConfirmQueue[LORA_MAC_MLME_CONFIRM_QUEUE_LEN];

/*!
 * Counts the number of MlmeConfirms to process
 */
static uint8_t MlmeConfirmQueueCnt;

MlmeConfirmQueue_t* BufferStart;
MlmeConfirmQueue_t* BufferEnd;

LoRaMacEventInfoStatus_t CommonStatus;


static void IncreaseBufferPointer( MlmeConfirmQueue_t* bufferPointer )
{
    bufferPointer++;
    if( bufferPointer == ( MlmeConfirmQueue + LORA_MAC_MLME_CONFIRM_QUEUE_LEN ) )
    {
        // Reset to the first element
        bufferPointer = MlmeConfirmQueue;
    }
}

static bool GetElement( Mlme_t request, MlmeConfirmQueue_t* bufferStart, MlmeConfirmQueue_t* bufferEnd, MlmeConfirmQueue_t* element )
{
    MlmeConfirmQueue_t* run = bufferStart;

    do
    {
        if( run->MlmeRequest == request )
        {
            // We have found the element
            element = run;

            return true;
        }
        else
        {
            IncreaseBufferPointer( run );
        }
    }while( run != bufferEnd )

    return false;
}


void LoRaMacConfirmQueueInit( LoRaMacPrimitives_t* primitives )
{
    Primitives = primitives;

    // Init counter
    MlmeConfirmQueueCnt = 0;

    // Init buffer
    BufferStart = MlmeConfirmQueue;
    BufferEnd = MlmeConfirmQueue;

    memset( &MlmeConfirmQueue, 0xFF, sizeof( MlmeConfirmQueue ) );

    // Common status
    CommonStatus = LORAMAC_EVENT_INFO_STATUS_ERROR;
}

bool LoRaMacConfirmQueueAdd( MlmeConfirmQueue_t* mlmeConfirm )
{
    if( MlmeConfirmQueueCnt >= LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
    {
        // Protect the buffer against overwrites
        return false;
    }

    // Add the element to the ring buffer
    BufferEnd.MlmeRequest = mlmeConfirm->MlmeRequest;
    BufferEnd.Status = mlmeConfirm->Status;
    // Increase counter
    MlmeConfirmQueueCnt++;
    // Update end pointer
    IncreaseBufferPointer( BufferEnd );

    return true;
}

bool LoRaMacConfirmQueueRemove( MlmeConfirmQueue_t* mlmeConfirm )
{
    if( MlmeConfirmQueueCnt == 0 )
    {
        return false;
    }

    // Increase counter
    MlmeConfirmQueueCnt--;
    // Update start pointer
    IncreaseBufferPointer( BufferStart );

    return;
}

void LoRaMacConfirmQueueSetStatus( LoRaMacEventInfoStatus_t status, Mlme_t request )
{
    MlmeConfirmQueue_t* element;

    if( MlmeConfirmQueueCnt > 0 )
    {
        if( GetElement( request, BufferStart, BufferEnd, element ) == true )
        {
            element->Status = status;
        }
    }
}

LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatus( Mlme_t req )
{
    MlmeConfirmQueue_t* element;
    
    if( MlmeConfirmQueueCnt > 0 )
    {
        if( GetElement( request, BufferStart, BufferEnd, element ) == true )
        {
            return element->Status;
        }
    }
    return LORAMAC_STATUS_PARAMETER_INVALID;
}

void LoRaMacConfirmQueueSetStatusCmn( LoRaMacEventInfoStatus_t status )
{
    MlmeConfirmQueue_t* element = BufferStart;

    CommonStatus = status;

    if( MlmeConfirmQueueCnt > 0 )
    {
        do
        {
            element->Status = status;
            IncreaseBufferPointer( element );
        }while( element != BufferEnd );
    }
}

LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatusCmn( void )
{
    return CommonStatus;
}

bool LoRaMacConfirmQueueIsCmdActive( Mlme_t req )
{
    MlmeConfirmQueue_t* element;

    return GetElement( request, BufferStart, BufferEnd, element );
}

void LoRaMacConfirmQueueHandleCb( void )
{
    uint8_t nbElements = MlmeConfirmQueueCnt;
    MlmeConfirmQueue_t* start = BufferStart;

    for( uint8_t i = 0; i < nbElements; i++ )
    {
        Primitives->MacMlmeConfirm( &BufferStart );

        // Increase the pointer afterwards to prevent overwrites
        MlmeConfirmQueueCnt--;
        IncreaseBufferPointer( BufferStart );
    }
    
}

uint8_t LoRaMacConfirmQueueGetCnt( void )
{
    return MlmeConfirmQueueCnt;
}
