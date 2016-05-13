/*
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
 * \brief Timer time variable definition
 */
#ifndef TimerTime_t
typedef uint32_t TimerTime_t;
#endif

/*!
 * \brief Structure holding the context date (saved before lunching a timeout)
 */
typedef struct
{
    uint8_t Date;
    uint8_t Month;
    uint8_t Year;
}DateContext_t;

/*!
 * \brief Structure holding the context Time (saved before lunching a timeout)
 */
typedef struct
{
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
    uint16_t Miliseconds;
}TimeContext_t;


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
 * \param[IN] timeout       Duration of the Timer
 */
void RtcSetTimeout( uint32_t timeout );

/*!
 * \brief Get the RTC timer value
 *
 * \retval RTC Timer value
 */
TimerTime_t RtcGetTimerValue( void );

/*!
 * \brief Get the RTC timer elapsed time since the last Alarm was set
 *
 * \retval RTC Elapsed time since the last alarm
 */
uint32_t RtcGetElapsedAlarmTime( void );

/*!
 * \brief Compute the timeout time of a future event in time
 *
 * \param[IN] futureEventInTime       value in time
 * \retval    time between now and the futureEventInTime
 */
TimerTime_t RtcComputeFutureEventTime( TimerTime_t futureEventInTime );

/*!
 * \brief Compute the elapsed time since a fix event in time
 *
 * \param[IN] eventInTime       value in time
 * \retval    elasped time since the eventInTime
 */
TimerTime_t RtcComputeElapsedTime( TimerTime_t eventInTime );

/*!
 * \brief This function block the MCU from going into Low Power mode
 *
 * \param [IN] Status enable or disable
 */
void BlockLowPowerDuringTask ( bool Status );

/*!
 * \brief Sets the MCU in low power STOP mode
 */
void RtcEnterLowPowerStopMode( void );

/*!
 * \brief Restore the MCU to its normal operation mode
 */
void RtcRecoverMcuStatus( void );

#endif // __RTC_BOARD_H__
