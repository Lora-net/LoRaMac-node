/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic driver for GPS receiver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __GPS_BOARD_H__
#define __GPS_BOARD_H__

/*!
 * \brief Low level handling of the PPS signal from the GPS receiver
 */
void GpsMcuOnPpsSignal( void );

/*!
 * \brief Invert the IRQ trigger edge on the PPS signal
 */
void GpsMcuInvertPpsTrigger( void );

/*!
 * \brief Low level Initialization of the UART and IRQ for the GPS
 */
void GpsMcuInit( void );

/*!
 * \brief Switch ON the GPS
 */
void GpsMcuStart( void );

/*!
 * \brief Switch OFF the GPS
 */
void GpsMcuStop( void );

/*!
 * Updates the GPS status
 */
void GpsMcuProcess( void );

#endif  // __GPS_BOARD_H__
