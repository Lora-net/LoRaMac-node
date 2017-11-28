/*!
 * \file      LoRaMacConfirmQueue.h
 *
 * \brief     LoRa MAC confirm queue implementation
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
 *              (C)2013 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \defgroup  LORAMACCONFIRMQUEUE LoRa MAC confirm queue implementation
 *            This module specifies the API implementation of the LoRaMAC confirm queue.
 *            This is a placeholder for a detailed description of the LoRaMac
 *            layer and the supported features.
 * \{
 */
#ifndef __LORAMAC_CONFIRMQUEUE_H__
#define __LORAMAC_CONFIRMQUEUE_H__


/*!
 * LoRaMac MLME-Confirm queue length
 */
#define LORA_MAC_MLME_CONFIRM_QUEUE_LEN             5

/*!
 * Structure to hold multiple MLME request confirm data
 */
typedef struct sMlmeConfirmQueue
{
    /*!
     * Holds the previously performed MLME-Request
     */
    Mlme_t MlmeRequest;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
}MlmeConfirmQueue_t;

// Add cmd -> req, def status
// Remove cmd -> req
// GetCnt -> none
// IsActive -> req
// Handle

void LoRaMacConfirmQueueInit( LoRaMacPrimitives_t* primitives );

bool LoRaMacConfirmQueueAdd( MlmeConfirmQueue_t* mlmeConfirm );

bool LoRaMacConfirmQueueRemove( MlmeConfirmQueue_t* mlmeConfirm );

void LoRaMacConfirmQueueSetStatus( LoRaMacEventInfoStatus_t status, Mlme_t request );

LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatus( Mlme_t req );

void LoRaMacConfirmQueueSetStatusCmn( LoRaMacEventInfoStatus_t status );

LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatusCmn( void );

bool LoRaMacConfirmQueueIsCmdActive( Mlme_t req );

LoRaMacEventInfoStatus_t LoRaMacConfirmQueueGetStatus( Mlme_t req );

void LoRaMacConfirmQueueHandleCb( void );

uint8_t LoRaMacConfirmQueueGetCnt( void );

#endif // __LORAMAC_CONFIRMQUEUE_H__
