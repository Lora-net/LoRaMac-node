/*!
 * \file  LmHandlerMsgDisplay.h
 *
 * \brief Common set of functions to display default messages from
 *        LoRaMacHandler.
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
#ifndef __LMHANDLER_MSG_DISPLAY_H__
#define __LMHANDLER_MSG_DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "utilities.h"
#include "LmHandler.h"

/*!
 * \brief Displays NVM context operation state
 *
 * \param [IN] state Indicates if we are storing (true) or
 *                    restoring (false) the NVM context
 *
 * \param [IN] size Number of data bytes which were stored or restored.
 */
void DisplayNvmDataChange( LmHandlerNvmContextStates_t state, uint16_t size );

/*!
 * \brief Displays updated network parameters
 *
 * \param [IN] commissioningParams Commissioning provided parameters
 */
void DisplayNetworkParametersUpdate( CommissioningParams_t* commissioningParams );

/*!
 * \brief Displays updated McpsRequest
 *
 * \param [IN] status McpsRequest execution status
 * \param [IN] mcpsReq McpsRequest command executed
 * \param [IN] nextTxIn Time to wait for the next uplink transmission
 */
void DisplayMacMcpsRequestUpdate( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn );

/*!
 * \brief Displays updated MlmeRequest
 *
 * \param [IN] status MlmeRequest execution status
 * \param [IN] mlmeReq MlmeRequest command executed
 * \param [IN] nextTxIn Time to wait for the next uplink transmission
 */
void DisplayMacMlmeRequestUpdate( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn );

/*!
 * \brief Displays updated JoinRequest
 *
 * \param [IN] params Executed JoinRequest parameters
 */
void DisplayJoinRequestUpdate( LmHandlerJoinParams_t* params );

/*!
 * \brief Displays Tx params
 *
 * \param [IN] params Tx parameters
 */
void DisplayTxUpdate( LmHandlerTxParams_t* params );

/*!
 * \brief Displays Rx params
 *
 * \param [IN] appData Receive data payload and port number
 * \param [IN] params Rx parameters
 */
void DisplayRxUpdate( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params );

/*!
 * \brief Displays beacon status update
 *
 * \param [IN] params Beacon parameters
 */
void DisplayBeaconUpdate( LoRaMacHandlerBeaconParams_t* params );

/*!
 * \brief Displays end-device class update
 *
 * \param [IN] deviceClass Current end-device class
 */
void DisplayClassUpdate( DeviceClass_t deviceClass );

/*!
 * \brief Displays application information
 */
void DisplayAppInfo( const char* appName, const Version_t* appVersion, const Version_t* gitHubVersion );

#ifdef __cplusplus
}
#endif

#endif // __LMHANDLER_MSG_DISPLAY_H__
