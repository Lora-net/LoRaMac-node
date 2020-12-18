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
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "timer.h"
#include "utilities.h"
#include "LoRaMac.h"
#include "LoRaMacConfirmQueue.h"


/*
 * LoRaMac Confirm Queue Context NVM structure
 */
typedef struct sLoRaMacConfirmQueueNvmData
{
    /*!
    * MlmeConfirm queue data structure
    */
    MlmeConfirmQueue_t MlmeConfirmQueue[LORA_MAC_MLME_CONFIRM_QUEUE_LEN];
    /*!
    * Counts the number of MlmeConfirms to process
    */
    uint8_t MlmeConfirmQueueCnt;
    /*!
    * Variable which holds a common status
    */
    LoRaMacEventInfoStatus_t CommonStatus;
} LoRaMacConfirmQueueNvmData_t;

/*
 * LoRaMac Confirm Queue Context structure
 */
typedef struct sLoRaMacConfirmQueueCtx
{
    /*!
    * LoRaMac callback function primitives
    */
    LoRaMacPrimitives_t* Primitives;
    /*!
    * Pointer to the first element of the ring buffer
    */
    MlmeConfirmQueue_t* BufferStart;
    /*!
    * Pointer to the last element of the ring buffer
    */
    MlmeConfirmQueue_t* BufferEnd;
    /*!
    * Non-volatile module context.
    */
    LoRaMacConfirmQueueNvmData_t Nvm;
} LoRaMacConfirmQueueCtx_t;

/*
 * Module context.
 */
static LoRaMacConfirmQueueCtx_t ConfirmQueueCtx;

static MlmeConfirmQueue_t* IncreaseBufferPointer( MlmeConfirmQueue_t* bufferPointer )
{
    if( bufferPointer == &ConfirmQueueCtx.Nvm.MlmeConfirmQueue[LORA_MAC_MLME_CONFIRM_QUEUE_LEN - 1] )
    {
        // Reset to the first element
        bufferPointer = ConfirmQueueCtx.Nvm.MlmeConfirmQueue;
    }
    else
    {
        // Increase
        bufferPointer++;
    }
    return bufferPointer;
}

static MlmeConfirmQueue_t* DecreaseBufferPointer( MlmeConfirmQueue_t* bufferPointer )
{
    if( bufferPointer == ConfirmQueueCtx.Nvm.MlmeConfirmQueue )
    {
        // Reset to the last element
        bufferPointer = &ConfirmQueueCtx.Nvm.MlmeConfirmQueue[LORA_MAC_MLME_CONFIRM_QUEUE_LEN - 1];
    }
    else
    {
        bufferPointer--;
    }
    return bufferPointer;
}

static bool IsListEmpty( uint8_t count )
{
    if( count == 0 )
    {
        return true;
    }
    return false;
}

static bool IsListFull( uint8_t count )
{
    if( count >= LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
    {
        return true;
    }
    return false;
}

static MlmeConfirmQueue_t* GetElement( Mlme_t request, MlmeConfirmQueue_t* bufferStart, MlmeConfirmQueue_t* bufferEnd )
{
    MlmeConfirmQueue_t* element = bufferStart;

    if( IsListEmpty( ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt ) == true )
    {
        return NULL;
    }

    for( uint8_t elementCnt = 0; elementCnt < ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt; elementCnt++ )
    {
        if( element->Request == request )
        {
            // We have found the element
            return element;
        }
        element = IncreaseBufferPointer( element );
    }

    return NULL;
}

void LoRaMacConfirmQueueInit( LoRaMacPrimitives_t* primitives )
{
    ConfirmQueueCtx.Primitives = primitives;

    // Init counter
    ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt = 0;

    // Init buffer
    ConfirmQueueCtx.BufferStart = ConfirmQueueCtx.Nvm.MlmeConfirmQueue;
    ConfirmQueueCtx.BufferEnd = ConfirmQueueCtx.Nvm.MlmeConfirmQueue;

    memset1( ( uint8_t* )ConfirmQueueCtx.Nvm.MlmeConfirmQueue, 0xFF, sizeof( ConfirmQueueCtx.Nvm.MlmeConfirmQueue ) );

    // Common status
    ConfirmQueueCtx.Nvm.CommonStatus = LORAMAC_EVENT_INFO_STATUS_ERROR;
}

bool LoRaMacConfirmQueueAdd( MlmeConfirmQueue_t* mlmeConfirm )
{
    if( IsListFull( ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt ) == true )
    {
        // Protect the buffer against overwrites
        return false;
    }

    // Add the element to the ring buffer
    ConfirmQueueCtx.BufferEnd->Request = mlmeConfirm->Request;
    ConfirmQueueCtx.BufferEnd->Status = mlmeConfirm->Status;
    ConfirmQueueCtx.BufferEnd->RestrictCommonReadyToHandle = mlmeConfirm->RestrictCommonReadyToHandle;
    ConfirmQueueCtx.BufferEnd->ReadyToHandle = mlmeConfirm->ReadyToHandle;
    // Increase counter
    ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt++;
    // Update end pointer
    ConfirmQueueCtx.BufferEnd = IncreaseBufferPointer( ConfirmQueueCtx.BufferEnd );

    return true;
}

bool LoRaMacConfirmQueueRemoveLast( void )
{
    if( IsListEmpty( ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt ) == true )
    {
        return false;
    }

    // Increase counter
    ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt--;
    // Update start pointer
    ConfirmQueueCtx.BufferEnd = DecreaseBufferPointer( ConfirmQueueCtx.BufferEnd );

    return true;
}

bool LoRaMacConfirmQueueRemoveFirst( void )
{
    if( IsListEmpty( ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt ) == true )
    {
        return false;
    }

    // Increase counter
    ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt--;
    // Update start pointer
    ConfirmQueueCtx.BufferStart = IncreaseBufferPointer( ConfirmQueueCtx.BufferStart );

    return true;
}

void LoRaMacConfirmQueueSetStatus( LoRaMacEventInfoStatus_t status, Mlme_t request )
{
    MlmeConfirmQueue_t* element = NULL;

    if( IsListEmpty( ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt ) == false )
    {
        element = GetElement( request, ConfirmQueueCtx.BufferStart, ConfirmQueueCtx.BufferEnd );
        if( element != NULL )
        {
            element->Status = status;
            element->ReadyToHandle = true;
        }
    }
}

LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatus( Mlme_t request )
{
    MlmeConfirmQueue_t* element = NULL;

    if( IsListEmpty( ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt ) == false )
    {
        element = GetElement( request, ConfirmQueueCtx.BufferStart, ConfirmQueueCtx.BufferEnd );
        if( element != NULL )
        {
            return element->Status;
        }
    }
    return LORAMAC_EVENT_INFO_STATUS_ERROR;
}

void LoRaMacConfirmQueueSetStatusCmn( LoRaMacEventInfoStatus_t status )
{
    MlmeConfirmQueue_t* element = ConfirmQueueCtx.BufferStart;

    ConfirmQueueCtx.Nvm.CommonStatus = status;

    if( IsListEmpty( ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt ) == false )
    {
        do
        {
            element->Status = status;
            // Set the status if it is allowed to set it with a call to
            // LoRaMacConfirmQueueSetStatusCmn.
            if( element->RestrictCommonReadyToHandle == false )
            {
                element->ReadyToHandle = true;
            }
            element = IncreaseBufferPointer( element );
        }while( element != ConfirmQueueCtx.BufferEnd );
    }
}

LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatusCmn( void )
{
    return ConfirmQueueCtx.Nvm.CommonStatus;
}

bool LoRaMacConfirmQueueIsCmdActive( Mlme_t request )
{
    if( GetElement( request, ConfirmQueueCtx.BufferStart, ConfirmQueueCtx.BufferEnd ) != NULL )
    {
        return true;
    }
    return false;
}

void LoRaMacConfirmQueueHandleCb( MlmeConfirm_t* mlmeConfirm )
{
    uint8_t nbElements = ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt;
    bool readyToHandle = false;
    MlmeConfirmQueue_t mlmeConfirmToStore;

    for( uint8_t i = 0; i < nbElements; i++ )
    {
        mlmeConfirm->MlmeRequest = ConfirmQueueCtx.BufferStart->Request;
        mlmeConfirm->Status = ConfirmQueueCtx.BufferStart->Status;
        readyToHandle = ConfirmQueueCtx.BufferStart->ReadyToHandle;

        if( readyToHandle == true )
        {
            ConfirmQueueCtx.Primitives->MacMlmeConfirm( mlmeConfirm );
        }
        else
        {
            // The request is not processed yet. Store the state.
            mlmeConfirmToStore.Request = ConfirmQueueCtx.BufferStart->Request;
            mlmeConfirmToStore.Status = ConfirmQueueCtx.BufferStart->Status;
            mlmeConfirmToStore.RestrictCommonReadyToHandle = ConfirmQueueCtx.BufferStart->RestrictCommonReadyToHandle;
        }

        // Increase the pointer afterwards to prevent overwrites
        LoRaMacConfirmQueueRemoveFirst( );

        if( readyToHandle == false )
        {
            // Add a request which has not been finished again to the queue
            LoRaMacConfirmQueueAdd( &mlmeConfirmToStore );
        }
    }
}

uint8_t LoRaMacConfirmQueueGetCnt( void )
{
    return ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt;
}

bool LoRaMacConfirmQueueIsFull( void )
{
    if( IsListFull( ConfirmQueueCtx.Nvm.MlmeConfirmQueueCnt ) == true )
    {
        return true;
    }
    else
    {
        return false;
    }
}
