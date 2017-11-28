/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: MCU RTC timer and low power modes management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __RTC_BOARD_H__
#define __RTC_BOARD_H__

/*!
 * \brief Timer time variable definition (time in RTC subseconds)
 */
#ifndef TimerTime_t
typedef uint64_t TimerTime_t;
#define TIMERTIME_MAX (0xFFFFFFFFFFFFFFFFULL)
#endif

#ifndef MsTime_t
typedef TimerTime_t MsTime_t;
#define MSTIME_MAX TIMERTIME_MAX
#endif

/*!
 * \brief Initializes the RTC timer
 *
 * \remark The timer is based on the RTC
 */
void RtcInit( void );

/*!
 * \brief Start the RTC timer
 *
 * \remark The timer is based on the RTC Alarm running at 32.768KHz
 *
 * \param[IN] timeout Duration of the Timer
 */
void RtcSetTimeout( MsTime_t timeout );

/*!
 * \brief Adjust the value of the timeout to handle wakeup time from Alarm and GPIO irq
 *
 * \param[IN] timeout Duration of the Timer without compensation for wakeup time
 * \retval new value for the Timeout with compensations
 */
MsTime_t RtcGetAdjustedTimeoutValue( MsTime_t timeout );

/*!
 * \brief Get the RTC timer value
 *
 * \retval RTC Timer value
 */
MsTime_t RtcGetTimerValue( void );

/*!
 * \brief Get the RTC timer elapsed time since the last Alarm was set
 *
 * \retval RTC Elapsed time since the last alarm
 */
MsTime_t RtcGetElapsedAlarmTime( void );

/*!
 * \brief Compute the timeout time of a future event in time
 *
 * \param[IN] futureEventInTime Value in time
 * \retval time Time between now and the futureEventInTime
 */
MsTime_t RtcComputeFutureEventTime( MsTime_t futureEventInTime );

/*!
 * \brief Compute the elapsed time since a fix event in time
 *
 * \param[IN] eventInTime Value in time
 * \retval elapsed Time since the eventInTime
 */
MsTime_t RtcComputeElapsedTime( MsTime_t eventInTime );

/*!
 * \brief This function blocks the MCU from going into Low Power mode
 *
 * \param [IN] status [true: Enable, false: Disable
 */
void BlockLowPowerDuringTask ( bool status );

/*!
 * \brief Sets the MCU into low power STOP mode
 */
void RtcEnterLowPowerStopMode( void );

/*!
 * \brief Restore the MCU to its normal operation mode
 */
void RtcRecoverMcuStatus( void );

#endif // __RTC_BOARD_H__
