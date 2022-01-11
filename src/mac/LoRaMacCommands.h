/*!
 * \file  LoRaMacCommands.h
 *
 * \brief LoRa MAC commands
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright Stackforce 2021. All rights reserved.
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
/*!
 * \addtogroup LORAMAC
 * \{
 *
 */
#ifndef __LORAMAC_COMMANDS_H__
#define __LORAMAC_COMMANDS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include "LoRaMacTypes.h"


/*
 * Number of MAC Command slots
 */
#define LORAMAC_COMMADS_MAX_NUM_OF_PARAMS   2

/*!
 * LoRaWAN MAC Command element
 */
typedef struct sMacCommand MacCommand_t;

struct sMacCommand
{
    /*!
     *  The pointer to the next MAC Command element in the list
     */
    MacCommand_t* Next;
    /*!
     * MAC command identifier
     */
    uint8_t CID;
    /*!
     * MAC command payload
     */
    uint8_t Payload[LORAMAC_COMMADS_MAX_NUM_OF_PARAMS];
    /*!
     * Size of MAC command payload
     */
    size_t PayloadSize;
    /*!
     * Indicates if it's a sticky MAC command
     */
    bool IsSticky;
};

/*!
 * LoRaMac Commands Status
 */
typedef enum eLoRaMacCommandsStatus
{
    /*!
     * No error occurred
     */
    LORAMAC_COMMANDS_SUCCESS = 0,
    /*!
     * Null pointer exception
     */
    LORAMAC_COMMANDS_ERROR_NPE,
    /*!
     * There is no memory left to add a further MAC command
     */
    LORAMAC_COMMANDS_ERROR_MEMORY,
    /*!
     * MAC command not found.
     */
    LORAMAC_COMMANDS_ERROR_CMD_NOT_FOUND,
    /*!
     * Unknown or corrupted command error occurred.
     */
    LORAMAC_COMMANDS_ERROR_UNKNOWN_CMD,
    /*!
     * Undefined Error occurred
     */
    LORAMAC_COMMANDS_ERROR,
}LoRaMacCommandStatus_t;

/*!
 * Signature of callback function to be called by this module when the
 * non-volatile needs to be saved.
 */
typedef void ( *LoRaMacCommandsNvmEvent )( void );

/*!
 * \brief Initialization of LoRaMac MAC commands module
 *
 * \retval                            - Status of the operation
 */
LoRaMacCommandStatus_t LoRaMacCommandsInit( void );

/*!
 * \brief Adds a new MAC command to be sent.
 *
 * \param[IN]   cid                - MAC command identifier
 * \param[IN]   payload            - MAC command payload containing parameters
 * \param[IN]   payloadSize        - Size of MAC command payload
 *
 * \retval                     - Status of the operation
 */
LoRaMacCommandStatus_t LoRaMacCommandsAddCmd( uint8_t cid, uint8_t* payload, size_t payloadSize );

/*!
 * \brief Remove a MAC command.
 *
 * \param[OUT]  cmd                - MAC command
 *
 * \retval                     - Status of the operation
 */
LoRaMacCommandStatus_t LoRaMacCommandsRemoveCmd( MacCommand_t* macCmd );

/*!
 * \brief Get the MAC command with corresponding CID.
 *
 * \param[IN]   cid                - MAC command identifier
 * \param[OUT]  cmd                - MAC command
 *
 * \retval                     - Status of the operation
 */
LoRaMacCommandStatus_t LoRaMacCommandsGetCmd( uint8_t cid, MacCommand_t** macCmd );

/*!
 * \brief Remove all none sticky MAC commands.
 *
 * \retval                     - Status of the operation
 */
LoRaMacCommandStatus_t LoRaMacCommandsRemoveNoneStickyCmds( void );

/*!
 * \brief Remove all sticky answer MAC commands.
 *
 * \retval                     - Status of the operation
 */
LoRaMacCommandStatus_t LoRaMacCommandsRemoveStickyAnsCmds( void );

/*!
 * \brief Get size of all MAC commands serialized as buffer
 *
 * \param[out]   size               - Available size of memory for MAC commands
 *
 * \retval                     - Status of the operation
 */
LoRaMacCommandStatus_t LoRaMacCommandsGetSizeSerializedCmds( size_t* size );

/*!
 * \brief Get as many as possible MAC commands serialized
 *
 * \param[IN]   availableSize      - Available size of memory for MAC commands
 * \param[out]  effectiveSize      - Size of memory which was effectively used for serializing.
 * \param[out]  buffer             - Destination data buffer
 *
 * \retval                     - Status of the operation
 */
LoRaMacCommandStatus_t LoRaMacCommandsSerializeCmds( size_t availableSize, size_t* effectiveSize,  uint8_t* buffer );

/*!
 * \brief Get the MAC command size with corresponding CID.
 *
 * \param[IN]   cid                - MAC command identifier
 *
 * \retval Size of the command.
 */
uint8_t LoRaMacCommandsGetCmdSize( uint8_t cid );

/*! \} addtogroup LORAMAC */

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_COMMANDS_H__

