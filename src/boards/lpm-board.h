/*!
 * \file  lpm-board.h
 *
 * \brief Target board low power modes management
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright MCD Application Team (C)( STMicroelectronics International ). All rights reserved.
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
#ifndef __LPM_BOARD_H__
#define __LPM_BOARD_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include "board-config.h"

/*!
 * Low power manager configuration
 */
typedef enum
{
    LPM_APPLI_ID   =                                ( 1 << 0 ),
    LPM_LIB_ID     =                                ( 1 << 1 ),
    LPM_RTC_ID     =                                ( 1 << 2 ),
    LPM_GPS_ID     =                                ( 1 << 3 ),
    LPM_UART_RX_ID =                                ( 1 << 4 ),
    LPM_UART_TX_ID =                                ( 1 << 5 ),
} LpmId_t;

/*!
 * Low Power Mode selected
 */
typedef enum
{
    LPM_ENABLE = 0,
    LPM_DISABLE,
} LpmSetMode_t;

typedef enum
{
    LPM_SLEEP_MODE,
    LPM_STOP_MODE,
    LPM_OFF_MODE,
} LpmGetMode_t;

/*!
 * \brief  This API returns the Low Power Mode selected that will be applied when the system will enter low power mode
 *         if there is no update between the time the mode is read with this API and the time the system enters
 *         low power mode.
 *
 * \retval mode Selected low power mode
 */
LpmGetMode_t LpmGetMode( void );

/*!
 * \brief  This API notifies the low power manager if the specified user allows the Stop mode or not.
 *         When the application does not require the system clock, it enters Stop Mode if at least one user disallow
 *         Off Mode. Otherwise, it enters Off Mode.
 *         The default mode selection for all users is Off mode enabled
 *
 * \param [IN] id   Process Id
 * \param [IN] mode Selected mode
 */
void LpmSetStopMode( LpmId_t id, LpmSetMode_t mode );

/*!
 * \brief  This API notifies the low power manager if the specified user allows the Off mode or not.
 *         When the application does not require the system clock, it enters Stop Mode if at least one user disallow
 *         Off Mode. Otherwise, it enters Off Mode.
 *         The default mode selection for all users is Off mode enabled
 *
 * \param [IN] id   Process Id
 * \param [IN] mode Selected mode
 */
void LpmSetOffMode(LpmId_t id, LpmSetMode_t mode );

/*!
 * \brief  This API shall be used by the application when there is no more code to execute so that the system may
 *         enter low-power mode. The mode selected depends on the information received from LpmOffModeSelection( ) and
 *         LpmSysclockRequest( )
 *         This function shall be called in critical section
 */
void LpmEnterLowPower( void );

/*!
 * \brief  This API is called by the low power manager in a critical section (PRIMASK bit set) to allow the
 *         application to implement dedicated code before entering Sleep Mode
 */
void LpmEnterSleepMode( void );

/*!
 * \brief  This API is called by the low power manager in a critical section (PRIMASK bit set) to allow the
 *         application to implement dedicated code before getting out from Sleep Mode
 */
void LpmExitSleepMode( void );

/*!
 * \brief  This API is called by the low power manager in a critical section (PRIMASK bit set) to allow the
 *         application to implement dedicated code before entering Stop Mode
 */
void LpmEnterStopMode( void );

/*!
 * \brief  This API is called by the low power manager in a critical section (PRIMASK bit set) to allow the
 *         application to implement dedicated code before getting out from Stop Mode. This is where the application
 *         should reconfigure the clock tree when needed
 */
void LpmExitStopMode( void );

/*!
 * \brief  This API is called by the low power manager in a critical section (PRIMASK bit set) to allow the
 *         application to implement dedicated code before entering Off mode. This is where the application could save
 *         data in the retention memory as the RAM memory content will be lost
 */
void LpmEnterOffMode( void );

/*!
 * @brief  This API is called by the low power manager in a critical section (PRIMASK bit set) to allow the
 *         application to implement dedicated code before getting out from Off mode. This can only happen when the
 *         Off mode is finally not entered. In that case, the application may reverse some configurations done before
 *         entering Off mode. When Off mode is successful, the system is reset when getting out from this low-power mode
 */
void LpmExitOffMode( void );

#ifdef __cplusplus
}
#endif

#endif /*__LPM_BOARD_H__ */
