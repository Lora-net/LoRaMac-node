/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Bleeper board Delay functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __DELAY_MCU_H__
#define __DELAY_MCU_H__

/*!
 * \brief The Delay function is based on the MCU Systick.
 *
 * \remark This function initialize the STM32L151RD SysTick for 1ms ticks
 */
void DelayMcuInit( void );

/*!
 * \brief Enables the SysTick IRQ
 */
void DelayMcuEnable( void );

/*!
 * \brief Disables the SysTick IRQ
 */
void DelayMcuDisable( void );

/*!
 * Reads the current timing counter value
 */
uint32_t DelayMcuRead( void );

/*!
 * \brief Write new value to the timing counter value
 *
 * \param timeout Delay value
 */
void DelayMcuWrite( uint32_t timeout );

#endif  // __DELAY_MCU_H__
