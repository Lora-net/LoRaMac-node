/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: MCU timer

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __TIMER_BOARD_H__
#define __TIMER_BOARD_H__

/*!
 * \brief Initializes the timer
 *
 * \remark The timer is based on TIM2 with a 10uS time basis
 */
void TimerHwInit( void );

/*!
 * \brief DeInitializes the timer
 */
void TimerHwDeInit( void );

/*!
 * \brief Return the minimum timeout the Timer is able to handle
 *
 * \retval minimum value for a timeout
 */
uint32_t TimerHwGetMinimumTimeout( void );

/*!
 * \brief Start the Standard Timer counter
 *
 * \param [IN] rtcCounter Timer duration
 */
void TimerHwStart( uint32_t rtcCounter );

/*!
 * \brief Perfoms a standard blocking delay in the code execution
 *
 * \param [IN] delay Delay value in ms
 */
void TimerHwDelayMs( uint32_t delay );

/*!
 * \brief Stop the the Standard Timer counter
 */
void TimerHwStop( void ); 

/*!
 * \brief Return the value on the timer counter
 */
uint64_t TimerHwGetTimerValue( void );

/*!
 * \brief Return the value on the timer Tick counter
 */
uint64_t TimerHwGetElapsedTime( void );

/*!
 * \brief Set the ARM core in Wait For Interrupt mode (only working if Debug mode is not used)
 */
void TimerHwEnterLowPowerStopMode( void );

#endif // __TIMER_BOARD_H__
