/*!
 * \file  LmPackage.h
 *
 * \brief Defines the packages API
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
#ifndef __LMH_PACKAGE_H__
#define __LMH_PACKAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "LmHandlerTypes.h"

/*!
 * Maximum number of packages
 */
#define PKG_MAX_NUMBER                              4

typedef struct LmhPackage_s
{
    uint8_t Port;
    /*
     *=========================================================================
     * Below callbacks must be initialized in package variable declaration
     *=========================================================================
     */

    /*!
     * Initializes the package with provided parameters
     *
     * \param [IN] params            Pointer to the package parameters
     * \param [IN] dataBuffer        Pointer to main application buffer
     * \param [IN] dataBufferMaxSize Main application buffer maximum size
     */
    void ( *Init )( void *params, uint8_t *dataBuffer, uint8_t dataBufferMaxSize );
    /*!
     * Returns the current package initialization status.
     *
     * \retval status Package initialization status
     *                [true: Initialized, false: Not initialized]
     */
    bool ( *IsInitialized )( void );
    /*!
     * Returns if a package transmission is pending or not.
     *
     * \retval status Package transmission status
     *                [true: pending, false: Not pending]
     */
    bool ( *IsTxPending )( void );
    /*!
     * Processes the internal package events.
     */
    void ( *Process )( void );
    /*!
     * Processes the MCSP Confirm
     *
     * \param [IN] mcpsConfirm MCPS confirmation primitive data
     */
    void ( *OnMcpsConfirmProcess )( McpsConfirm_t *mcpsConfirm );
    /*!
     * Processes the MCPS Indication
     *
     * \param [IN] mcpsIndication     MCPS indication primitive data
     */
    void ( *OnMcpsIndicationProcess )( McpsIndication_t *mcpsIndication );
    /*!
     * Processes the MLME Confirm
     *
     * \param [IN] mlmeConfirm MLME confirmation primitive data
     */
    void ( *OnMlmeConfirmProcess )( MlmeConfirm_t *mlmeConfirm );
    /*!
     * Processes the MLME Indication
     *
     * \param [IN] mlmeIndication     MLME indication primitive data
     */
    void ( *OnMlmeIndicationProcess )( MlmeIndication_t *mlmeIndication );

    /*
     *=========================================================================
     * Below callbacks must be initialized in LmHandler initialization with
     * provideded LmHandlerSend and OnMacRequest functions
     *=========================================================================
     */

    /*!
     * Notifies the upper layer that a MCPS request has been made to the MAC layer
     *
     * \param   [IN] status      - Request returned status
     * \param   [IN] mcpsRequest - Performed MCPS-Request. Refer to \ref McpsReq_t.
     * \param   [IN] nextTxDelay - Time to wait until another TX is possible.
     */
    void ( *OnMacMcpsRequest )( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxDelay );
    /*!
     * Notifies the upper layer that a MLME request has been made to the MAC layer
     *
     * \param   [IN] status      - Request returned status
     * \param   [IN] mlmeRequest - Performed MLME-Request. Refer to \ref MlmeReq_t.
     * \param   [IN] nextTxDelay - Time to wait until another TX is possible.
     */
    void ( *OnMacMlmeRequest )( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxDelay );
    /*!
    * Join a LoRa Network in classA
    *
    * \Note if the device is ABP, this is a pass through function
    *
    * \param [IN] isOtaa Indicates which activation mode must be used
    */
    void ( *OnJoinRequest )( bool isOtaa );
    /*!
    * Requests network server time update
    *
    * \retval status Returns \ref LORAMAC_HANDLER_SET if joined else \ref LORAMAC_HANDLER_RESET
    */
    LmHandlerErrorStatus_t ( *OnDeviceTimeRequest )( void );
#if( LMH_SYS_TIME_UPDATE_NEW_API == 1 )
    /*!
     * Notifies the upper layer that the system time has been updated.
     *
     * \param [in] isSynchronized Indicates if the system time is synchronized in the range +/-1 second
     * \param [in] timeCorrection Received time correction value
     */
    void ( *OnSysTimeUpdate )( bool isSynchronized, int32_t timeCorrection );
#else
    /*!
     * Notifies the upper layer that the system time has been updated.
     */
    void ( *OnSysTimeUpdate )( void );
#endif
}LmhPackage_t;

#ifdef __cplusplus
}
#endif

#endif // __LMH_PACKAGE_H__
