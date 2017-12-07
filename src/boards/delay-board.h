/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board delay implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __DELAY_BOARD_H__
#define __DELAY_BOARD_H__

#include <stdint.h>

/*!
 * \brief Blocking delay of "ms" milliseconds
 *
 * \param [IN] ms    delay in milliseconds
 */
void DelayMsMcu( uint32_t ms );

#endif // __DELAY_BOARD_H__
